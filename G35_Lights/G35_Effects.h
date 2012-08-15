/* GE G-35 ColorEffects Effects Header File
 * (C) 2012 Edward F Long Jr.
 *
 * This software is licensed under the MIT license
 * For more information please refer to the MIT-LICENSE file
 */

#ifndef G35_EFFECTS
#define G35_EFFECTS

#include "Arduino.h"
#include <G35_Lights.h>
#include "Metro.h"


class G35_Effects {
  
  public:
    G35_Effects(G35_Lights *led);
    void setEffects(unsigned char, unsigned int, unsigned int);
    void thread();

  private:  
    // pointer to G35_Lights instance
    G35_Lights *_led;

    // FX state management;
    unsigned char _fxMode;
    bool          _fxFadeInto;
    unsigned int  _fxCurrIndex;
    unsigned char _fxCurrColor;
    unsigned char _fxCurrOffset;
    unsigned char _fxDirection;

    // FX transition methods
    void _nextTransition();
    void _nextRandomColor();

    // FX Presets
    void _presetOFF();
    void _presetDirectColor(unsigned int);
    void _presetDirectBlack();
    void _presetDirectRed(); 
    void _presetDirectOrange(); 
    void _presetDirectYellow(); 
    void _presetDirectGreen(); 
    void _presetDirectCyan(); 
    void _presetDirectBlue(); 
    void _presetDirectAtomicBlue(); 
    void _presetDirectMagenta(); 
    void _presetDirectDeepMagenta(); 
    void _presetDirectViolet(); 
    void _presetDirectWhite(); 
    void _presetDirectIncandescent();

    void _presetFadeOneByOne();
    void _presetFadePodByPod();
    void _presetFadeNorthOrSouth();
    void _presetFadeEastOrWest();
    void _presetFadeDiagonal();

//    void _presetFadeEastToWest();

//    void _presetPolice();
//
//    void _presetThreeColorFlag(unsigned int, unsigned int, unsigned int);
//    void _presetUSA();
//    void _presetFrance();
//    void _presetIreland();
//
//    void _presetPodByPod();
//    void _presetAllFade();
//    void _presetBars();
//    void _presetMovie();
//    void _presetXFader();
//    void _presetPiano();
//

};

#endif
