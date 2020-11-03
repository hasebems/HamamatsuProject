//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5Stack
//
//    Aug.8 , 2020 FabLab Hamamatsu
//

#include <M5Stack.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <MIDI.h>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

// WiFi 設定
const char ssid[] = "";         //  #### Your Wifi ID
const char password[] = "";     //  #### Your Wifi PW
WiFiClient wifiClient;

// MQTT settings
const char* mqttBrokerAddr = "";
const char* mqttUserName = "";
const char* mqttPassword = "";
const int mqttPort = 0;
const char* mqttClientID = "HMMTPIANO1101";  //  #### Your ClientID
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

// MIDI
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

void printSomewhere(int num);
void printSomewhere(const char* txt);

//-------------------------------
//  Arduino Functions
//-------------------------------
void setup() {
  int wifiCheckCount = 0;
  int wifiExitCount = 0;

  M5.begin();
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    printSomewhere(".");
    delay(500);
    if ( ++wifiCheckCount > 10 ){ // 5sec
      printSomewhere("\n");
      printSomewhere("Retry Wifi Connection.\n");
      WiFi.begin(ssid, password);
      wifiCheckCount = 0;
      if ( ++wifiExitCount > 10 ){
        printSomewhere("\n");
        printSomewhere("No Wifi Connection!\n");
        break;
      }
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

  //  for beep
  M5.Speaker.begin();

  // 別タスクの起動
  xTaskCreatePinnedToCore(task0, "Task0", 4096, NULL, 1, NULL, 0);
}

// MQTTの再接続に時間がかかるみたいなので、別タスクで動かす
void task0(void* param)
{
  while (true) {
    if(checkWifi()) {
      if(checkMQTT()) {
        mqttClient.loop();
      } else {
        vTaskDelay(1000); // 再接続まで間隔を開ける
      }
    } else {
    }
    vTaskDelay(1); // タスク内の無限ループには必ず入れる
  }
}

void loop() {
  unsigned long t = millis();
  M5.update();

  MIDI.read();

  // update a value from sensor every 10ms
  if (t - lastUpdateTime10ms >= 10) {
    lastUpdateTime10ms = t;
    beepOff();
  }

  // Switch Enable/Disable MQTT transfer by pushing button B
  if (M5.BtnB.wasPressed()) {
    sending = !sending;
    if (sending){
      printSomewhere("Available sending.");
    }
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
  // Push Button C
  if (M5.BtnC.wasPressed()) {
    // Clear LCD
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
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
  printSomewhere("*");

  // Add conditions
  if ( type.equals(midiTopic) ){
    //playMidi(ev, payload, length);
  }
  else if ( type.equals(gyroTopic) ){
    
  }
}

boolean checkWifi() {
  static int lastWiFiStatus = WL_DISCONNECTED;
  int wifiStatus = WiFi.status();
  if (lastWiFiStatus != WL_CONNECTED && wifiStatus == WL_CONNECTED) {
    printSomewhere("WiFi connected.");
    printSomewhere("IP address: ");
    printSomewhere(WiFi.localIP().toString().c_str());
  } else if (lastWiFiStatus == WL_CONNECTED && wifiStatus != WL_CONNECTED) {
    printSomewhere("WiFi disconnected.");
  }
  lastWiFiStatus = wifiStatus;
  return (wifiStatus == WL_CONNECTED);
}

boolean checkMQTT() {
  unsigned long t = millis();
  boolean conn = mqttClient.connected();
  if (!conn) {
    printSomewhere("MQTT Disconnect. Connecting...");
    conn = mqttClient.connect(mqttClientID, mqttUserName, mqttPassword);
    if (conn) {
      printSomewhere("MQTT Connect OK.");
      mqttClient.subscribe("#");
    } else {
      printSomewhere("MQTT Connect failed, rc=");
      // http://pubsubclient.knolleary.net/api.html#state に state 一覧が書いてある
      printSomewhere(mqttClient.state());
    }
  }
  return conn;
}

//-------------------------------
//  Print for Debug
//-------------------------------
void printSomewhere(int num)
{
  char strx[128]={0};
  sprintf(strx,"%d",num);
  M5.Lcd.printf("%s",strx);
  Serial.println(strx);
}

void printSomewhere(const char* txt)
{
  M5.Lcd.printf(txt);
  Serial.println(txt);
}

//-------------------------------
//  Send MIDI message
//-------------------------------
void beepOff(void)
{
  if (soundCnt >= 0){
    if (++soundCnt > 10){
      soundCnt = -1;
      M5.Speaker.mute();
    }
  }
}

void beepOn(byte pitch)
{
  float freq = 440 * pow(2.0,((float)pitch - 69)/12);
  ledcSetup( TONE_PIN_CHANNEL, (int)freq, 13);
  ledcWrite( TONE_PIN_CHANNEL, 0x0FF );
  soundCnt = 0;
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
void playMidi(String ev, byte* payload, unsigned int length)
{
  String msg = (char*)payload;
  msg = msg.substring(0,length);
  int sp1 = msg.indexOf('-');
  int sp2 = msg.lastIndexOf('-');
  const String ch = msg.substring(0,sp1);
  const String nt = msg.substring(sp1+1,sp2);
  const String vl = msg.substring(sp2+1,msg.length());

  if (ev.equals(nton)){
    int note = atoi(nt.c_str());
    MIDI.sendNoteOn(note, atoi(vl.c_str()), atoi(ch.c_str()));
    beepOn(note);
  }
  else if (ev.equals(ntof)){
    MIDI.sendNoteOff(atoi(nt.c_str()), atoi(vl.c_str()), atoi(ch.c_str()));
  }
}
