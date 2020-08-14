# HamamatsuProject
 Data Transfer System by MQTT

## M5StickC

### Required library
- M5Stack / M5StickC
- MIDI Library
- PubSubClient

### How to set up your MIDI instrument
- You need MIDI shield or board that has same functionarity.
    - https://www.sparkfun.com/products/12898
- connect 4lines to M5stick (Rx/Tx/5v/Gnd)
- connect to MIDI instrument

### How to modify a sketch "M5StickC_CloudMQTT_MIDI.ino"
- write your WiFi SSID/PW.
    - const char ssid[] = "xxxxxxxx"; //  #### Your Wifi ID
    - const char password[] = "xxxxxxxx"; //  #### Your Wifi PW
- write your device name.
    - const String yourDevice("xxxxxxxx"); //  #### Your Device
- Install this firmware to your M5StickC.



## Processing

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
