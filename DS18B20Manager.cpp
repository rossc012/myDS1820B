#include "DS18B20Manager.h"

// Constructor to initialize with GPIO pin
DS18B20Manager::DS18B20Manager(uint8_t busPin) 
    : oneWire(busPin), sensors(&oneWire), lastPollTime(0), pollInterval(1000) {}

// Begin the library, initialize sensors and EEPROM
void DS18B20Manager::begin() {
    EEPROM.begin(512);  // Initialize EEPROM with size (512 bytes for example)
    sensors.begin();  // Initialize the sensors
    delay(1000);  // Allow time for sensor initialization
    discoverSensors();  // Discover and name sensors
}

// Discover all connected sensors
void DS18B20Manager::discoverSensors() {
    int sensorCount = sensors.getDeviceCount();
    Serial.printf("Found %d sensors\n", sensorCount);

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
}

// Main loop to poll sensors and manage timing
void DS18B20Manager::loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastPollTime >= pollInterval) {
        lastPollTime = currentMillis;

        // Poll sensors for new data
        pollSensors();
        storeSensorData();  // Store data (or transmit) after reading
    }
}

// Poll all sensors (non-blocking)
void DS18B20Manager::pollSensors() {
    sensors.requestTemperatures();  // Request temperature conversion
}

// Store sensor data (could be used for further processing or exporting)
void DS18B20Manager::storeSensorData() {
    // Example: You can store the data or transmit it somewhere
    // You can export sensor data here as needed
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        DeviceAddress sensorAddress;
        if (sensors.getAddress(sensorAddress, i)) {
            float temperature = sensors.getTempC(sensorAddress);
            String sensorID = getSensorID(sensorAddress);
            String sensorName = getSensorName(sensorID);
            // Do something with the sensor data
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

// Get the temperature value for a given sensor ID
float DS18B20Manager::getSensorValue(const String& sensorID) {
    DeviceAddress sensorAddress;
    if (getSensorAddress(sensorID, sensorAddress)) {
        return sensors.getTempC(sensorAddress);
    }
    return NAN;  // Return NaN if the sensor is not found
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

// Prompt the user for a sensor name
String DS18B20Manager::promptForSensorName(const String& sensorID) {
    // In your real implementation, this could be a serial input or a UI prompt
    // For now, we'll just return a dummy name
    return "Sensor_" + sensorID;
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
