#include <iostream>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

// INA219 Registers
#define INA219_ADDRESS 0x40
#define INA219_REG_CONFIG 0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE 0x02
#define INA219_REG_POWER 0x03
#define INA219_REG_CURRENT 0x04
#define INA219_REG_CALIBRATION 0x05

// Config settings for INA219
#define INA219_CONFIG_BVOLTAGERANGE_MASK 0x2000
#define INA219_CONFIG_GAIN_MASK 0x1800
#define INA219_CONFIG_BADCRES_MASK 0x0780
#define INA219_CONFIG_SADCRES_MASK 0x0078
#define INA219_CONFIG_MODE_MASK 0x0007

// INA219 Operating Modes
#define INA219_MODE_POWERDOWN 0x00
#define INA219_MODE_SVOLT_TRIGGERED 0x01
#define INA219_MODE_BVOLT_TRIGGERED 0x02
#define INA219_MODE_SANDBVOLT_TRIGGERED 0x03
#define INA219_MODE_ADCOFF 0x04
#define INA219_MODE_SVOLT_CONTINUOUS 0x05
#define INA219_MODE_BVOLT_CONTINUOUS 0x06
#define INA219_MODE_SANDBVOLT_CONTINUOUS 0x07

// Calibration values (adjust according to your setup)
#define INA219_CALIBRATION_VALUE 4096.0 // Calibration value (you may need to adjust this)
#define INA219_CURRENT_LSB 0.001 // Current LSB value (depend on the settings)

// Sensor values
#define INA219_SHUNT_RESISTANCE 0.1
#define INA219_MAX_CURRENT 0.09
#define INA219_RESOLUTION 24

uint16_t calculateCalibrationValue(float shuntRes, float maxCurrent, int resolution) {
	// consts
	const float currentLSB = INA219_CURRENT_LSB;
	const float powerLSB = 20.0;
	
	// calculate
	
	float calibrationValueFloat = (0.04096 / (shuntRes * maxCurrent) * pow(2, resolution));
	uint16_t calibrationValue = static_cast<uint16_t>(round(calibrationValueFloat));
	
	std::cerr << calibrationValueFloat << '\n';
	//7.6355e+07
	
	return calibrationValue;
}


class INA219 {
private:
    int i2cHandle;
    float currentLSB;
public:
    INA219(int address);
    void setConfig(uint16_t config);
    void setCalibration(int cal);
    uint16_t readRegister(uint8_t reg);
    float getShuntVoltage();
    float getBusVoltage();
    float getCurrent();
    float getPower();
};

INA219::INA219(int address) {
    i2cHandle = wiringPiI2CSetup(address);
    currentLSB = INA219_CURRENT_LSB;
}

void INA219::setConfig(uint16_t config) {
    wiringPiI2CWriteReg16(i2cHandle, INA219_REG_CONFIG, config);
}

void INA219::setCalibration(int cal) {
	wiringPiI2CWriteReg16(i2cHandle, INA219_REG_CALIBRATION, cal);
}

uint16_t INA219::readRegister(uint8_t reg) {
    return wiringPiI2CReadReg16(i2cHandle, reg);
}

float INA219::getShuntVoltage() {
    int16_t rawValue = readRegister(INA219_REG_SHUNTVOLTAGE);
    return rawValue * 0.01; // Shunt voltage LSB is 10uV
}

float INA219::getBusVoltage() {
    int16_t rawValue = wiringPiI2CReadReg16(i2cHandle, 0x02); // Read bus voltage register
    // Convert raw value to voltage (assumes LSB = 4mV)
    float voltage = static_cast<float>(rawValue) * 0.0004;
    return voltage;
}

float INA219::getCurrent() {
    int16_t rawValue = wiringPiI2CReadReg16(i2cHandle, 0x04); // Read current register
    // Convert raw value to current (LSB based on shunt resistor value)
    float current = static_cast<float>(rawValue) * currentLSB;
    return current;
}

float INA219::getPower() {
    int16_t rawValue = readRegister(INA219_REG_POWER);
    return rawValue * 20.0; // Power LSB is 20uW
}

INA219 ina219(INA219_ADDRESS);
bool allowDebug = false;
int setupINA(int cal) {
	const char *i2cDevice = "/dev/i2c-1";
	int file;
	
	if((file = open(i2cDevice, O_RDWR)) < 0) {
		if(allowDebug) std::cerr << "Failed to open I2C device\n";
		return 1;
	}
	
	if (ioctl(file, I2C_SLAVE, INA219_ADDRESS) < 0) {
		if(allowDebug) std::cerr << "Failed to set I2C slave address\n";
		return 2;
	}
	
	
    // Set configuration
    uint16_t config = INA219_CONFIG_BVOLTAGERANGE_MASK |
                      INA219_CONFIG_GAIN_MASK |
                      INA219_CONFIG_BADCRES_MASK |
                      INA219_CONFIG_SADCRES_MASK |
                      INA219_MODE_SVOLT_CONTINUOUS;
    
    ina219.setConfig(config);
    
    ina219.setCalibration(cal);
    
    if(allowDebug) std::cerr << "INA219 setup complete.\n";
	return 0;
}

/*// INA219.hpp
#define INA219_ADDRESS 0x40

#define INA219_REG_CALIBRATION 0x05

#ifndef INA219_HPP
#define INA219_HPP

#include <wiringPiI2C.h>
#include <cstdint>

class INA219 {
private:
    int i2cHandle;
    float shuntResistance; // Shunt resistance value in ohms
    float currentLSB; // Current LSB value in A
    float powerLSB; // Power LSB value in W

public:
    INA219(int address);
    float getBusVoltage();
    float getCurrent();
    uint16_t getRawCurrent();
    void setCalibration();
};

INA219::INA219(int address) {
    i2cHandle = wiringPiI2CSetup(address);
    shuntResistance = 0.1;
    
    // Calibration values based on the provided shunt resistor value
    currentLSB = 0.01 / shuntResistance; // Current LSB = 10mV / shunt resistor value
    powerLSB = 20.0 * currentLSB; // Power LSB = 20mW / shunt resistor value

    // Configure INA219 settings (adjust as needed)
    uint16_t config = 0x019F; // Config register settings: 12-bit bus voltage, 12-bit shunt voltage, continuous shunt and bus voltage
    wiringPiI2CWriteReg16(i2cHandle, 0x00, config); // Write to Config register
    
}

INA219 ina219(INA219_ADDRESS);

float INA219::getBusVoltage() {
    int16_t rawValue = wiringPiI2CReadReg16(i2cHandle, 0x02); // Read bus voltage register
    // Convert raw value to voltage (assumes LSB = 4mV)
    float voltage = static_cast<float>(rawValue) * 0.0004;
    return voltage;
}

float INA219::getCurrent() {
    int16_t rawValue = wiringPiI2CReadReg16(i2cHandle, 0x04); // Read current register
    // Convert raw value to current (LSB based on shunt resistor value)
    float current = static_cast<float>(rawValue) * currentLSB / 200.0f;
    return current;
}

uint16_t INA219::getRawCurrent() {
	uint16_t rawCurrent = wiringPiI2CReadReg16(i2cHandle, 0x04);
	return rawCurrent;
}

void INA219::setCalibration() {
	wiringPiI2CWriteReg16(i2cHandle, INA219_REG_CALIBRATION, 1585);
}

#endif // INA219_HPP









*/
