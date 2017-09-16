// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <Adafruit_GPS.h>

// what's the name of the hardware serial port?
// #define GPSSerial Serial
// If using software serial (sketch example default):
//   Connect the GPS TX (transmit) pin to Digital 3
//   Connect the GPS RX (receive) pin to Digital 2
SoftwareSerial GPSSerial(2, 3);

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false

#define GPSLOG true

#define DEBUG_SPEED false

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define TOTALPIXELS      79 
 
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(TOTALPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 5; // delay for half a second
int j = 0;
int inc = 5;

int speed = -1;
int sats = 0;
int qual = 0;

uint32_t base_color_fade = pixels.Color(0x5, 0, 0);
uint32_t base_color = pixels.Color(0xFF, 0, 0);

#define VLINES 20
#define VLINES0 VLINES
#define VLINES1 34
#define VLINES2 25

#define CENTER (VLINES+VLINES1/2)

boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  Serial.begin(9600);

  pixels.begin(); // This initializes the NeoPixel library.

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // 1 Hz update rate
  
  //GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  GPSSerial.println(PMTK_Q_RELEASE);

  //GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // 1 Hz update rate
  
  useInterrupt(true);

  pinMode(LED_BUILTIN, OUTPUT);
  
}

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}


uint32_t timer = millis();
int debug_speed = 0;
void updateGPS()
{
  if (! usingInterrupt) {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
  }
    
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse_rmc(GPS.lastNMEA()))
      return; // we can fail to parse a sentence in which case we should just wait for another
    if (GPSLOG)
      Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
  }
  
  if (timer > millis()) timer = millis();
  
  if (millis() - timer > 1000) {
    //Serial.println("GOOD Nmea"); // this also sets the newNMEAreceived() flag to false
    timer = millis(); // reset the timer
    
    sats = (int)GPS.satellites;
    qual = (int)GPS.fixquality;
    //Serial.print(sats); Serial.print(" "); Serial.println(qual);
          
    if (GPS.fix && GPS.speed >= 0 && GPS.speed < 100) {
      //Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      //Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      speed = (int)(GPS.speed * 18) / 10; // from knots
    }else{
      speed = -1;
    }

    if (DEBUG_SPEED) {
      speed = debug_speed;
      debug_speed = debug_speed+1;
      if (debug_speed > 50) debug_speed = 0;
    }
    
    Serial.print("Speed "); Serial.println(speed);
  }
  
}


int coff = 0;
int fill_count = 0;
uint32_t led_timer = millis();

void drawVertical(int coff)
{
  coff = coff % (VLINES0);
  
  pixels.setPixelColor(coff, base_color);
  pixels.setPixelColor(TOTALPIXELS - VLINES2 - coff * VLINES1 / VLINES0, base_color);

  if (coff == 0)
  {
    for (int i = coff; i < TOTALPIXELS; i++)
    {
      pixels.setPixelColor(VLINES0+VLINES1+i, base_color);
    }
  }  
}

//uint8_t delays[] = { 500, 400, 300, 300, 300, }

void loop()
{

  if (led_timer > millis()) led_timer = millis();     
  
  if (millis() - led_timer > 1 * max(0,60 - speed*2))
  //if (false)
  {
      led_timer = millis();
      
      auto r = 0;
      if (speed == -1)
      {
        r = random(4);
      }
      for(int i=0;i<TOTALPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(r,r,r));
      }
      for(int i=VLINES0;i<VLINES0+speed;i++){
        pixels.setPixelColor(i, pixels.Color(3,0,0));
      }
      /*
      for(int i=TOTALPIXELS-qual; i<TOTALPIXELS; i++){
        pixels.setPixelColor(i, pixels.Color(0,200,0));
      }*/

      drawVertical(coff);
      coff = coff + 1;            
      if (coff > VLINES0)
          coff = 0;

      /*
      pixels.setPixelColor(coff-1, base_color_fade);
      pixels.setPixelColor(coff, base_color);
      pixels.setPixelColor(coff+1, base_color_fade);
      pixels.setPixelColor(TOTALPIXELS - coff * VLINES2 / VLINES, base_color);
                  
      for(int i=0; i<fill_count; i++){
        pixels.setPixelColor(CENTER - i, base_color);
        pixels.setPixelColor(CENTER + i, base_color);
      }

      if (coff == CENTER - fill_count)
      {
        fill_count = fill_count+1;
        coff = 0;
      }
      if (fill_count > CENTER)
        fill_count = 0;
      
      coff = coff + 1;            
      if (coff > CENTER)
          coff = 0;
      */
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
    
    updateGPS();
      
/*
      for(int i=coff; i < coff+NUMPIXELS; i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        //pixels.setPixelColor(i, pixels.Color(random(250),random(250),random(250))); // Moderately bright green color.
        int scale = ((i-coff)%NUMPIXELS);
        pixels.setPixelColor(i, pixels.Color(0xFF*scale/NUMPIXELS, 0x80*scale/NUMPIXELS ,0x80*scale/NUMPIXELS)); // Moderately bright green color.
            
      }
      

      delay(50); // Delay for a period of time (in milliseconds).
        
      j = j + inc;
      if (j > 240)
          inc = -inc;
      if (j <= 0)
          inc = -inc;
          
*/
  
}
