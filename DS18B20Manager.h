#ifndef DS18B20MANAGER_H
#define DS18B20MANAGER_H

#include <DallasTemperature.h>
#include <OneWire.h>
#include <EEPROM.h>

#define SENSOR_DATA_START_ADDRESS 0
#define EEPROM_SIZE 512

class DS18B20Manager {
private:
    OneWire oneWire;
    DallasTemperature sensors;
    unsigned long lastPollTime;
    unsigned long pollInterval;
    uint8_t resolution;

public:
    DS18B20Manager(uint8_t busPin);

    void begin();
    void setResolution();
    void discoverSensors();
    void listAllSensors();
    void deleteUnconnectedSensors();
    bool isSensorConnected(const String& sensorID);
    void loop();
    void pollSensors();
    void storeSensorData();

    String getSensorID(const DeviceAddress& sensorAddress);
    String getSensorName(const String& sensorID);
    bool isSensorNamed(const String& sensorID);
    String promptForSensorName(const String& sensorID);
    void storeSensorName(const String& sensorID, const String& name);
    int getEEPROMAddress(const String& sensorID);
    bool getSensorAddress(const String& sensorID, DeviceAddress& address);

    DallasTemperature& getSensors();  // Added getter to access DallasTemperature object

};

#endif
