#include "DS18B20Manager.h"

// Constructor to initialize with GPIO pin
DS18B20Manager::DS18B20Manager(uint8_t busPin) 
    : oneWire(busPin), sensors(&oneWire), lastPollTime(0), pollInterval(1000), resolution(9) {}

// Begin the library, initialize sensors and EEPROM
void DS18B20Manager::begin() {
    EEPROM.begin(512);  // Initialize EEPROM with size (512 bytes for example)
    sensors.begin();  // Initialize the sensors
    setResolution();  // Set sensor resolution to 9-bit (lowest)
    delay(1000);  // Allow time for sensor initialization
    discoverSensors();  // Discover and name sensors
}

// Set the DS18B20 resolution (9-bit for fastest, lowest resolution)
void DS18B20Manager::setResolution() {
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        DeviceAddress sensorAddress;
        if (sensors.getAddress(sensorAddress, i)) {
            sensors.setResolution(sensorAddress, resolution);
        }
    }
}

// Discover all connected sensors and name them if not already named
void DS18B20Manager::discoverSensors() {
    int sensorCount = sensors.getDeviceCount();
    Serial.printf("Found %d sensors\n", sensorCount);

    // Loop through all found sensors
    for (int i = 0; i < sensorCount; i++) {
        DeviceAddress sensorAddress;
        if (sensors.getAddress(sensorAddress, i)) {
            String sensorID = getSensorID(sensorAddress);
            if (!isSensorNamed(sensorID)) {
                String name = promptForSensorName(sensorID);
                storeSensorName(sensorID, name);
            }
        }
    }

    // Check EEPROM for sensors that are not connected
    listAllSensors();

    // Ask if the user wants to delete sensors that aren't connected
    Serial.println("Do you want to delete sensors not connected? (y/n)");
    while (Serial.available() == 0) {}
    char deleteChoice = Serial.read();
    if (deleteChoice == 'y' || deleteChoice == 'Y') {
        deleteUnconnectedSensors();
    }
}

// List all connected sensors and those in EEPROM (including those not connected)
void DS18B20Manager::listAllSensors() {
    int sensorCount = sensors.getDeviceCount();
    Serial.println("Listing all connected sensors and those in EEPROM...");

    for (int i = 0; i < sensorCount; i++) {
        DeviceAddress sensorAddress;
        if (sensors.getAddress(sensorAddress, i)) {
            String sensorID = getSensorID(sensorAddress);
            String sensorName = getSensorName(sensorID);
            Serial.printf("Connected Sensor: %s (%s)\n", sensorName.c_str(), sensorID.c_str());
        }
    }

    // Check and list all sensors stored in EEPROM
    for (int i = 0; i < EEPROM_SIZE; i++) {
        String sensorID = getSensorIDFromEEPROM(i);
        if (sensorID != "") {
            String sensorName = getSensorName(sensorID);
            Serial.printf("Stored in EEPROM: %s (%s)\n", sensorName.c_str(), sensorID.c_str());
        }
    }
}

// Delete sensors from EEPROM that are no longer connected
void DS18B20Manager::deleteUnconnectedSensors() {
    int sensorCount = sensors.getDeviceCount();
    for (int i = 0; i < EEPROM_SIZE; i++) {
        String sensorID = getSensorIDFromEEPROM(i);
        if (sensorID != "" && !isSensorConnected(sensorID)) {
            deleteSensorFromEEPROM(sensorID);
            Serial.printf("Deleted %s from EEPROM\n", sensorID.c_str());
        }
    }
}

// Check if the sensor is currently connected
bool DS18B20Manager::isSensorConnected(const String& sensorID) {
    DeviceAddress sensorAddress;
    return getSensorAddress(sensorID, sensorAddress);
}

// Main loop to poll sensors and manage timing
void DS18B20Manager::loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastPollTime >= pollInterval) {
        lastPollTime = currentMillis;
        pollSensors();
        storeSensorData();  // Store or transmit the data after reading
    }
}

// Poll all sensors (non-blocking)
void DS18B20Manager::pollSensors() {
    sensors.requestTemperatures();  // Request temperature conversion
}

// Store sensor data (could be used for further processing or exporting)
void DS18B20Manager::storeSensorData() {
    // Example: You can store the data or transmit it somewhere
    // Export sensor data here as needed
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        DeviceAddress sensorAddress;
        if (sensors.getAddress(sensorAddress, i)) {
            float temperature = sensors.getTempC(sensorAddress);
            String sensorID = getSensorID(sensorAddress);
            String sensorName = getSensorName(sensorID);
            // Export the sensor name, ID, value, and health status
            Serial.printf("Sensor %s (%s) = %.2f°C\n", sensorName.c_str(), sensorID.c_str(), temperature);
        }
    }
}

// Helper function: Convert sensor address to string ID
String DS18B20Manager::getSensorID(const DeviceAddress& sensorAddress) {
    char idStr[17];
    for (uint8_t i = 0; i < 8; i++) {
        sprintf(idStr + (i * 2), "%02X", sensorAddress[i]);
    }
    return String(idStr);
}

// Get the sensor name from EEPROM for a given sensor ID
String DS18B20Manager::getSensorName(const String& sensorID) {
    int address = getEEPROMAddress(sensorID);
    String name = "";
    for (int i = 0; i < 32; i++) {
        char c = EEPROM.read(address + i);
        if (c == '\0') break;
        name += c;
    }
    return name;
}

// Check if a sensor is named (i.e., its name exists in EEPROM)
bool DS18B20Manager::isSensorNamed(const String& sensorID) {
    int address = getEEPROMAddress(sensorID);
    return EEPROM.read(address) != -1;  // If the first byte is not -1, it means the sensor has a name
}

// Prompt the user for a sensor name via Serial
String DS18B20Manager::promptForSensorName(const String& sensorID) {
    Serial.printf("Enter name for sensor %s: ", sensorID.c_str());
    // Wait for user input
    String name = "";
    while (name.length() == 0) {
        if (Serial.available()) {
            name = Serial.readStringUntil('\n');
            name.trim();  // Remove any leading/trailing whitespace
        }
    }
    return name;
}

// Store the sensor name in EEPROM
void DS18B20Manager::storeSensorName(const String& sensorID, const String& name) {
    int address = getEEPROMAddress(sensorID);
    for (int i = 0; i < name.length(); i++) {
        EEPROM.write(address + i, name[i]);
    }
    EEPROM.write(address + name.length(), '\0');  // Null-terminate the string
    EEPROM.commit();
}

// Helper function: Get the EEPROM address for a given sensor ID
int DS18B20Manager::getEEPROMAddress(const String& sensorID) {
    unsigned long hash = 0;
    for (int i = 0; i < sensorID.length(); i++) {
        hash = (hash * 31) + sensorID[i];  // Simple hash algorithm (a form of "rolling hash")
    }
    return SENSOR_DATA_START_ADDRESS + (hash % 128);  // Ensure we stay within the EEPROM bounds
}

// Helper function: Get the sensor's address from its ID
bool DS18B20Manager::getSensorAddress(const String& sensorID, DeviceAddress& address) {
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        if (sensors.getAddress(address, i)) {
            if (getSensorID(address) == sensorID) {
                return true;
            }
        }
    }
    return false;
}
