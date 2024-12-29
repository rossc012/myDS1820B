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
            sensors.setResolution(sensorAddress, resolution);  // Use sensors object correctly
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

// Getter for sensor ID from DeviceAddress
String DS18B20Manager::getSensorID(const DeviceAddress& sensorAddress) {
    String sensorID = "";
    for (uint8_t i = 0; i < 8; i++) {
        sensorID += String(sensorAddress[i], HEX);
    }
    return sensorID;
}

// Getter for sensor name, defined by the user or the EEPROM
String DS18B20Manager::getSensorName(const String& sensorID) {
    return "Sensor " + sensorID;  // Placeholder for actual name retrieval
}

bool DS18B20Manager::isSensorNamed(const String& sensorID) {
    // Placeholder: implement your logic for checking if the sensor is named
    return false;
}

String DS18B20Manager::promptForSensorName(const String& sensorID) {
    return "Sensor_" + sensorID;  // Placeholder for actual name assignment logic
}

void DS18B20Manager::storeSensorName(const String& sensorID, const String& name) {
    EEPROM.write(getEEPROMAddress(sensorID), name.length());
    for (size_t i = 0; i < name.length(); i++) {
        EEPROM.write(getEEPROMAddress(sensorID) + i + 1, name[i]);
    }
    EEPROM.commit();
}

int DS18B20Manager::getEEPROMAddress(const String& sensorID) {
    return SENSOR_DATA_START_ADDRESS + sensorID.hashCode() % EEPROM_SIZE;
}

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

DallasTemperature& DS18B20Manager::getSensors() {
    return sensors;
}
