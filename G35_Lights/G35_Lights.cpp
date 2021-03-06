/* GE G-35 ColorEffects LED Driver Implementation File
 * (C) 2012 Edward F Long Jr.
 *
 * Portions of this code are based on the previous code and knowledge of:
 *  Scott Harris <scottrharris@gmail.com>  
 *     scottrharris.blogspot.com
 *  Robert Quattlebaum <darco@deepdarc.com> 
 *     http://www.deepdarc.com/2010/11/27/hacking-christmas-lights/
 *
 * This software is licensed under the MIT license
 * For more information please refer to the MIT-LICENSE file
 */
 
#include "Arduino.h"
#include "G35_Lights.h"

G35_Lights::G35_Lights(int dataPin, int statusPin) {
  
   _dataPin   = dataPin;
   _statusPin = statusPin;
   _lockFlag  = false;
   _enumerateFlag = false;
}


/* hasInitialized:
 *
 * Returns TRUE if the bulbs have been enumerated and are ready for commands
 * otherwise returns FALSE
 */
bool G35_Lights::hasInitialized() {
    return _enumerateFlag;
}

void G35_Lights::enumerateBulbs() {
   // Configure the hardware
   pinMode(_dataPin, OUTPUT);
   pinMode(_statusPin, OUTPUT);
   digitalWrite(_dataPin, 0);
   digitalWrite(_statusPin, 0);

   // settle the bus
   delay(200);

   // Enumerate the bulbs

   /* This MAP redefines the addresses of the nodes such that the following
    * numbers make sense
    *
    *   00,01,02,   03,04,05,   06,07,08,   09,10,11,
    *   12,13,14,   15,16,17,   18,19,20,   21,22,23,    24,25,26,
    *   27,28,29,   30,31,32,   33,34,35,   36,37,38,    39,40,41,
    *
    */
    unsigned char enumeration_map[42] = { 0, 1, 2,    3, 4, 5,    6, 7, 8,    9,10,11,
                                         21,22,23,   18,19,20,   15,16,17,   12,13,14,
                                         27,28,29,   30,31,32,   33,34,35,   36,37,38,  39,40,41,  26,25,24};

   for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
       _targetBrightness[bulb] = MAX_BRIGHTNESS;
       _targetColor[bulb]      = BLACK;
       if(bulb < 42) {
           // remap Studio9 addresses for preset simplicity.
           _tx(enumeration_map[bulb], _targetBrightness[bulb], _targetColor[bulb]);
       } else {
           // handle overflow on fosscon demo box (the box has 63 bulbs)
           _tx(bulb, _targetBrightness[bulb], _targetColor[bulb]);
       }
   }

   // add additional delay
   _enumerateFlag = true;
}

// Send start timing signal
void G35_Lights::_start() {
   // turn off interrupts
   noInterrupts();

   digitalWrite(_dataPin, HIGH);   // gate HI
   delayMicroseconds(DELAY_SHORT); // hold 10uS
}

// Send a '1' bit
void G35_Lights::_one() {

    digitalWrite(_dataPin, LOW);    // gate LO
    delayMicroseconds(DELAY_LONG);  // hold 20uS
    digitalWrite(_dataPin, HIGH);   // gate HI
    delayMicroseconds(DELAY_SHORT); // hold 10uS
}

// Send a '0' bit
void G35_Lights::_zero() {

   digitalWrite(_dataPin, LOW);    // gate LO
   delayMicroseconds(DELAY_SHORT); // hold 10uS
   digitalWrite(_dataPin, HIGH);   // gate HI
   delayMicroseconds(DELAY_LONG);  // hold 20uS
}

// Send the end timing signal
void G35_Lights::_end() {

   digitalWrite(_dataPin, LOW);     // gate LO
   delayMicroseconds(DELAY_END);    // hold 30uS

   // turn interrupts back on
   interrupts();
}

/* Use this method to set the desired colors of the LEDs
*  Then call G35_Lights::tx() to transmit the commands to the bulbs */ 
void G35_Lights::setColor(unsigned char bulb, unsigned char colorIndex) {
   setColor(bulb, colorIndex, MAX_BRIGHTNESS);
}

/* Use this method to set the desired colors of the LEDs
*  Then call G35_Lights::tx() to transmit the commands to the bulbs */ 
void G35_Lights::setColor(unsigned char bulb, unsigned char colorIndex, unsigned char brightness) {
    // colors array
    if (colorIndex > 12) { colorIndex = 12; }
    unsigned int _colors[13] = {
         /*  0 */ BLACK, 
         /*  1 */ RED, 
         /*  2 */ ORANGE, 
         /*  3 */ YELLOW, 
         /*  4 */ GREEN, 
         /*  5 */ CYAN, 
         /*  6 */ BLUE, 
         /*  7 */ ATOMIC_BLUE, 
         /*  8 */ MAGENTA, 
         /*  9 */ DEEP_MAGENTA, 
         /* 10 */ VIOLET, 
         /* 11 */ WHITE, 
         /* 12 */ INCANDESCENT
    };
    setRGBColor(bulb, _colors[colorIndex], brightness);
}

/* Use this method to set the desired colors of the LEDs
*  Then call G35_Lights::tx() to transmit the commands to the bulbs */ 
void G35_Lights::setRGBColor(unsigned char bulb, unsigned int color, unsigned char brightness) {
   if (bulb > NUM_LEDS) { bulb = NUM_LEDS; }
   _targetBrightness[bulb] = brightness;
   _targetColor[bulb] = color;
}

