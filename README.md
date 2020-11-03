# HamamatsuProject
 Data Transfer System by MQTT

## M5StickC_serial_cushion(Nov.3 2020)
 Filename is 'serial_cushion.ino'.
 Added for the cushion at Hamamatsu side on this event.
 It's based on muse_chair. 
 This program output On/Off of GPIO for relay of cushion when it receive MQTT from Bankok.
 One output sequence data describe when massage machine will turn on in the cushion.

### Required library
- M5Stack
- PubSubClient


## M5Stack_CloudMQTT_MIDI_OUT(Oct.24 2020)
 Filename is 'M5Stack_CloudMQTT_MIDI_OUT.ino'.
 Added for MIDI keyboard at Hamamatsu side on this event.
 When someone play this keyboard, this program send MQTT-MIDI data to Bankok. 

### Required library
- M5Stack
- MIDI Library
- PubSubClient

## M5StickC_CloudMQTT_muse_chair(Oct.22 2020)
 Filename is 'M5Stack_CloudMQTT_MIDI_OUT.ino'. 
 This program send serial data to Arduino Uno of massage chair when it receive MQTT from Bankok.
 One serial data describe how 5 each cylinder will move in massage chair.

### Required library
- M5StickC
- PubSubClient

## M5Stack-MQTT-MIDI-MP3(Sep.13 2020)
Filename is "M5Stack_CloudMQTT_MIDI_MP3.ino".  
You can hear Hamamatsu Sound from M5stack by pushing buttonB and receiving MIDI.

### Required library
- M5Stack
- MIDI Library
- PubSubClient
- ESP8266Audio https://github.com/earlephilhower/ESP8266Audio

### How to prepare Hamamatsu Sound
- Set MP3 file to M5Stack.
    1. There are 21 files in folder "HamamatsuSound" below.
        - https://github.com/hasebems/HamamatsuProject/tree/master/HamamatsuSound
    1. Prepare a microSD card.
    1. Copy those files to root directory of the microSD card.
    1. Insert the microSD card into M5stack.

### How to modify a sketch "M5Stack_CloudMQTT_MIDI_MP3".
- write your WiFi SSID/PW.
    - const char ssid[] = "xxxxxxxx"; //  #### Your Wifi ID
    - const char password[] = "xxxxxxxx"; //  #### Your Wifi PW
- write MQTT settings. (You should get these information from Hamamatsu)
    - const char* mqttBrokerAddr = "xxxxxxxx";
    - const char* mqttUserName = "xxxxxxxx";
    - const char* mqttPassword = "xxxxxxxx";
- write your client ID. Anything you like is OK.
    - const char* mqttClientID = "xxxxxxxx"; // #### Your Client ID
- Install this firmware to your M5Stack.

### How to use this sketch
- If you push buttonB(center button), you can hear Hamamatsu Sound randomly.
- If M5Stack get MQTT-MIDI, you can hear Hamamatsu Sound randomly.

## M5Stack-MQTT-MIDI
 Filename is 'M5StickC_CloudMQTT_MIDI.ino', but it's for M5Stack.
 You may prepare only M5Stack.

### Required library
- M5Stack
- MIDI Library
- PubSubClient

### How to set up your MIDI instrument
- When it receives MIDI data, it generates short beep sound.
    - Evenif you don't have any MIDI instrument, you can recognize receiving MIDI.
- If you want to use a real MIDI instrument, You need MIDI shield or board that has same functionarity.
    - https://www.sparkfun.com/products/12898
    - connect 4lines to M5stack (Rx/Tx/5v/Gnd)
    - connect to MIDI instrument

### How to modify a sketch "M5StickC_CloudMQTT_MIDI.ino"
- write your WiFi SSID/PW.
    - const char ssid[] = "xxxxxxxx"; //  #### Your Wifi ID
    - const char password[] = "xxxxxxxx"; //  #### Your Wifi PW
- write MQTT settings. (You should get these information from Hamamatsu)
    - const char* mqttBrokerAddr = "xxxxxxxx";
    - const char* mqttUserName = "xxxxxxxx";
    - const char* mqttPassword = "xxxxxxxx";
- write your client ID. Anything you like is OK.
    - const char* mqttClientID = "xxxxxxxx"; // #### Your Client ID
- Install this firmware to your M5Stack.


## Processing_catch_MQTT(Nov.3 2020)
- It's just for watching MQTT message.


## Processing_MQTT_MIDI

### Mac setting
- Open "Audio MIDI Setup"
    - Applications -> Utilities -> Audio MIDI Setup
- Choose "Show MIDI Studio" from pulldown menu
- Double click "IAC Driver"
- Choose "Ports" tab
- Add new port by clicking "+", and name it freely
- Install "Processing"
    - https://processing.org/download/

### How to modify a sketch "Processing_MQTT_MIDI.pde"
- write topicName that you want to get.
    - String topicNtOn = "HMMT_hasebe/MIDI/note_on";  //  ####
    - String topicNtOf = "HMMT_hasebe/MIDI/note_off"; //  ####
- write MIDI port name that you wrote in "Audio MIDI Setup".
    - String myMIDIIn = "xxxxxxxx";                  //  ####
    - String myMIDIOut = "xxxxxxxx";                  //  ####

### How to start
- Start "Garage Band"
- Start "Processing" and Open "Processing_MQTT_MIDI.pde"
- Click Start(Play) Button in Processing
