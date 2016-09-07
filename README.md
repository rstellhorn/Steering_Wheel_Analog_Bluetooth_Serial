# Steering_Wheel_Analog_Bluetooth_Serial
Car Steering Wheel Media button interface to Bluetooth Module and USB HID for Fixed Installation of a Tablet

Analog Steering Wheel Controls Interface

This uses an Analog input to measure button presses
then sends the appropiate commands to an BlueCreations.com
BC127 bluetooth module that is running Melody Audio v6 
via Serial.  It also uses HID to do screen unlock and app
switching of a USB OTG connected tablet

I'm using a SparkFun.com PurpleTooth 
Jamboree BC127 Development Board. I've updated it to 
the newest firmware, Melody 6.0.48, and configured it
to support multiple connections for audio and phone.
The Support -> Tools section of their site has the 
firmware update tools and also offers
a tool to activate APTX and CVC codecs for testing
for free. I will include my running configuration as
a text file in this package.

https://www.sparkfun.com/products/11924

Note that I did not use the BC127 Library that is provided.
It was written for an old firmware version and does not
really support the features needed for this task.

Requires https://github.com/NicoHood/HID

The values below are what I measured. If yours differs
use the analog to serial example sketch to retrieve
new numbers.  NOTE: if you enable more Digital Output pins
these values will change.

Vol Up 47-49
Vol Down 55-57
Seek Up 75-77
Seed Down 63-65
Source Up 167-169
Source Down 282-283
Talk 92-94
Prog 118-120 
normal 1018
with input_pullup

On a 05 chevy silverado the two wires are connected into the BCM
on Connector 3 (Brown) Pin A7 (Lite Green), 
and Connector 2 (Grey) Pin A2 (Dark Blue).
I cut the wires just short of the connectors and spliced them
to wires going to GND and A0 on the Leonardo.