// this function conditionally sets bulb states
void G35_Lights::tx(boolean fadeInfo=false) {
  unsigned char c_red, c_green, c_blue;
  unsigned char t_red, t_green, t_blue, brightness, bulb;
  unsigned int color;
  
  for(bulb=0; bulb < NUM_LEDS; bulb++) {

    // have any parameters of this bulb changed ?
    if(_targetBrightness[bulb] != _currentBrightness[bulb] || _targetColor[bulb] != _currentColor[bulb]) {
      /* Developer Note
       * 
       *   I choose not to fade the brightness parameter but its certainly ok to do so. 
       *   your method needs to be written in a way that we end up at the correct brightness 
       *   at the same time as the colors arrive at their correct values or visual effects will
       *   look glitchy.  
       *
       *   On a similar note, i make no attempt to make the colors transition smootly so that they
       *   get to their final targets at the same time, however they tend to get close quick enough 
       *   where it doesnt matter as much.  
       *
       *   On a future iteration of this method, I will make smart fading for brightness and color 
       *   such that all transitions of R, G, B, and brightness will occur evenly within 16 steps
       */

      // dont fade brightness
      brightness = _targetBrightness[bulb];

      if(fadeInfo) {
        /* Fade to desired color:
         *   ramp up or down till we are on target for each color component  
         */

        // Red
        c_red = (_currentColor[bulb]) & B1111;  
        t_red = (_targetColor[bulb] ) & B1111;
        if (t_red > c_red) { c_red++; } else if (t_red < c_red) { c_red--; }

        // Green
        c_green = (_currentColor[bulb] >> 4) & B1111;
        t_green = (_targetColor[bulb]  >> 4) & B1111;
        if (t_green > c_green) { c_green++; } else if (t_green < c_green) { c_green--; }

        // Blue
        c_blue = (_currentColor[bulb] >> 8) & B1111;
        t_blue = (_targetColor[bulb]  >> 8) & B1111;        
        if (t_blue > c_blue) { c_blue++; } else if (t_blue < c_blue) { c_blue--; }
        
        // set color structure        
        color = COLOR(c_red, c_green, c_blue);
        
      } else {
        // direct control, go right to target!
        color = _targetColor[bulb];
      }
      
      // update the bulb!
      _tx(bulb, brightness, color);
    }
  }
}


/* low level led command transmission method
 *
 * portions of this method are based off the works of:
 *   Scott Harris, Robert Quattlebaum with modifications
 *   by Edward Long.
 */
void G35_Lights::_tx(unsigned char bulb, unsigned char brightness, unsigned int color) {

   if (_lockFlag) {
     // another thread is running, exit now!
     return;
   } else {
     // acquire lock and continue
     _lockFlag = true;
   }
   
   // color map to bitwise color   
   unsigned char red   = (color     ) & B1111;
   unsigned char green = (color >> 4) & B1111;
   unsigned char blue  = (color >> 8) & B1111;
   
   // set current color and brightness
   _currentBrightness[bulb] = _targetBrightness[bulb]; // brightness
   _currentColor[bulb]      = color;                   // color
  
   // Start command transmission 
   _start();

   // Send LED Bulb ID Address  (6 bits, MSB first)
   if(bulb & B100000) _one(); else _zero();
   if(bulb & B010000) _one(); else _zero();
   if(bulb & B001000) _one(); else _zero();
   if(bulb & B000100) _one(); else _zero();
   if(bulb & B000010) _one(); else _zero();
   if(bulb & B000001) _one(); else _zero();
 
   // Send Brightness (8 bits, MSB First)
   if(brightness > MAX_BRIGHTNESS) {
     /* WARNING: Risk of LED bulb damage !!!
      *
      * The stock LEDs never exceed 0xCC brightness !
      *
      * Do not set the brightness over this level or you may burn out
      * the bulbs prematurely or consume more current then your transformer
      * can serve if using more then 50 bulbs on a single run.
      *
      * Override this limitor at your own risk!
      */
     brightness = MAX_BRIGHTNESS; 
   }

   if(brightness & B10000000) _one(); else _zero();
   if(brightness & B01000000) _one(); else _zero();
   if(brightness & B00100000) _one(); else _zero();
   if(brightness & B00010000) _one(); else _zero();
   if(brightness & B00001000) _one(); else _zero();
   if(brightness & B00000100) _one(); else _zero();
   if(brightness & B00000010) _one(); else _zero();
   if(brightness & B00000001) _one(); else _zero();
  
   // Send BLUE component (4 bits, MSB First)
   if(blue & B1000) _one(); else _zero();
   if(blue & B0100) _one(); else _zero();
   if(blue & B0010) _one(); else _zero();
   if(blue & B0001) _one(); else _zero();

   // Send GREEN component (4 bits, MSB First)
   if(green & B1000) _one(); else _zero();
   if(green & B0100) _one(); else _zero();
   if(green & B0010) _one(); else _zero();
   if(green & B0001) _one(); else _zero();

   // Send RED component (4 bits, MSB First)
   if(red & B1000) _one(); else _zero();
   if(red & B0100) _one(); else _zero();
   if(red & B0010) _one(); else _zero();
   if(red & B0001) _one(); else _zero();

   // Send END signal
   _end();
   _lockFlag = false;
}

