Lighting Stuff
==============

About
=====
This library is used to control GE ColorEffects G-35 RGB LED Christmas lights to do 
cool things:


Features
========
    Smooth fading between colors
    Basic support for transitions
    Support to reassign bulb ids
    External control from 3rd party systems



Hardware Specifications
=======================
  GE ColorEffects G-35 RGB LED Lights
  Arduino UNO

How the LEDS work
=================
The LEDS consist of a series of bulbs that are self-addressing and self-clocking and contain a
red, green, and blue LED.  Each bulb is networked together using a 1-way linear daisy chain network.

CONTROLLER ---> BULB0 --> BULB1 --> ... BULBn

When initially powered up, all bulbs go into an "addressing" mode until reception of a command
After the first command is received, the bulb goes into its "normal" operation mode.

During addressing mode:
 * When a command is received on the DATA IN line:
   * The bulb will set its internal address to whatever address is was included in the command it received.
   * The bulb will go to the specified color based on the protocol.
   * The bulb will NOT echo the command on the DATA out line.
   * The bulb will transition to "normal" operation mode

During normal operation mode:
 * When a command is received on the DATA IN line:
   * If the command matches the address of the bulb(OR the address is the BROADCAST address):

   * The bulb will set its internal address to whatever address is was included in the command it received.
   * The bulb will go to the specified color based on the protocol.
   * The bulb will NOT echo the command on the DATA out line.
   * The bulb will transition to "normal" operation mode

   on the DATA IN line
 * The bulb will pod will illuminate to
 * The pod will not echo the command on the DATA OUT line

During normal mode:

e first command
from the address field of the networking protocol.

During


during addressing mode, the pod will configure itself with the address of the command coming
down the wire and 

normally the DATA line is gated LO.

using a daisy chain network Each pod has a DATA IN and a DATA OUT They are connected together with common 

When initially powered on, each pod goes into 'addressing mode'   


General Disclaimer
==================
While these modifications are fairly safe, due to manufacturing changes your milage may 
vary.  I've noticed that the Arduino UNO and these lights are fairly sensitive to EMI.  
More so if you choose to extend the distance between the LEDS.  Make sure you properly
terminate your wires in a safe manor especially if you choose to use these outdoors.

I provide these instructions and source code "as-is" without any liability under the 
MIT license with the following warnings:

 * A 50 bulb set pulls 3-4 amps at 5V DC.  
   * Wire it backwards and you will fry your LEDS and/or the Arduino.
   * So, Check your polarity!!!!
     * -5VDC is the ridged wire.
     * DATA is the middle wire.
     * +5VDC is the remaining wire.
 * Always splice and soldier the connections you make to ensure a clean connection.
 * Always cover your splices with waterproof heatshrink tubing or use dielectric grease.
 * Always enclose your arduino in a waterproof project case if used outdoors.
 * Use caution when extending the distance between bulbs.  
   * The DATA line (middle wire) can pick up induced currents from the adjacent +5VDC and -5VDC lights
     * Its strongly recommened to limit the length of the runs and dont run the dataline adjacent to 
       ANY powerlines (including the +/- 5VDC lines if extending the lines)


The Arduino UNO and these lights are fairly sensitive to EMI

If you splice and sodier the wires, make sure you soder and cover them 
correctly, (even more so if you're using these outside)




