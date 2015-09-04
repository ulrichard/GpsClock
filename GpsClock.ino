#include <Adafruit_SSD1306.h>
#include <Adafruit_GPS.h>
#include <MemoryFree.h>
#include "leaper.h"
// a string for testing : 
// $GPRMC,154653,V,4428.2011,N,00440.5161,W,000.5,342.8,050407,,,N*7F
// $GPRMC,000138.799,V,,,,,0.00,0.00,060180,,,N*4F

HardwareSerial& mySerial = Serial;
Adafruit_GPS* pGPS = nullptr;

bool usingInterrupt = false;
uint8_t hour, minute, seconds, year, month, day;
float speed;
bool fix = false;

void useInterrupt(bool);
void DisplayLeaper();
void DisplayInfos();
void CopyData(const Adafruit_GPS& gps);

void setup()
{
    DisplayLeaper();

    {
        Adafruit_GPS GPS(&mySerial);
        GPS.begin(9600);
        GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
        GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    }

    Serial.println("Starting GpsClock");
    Serial.println(freeMemory(), DEC);


    useInterrupt(true);

    delay(1000);
}

void CopyData(const Adafruit_GPS& gps)
{
    hour    = gps.hour;
    minute  = gps.minute;
    seconds = gps.seconds;
    year    = gps.year;
    month   = gps.month;
    day     = gps.day;
    speed   = gps.speed;
    fix     = gps.fix;
}

void DisplayLeaper()
{
    Serial.println("DisplayLeaper");

    Adafruit_SSD1306 display(4);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	display.clearDisplay();
	display.drawBitmap(0, 15, leaper, 128, 48, 1);

	display.setTextColor(WHITE);
    display.setCursor(0, 0);

    if(!fix)
	    display.print("no GPS signal ");
    else
    {
	    display.print("no GPS   ");
        if(hour < 10)
	        display.print("0");
	    display.print(hour, DEC);
	    display.print(":");
        if(minute < 10)
	        display.print("0");
	    display.print(minute,  DEC);
	    display.print(":");
        if(seconds < 10)
	        display.print("0");
	    display.print(seconds,  DEC);
    }

	display.display();

    Serial.println("done DisplayLeaper");
}

void DisplayInfos()
{
    Serial.println("DisplayInfos");

    Adafruit_SSD1306 display(4);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // print groundspeed
    display.setTextSize(2);
    const uint16_t speed = speed * 1.852;
    display.print(speed / 10, DEC);
    display.print('.');
    display.print(speed % 10, DEC);
    display.println(" kmh");

    // print time
    display.setTextSize(3);
    display.print(hour, DEC);
    display.print(":");
    display.print(minute, DEC);
    display.setTextSize(2);
    display.print(":");
    display.print(seconds, DEC);
    display.print("\n");
    display.display();

    // print date
    display.setTextSize(1);
    display.print(day);
    display.print(".");
    display.print(month);
    display.print(".");
    display.print(year);
    display.display();

    Serial.println("done DisplayInfos");
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect)
{
    if(pGPS != nullptr)
        char c = pGPS->read();
}

void useInterrupt(bool v)
{
    if(v)
    {
        // Timer0 is already used for millis() - we'll just interrupt somewhere
        // in the middle and call the "Compare A" function above
        OCR0A = 0xAF;
        TIMSK0 |= _BV(OCIE0A);
        usingInterrupt = true;
    }
    else
    {
        // do not call the interrupt function COMPA anymore
        TIMSK0 &= ~_BV(OCIE0A);
        usingInterrupt = false;
    }
}

uint32_t timer = millis();
void loop()                     // run over and over again
{
    // in case you are not using the interrupt above, you'll
    // need to 'hand query' the GPS, not suggested :(
    if(! usingInterrupt)
    {
        // read data from the GPS in the 'main loop'
        if(pGPS != nullptr)
            char c = pGPS->read();
    }

    {
    
        Adafruit_GPS GPS(&mySerial);
        pGPS = &GPS;
        GPS.begin(9600);
    
        delay(1000);

        // if a sentence is received, we can check the checksum, parse it...
        if(GPS.newNMEAreceived())
        {
            // a tricky thing here is if we print the NMEA sentence, or data
            // we end up not listening and catching other sentences! 
            // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
            //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

            if(!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
                return;  // we can fail to parse a sentence in which case we should just wait for another
            CopyData(GPS);
        }
    
        pGPS = nullptr;
    }

    // if millis() or timer wraps around, we'll just reset it
    if(timer > millis())
        timer = millis();

    // approximately every 2 seconds or so, print out the current stats
    if(millis() - timer > 2000)
    { 
        timer = millis(); // reset the timer
    
        if(fix)
            DisplayInfos();
        else
            DisplayLeaper();
    }
}
