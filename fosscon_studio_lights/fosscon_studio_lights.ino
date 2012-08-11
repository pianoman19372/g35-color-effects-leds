// GE Christmas light control for Arduino  
// Edward Long
//
// Based off of code by Scott Harris <scottrharris@gmail.com>  
//     For more information, see <scottrharris.blogspot.com>
// Based off of code by Robert Quattlebaum <darco@deepdarc.com>  
//     For more information, see <http://www.deepdarc.com/2010/11/27/hacking-christmas-lights/>.
//


// need to include this prior to including G35_Effects.h
#include <Metro.h>

/* NOTE:  
 *
 * The following section is used by an external home automation controller
 * You dont need this section for normal use.
 */
#include <SoftwareSerial.h>
#define SERIAL_RX_PIN          (2)
#define SERIAL_TX_PIN          (3)
#define SERIAL_INVERTED     (true)
#define SERIAL_BAUD         (9600)
SoftwareSerial amx(SERIAL_RX_PIN, SERIAL_TX_PIN, SERIAL_INVERTED);

#include <G35_Lights.h>
#include <G35_Effects.h>

// LED Strand
#define LED_DATA_PIN           (7)
#define LED_STATUS_PIN         (1)
G35_Lights     led = G35_Lights(LED_DATA_PIN, LED_STATUS_PIN);
G35_Effects    fx  = G35_Effects(&led);


Metro tester = Metro(1000);
unsigned char test_index = 0;

void setup() { 
  // Configure LED Hardware
  led.enumerateBulbs();
  //delay(1000);
  //fx.setEffects(FX_POLICE, 100, 9999);
  //fx.setEffects(FX_POLICE, 50, 9999);
  //fx.setEffects(FX_IRELAND, 10000, 10);
  
  fx.setEffects(26, 200, 50);
  // Configure USB Serial Port
  Serial.begin(9600);
  
  // Configure Home Automation Serial Port
  amx.begin(9600);  
}

void loop() {  
  processSerialCommands();
  fx.thread();
}


// Method to handle serial commands
void processSerialCommands() {
  if(amx.available() or Serial.available()) {
    char c = 0;  
    if      (amx.available())    { c = amx.read();    }
    else if (Serial.available()) { c = Serial.read(); }
    
    if(c ==  65) { fx.setEffects( 0,    25, 9999); }  // A: off
    //if(c ==  66) { fx.setEffects(18,   333, 9999); }  // B: movie
    //if(c ==  67) { fx.setEffects(19,   125, 9999); }  // C: pod by pod
    //if(c ==  68) { fx.setEffects(20,   125, 9999); }  // D: all fade
    if(c ==  69) { fx.setEffects( 1,    50, 9999); }  // E: police
    //if(c ==  70) { fx.setEffects(21,   125, 9999); }  // F: bars
    //if(c ==  71) { fx.setEffects(22,   125, 9999); }  // G: xfader
    //if(c ==  72) { fx.setEffects(23,    25, 9999); }  // H: piano
    //if(c ==  75) { fx.setEffects(15, 10000,   10); }  // K: usa
    
    if(c ==  76) { fx.setEffects(16,   200,   50); }  // L: ireland
    if(c ==  77) { fx.setEffects(17,   200,   50); }  // M: france
    if(c ==  78) { fx.setEffects(14,    50,   50); }  // N: fade 1x1
    if(c ==  79) { fx.setEffects(24,   200,   50); }  // O: fade N-S
    if(c ==  80) { fx.setEffects(25,   200,   50); }  // P: fade E-W
        
    if(c ==  97) { fx.setEffects( 0,    10, 10); }  // a: black
    if(c ==  98) { fx.setEffects( 2,    10, 10); }  // b: red
    if(c ==  99) { fx.setEffects( 3,    10, 10); }  // c: orange
    if(c == 100) { fx.setEffects( 4,    10, 10); }  // d: yellow
    if(c == 101) { fx.setEffects( 5,    10, 10); }  // e: green
    if(c == 102) { fx.setEffects( 6,    10, 10); }  // f: cyan
    if(c == 103) { fx.setEffects( 7,    10, 10); }  // g: blue
    if(c == 104) { fx.setEffects( 8,    10, 10); }  // h: atomic blue
    if(c == 105) { fx.setEffects( 9,    10, 10); }  // i: magenta
    if(c == 106) { fx.setEffects(10,    10, 10); }  // j: deep magenta
    if(c == 107) { fx.setEffects(11,    10, 10); }  // k: violet
    if(c == 108) { fx.setEffects(12,    10, 10); }  // l: white
    if(c == 109) { fx.setEffects(13,    10, 10); }  // m: incandescent    

  }
}
