/* GE G-35 ColorEffects Implementation File
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
    
//    //if(_fxMode ==  ) { _presetPolice(); }
//
//    if(_fxMode == 15) { _presetUSA(); }
//    if(_fxMode == 16) { _presetIreland(); }
//    if(_fxMode == 17) { _presetFrance(); }
//    if(_fxMode == 19) { _presetPodByPod(); }
//    if(_fxMode == 20) { _presetAllFade(); }
//    if(_fxMode == 21) { _presetBars(); }
//    if(_fxMode == 18) { _presetMovie(); }
//    if(_fxMode == 22) { _presetXFader(); }
//    if(_fxMode == 23) { _presetPiano(); }
//
//    if(_fxMode == 24) { _presetFadeNorthToSouth(); }
//    if(_fxMode == 25) { _presetFadeEastToWest();   }

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
    _fxCurrIndex = (_fxCurrIndex + 1) % (42);
}

/* _presetFadePodByPod:
 *
 * Fades lights 3 at a time to the next random color for the 14 pods in Studio9
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
    _fxCurrIndex = (_fxCurrIndex + 1) % (int(42 / 3));
}


// WINDOW: 2        1         2
// SIDE     1      3 2       1
//         3                  3


/* _presetFadeDiagonal
 *
 * Fades lights from north-east to south-west or vice versa
 * This is based on the physical locations of the pods in Studio9
 * this ones gonna be tricky!!
 */
