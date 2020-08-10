//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5Stick 
//
//    Aug.8 , 2020 FabLab Hamamatsu
//
#include <M5Stack.h>
//#include <M5StickC.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <MIDI.h>

// WiFi settings
const char ssid[] = "xxxxxxxx";         //  #### Your Wifi ID
const char password[] = "xxxxxxxx";     //  #### Your Wifi PW
WiFiClient wifiClient;

// MQTT settings
const char* mqttBrokerAddr = "tailor.cloudmqtt.com";
const char* mqttUserName = "ovjirwrr";
const char* mqttPassword = "S0f6ZlAbEwqp";
const int mqttPort = 11333;
const char* mqttClientID = "M5Stack(hasebe)";
PubSubClient mqttClient(wifiClient);

const String midiTopic("HMMT_hasebe:MIDI");    //  #### Your Topic Name for MIDI
const String gyroTopic("HMMT_hasebe:Gyro");    //  #### Your Topic Name for Gyro
const String swTopic("HMMT_hasebe:M5SW");      //  #### Your Topic Name for M5stack Switch


// Global Variables
float gyroCurtX, gyroCurtY, gyroCurtZ;  // Gyro Value
boolean sending = false;
unsigned long lastUpdateTime = 0;

// MIDI
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  int wifiCheckCount = 0;
  
  M5.begin();
//  Serial.begin(115200);
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
  mqttClient.setServer(mqttBrokerAddr, mqttPort);
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
  
  // 500msごとにセンサー値を更新
  if (t - lastUpdateTime >= 500) {
    lastUpdateTime = t;
    readGyro();
    updateLcd();
    if (sending) {
      // Send to cloud
      const String msgType(":XYZ");
      const String topicStr = gyroTopic + msgType;
      const char* const topic = topicStr.c_str();
      const char* const msg = (String(gyroCurtX)+"-"+String(gyroCurtY)+"-"+String(gyroCurtZ)).c_str();
      printSomewhere(topic);
      printSomewhere(msg);
      mqttClient.publish(topic, msg);
    }
  }
  // ボタンBが押されたら送信の有効無効を切り替える
  if (M5.BtnB.wasPressed()) {
    sending = !sending;
  }
  // ボタンAが押されたら
  if (M5.BtnA.wasPressed()) {
    // Send
    const char* const topic = swTopic.c_str();
    const char* const msg = "Pressed";
    printSomewhere(topic);
    printSomewhere(msg);
    mqttClient.publish(topic, msg);
  }
  // ボタンAが離されたら
  if (M5.BtnA.wasReleased()) {
    // Send
    const char* const topic = swTopic.c_str();
    const char* const msg = "Released";
    printSomewhere(topic);
    printSomewhere(msg);
    mqttClient.publish(topic, msg);
  }
}

void printSomewhere(int num)
{
  char strx[128]={0};
  sprintf(strx,"%d",num);
  M5.Lcd.printf("%s",strx);
//  Serial.println(txt);
}

void printSomewhere(const char* txt)
{
  M5.Lcd.printf(txt);
//  Serial.println(txt);
}

//  Gyro
void initGyro(void){
//  M5.MPU6886.Init();
}

void readGyro() {
//  float gyroRawX, gyroRawY, gyroRawZ; // ジャイロ生データ
//  M5.MPU6886.getGyroData(&gyroRawX, &gyroRawY, &gyroRawZ);
//  gyroCurtX = gyroRawX * M5.MPU6886.gRes;
//  gyroCurtY = gyroRawY * M5.MPU6886.gRes;
//  gyroCurtZ = gyroRawZ * M5.MPU6886.gRes;
}

void updateLcd() {
//  M5.Lcd.setCursor(0, 0);
//  M5.Lcd.printf(sending ? "Sending" : "       ");
//  M5.Lcd.setCursor(0, 30);
//  M5.Lcd.printf("Gyro\nX: %7.2f\nY: %7.2f\nZ: %7.2f\n          mg",
//                gyroCurtX, gyroCurtY, gyroCurtZ);
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  const String msgType(":note_on");
  const String topicStr = midiTopic + msgType;
  const char* const topic = topicStr.c_str();
  const char* const msg = (String(144+channel)+"-"+String(pitch)+"-"+String(velocity)).c_str();
  printSomewhere(topic);
  printSomewhere(msg);
  mqttClient.publish(topic, msg);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  const String msgType(":note_off");
  const String topicStr = midiTopic + msgType;
  const char* const topic = topicStr.c_str();
  const char* const msg = (String(128+channel)+"-"+String(pitch)+"-"+String(velocity)).c_str();
  printSomewhere(topic);
  printSomewhere(msg);
  mqttClient.publish(topic, msg);
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
