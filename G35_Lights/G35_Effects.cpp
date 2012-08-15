/* GE G-35 ColorEffects Effects Implementation File
 * (C) 2012 Edward F Long Jr.
 *
 * This software is licensed under the MIT license
 * For more information please refer to the MIT-LICENSE file
 */
 
#include "Arduino.h"
#include "G35_Effects.h"
#include <G35_Lights.h>

// these dont work when passed by reference
// so just do it here... yeah, its a hack!
#include <Metro.h>
Metro call_timer = Metro(9999);
Metro fade_timer = Metro(9999);


G35_Effects::G35_Effects(G35_Lights *led) {
    _led  = led;
    _fxMode      = 1;
    _fxFadeInto  = false;
    _fxCurrIndex = 0;
}

/* G35_Effects :: thread
 *
 * This method performs the threaded operations, normally you
 * want to call this method from the loop() function
 */
void G35_Effects::thread() {

    if (!_led->hasInitialized()) {
        // bulbs are NOT ready to be controlled, 
        return;
    }

   // Call FX Engine method to setup next FX transition
   if (call_timer.check()) {
       _nextTransition();
   }

   // Fire off next LED transition
   if (fade_timer.check()) {
      _led->tx(_fxFadeInto);
   }
}


void G35_Effects::setEffects(unsigned char fx, unsigned int call_time, unsigned int fade_time) {

    _fxMode      = fx;
    _fxCurrIndex = 0;
    call_timer.interval(call_time);
    fade_timer.interval(fade_time);

    _nextTransition();
}


void G35_Effects::_nextTransition() {

    // whole color fx transitions
    if(_fxMode ==  1) { _presetDirectBlack();        } 
    if(_fxMode ==  2) { _presetDirectRed();          }
    if(_fxMode ==  3) { _presetDirectOrange();       }
    if(_fxMode ==  4) { _presetDirectYellow();       }
    if(_fxMode ==  5) { _presetDirectGreen();        }
    if(_fxMode ==  6) { _presetDirectCyan();         }
    if(_fxMode ==  7) { _presetDirectBlue();         }
    if(_fxMode ==  8) { _presetDirectAtomicBlue();   }
    if(_fxMode ==  9) { _presetDirectMagenta();      }
    if(_fxMode == 10) { _presetDirectDeepMagenta();  }
    if(_fxMode == 11) { _presetDirectViolet();       }
    if(_fxMode == 12) { _presetDirectWhite();        }
    if(_fxMode == 13) { _presetDirectIncandescent(); }

    if(_fxMode == 30) { _presetFadeOneByOne();       }
    if(_fxMode == 31) { _presetFadePodByPod();       }
    if(_fxMode == 32) { _presetFadeNorthOrSouth();   }
    if(_fxMode == 33) { _presetFadeEastOrWest();     }
    if(_fxMode == 34) { _presetFadeDiagonal();       }
    if(_fxMode == 35) { _presetFadeChecker(false);   }
    if(_fxMode == 36) { _presetFadeChecker(true);    }
    if(_fxMode == 37) { _presetMovie(true);          }
    if(_fxMode == 38) { _presetMovie(false);         }
    if(_fxMode == 39) { _presetPiano();              }
    if(_fxMode == 40) { _presetPolice();             }
    if(_fxMode == 41) { _presetIreland();            }
    if(_fxMode == 42) { _presetUSA();                }
    if(_fxMode == 43) { _presetIrelandFade();        }
    if(_fxMode == 44) { _presetUSAFade();            }

    _led->tx(_fxFadeInto);
}

// used by all 'direct' presets!
void G35_Effects::_presetDirectColor(unsigned int color) {
    _fxFadeInto = true;
    _fxCurrColor = color;
    for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
        _led->setRGBColor(bulb, color, MAX_BRIGHTNESS);
    }
}

void G35_Effects::_presetDirectBlack()        { _presetDirectColor(BLACK       ); }
void G35_Effects::_presetDirectRed()          { _presetDirectColor(RED         ); } 
void G35_Effects::_presetDirectOrange()       { _presetDirectColor(ORANGE      ); } 
void G35_Effects::_presetDirectYellow()       { _presetDirectColor(YELLOW      ); } 
void G35_Effects::_presetDirectGreen()        { _presetDirectColor(GREEN       ); } 
void G35_Effects::_presetDirectCyan()         { _presetDirectColor(CYAN        ); } 
void G35_Effects::_presetDirectBlue()         { _presetDirectColor(BLUE        ); } 
void G35_Effects::_presetDirectAtomicBlue()   { _presetDirectColor(ATOMIC_BLUE ); } 
void G35_Effects::_presetDirectMagenta()      { _presetDirectColor(MAGENTA     ); } 
void G35_Effects::_presetDirectDeepMagenta()  { _presetDirectColor(DEEP_MAGENTA); } 
void G35_Effects::_presetDirectViolet()       { _presetDirectColor(VIOLET      ); } 
void G35_Effects::_presetDirectWhite()        { _presetDirectColor(WHITE       ); } 
void G35_Effects::_presetDirectIncandescent() { _presetDirectColor(INCANDESCENT); }


