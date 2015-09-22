#pragma once
#include <Arduino.h>

static const uint8_t i2dAddrGpsBoard     = 15;

struct GpsClockData
{
	uint8_t hour, minute, seconds, year, month, day;
	float speed;
	bool fix;
};

union GpsClockDataSerializable
{
    GpsClockData g;
    char c[sizeof(GpsClockData)];
};

