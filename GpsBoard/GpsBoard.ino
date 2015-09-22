#include "../GpsClockData.h"
#include <Adafruit_GPS.h>
#include <MemoryFree.h>
#include <Wire.h>

// a string for testing : 
// $GPRMC,154653,V,4428.2011,N,00440.5161,W,000.5,342.8,050407,,,N*7F
// $GPRMC,000138.799,V,,,,,0.00,0.00,060180,,,N*4F

HardwareSerial& mySerial = Serial;
Adafruit_GPS GPS(&mySerial);

bool usingInterrupt = false;
GpsClockDataSerializable gpsData[2];
uint8_t gpsBufNum = 0;

void useInterrupt(bool);
void CopyData(const Adafruit_GPS& gps, GpsClockData& gcd);
void receiveEvent(int howMany);

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting GpsClock GpsBoard");
    Serial.println(freeMemory(), DEC);

    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

    useInterrupt(true);

    delay(1000);

    Wire.begin(i2dAddrGpsBoard);
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int howMany)
{
    for(uint16_t i = 0; i < sizeof(GpsClockData); ++i)
        Wire.write(gpsData[gpsBufNum].c[i]);
}

void CopyData(const Adafruit_GPS& gps, GpsClockData& gcd)
{
    gcd.hour    = gps.hour;
    gcd.minute  = gps.minute;
    gcd.seconds = gps.seconds;
    gcd.year    = gps.year;
    gcd.month   = gps.month;
    gcd.day     = gps.day;
    gcd.speed   = gps.speed;
    gcd.fix     = gps.fix;
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect)
{
    char c = GPS.read();
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
    if(!usingInterrupt)
    {
        // read data from the GPS in the 'main loop'
        char c = GPS.read();
    }

    {
        // if a sentence is received, we can check the checksum, parse it...
        if(GPS.newNMEAreceived())
        {
            // a tricky thing here is if we print the NMEA sentence, or data
            // we end up not listening and catching other sentences! 
            // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
            //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

            if(!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
                return;  // we can fail to parse a sentence in which case we should just wait for another
            CopyData(GPS, gpsData[gpsBufNum].g);
            gpsBufNum = (gpsBufNum + 1) % 2;
        }
    }

    // if millis() or timer wraps around, we'll just reset it
    if(timer > millis())
        timer = millis();

    // approximately every 2 seconds or so, print out the current stats
    if(millis() - timer > 2000)
    { 
        timer = millis(); // reset the timer
    
		// ToDo : call the DisplayBoard over i2c
        
    }
}
