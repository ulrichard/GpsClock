#include "leaper.h"
#include "../GpsClockData.h"
#include <Adafruit_SSD1306.h>
#include <MemoryFree.h>
#include <Wire.h>

Adafruit_SSD1306 display(4);

void AcquireGpsData(GpsClockDataSerializable& gpsData);
void DisplayLeaper(const GpsClockData& gpsData);
void DisplayInfos(const GpsClockData& gpsData);

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting GpsClock DisplayBoard");
    Serial.println(freeMemory(), DEC);

    delay(200);

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    GpsClockData gpsData; 
    gpsData.fix = false;
    DisplayLeaper(gpsData);
}

void AcquireGpsData(GpsClockDataSerializable& gpsData)
{
    Wire.requestFrom(i2dAddrGpsBoard, sizeof(GpsClockData));
    for(uint16_t i = 0; i < sizeof(GpsClockData) && Wire.available(); ++i)
    {
        char cc = Wire.read();
        gpsData.c[i] = cc;
    }
}

void DisplayLeaper(const GpsClockData& gpsData)
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
        if(gpsData.hour < 10)
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

void DisplayInfos(const GpsClockData& gpsData)
{
    Serial.println("DisplayInfos");

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // print groundspeed
    display.setTextSize(2);
    const uint16_t speed = gpsData.speed * 18.52;
    display.print(speed / 10, DEC);
    display.print('.');
    display.print(speed % 10, DEC);
    display.println(" kmh");

    // print time
    display.setTextSize(3);
    if(gpsData.hour < 10)
        display.print("0");
    display.print(gpsData.hour, DEC);
    display.print(":");
    if(gpsData.minute < 10)
        display.print("0");
    display.print(gpsData.minute, DEC);
    display.setTextSize(2);
    display.print(":");
    if(gpsData.seconds < 10)
        display.print("0");
    display.print(gpsData.seconds, DEC);
    display.print("\n");
    display.display();

    // print date
    display.setTextSize(1);
    display.print(gpsData.day);
    display.print(".");
    display.print(gpsData.month);
    display.print(".20");
    display.print(gpsData.year);
    display.display();
}

void loop()
{
    GpsClockDataSerializable gpsData; 
    
    AcquireGpsData(gpsData);

    // hardcoded time zone for Swiss daylight saving time
    gpsData.g.hour = (gpsData.g.hour + 2) % 24;

    if(gpsData.g.fix)
        DisplayInfos(gpsData.g);
    else
        DisplayLeaper(gpsData.g);

    delay(300);
}
