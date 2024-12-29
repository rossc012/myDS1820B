#ifndef DS18B20MANAGER_H
#define DS18B20MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

class DS18B20Manager {
public:
    DS18B20Manager(uint8_t busPin);  // Constructor to initialize with GPIO pin

    void begin();  // Begin the library, initialize sensors and EEPROM
    void discoverSensors();  // Discover all connected sensors
    void loop();  // Main loop to poll sensors and manage timing
    float getSensorValue(const String& sensorID);  // Retrieve sensor temperature value
    String getSensorName(const String& sensorID);  // Get sensor's name from EEPROM

    void pollSensors();  // Function to request temperature conversions
    void storeSensorData();  // Function to store sensor data (can be used for transmitting or processing)

private:
    OneWire oneWire;  // OneWire instance to interface with DS18B20
    DallasTemperature sensors;  // DallasTemperature instance for DS18B20 management
    unsigned long lastPollTime;  // Timestamp for non-blocking sensor polling
    uint32_t pollInterval;  // Interval time between sensor polling (in ms)

    const int SENSOR_DATA_START_ADDRESS = 128;  // EEPROM address where sensor data starts

    // Helper functions
    String getSensorID(const DeviceAddress& sensorAddress);  // Convert sensor address to string ID
    bool isSensorNamed(const String& sensorID);  // Check if a sensor has been named in EEPROM
    String promptForSensorName(const String& sensorID);  // Prompt the user for sensor name
    void storeSensorName(const String& sensorID, const String& name);  // Store sensor name in EEPROM
    int getEEPROMAddress(const String& sensorID);  // Get EEPROM address for a sensor
    bool getSensorAddress(const String& sensorID, DeviceAddress& address);  // Get the sensor's address from its ID
};

#endif // DS18B20MANAGER_H