/* _nextRandomColor:
 * TODO: make this smart enough to not choose a new color until all other colors are choosen
 * TODO: make this smart enough to not choose colors of similar hue's for successive calls to random
 *       (harder)
 *
 * Returns a new random color that is
 *   NOT black
 *   NOT the same color that we already choose
 */
void G35_Effects::_nextRandomColor() {
    unsigned char last_color = _fxCurrColor;
    while(last_color == _fxCurrColor) { _fxCurrColor = random(1,14); }
}

/* _presetFadeOneByOne:
 *
 * Fades lights 1 at a time to the next random color
 */
void G35_Effects::_presetFadeOneByOne() {
    _fxFadeInto = true;
    if(_fxCurrIndex == 0) { _nextRandomColor(); }
    _led->setColor(_fxCurrIndex, _fxCurrColor);
    _fxCurrIndex = (_fxCurrIndex + 1) % (NUM_LEDS);
}

/* _presetFadePodByPod:
 *
 * Fades lights a pod at a time in a zig-zag configuration.
 */
void G35_Effects::_presetFadePodByPod() {
    _fxFadeInto = true;
    if(_fxCurrIndex == 0) { _nextRandomColor(); }
    for (unsigned char offset = 0; offset < 3; offset++) {
        if ((_fxCurrIndex >= 4) && (_fxCurrIndex <= 8)) {
            // reverse the order of the 2nd row :)
           _led->setColor(((12 - _fxCurrIndex) * 3) + offset, _fxCurrColor);
        } else {
           _led->setColor((_fxCurrIndex * 3) + offset, _fxCurrColor);
        }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % (int(NUM_LEDS / 3));
}

/* _presetFadeChecker
 *
 * Fades lights in a checkerboard/chessboard style layout
 * If randomColors is false, use 2 colors, otherwise use random colors for
 * each section on the 'grid'
 */
void G35_Effects::_presetFadeChecker(bool randomColors=false) {
    _fxFadeInto = true;
    if((_fxCurrIndex == 0) || (_fxCurrIndex == 7) || randomColors) { 
        _nextRandomColor();
    }
    unsigned char bulb_presets[NUM_LEDS] = { 0, 0, 0,   8, 8, 8,   3, 3, 3,  11,11,11,
                                             7, 7, 7,   2, 2, 2,  10,10,10,   5, 5, 5,  13,13,13,
                                             1, 1, 1,   9, 9, 9,   4, 4, 4,  12,12,12,   6, 6, 6};

    for (unsigned char bulb = 0; bulb < NUM_LEDS; bulb++) {
       if (bulb_presets[bulb] == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % (100);
}

/* _presetFadeDiagonal
 *
 * Fades lights from north-east to south-west or vice versa
 * This is based on the physical locations of the pods in Studio9
 */
void G35_Effects::_presetFadeDiagonal() {
    _fxFadeInto = true;
    if(_fxCurrIndex == 0) { 
        _nextRandomColor();
        _fxDirection = random(0, 2);
    }
    unsigned char bulb_presets[NUM_LEDS] = { 4, 4, 5,   6, 6, 7,   8, 8, 9,  12,12,13,
                                             2, 3, 2,   4, 5, 4,   6, 7, 6,   8, 9, 8,  10,11,12,
                                             1, 0, 1,   3, 2, 3,   5, 4, 5,   7, 6, 7,   9, 8, 9};

    for (unsigned char bulb = 0; bulb < NUM_LEDS; bulb++) {
        if(_fxDirection == 0) {
           // NE -> SW
           if (bulb_presets[bulb] == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        } else {
           // SW -> NE
           if ((13 - bulb_presets[bulb]) == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % (26);
}


/* _presetFadeNorthOrSouth
 *
 * Fades lights from north to south or vice versa
 * This is based on the physical locations of the pods in Studio9
 */
void G35_Effects::_presetFadeNorthOrSouth() {
    _fxFadeInto = true;
    if(_fxCurrIndex == 0) { 
        _nextRandomColor();
        _fxDirection = random(0, 2);
    }
    unsigned char bulb_presets[NUM_LEDS] = {1,0,0, 1,0,0, 1,0,0, 1,0,0,
                                            3,2,4, 3,2,4, 3,2,4, 3,2,4, 3,3,3,
                                            5,6,6, 5,6,6, 5,6,6, 5,6,6, 5,6,6 };

    for (unsigned char bulb = 0; bulb < NUM_LEDS; bulb++) {
        if(_fxDirection == 0) {
           // north
           if (bulb_presets[bulb] == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        } else {
           // south
           if ((6 - bulb_presets[bulb]) == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % 20;
}

/* _presetFadeEastOrWest
 *
 * Fades lights from east to west or vice versa
 * This is based on the physical locations of the pods in Studio9
 */
void G35_Effects::_presetFadeEastOrWest() {
    _fxFadeInto = true;
    if(_fxCurrIndex == 0) { 
        _nextRandomColor();
        _fxDirection = random(0, 2);
    }
    unsigned char bulb_presets[NUM_LEDS] = { 1, 0, 2,   4, 3, 5,   7, 6, 8,   10, 9,11,
                                             0, 1, 1,   3, 4, 4,   6, 7, 7,    9,10,10,   13,14,18,
                                             1, 0, 2,   4, 3, 5,   7, 6, 8,   10, 9,11,   16,15,17,
                                     };
    for (unsigned char bulb = 0; bulb < NUM_LEDS; bulb++) {
        if(_fxDirection == 0) {
           // east
           if (bulb_presets[bulb] == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        } else {
           // west
           if ((18 - bulb_presets[bulb]) == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % (36);
}

/* _presetPolice
 *
 * Fades lights in a beating strobe pattern with no fading
 * all -> ((RED -> BLACK) x 6) -> ((BLUE -> BLACK) x 6)
 */
void G35_Effects::_presetPolice() {
    _fxFadeInto = false;
    unsigned char _fxPoliceBlinks = 6;

    for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {

       if (_fxCurrIndex % 2 == 0) {
           _led->setRGBColor(bulb, BLACK, MAX_BRIGHTNESS);
       } else if (_fxCurrIndex < (_fxPoliceBlinks * 2)) {
           _led->setRGBColor(bulb, RED, MAX_BRIGHTNESS);
       } else {
           _led->setRGBColor(bulb, BLUE, MAX_BRIGHTNESS);
       }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % (_fxPoliceBlinks * 4);
}

/* _presetThreeColorFlag
 *
 * Sets each row of lights to the specified colors
 */
void G35_Effects::_presetThreeColorFlag(unsigned int color1, unsigned int color2, unsigned int color3) {
  _fxFadeInto = true; 
  for (unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
    if(bulb < 12     ) { _led->setRGBColor(bulb, color1, MAX_BRIGHTNESS); }
    else if(bulb < 27) { _led->setRGBColor(bulb, color2, MAX_BRIGHTNESS); }
    else               { _led->setRGBColor(bulb, color3, MAX_BRIGHTNESS); }
  }
}

/* _presetUSA
 *
 * Calls _presetThreeColor Flag with RED,WHITE, & BLUE
 */
void G35_Effects::_presetUSA()      { _presetThreeColorFlag(RED, WHITE, BLUE); }

/* _presetUSAFade
 *
 * Fades each pod thru RED,WHITE, & BLUE
 */
void G35_Effects::_presetUSAFade()  {
    _fxFadeInto = true;
    unsigned int colors[3] = {RED, WHITE, BLUE};
    for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
       _led->setRGBColor(bulb, colors[(bulb + _fxCurrIndex) % 3], MAX_BRIGHTNESS);
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % 3;
}

/* _presetIreland
 *
 * Calls _presetThreeColor Flag with GREEN, WHITE, & ORANGE
 */
void G35_Effects::_presetIreland()  { _presetThreeColorFlag(GREEN, WHITE, ORANGE); }

/* _presetIrelandFade
 *
 * Fades each pod thru GREE, WHITE, & ORANGE
 */
void G35_Effects::_presetIrelandFade()  {
    _fxFadeInto = true;
    unsigned int colors[3] = {GREEN, WHITE, ORANGE};
    for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
       _led->setRGBColor(bulb, colors[(bulb + _fxCurrIndex) % 3], MAX_BRIGHTNESS);
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % 3;
}


/* _presetMovie
 *
 * Fades lights into movie preset
 */
void G35_Effects::_presetMovie(bool useCandlelight=true) {
    _fxFadeInto = true;

    unsigned char bulb_presets[9] = {0, 1, 2, 27, 28, 29, 26, 24};
    unsigned char bulb_brights[7] = {MAX_BRIGHTNESS, MAX_BRIGHTNESS - 10, MAX_BRIGHTNESS - 20, MAX_BRIGHTNESS - 30,
                                     MAX_BRIGHTNESS - 40, MAX_BRIGHTNESS - 50, MAX_BRIGHTNESS - 70};

    for (unsigned char bulb = 0; bulb < NUM_LEDS; bulb++) {
        _led->setColor(bulb, BLACK);
    };

    for (unsigned char bulb = 0; bulb < 9; bulb++) {
        if(useCandlelight) {
            _led->setRGBColor(bulb_presets[bulb], INCANDESCENT, bulb_brights[random(0,7)]);
        } else {
            _led->setRGBColor(bulb_presets[bulb], INCANDESCENT, MAX_BRIGHTNESS - 50);
        }
    };
}

/* _presetPiano
 *
 * Fades lights into piano preset
 */
void G35_Effects::_presetPiano() {
    _fxFadeInto = true;
    unsigned char bulb_presets[4] = {30, 31, 32, 25};
    for (unsigned char bulb = 0; bulb < NUM_LEDS; bulb++) {
        _led->setColor(bulb, BLACK);
    };
    for (unsigned char bulb = 0; bulb < 4; bulb++) {
        _led->setRGBColor(bulb_presets[bulb], INCANDESCENT, MAX_BRIGHTNESS);
    };
}
