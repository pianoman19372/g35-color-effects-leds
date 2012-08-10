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
}

void G35_Lights::enumerateBulbs() {
   // Configure the hardware
   pinMode(_dataPin, OUTPUT);
   pinMode(_statusPin, OUTPUT);
   digitalWrite(_dataPin, 0);
   digitalWrite(_statusPin, 0);

   // settle the bus
   delay(250); 

   // Enumerate the bulbs
   for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
       _targetBrightness[bulb] = MAX_BRIGHTNESS;
       _targetColor[bulb]      = BLACK;
       _tx(bulb, _targetBrightness[bulb], _targetColor[bulb]);
       delay(5);
   }

   // add additional delay
   delay(250);
}
  
/* Send start timing signal
 *
 * Note: The timings here are a little unusual, this is to compensate for overhead on
 *       The arduino uno.  The timings specified in the comments are the actual
 *       measured timings of the stock led lights.  the additional (unnessary) gates
 *       to LOW are used to eat up a few us for timing purposes.
 *
 *       credits: Scott Harris, Robert Quattlebaum
 */
void G35_Lights::_start() {
   digitalWrite(_statusPin, 1); // turn on STATUS_PIN

   digitalWrite(_dataPin, 1);   // gate HIGH
   delayMicroseconds(7);        // hold 10us
   digitalWrite(_dataPin, 0);   // gate LOW
}  

// Send a '1' bit
void G35_Lights::_one() {  
   digitalWrite(_dataPin, 0);   // gate LOW
   delayMicroseconds(11);       // hold 20uS 
   digitalWrite(_dataPin, 1);   // gate HIGH
   delayMicroseconds(7);        // hold 10uS
   digitalWrite(_dataPin, 0);   // gate LOW
}  
   
// Send a '0' bit
void G35_Lights::_zero() {  
   digitalWrite(_dataPin, 0);   // gate LOW
   delayMicroseconds(2);        // hold 10uS
   digitalWrite(_dataPin, 1);   // gate HIGH  
   delayMicroseconds(17);       // hold 20uS  
   digitalWrite(_dataPin, 0);   // gate LOW
}  
   
// Send the end timing signal
void G35_Lights::_end() {  
   digitalWrite(_dataPin, 0);   // gate LOW
   delayMicroseconds(40);       // hold: 40us  

   digitalWrite(_statusPin, 0); // turn off STATUS_PIN
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