void G35_Effects::_presetFadeDiagonal() {
    _fxFadeInto = true;
    if(_fxCurrIndex == 0) { 
        _nextRandomColor();
        _fxDirection = random(0, 2);
    }
    unsigned char bulb_presets[42] = {0,0,0, 0,0,0, 0,0,0, 0,0,0,
                                      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
                                      0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0 };

    for (unsigned char bulb = 0; bulb < 42; bulb++) {
        if(_fxDirection == 0) {
           // north
           if (bulb_presets[bulb] == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        } else {
           // south
           if (bulb_presets[7 - bulb] == _fxCurrIndex) { _led->setColor(bulb, _fxCurrColor); }
        }
    }
    _fxCurrIndex = (_fxCurrIndex + 1) % (7);
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
    unsigned char bulb_presets[42] = {1,0,0, 1,0,0, 1,0,0, 1,0,0,
                                      3,2,4, 3,2,4, 3,2,4, 3,2,4, 3,3,3,
                                      5,6,6, 5,6,6, 5,6,6, 5,6,6, 5,6,6 };

    for (unsigned char bulb = 0; bulb < 42; bulb++) {
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
    unsigned char bulb_presets[42] = { 1, 0, 2,   4, 3, 5,   7, 6, 8,   10, 9,11,
                                       0, 1, 1,   3, 4, 4,   6, 7, 7,    9,10,10,   13,14,18,
                                       1, 0, 2,   4, 3, 5,   7, 6, 8,   10, 9,11,   16,15,17,
                                     };
    for (unsigned char bulb = 0; bulb < 42; bulb++) {
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





//                4,2,3, 4,2,3, 4,2,3, 4,2,3,
//                5,6,6, 5,6,6, 5,6,6, 5,6,6, 5,6,6,
//                3,3,3}

//  bulb_ids = { 0, 1, 2,   3, 4, 5,   6, 7, 8,   9,10,11,
//              23,22,21,  20,19,18,  17,16,15,  14,13,12,  41,40,39,
//              24,25,26,  27,28,29,  30,31,32,  33,34,35,  36,37,38}; 

   
// void G35_Effects::_presetPolice() {
//     _fxFadeInto = false;
//     unsigned char _fxPoliceBlinks = 6;
// 
//     for(unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
// 
//        if (_fxCurrIndex % 2 == 0) {
//            _led->setRGBColor(bulb, BLACK, MAX_BRIGHTNESS);
//        } else if (_fxCurrIndex < (_fxPoliceBlinks * 2)) {
//            _led->setRGBColor(bulb, RED, MAX_BRIGHTNESS);
//        } else {
//            _led->setRGBColor(bulb, BLUE, MAX_BRIGHTNESS);
//        }
//     }
//     _fxCurrIndex = (_fxCurrIndex + 1) % (_fxPoliceBlinks * 4);
// }
// 
// void G35_Effects::_presetThreeColorFlag(unsigned int color1, unsigned int color2, unsigned int color3) {
//   _fxFadeInto = true; 
//   for (unsigned char bulb=0; bulb < NUM_LEDS; bulb++) {
//     if(bulb < 21     ) { _led->setRGBColor(bulb, color3, MAX_BRIGHTNESS); }
//     else if(bulb < 42) { _led->setRGBColor(bulb, color2, MAX_BRIGHTNESS); }
//     else               { _led->setRGBColor(bulb, color1, MAX_BRIGHTNESS); }
//   }  
// }
// 
// 
// void G35_Effects::_presetUSA()      { _presetThreeColorFlag(RED, WHITE, BLUE); }
// void G35_Effects::_presetFrance()   { _presetThreeColorFlag(BLUE, WHITE, RED); } 
// void G35_Effects::_presetIreland()  { _presetThreeColorFlag(GREEN, WHITE, ORANGE); }
// 
// void G35_Effects::_presetPodByPod() { /* TODO */ }
// void G35_Effects::_presetAllFade()  { /* TODO */ }
// void G35_Effects::_presetBars()     { /* TODO */ }
// void G35_Effects::_presetXFader()   { /* TODO */ }
// void G35_Effects::_presetPiano()    { /* TODO */ }
// void G35_Effects::_presetMovie()    { /* TODO */ }
// 
/*
  
static unsigned char effects_pod_by_pod_color  = 0;
static unsigned char effects_pod_by_pod_cycles = 0;
static unsigned char effects_pod_by_pod_width  = 0;
static unsigned char effects_pod_by_pod_mode   = 0;
static unsigned char effects_pod_by_pod_last_color = 0;

//  Studio9 LEDS
//
//  bulb_ids = { 0, 1, 2,   3, 4, 5,   6, 7, 8,   9,10,11,
//              23,22,21,  20,19,18,  17,16,15,  14,13,12,  41,40,39,
//              24,25,26,  27,28,29,  30,31,32,  33,34,35,  36,37,38}; 

   
  
static unsigned char effects_pod_by_pod_fade_ew[15][4] = {
 {  1, 21, 25, 99}, // row 1
 {  0, 23, 22, 24}, // row 2
 {  2, 26, 99, 99}, // row 3
 {  4, 18, 28, 99}, // row 4 
 
 void effects_police() {
   unsigned char blinks = 6;  
   call_timer.interval(50);
   fade_timer.interval(15); 
   mode_fade = false;

   for(unsigned char bulb_id=0; bulb_id < NUM_LEDS; bulb_id++) {
     if      (_fxCurrIndex % 2 == 0      ) { led_set_color(bulb_id, LED_MAX_BRIGHTNESS, BLACK); }
     else if (_fxCurrIndex < (blinks * 2)) { led_set_color(bulb_id, LED_MAX_BRIGHTNESS, RED  ); }
     else                                  { led_set_color(bulb_id, LED_MAX_BRIGHTNESS, BLUE ); }
   }
   _fxCurrIndex = (_fxCurrIndex + 1) % (blinks * 4);
 }
 {  3, 20, 19, 27}, // row 5
 {  5, 29, 99, 99}, // row 6
 {  7, 15, 31, 99}, // row 7
 {  6, 17, 16, 30}, // row 8
 {  8, 32, 99, 99}, // row 9
 { 10, 12, 34, 99}, // row 10
 {  9, 14, 13, 33}, // row 11
 { 11, 35, 99, 99}, // row 12
 { 37, 41, 99, 99}, // row 13
 { 36, 40, 99, 99}, // row 14
 { 38, 39, 99, 99}};// row 15
 
    
void effects_xfader() {
  call_timer.interval(15);
  fade_timer.interval(1); 
  unsigned char max_brite = 30;
  unsigned int my_colors[3] = {INCANDESCENT, INCANDESCENT, INCANDESCENT};
  unsigned int modes[3];
  
  _fxCurrIndex = (_fxCurrIndex + 1) % (max_brite * 2);  
  modes[0] = (_fxCurrIndex);
  modes[1] = ((_fxCurrIndex + 20) % (max_brite * 2));  
  modes[2] = ((_fxCurrIndex + 40) % (max_brite * 2));  
  
  for(unsigned char bulb_id=0; bulb_id < NUM_LEDS; bulb_id++) {
    unsigned char my_mode = bulb_id % 3;
    if (modes[my_mode] <= max_brite) {
      led_set_color(bulb_id, modes[my_mode], my_colors[my_mode]);
    } else {
      led_set_color(bulb_id, (max_brite - (modes[my_mode] - max_brite)), my_colors[my_mode]);
    }
  }
}



 void effects_bars() {
   call_timer.interval(5000);
   fade_timer.interval(15); 
   mode_fade = true;  
  
   unsigned char a = random(1,13);  unsigned char b = random(1,13);  unsigned char c = random(1,13);
   unsigned char d = random(1,13);  unsigned char e = random(1,13);
   while(b == a) { b = random(1, 13); }  while(c == b) { c = random(1, 13); }
   while(d == c) { d = random(1, 13); }  while(e == d) { e = random(1, 13); }
   unsigned int color[NUM_LEDS] = {a,a,a,  b,b,b,  c,c,c,  d,d,d,
                                   d,d,d,  c,c,c,  b,b,b,  a,a,a,
                                   a,a,a,  b,b,b,  c,c,c,  d,d,d,  e,e,e,
                                   d,d,d};
 for(unsigned char bulb_id=0; bulb_id < NUM_LEDS; bulb_id++) {
     led_set_color(bulb_id, LED_MAX_BRIGHTNESS, COLORS[color[bulb_id]]);
   }        
 }



 void effects_movie_mode() {
   unsigned int color[NUM_LEDS] = {12,12,12,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                                    0, 0, 0,  0, 0, 0,  0, 0, 0,  0, 0, 0,
                                   12,12,12,  0, 0, 0,  0, 0, 0,  0, 0, 0,  12,12,12,
                                   12,12,12};
   
   call_timer.interval(200);
   for(unsigned char bulb_id=0; bulb_id < NUM_LEDS; bulb_id++) {
     unsigned char bias = random(0, 80);
     unsigned char brightness = LED_MAX_BRIGHTNESS - bias;
     if (bias > 75) { brightness = 50; }
     led_set_color(bulb_id, brightness, COLORS[color[bulb_id]]);
   }        
 }
 


void effects_piano() {
   unsigned int color[NUM_LEDS] = {0,0,0,  0,0,0,  0,0,0,  0,0,0,
                                   0,0,0,  0,0,0,  0,5,0,  0,0,0,
                                   0,0,5,  5,5,5,  5,0,0,  0,0,0,  0,0,0,
                                   0,5,0};
   for(unsigned char bulb_id=0; bulb_id < NUM_LEDS; bulb_id++) {
     led_set_color(bulb_id, LED_MAX_BRIGHTNESS, COLORS[color[bulb_id]]);
   }        
 }


 // color changer pod by pod
 void effects_pod_by_pod_fade() {
   call_timer.interval(125);
   fade_timer.interval(30);    
   unsigned char address           =   99;
   static unsigned int effect_time =  500;   // time per effect
   static unsigned int hold_time   =  500;   // time to hold at current color when transition complete

   
   if(_fxCurrIndex == 0) { 
     // prevent reuse of last colors
     effects_pod_by_pod_last_color = effects_pod_by_pod_color;
     while(effects_pod_by_pod_color == effects_pod_by_pod_last_color) {
       effects_pod_by_pod_color  = random(1,13);
     }
     effects_pod_by_pod_mode   = random(0,6);

     // sequential     
     if((effects_pod_by_pod_mode == 0) || (effects_pod_by_pod_mode == 1)) {
       effects_pod_by_pod_cycles = NUM_LEDS;
       effects_pod_by_pod_width  = 0;
     }
     
     // north or south effects
     if ((effects_pod_by_pod_mode == 2) || (effects_pod_by_pod_mode == 3)) {
       effects_pod_by_pod_cycles =  7;  // number of rows in multidim array
       effects_pod_by_pod_width  = 10;  // number of cells in row
     }
     // east or west effects
     if ((effects_pod_by_pod_mode == 4) || (effects_pod_by_pod_mode == 5)) {
       effects_pod_by_pod_cycles = 15; // number of rows in multidim array
       effects_pod_by_pod_width  = 4;  // number of cells in row
     }
     // set chase time
     call_timer.interval(int(effect_time / effects_pod_by_pod_cycles));
     // call_timer.interval(125);
   }
   
   // FADE POD BY POD EFFECT
   if((effects_pod_by_pod_mode == 0) || (effects_pod_by_pod_mode == 1)) {
     if(effects_pod_by_pod_mode == 0) { address =                                 _fxCurrIndex; }
     else                             { address = effects_pod_by_pod_cycles - 1 - _fxCurrIndex; } 
     led_set_color(address, LED_MAX_BRIGHTNESS, COLORS[effects_pod_by_pod_color]); 
   }
   
   // FADE N/S/E/W EFFECT
   if((effects_pod_by_pod_mode == 2) || (effects_pod_by_pod_mode == 3) || (effects_pod_by_pod_mode == 4) || (effects_pod_by_pod_mode == 5)) {
     for(int ptr=0;ptr<effects_pod_by_pod_width;ptr++) {
       if     (effects_pod_by_pod_mode == 2) { address = effects_pod_by_pod_fade_ns[                                _fxCurrIndex][ptr]; }
       else if(effects_pod_by_pod_mode == 3) { address = effects_pod_by_pod_fade_ns[effects_pod_by_pod_cycles - 1 - _fxCurrIndex][ptr]; }
       else if(effects_pod_by_pod_mode == 4) { address = effects_pod_by_pod_fade_ew[                                _fxCurrIndex][ptr]; }
       else if(effects_pod_by_pod_mode == 5) { address = effects_pod_by_pod_fade_ew[effects_pod_by_pod_cycles - 1 - _fxCurrIndex][ptr]; }
       if(address < NUM_LEDS)                { led_set_color(address, LED_MAX_BRIGHTNESS, COLORS[effects_pod_by_pod_color]);  }
     }
   }
   
   // increment ring counter
   _fxCurrIndex = (_fxCurrIndex + 1) % (effects_pod_by_pod_cycles);
   
   if (_fxCurrIndex == 0) {
      // we reached the end of the transition, change call time
      call_timer.interval(hold_time);
   }
 }





 // color changer all
 void effects_all_by_fade() {
   call_timer.interval(4000);
   fade_timer.interval(10);
   if(mode_rand) { _fxCurrIndex = random(1,13); }
   else          { _fxCurrIndex++; }
   for(unsigned char address=0; address < NUM_LEDS; address++) { 
     led_set_color(address, LED_MAX_BRIGHTNESS, COLORS[_fxCurrIndex]); 
   }
   if ((!mode_rand) && (_fxCurrIndex >= 12)) { _fxCurrIndex = 0; }
 }
*/


