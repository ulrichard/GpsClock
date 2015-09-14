#pragma once

struct GpsClockData
{
	uint8_t hour, minute, seconds, year, month, day;
	float speed;
	bool fix = false;
};

