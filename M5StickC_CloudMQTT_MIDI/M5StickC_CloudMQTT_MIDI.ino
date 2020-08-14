//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5StickC 
//
//    Aug.8 , 2020 FabLab Hamamatsu
//
#define M5STACK
//#define M5STICKC

#ifdef M5STACK
  #include <M5Stack.h>
#endif
#ifdef M5STICKC
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
boolean sending = true;
unsigned long lastUpdateTime = 0;

// MIDI
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
//MIDI_CREATE_DEFAULT_INSTANCE();

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
  MIDI.turnThruOff();
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
      sendGyroData();
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
#ifdef M5STICKC
  Serial.begin(115200);
#endif
}

void printSomewhere(int num)
{
  char strx[128]={0};
  sprintf(strx,"%d",num);
#ifdef M5STACK
  M5.Lcd.printf("%s",strx);
#endif
#ifdef M5STICKC
  Serial.println(txrx);
#endif
}

void printSomewhere(const char* txt)
{
#ifdef M5STACK
  M5.Lcd.printf(txt);
#endif
#ifdef M5STICKC
  Serial.println(txt);
#endif
}

//-------------------------------
//  Gyro
//-------------------------------
void initGyro(void){
  gyroCurtX = gyroCurtY = gyroCurtZ = 0;
#ifdef M5STICKC
  M5.MPU6886.Init();
#endif
}

bool readGyro() {
#ifdef M5STICKC
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
#ifdef M5STICKC
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf(sending ? "Sending" : "       ");
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf("Gyro\nX: %7.2f\nY: %7.2f\nZ: %7.2f\n          mg",
                gyroCurtX, gyroCurtY, gyroCurtZ);
#endif
}

void sendGyroData(void)
{ // Send to cloud
  const String msgType("/XYZ");
  const String topicStr = yourDevice+"/"+gyroTopic+msgType;
  const char* const topic = topicStr.c_str();
  const char* const msg = (String(gyroCurtX)+"-"+String(gyroCurtY)+"-"+String(gyroCurtZ)).c_str();
  printSomewhere(topic);
  printSomewhere(msg);
  mqttClient.publish(topic, msg);
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
    MIDI.sendNoteOn(atoi(nt.c_str()), atoi(vl.c_str()), atoi(ch.c_str()));
  }
  else if (ev.equals(ntof)){
    MIDI.sendNoteOff(atoi(nt.c_str()), atoi(vl.c_str()), atoi(ch.c_str()));
  }
}
