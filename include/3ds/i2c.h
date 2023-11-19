#pragma once
#include <3ds/types.h>

Result i2cEEPInit(Handle* i2cEEPHandle);

void i2cExit(Handle* i2cHandle);

Result I2C_ReadEEPROM(Handle i2cHandle, u8 device, void* out, u16 offset, size_t length);
