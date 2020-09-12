//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5Stack 
//
//    Sep.10 , 2020 FabLab Hamamatsu
//

#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <MIDI.h>
#include <string>

#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// WiFi settings
const char ssid[] = "xxxxxxxx";         //  #### Your Wifi ID
const char password[] = "xxxxxxxx";     //  #### Your Wifi PW
WiFiClient wifiClient;

// MQTT settings
const char* mqttBrokerAddr = "xxxxxxxx";
const char* mqttUserName = "xxxxxxxx";
const char* mqttPassword = "xxxxxxxx";
const int mqttPort = 11333;
const char* mqttClientID = "xxxxxxxx";  //  #### Your ClientID
PubSubClient mqttClient(wifiClient);

const String yourDevice("HMMT_hasebe");        //  #### Your Device
const String midiTopic("MIDI");
const String gyroTopic("GYRO");
const String swTopic("M5SW");


// Global Variables
float gyroCurtX, gyroCurtY, gyroCurtZ;  // Gyro Value
boolean sending = true;
unsigned long lastUpdateTime = 0;
unsigned long lastUpdateTime10ms = 0;
long soundCnt = -1;
int allNote[128];
boolean mp3byMIDI = false;
int cntr5sec = 0;

// Macro
#define   MP3_SOUND   0
#define   BEEP_SOUND  1
#define   SOUND_TYPE  MP3_SOUND

// MIDI
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

//  MP3
AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

//-------------------------------
//  Arduino Functions
//-------------------------------
void setup() {
  int wifiCheckCount = 0;
  mp3 = nullptr;
  for (int i=0; i<128; i++){ allNote[i] = 0;}

  M5.begin();
  initPrintSomewhere();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    printSomewhere(".");
    delay(500);
    if ( ++wifiCheckCount > 2000 ){
      printSomewhere("\n");
      printSomewhere("No Wifi Connection!\n");
      break;
    }
  }

  //  MQTT
  mqttClient.setServer(mqttBrokerAddr, mqttPort);
  mqttClient.setCallback(mqttCallback);
  printSomewhere("\n");

  //  Initialize MIDI
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin();
  MIDI.turnThruOff();
}

void loop() {
  unsigned long t = millis();
  M5.update();
  reConnect();
  mqttClient.loop();

  MIDI.read();
  loopMp3();

  // update a value from sensor every 10ms
  if (t - lastUpdateTime10ms >= 10) {
    lastUpdateTime10ms = t;
    //  something every 10msec
  }

  // update a value from sensor every 1000ms
  if (t - lastUpdateTime >= 1000) {
    lastUpdateTime = t;
    //  something every 1000msec
    checkMIDINote();
  }

  // Switch Enable/Disable MQTT transfer by pushing button B
  if (M5.BtnB.wasPressed()) {
    togglePlayMP3();
  }
  // Push Button A
  if (M5.BtnA.wasPressed()) {
    // Send
    const String topicStr = yourDevice+"/"+swTopic+"/A";
    const char* const topic = topicStr.c_str();
    const char* const msg = "Pressed";
    printSomewhere(topic);
    printSomewhere(msg);
    mqttClient.publish(topic, msg);
  }
  // Release Button A
  if (M5.BtnA.wasReleased()) {
    // Send
    const String topicStr = yourDevice+"/"+swTopic+"/A";
    const char* const topic = topicStr.c_str();
    const char* const msg = "Released";
    printSomewhere(topic);
    printSomewhere(msg);
    mqttClient.publish(topic, msg);
  }
}

//-------------------------------
//  MQTT
//-------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  String yd = topic;
  int sp1 = yd.indexOf('/');
  int sp2 = yd.lastIndexOf('/');
  String dev = yd.substring(0,sp1);
  String type = yd.substring(sp1+1,sp2);
  String ev = yd.substring(sp2+1,yd.length());
  printSomewhere("**Message has come!**");

  // Add conditions
  if ( type.equals(midiTopic) ){
    playMidi(ev, payload, length);
  }
  else if ( type.equals(gyroTopic) ){
    
  }
}

void reConnect() { // 接続が切れた際に再接続する
  static unsigned long lastFailedTime = 0;
  static boolean lastConnect = false;
  unsigned long t = millis();
  if (!mqttClient.connected()) {
    if (lastConnect || t - lastFailedTime >= 5000) {
      if (mqttClient.connect(mqttClientID, mqttUserName, mqttPassword)) {
        printSomewhere("MQTT Connect OK.");
        lastConnect = true;
        mqttClient.subscribe("#");
      } else {
        printSomewhere("MQTT Connect failed, rc=");
        // http://pubsubclient.knolleary.net/api.html#state に state 一覧が書いてある
        printSomewhere(mqttClient.state());
        lastConnect = false;
        lastFailedTime = t;
      }
    }
  }
}

