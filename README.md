# HamamatsuProject
 Data Transfer System by MQTT

## Required library
- M5Stack / M5StickC
- MIDI Library
- PubSubClient

## How to set up your MIDI instrument
- You need MIDI shield or board that has same functionarity.
    - https://www.sparkfun.com/products/12898
- connect 4lines to M5stick (Rx/Tx/5v/Gnd)
- connect to MIDI instrument

## How to modify a sketch "M5StickC_CloudMQTT_MIDI.ino"
- write your WiFi SSID/PW
    - const char ssid[] = "xxxxxxxx"; //  #### Your Wifi ID
    - const char password[] = "xxxxxxxx"; //  #### Your Wifi PW
- write your device name
    - const String yourDevice("xxxxxxxx"); //  #### Your Device
