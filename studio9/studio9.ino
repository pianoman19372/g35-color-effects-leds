// Studio9 RGB Lighting Controller v2.00 for Arduino  
// Copyright (C) 2012 Edward F Long Jr.
//

// need to include this prior to including G35_Effects.h
#include <Metro.h>

// Initialize SoftwareSerial for external control via an AMX NI-2100
#include <SoftwareSerial.h>
#define SERIAL_RX_PIN          (2)
#define SERIAL_TX_PIN          (3)
#define SERIAL_INVERTED     (true)
#define SERIAL_BAUD         (9600)
SoftwareSerial amx(SERIAL_RX_PIN, SERIAL_TX_PIN, SERIAL_INVERTED);

// Include GE G-35 LED Lights Code
#include <G35_Lights.h>
#include <G35_Effects.h>

// LED Strand
#define LED_DATA_PIN           (7)
#define LED_STATUS_PIN         (1)
G35_Lights     led = G35_Lights(LED_DATA_PIN, LED_STATUS_PIN);
G35_Effects    fx  = G35_Effects(&led);

void setup() { 

  // Configure USB Serial Port
  Serial.begin(9600);

  // Configure Home Automation Serial Port
  amx.begin(9600);
  
  // Configure LED Hardware
  led.enumerateBulbs();
  fx.setEffects(37, 150,  50);
}

void loop() {  
  processSerialCommands();
  fx.thread();
}

// Method to handle serial commands
void processSerialCommands() {

  if(Serial.available() || amx.available()) {
    char c;
    
    // try until both Serial and amx buffers are empty!
    while (Serial.available() || amx.available()) {
      if(Serial.available())    { c = Serial.read(); }
      else if (amx.available()) { c = amx.read(); }
    }

    // Direct Color Presets
    if(c ==  97) { fx.setEffects( 1,   1,  50); }  // a: black / off
    if(c ==  98) { fx.setEffects( 2,   1,  50); }  // b: red
    if(c ==  99) { fx.setEffects( 3,   1,  50); }  // c: orange
    if(c == 100) { fx.setEffects( 4,   1,  50); }  // d: yellow
    if(c == 101) { fx.setEffects( 5,   1,  50); }  // e: green
    if(c == 102) { fx.setEffects( 6,   1,  50); }  // f: cyan
    if(c == 103) { fx.setEffects( 7,   1,  50); }  // g: blue
    if(c == 104) { fx.setEffects( 8,   1,  50); }  // h: atomic blue
    if(c == 105) { fx.setEffects( 9,   1,  50); }  // i: magenta
    if(c == 106) { fx.setEffects(10,   1,  50); }  // j: deep magenta
    if(c == 107) { fx.setEffects(11,   1,  50); }  // k: violet
    if(c == 108) { fx.setEffects(12,   1,  50); }  // l: white
    if(c == 109) { fx.setEffects(13,   1,  50); }  // m: incandescent

    if(c ==  65) { fx.setEffects(30,  50,  50); }  // A: 1x1 fade
    if(c ==  66) { fx.setEffects(31, 100,  25); }  // B: PODxPOD fade
    if(c ==  67) { fx.setEffects(32, 200,  50); }  // C: North/South fade
    if(c ==  68) { fx.setEffects(33, 200,  50); }  // D: East/West fade
    if(c ==  69) { fx.setEffects(34, 200,  50); }  // E: Diagonal fade
    if(c ==  70) { fx.setEffects(35,  50,  50); }  // F: Checker fade
    if(c ==  71) { fx.setEffects(36,  50,  50); }  // G: Random Pod Color fade    
    if(c ==  72) { fx.setEffects(37, 150,  50); }  // H: Movie (candlelight)    
    if(c ==  73) { fx.setEffects(38, 150,  50); }  // I: Movie (no flicker)    
    if(c ==  74) { fx.setEffects(39, 150,  50); }  // J: Piano      
    if(c ==  75) { fx.setEffects(40,  50,9999); }  // K: Police Strobe    
    if(c ==  76) { fx.setEffects(41,  50,9999); }  // L: Ireland Flag        
    if(c ==  77) { fx.setEffects(42,  50,9999); }  // M: USA Flag    
    if(c ==  78) { fx.setEffects(43,10000, 50); }  // N: Ireland Flag (FADE)        
    if(c ==  79) { fx.setEffects(44,10000, 50); }  // O: USA Flag (FADE)
    if(c ==  80) { /* random */                 }  // P: Random Effect every 10 seconds
  }
}