//-------------------------------
//  Print for Debug
//-------------------------------
void initPrintSomewhere(void)
{
}
void printSomewhere(int num)
{
  char strx[128]={0};
  sprintf(strx,"%d",num);
  M5.Lcd.printf("%s",strx);
}
void printSomewhere(const char* txt)
{
  M5.Lcd.printf(txt);
}

//-------------------------------
//  MP3
//-------------------------------
const char* soundFileName[] = {
"/airforce.mp3",
"/ambulance.mp3",
"/ball.mp3",
"/bird.mp3",
"/birdshop.mp3",
"/cicada.mp3",
"/cicada2.mp3",
"/heli.mp3",
"/plane.mp3",
"/race.mp3",
"/rain.mp3",
"/rain2.mp3",
"/shinkansen.mp3",
"/shop.mp3",
"/shore.mp3",
"/shore2.mp3",
"/train.mp3",
"/tunnel.mp3",
"/walk.mp3",
"/waterfall.mp3",
"/wave.mp3"
};

void togglePlayMP3(void)  //  for Switch On/Off
{
  if (mp3 != nullptr){
    if (mp3->isRunning()){
      stopMP3();
    }
  }
  else {
    playMP3Random();
  }
}

void playMP3Random(void)
{
  printSomewhere("Play MP3: ");
  const char* name = soundFileName[random(21)];
  printSomewhere(name);
  printSomewhere("\r\n");
  playMP3(const_cast<char*>(name));
}

void playMP3(char *filename){
  file = new AudioFileSourceSD(filename);
  id3 = new AudioFileSourceID3(file);
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  out->SetGain(0.2);  //  音量1/5
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);
}
void stopMP3(void)
{
  mp3->stop();
  mp3 = nullptr;
}
void loopMp3(void)
{
  if ((mp3 != nullptr ) && (mp3->isRunning())) {
    if (!mp3->loop()){
      stopMP3();
      playMP3Random();
    }
  }  
}

//-------------------------------
//  Send MIDI message
//-------------------------------
const String nton("note_on");
const String ntof("note_off");
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (sending==false) return;
  const String topicStr = yourDevice+"/"+midiTopic+"/"+nton;
  const char* topic = topicStr.c_str();
  const String msgStr = String(channel)+"-"+String(pitch)+"-"+String(velocity);
  const char* msg = msgStr.c_str();
  printSomewhere(topic);
  printSomewhere(msg);
  mqttClient.publish(topic, msg);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (sending==false) return;
  const String topicStr = yourDevice+"/"+midiTopic+"/"+ntof;
  const char* topic = topicStr.c_str();
  const String msgStr = String(channel)+"-"+String(pitch)+"-"+String(velocity);
  const char* msg = msgStr.c_str();
  printSomewhere(topic);
  printSomewhere(msg);
  mqttClient.publish(topic, msg);
}

//-------------------------------
//  Play MIDI message
//-------------------------------
void soundOn(byte pitch)
{
  allNote[pitch]++;
  if ((mp3byMIDI == false) && (mp3 == nullptr)){
    playMP3Random();
  }
  mp3byMIDI = true;
}
void soundOff(byte pitch)
{
  if ( allNote[pitch] != 0 ){
    allNote[pitch]--;
  }
}
void checkMIDINote(void)
{
  cntr5sec++;
  if (cntr5sec <= 5){
    return;
  }
  cntr5sec = 0;
  
  //  Check whether all notes are off or not.
  boolean allOff = true;
  for (int i=0; i<128; i++){
    if (allNote[i] != 0){
      allOff = false;
      break;
    }
  }
  if (allOff == true){
    if ((mp3 != nullptr) && (mp3byMIDI == true)){ stopMP3();}
    mp3byMIDI = false;
  }
}
void playMidi(String ev, byte* payload, unsigned int length)
{
  String msg = (char*)payload;
  msg = msg.substring(0,length);
  int sp1 = msg.indexOf('-');
  int sp2 = msg.lastIndexOf('-');
  const String ch = msg.substring(0,sp1);
  const String nt = msg.substring(sp1+1,sp2);
  const String vl = msg.substring(sp2+1,msg.length());

  int note = atoi(nt.c_str());
  int vel = atoi(vl.c_str());
  int channel = atoi(ch.c_str());
  if (ev.equals(nton)){
    MIDI.sendNoteOn(note, vel, channel);
    soundOn(note);
  }
  else if (ev.equals(ntof)){
    MIDI.sendNoteOff(note, vel, channel);
    soundOff(note);
  }
}
