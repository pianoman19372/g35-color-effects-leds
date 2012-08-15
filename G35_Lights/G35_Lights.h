/* GE G-35 ColorEffects LED Driver Header File
 * (C) 2012 Edward F Long Jr.
 *
 * This software is licensed under the MIT license
 * For more information please refer to the MIT-LICENSE file
 */

#ifndef G35_LIGHTS
#define G35_LIGHTS

#include "Arduino.h"

#define NUM_LEDS        (63)
#define MAX_BRIGHTNESS  (0xCC)

// Color Definitions
#define COLOR(red, green, blue)    ((red) + ((green) << 4) + ((blue) << 8))  

#define BLACK        (COLOR(  0,  0,  0))
#define RED          (COLOR( 15,  0,  0))
#define ORANGE       (COLOR( 15,  2,  0))
#define YELLOW       (COLOR( 15, 15,  0))
#define GREEN        (COLOR(  0, 15,  0))
#define CYAN         (COLOR(  0, 15, 15))
#define BLUE         (COLOR(  0,  0, 15))
#define ATOMIC_BLUE  (COLOR(  0,  4, 15))
#define MAGENTA      (COLOR( 15,  0,  7))
#define DEEP_MAGENTA (COLOR( 15,  0,  2))
#define VIOLET       (COLOR( 10,  0, 15))
#define WHITE        (COLOR( 15, 13, 11))
#define INCANDESCENT (COLOR( 15,  7,  2))

class G35_Lights {
  
  public:
    G35_Lights(int dataPin, int statusPin);
    void enumerateBulbs();
    void tx(boolean fadeInto);
    bool hasInitialized();

    void setColor(unsigned char bulb, unsigned char colorIndex);
    void setColor(unsigned char bulb, unsigned char colorIndex, unsigned char brightness);
    void setRGBColor(unsigned char bulb, unsigned int color, unsigned char brightness);
    
    
  private:  
    // hardware assignments
    int _dataPin;
    int _statusPin;
    bool _lockFlag;
    bool _enumerateFlag;

    // bulb state management
    unsigned char _targetBrightness[NUM_LEDS];
    unsigned char _currentBrightness[NUM_LEDS];
    unsigned int  _targetColor[NUM_LEDS];
    unsigned int  _currentColor[NUM_LEDS];

    // Signaling methods
    void _start();
    void _one();
    void _zero();
    void _end();

    // Packet transmission method
    void _tx(unsigned char bulb, unsigned char brightness, unsigned int color);
};

#endif
