//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5Stick 
//
//    Aug.8 , 2020 FabLab Hamamatsu
//
#define M5STACK   0
#define M5STICK   1
#define YOUR_DEVICE  M5STACK             //  #### Your Device M5STACK or M5STICK

#if ( YOUR_DEVICE == M5STACK )
  #include <M5Stack.h>
#elif ( YOUR_DEVICE == M5STICK )
  #include <M5StickC.h>
#endif
#include <WiFi.h>
#include <PubSubClient.h>
#include <MIDI.h>
#include <string>

// WiFi settings
const char ssid[] = "xxxxxxxx";         //  #### Your Wifi ID
const char password[] = "xxxxxxxx";     //  #### Your Wifi PW
WiFiClient wifiClient;

// MQTT settings
const char* mqttBrokerAddr = "tailor.cloudmqtt.com";
const char* mqttUserName = "ovjirwrr";
const char* mqttPassword = "S0f6ZlAbEwqp";
const int mqttPort = 11333;
const char* mqttClientID = "M5Stack(hasebe)";  //  #### Your ClientID
PubSubClient mqttClient(wifiClient);

const String yourDevice("HMMT_hasebe");        //  #### Your Device
const String midiTopic("MIDI");
const String gyroTopic("GYRO");
const String swTopic("M5SW");


// Global Variables
float gyroCurtX, gyroCurtY, gyroCurtZ;  // Gyro Value
boolean sending = false;
unsigned long lastUpdateTime = 0;

// MIDI
MIDI_CREATE_DEFAULT_INSTANCE();

//-------------------------------
//  Arduino Functions
//-------------------------------
void setup() {
  int wifiCheckCount = 0;
  
  M5.begin();
  initPrintSomewhere();
  initGyro();
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
}

void loop() {
  unsigned long t = millis();
  M5.update();
  reConnect();
  mqttClient.loop();

  MIDI.read();
  
  // update a value from sensor every 500ms
  if (t - lastUpdateTime >= 500) {
    lastUpdateTime = t;
    if (sending && readGyro()) {
      updateLcd();
      // Send to cloud
      const String msgType(":XYZ");
      const String topicStr = yourDevice + ":" + gyroTopic + msgType;
      const char* const topic = topicStr.c_str();
      const char* const msg = (String(gyroCurtX)+"-"+String(gyroCurtY)+"-"+String(gyroCurtZ)).c_str();
      printSomewhere(topic);
      printSomewhere(msg);
      mqttClient.publish(topic, msg);
    }
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
    const String topicStr = yourDevice + ":" + swTopic;
    const char* const topic = topicStr.c_str();
    const char* const msg = "Pressed";
    printSomewhere(topic);
    printSomewhere(msg);
    mqttClient.publish(topic, msg);
  }
  // Release Button A
  if (M5.BtnA.wasReleased()) {
    // Send
    const String topicStr = yourDevice + ":" + swTopic;
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
  int sp1 = yd.indexOf(':');
  int sp2 = yd.lastIndexOf(':');
  String dev = yd.substring(0,sp1);
  String type = yd.substring(sp1+1,sp2);
  String ev = yd.substring(sp2+1,yd.length());
  printSomewhere("**Message has come!**");

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
#if ( YOUR_DEVICE == M5STICK )
  Serial.begin(115200);
#endif
}

void printSomewhere(int num)
{
  char strx[128]={0};
  sprintf(strx,"%d",num);
#if ( YOUR_DEVICE == M5STACK )
  M5.Lcd.printf("%s",strx);
#elif ( YOUR_DEVICE == M5STICK )
  Serial.println(txrx);
#endif
}

void printSomewhere(const char* txt)
{
#if ( YOUR_DEVICE == M5STACK )
  M5.Lcd.printf(txt);
#elif ( YOUR_DEVICE == M5STICK )
  Serial.println(txt);
#endif
}

//-------------------------------
//  Gyro
//-------------------------------
void initGyro(void){
  gyroCurtX = gyroCurtY = gyroCurtZ = 0;
#if ( YOUR_DEVICE == M5STICK )
  M5.MPU6886.Init();
#endif
}

bool readGyro() {
#if ( YOUR_DEVICE == M5STICK )
  float gyroRawX, gyroRawY, gyroRawZ; // Gyro Raw Data
  M5.MPU6886.getGyroData(&gyroRawX, &gyroRawY, &gyroRawZ);
  gyroCurtX = gyroRawX * M5.MPU6886.gRes;
  gyroCurtY = gyroRawY * M5.MPU6886.gRes;
  gyroCurtZ = gyroRawZ * M5.MPU6886.gRes;
  return true;
#else
  return false;
#endif
}

void updateLcd() {
#if ( YOUR_DEVICE == M5STICK )
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf(sending ? "Sending" : "       ");
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf("Gyro\nX: %7.2f\nY: %7.2f\nZ: %7.2f\n          mg",
                gyroCurtX, gyroCurtY, gyroCurtZ);
#endif
}

//-------------------------------
//  Send MIDI message
//-------------------------------
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  if (sending==false) return;
  const String msgType(":note_on");
  const String topicStr = yourDevice + ":" + midiTopic + msgType;
  const char* const topic = topicStr.c_str();
  const char* const msg = (String(144+channel)+"-"+String(pitch)+"-"+String(velocity)).c_str();
  printSomewhere(topic);
  printSomewhere(msg);
  mqttClient.publish(topic, msg);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  if (sending==false) return;
  const String msgType(":note_off");
  const String topicStr = yourDevice + ":" + midiTopic + msgType;
  const char* const topic = topicStr.c_str();
  const char* const msg = (String(128+channel)+"-"+String(pitch)+"-"+String(velocity)).c_str();
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
  int sp1 = msg.indexOf('-');
  int sp2 = msg.lastIndexOf('-');
  String ch = msg.substring(0,sp1);
  String nt = msg.substring(sp1+1,sp2);
  String vl = msg.substring(sp2+1,msg.length());
  
  if (ev.equals(String('note_on'))){
    MIDI.sendNoteOn(atoi(nt.c_str()), atoi(vl.c_str()), atoi(ch.c_str()));
  }
  else if (ev.equals(String('note_off'))){
    MIDI.sendNoteOff(atoi(nt.c_str()), atoi(vl.c_str()), atoi(ch.c_str()));
  }
}
