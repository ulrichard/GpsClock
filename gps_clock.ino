#include "Adafruit_SSD1306-1.0.0/Adafruit_SSD1306.h"
#include "leaper.h"
// a string for testing : 
// $GPRMC,154653,V,4428.2011,N,00440.5161,W,000.5,342.8,050407,,,N*7F
// $GPRMC,000138.799,V,,,,,0.00,0.00,060180,,,N*4F

Adafruit_SSD1306 display(4);

bool gotGPS;
struct Time
{
	uint8_t hour, min, sec;
} currTime;

const char cmd[7] = "$GPRMC";
int counter1 = 0; // counts how many bytes were received (max 300)
int counter2 = 0; // counts how many commas were seen
int offsets[13];
char buf[260] = "";

void DisplayLeaper();
void DisplayInfos();

void setup()
{
	gotGPS = false;
	currTime.hour = currTime.min = currTime.sec = 0;
	
    memset(buf, '0', sizeof(buf));
    memset(offsets, 0, sizeof(offsets));
    ResetBuffer();

    Serial.begin(9600);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    DisplayLeaper();
}
 
void ResetBuffer()
{
    counter1 = 0;
    counter2 = 0;
    offsets[0] = 0;
}
 
int GetSize(int offset)
{
    return offsets[offset+1] - offsets[offset] - 1;
}
 
int HandleByte(int byteGPS)
{
    buf[counter1] = byteGPS;
    Serial.print((char)byteGPS);
    counter1++;
    if(counter1 >= sizeof(buf))
        return 0;
    if(byteGPS == ',')
    {
        counter2++;
        offsets[counter2] = counter1;
        if(counter2 >= sizeof(offsets) / sizeof(int))
            return 0;
    }
    else if(byteGPS == '*')
        offsets[12] = counter1;
    else if(byteGPS == 10) // Check if we got a <LF>, which indicates the end of line
    {
        // Check that we got 12 pieces, and that the first piece is 6 characters
        if(counter2 < 12 || (GetSize(0) != 6))
            return 0;

        // Check that we received $GPRMC
        for(int j=0; j<6; j++)
            if(buf[j] != cmd[j])
                return 0;

        // Check that time is well formed
        if(GetSize(1) < 6)
        {
            DisplayLeaper();
            return 0;
        }

        // Check that date is well formed
        if(GetSize(9) != 6)
        {
            DisplayLeaper();
            return 0;
        }

        // Check that velocity is well formed
        if(GetSize(7) == 0)
        {
            DisplayLeaper();
            return 0;
        }

        // TODO: compute and validate checksum

        // TODO: handle timezone offset

		char tmp[5];
		tmp[2] = '\0';
		tmp[0] = buf[offsets[1]+0];
		tmp[1] = buf[offsets[1]+1];
		currTime.hour = atoi(tmp);
		tmp[0] = buf[offsets[1]+2];
		tmp[1] = buf[offsets[1]+3];
		currTime.min = atoi(tmp);
		tmp[0] = buf[offsets[1]+4];
		tmp[1] = buf[offsets[1]+5];
		currTime.sec = atoi(tmp);
        gotGPS = true;

        // Check that lat/lon are well formed
        if(GetSize(3) < 4 || GetSize(4) == 0 || GetSize(5) < 4 || GetSize(6) == 0)
        {
            DisplayLeaper();
            return 0;
        }

        DisplayInfos();

        return 0;
    }
    return 1;
}
 
/**
 * Main loop
 */
void loop()
{
    if(Serial.available() <= 0)
        delay(100);
    else
    {
        const int byteGPS = Serial.read(); // Read a byte of the serial port
        if(byteGPS < 0)
            delay(100);
        else if(!HandleByte(byteGPS))
            ResetBuffer();
    }
}

void DisplayLeaper()
{
	display.clearDisplay();
	display.drawBitmap(0, 15, leaper, 128, 48, 1);

	display.setTextColor(WHITE);
    display.setCursor(0, 0);

    if(!gotGPS)
	    display.print("no GPS signal ");
    else
    {
	    display.print("no GPS   ");
        if(currTime.hour < 10)
	        display.print("0");
	    display.print(currTime.hour, DEC);
	    display.print(":");
        if(currTime.min < 10)
	        display.print("0");
	    display.print(currTime.min,  DEC);
	    display.print(":");
        if(currTime.sec < 10)
	        display.print("0");
	    display.print(currTime.sec,  DEC);
    }

	display.display();
}

void DisplayInfos()
{
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // print groundspeed
    display.setTextSize(2);
    const uint16_t speed = 18.52 * atof(buf + offsets[7]);
    display.print(speed / 10, DEC);
    display.print('.');
    display.print(speed % 10, DEC);
    display.println(" kmh");

    // print time
    display.setTextSize(3);
    for(int j=0; j<6; j++)
    {
        display.print(buf[offsets[1]+j]);
        if(j==1)
            display.print(":");
        else if(j==3)
        {
            display.setTextSize(2);
            display.print(":");
        }
    }
    display.print("\n");
    display.display();

    // print date
    display.setTextSize(1);
    for(int j=0; j<6; j++)
    {
        display.print(buf[offsets[9]+j]);
        if(j==1 || j==3)
            display.print(".");
    }
    display.display();
}

