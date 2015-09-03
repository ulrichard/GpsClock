#include "Adafruit_SSD1306/Adafruit_SSD1306.h"
#include "Adafruit-GPS-Library/Adafruit_GPS.h"
#include "leaper.h"
// a string for testing : 
// $GPRMC,154653,V,4428.2011,N,00440.5161,W,000.5,342.8,050407,,,N*7F
// $GPRMC,000138.799,V,,,,,0.00,0.00,060180,,,N*4F

Adafruit_SSD1306 display(4);
HardwareSerial& mySerial = Serial;
Adafruit_GPS GPS(&mySerial);

bool usingInterrupt = false;

void useInterrupt(bool);
void DisplayLeaper();
void DisplayInfos();

void setup()
{
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

    Serial.println("Starting GpsClock");

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    DisplayLeaper();

    useInterrupt(true);

    delay(1000);
}

void DisplayLeaper()
{
    Serial.println("DisplayLeaper");

	display.clearDisplay();
	display.drawBitmap(0, 15, leaper, 128, 48, 1);

	display.setTextColor(WHITE);
    display.setCursor(0, 0);

    if(!GPS.fix)
	    display.print("no GPS signal ");
    else
    {
	    display.print("no GPS   ");
        if(GPS.hour < 10)
	        display.print("0");
	    display.print(GPS.hour, DEC);
	    display.print(":");
        if(GPS.minute < 10)
	        display.print("0");
	    display.print(GPS.minute,  DEC);
	    display.print(":");
        if(GPS.seconds < 10)
	        display.print("0");
	    display.print(GPS.seconds,  DEC);
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
    const uint16_t speed = GPS.speed * 1.852;
    display.print(speed / 10, DEC);
    display.print('.');
    display.print(speed % 10, DEC);
    display.println(" kmh");

    // print time
    display.setTextSize(3);
    display.print(GPS.hour, DEC);
    display.print(":");
    display.print(GPS.minute, DEC);
    display.setTextSize(2);
    display.print(":");
    display.print(GPS.seconds, DEC);
    display.print("\n");
    display.display();

    // print date
    display.setTextSize(1);
    display.print(GPS.day);
    display.print(".");
    display.print(GPS.month);
    display.print(".");
    display.print(GPS.year);
    display.display();

    Serial.println("done DisplayInfos");
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
    if(! usingInterrupt)
    {
        // read data from the GPS in the 'main loop'
        char c = GPS.read();
    }

    // if a sentence is received, we can check the checksum, parse it...
    if(GPS.newNMEAreceived())
    {
        // a tricky thing here is if we print the NMEA sentence, or data
        // we end up not listening and catching other sentences! 
        // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
        //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

        if(!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
            return;  // we can fail to parse a sentence in which case we should just wait for another
    }

    // if millis() or timer wraps around, we'll just reset it
    if(timer > millis())
        timer = millis();

    // approximately every 2 seconds or so, print out the current stats
    if(millis() - timer > 2000)
    { 
        timer = millis(); // reset the timer
    
        if(GPS.fix)
            DisplayInfos();
        else
            DisplayLeaper();
    }
}
