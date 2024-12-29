#ifndef DS18B20Manager_h
#define DS18B20Manager_h

#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

// Define the starting EEPROM address for sensor data storage
#define SENSOR_DATA_START_ADDRESS 0

class DS18B20Manager {
public:
    // Constructor to initialize with GPIO pin
    DS18B20Manager(uint8_t busPin);

    // Begin the library, initialize sensors, and EEPROM
    void begin();

    // Main loop to poll sensors and manage timing
    void loop();

    // Discover and name sensors if not already named
    void discoverSensors();

    // Set the DS18B20 resolution (9-bit for fastest, lowest resolution)
    void setResolution();

    // List all connected sensors and those stored in EEPROM
    void listAllSensors();

    // Delete sensors from EEPROM that are no longer connected
    void deleteUnconnectedSensors();

    // Store sensor data (to transmit or process)
    void storeSensorData();

    // Poll all sensors (non-blocking)
    void pollSensors();

    // Get sensor name from EEPROM
    String getSensorName(const String& sensorID);

    // Check if a sensor is named (exists in EEPROM)
    bool isSensorNamed(const String& sensorID);

    // Prompt the user for a sensor name via Serial
    String promptForSensorName(const String& sensorID);

    // Store the sensor name in EEPROM
    void storeSensorName(const String& sensorID, const String& name);

private:
    OneWire oneWire;  // OneWire instance for sensor communication
    DallasTemperature sensors;  // DallasTemperature instance
    unsigned long lastPollTime;  // Track last poll time for non-blocking loop
    unsigned long pollInterval;  // Time between polls
    int resolution;  // Sensor resolution
    static const int EEPROM_SIZE = 512;  // EEPROM size

    // Helper function to convert sensor address to string ID
    String getSensorID(const DeviceAddress& sensorAddress);

    // Helper function to get EEPROM address for a sensor ID
    int getEEPROMAddress(const String& sensorID);

    // Helper function to check if the sensor is currently connected
    bool isSensorConnected(const String& sensorID);

    // Helper function to get sensor's address from its ID
    bool getSensorAddress(const String& sensorID, DeviceAddress& address);

    // Helper function to list sensors in EEPROM
    String getSensorIDFromEEPROM(int index);

    // Helper function to delete a sensor from EEPROM
    void deleteSensorFromEEPROM(const String& sensorID);
};

#endif
