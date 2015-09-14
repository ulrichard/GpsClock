#include <Adafruit_SSD1306.h>
#include <MemoryFree.h>
#include "leaper.h"
#include "GpsClockData.h"

uint8_t hour, minute, seconds, year, month, day;
float speed;

Adafruit_SSD1306 display(4);
GpsClockData gpsData;

void DisplayLeaper();
void DisplayInfos();

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting GpsClock DisplayBoard");
    Serial.println(freeMemory(), DEC);

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    DisplayLeaper();
}

void DisplayLeaper()
{
    Serial.println("DisplayLeaper");

    Serial.println(freeMemory(), DEC);

	display.clearDisplay();
	display.drawBitmap(0, 15, leaper, 128, 48, 1);

	display.setTextColor(WHITE);
    display.setCursor(0, 0);

    if(!gpsData.fix)
	    display.print("no GPS signal ");
    else
    {
	    display.print("no GPS   ");
        if(hour < 10)
	        display.print("0");
	    display.print(gpsData.hour, DEC);
	    display.print(":");
        if(gpsData.minute < 10)
	        display.print("0");
	    display.print(gpsData.minute,  DEC);
	    display.print(":");
        if(gpsData.seconds < 10)
	        display.print("0");
	    display.print(gpsData.seconds,  DEC);
    }

	display.display();

    Serial.println("done DisplayLeaper");
}

void DisplayInfos()
{
    Serial.println("DisplayInfos");

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // print groundspeed
    display.setTextSize(2);
    const uint16_t speed = gpsData.speed * 1.852;
    display.print(speed / 10, DEC);
    display.print('.');
    display.print(speed % 10, DEC);
    display.println(" kmh");

    // print time
    display.setTextSize(3);
    display.print(gpsData.hour, DEC);
    display.print(":");
    display.print(gpsData.minute, DEC);
    display.setTextSize(2);
    display.print(":");
    display.print(gpsData.seconds, DEC);
    display.print("\n");
    display.display();

    // print date
    display.setTextSize(1);
    display.print(gpsData.day);
    display.print(".");
    display.print(gpsData.month);
    display.print(".");
    display.print(gpsData.year);
    display.display();

    Serial.println("done DisplayInfos");
}

void loop()
{
    // todo: configure as i2c slave
		if(gpsData.fix)
            DisplayInfos();
        else
            DisplayLeaper();
}
