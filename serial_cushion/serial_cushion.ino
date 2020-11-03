
//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5StickC
//
//    Oct.18 , 2020 FabLab Hamamatsu
//

#include <M5StickC.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string>




// WiFi settings
const char ssid[] = "";         //  #### Your Wifi ID
const char password[] = "";     //  #### Your Wifi PW

WiFiClient wifiClient;

// MQTT settings
const char* mqttBrokerAddr = "";
const char* mqttUserName = "";
const char* mqttPassword = "";
const int mqttPort = 0;
const char* mqttClientID = "HMMTCUSHION";   // unique id that anything you like
PubSubClient mqttClient(wifiClient);
const String yourDevice("HMMT_cushion");        //  #### Your Device

const String swTopic("M5SW");


const String cushionTopic("cushion");


const int RELAY_PIN(33);

// Global Variables
unsigned long lastUpdateTime = 0;
unsigned long lastUpdateTime10ms = 0;
int ptnToggle = 0;

#define MAX_PATTERN_NUM   3

//-------------------------------
//  Arduino Functions
//-------------------------------
void setup() {

  int wifiCheckCount = 0;
  int wifiExitCount = 0;
  initPattern();

  M5.begin();
  initPrintSomewhere();
  WiFi.begin(ssid, password);
  Serial1.begin(9600, SERIAL_8N1, 0, 26);         // RX=32   TX=33
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
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
}

void loop() {
  unsigned long t = millis();
  M5.update();
  reConnect();
  mqttClient.loop();

  // update a value from sensor every 10ms
  if (t - lastUpdateTime10ms >= 10) {
    lastUpdateTime10ms = t;
    //  something every 10msec
    periodicForChair();
  }

  // update a value from sensor every 1000ms
  if (t - lastUpdateTime >= 1000) {
    lastUpdateTime = t;
    //  something every 1000msec

  }

  // Switch Enable/Disable MQTT transfer by pushing button B
  if (M5.BtnB.wasPressed()) {
//    const String topicStr = yourDevice+"/"+swTopic+"/B";
//    const char* const topic = topicStr.c_str();
//    const char* const msg = "Pressed";
//    printSomewhere(topic);
//    printSomewhere(msg);
//    mqttClient.publish(topic, msg);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
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
    startPattern(ptnToggle);
    //digitalWrite(RELAY_PIN, HIGH);
    ptnToggle++;
    if ( ptnToggle >= MAX_PATTERN_NUM ){ ptnToggle = 0;}
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
    //digitalWrite(RELAY_PIN, LOW);    
  }
}

//-------------------------------
//  MQTT
//-------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  String yd = topic;
//  int sp1 = yd.indexOf('/');
//  int sp2 = yd.lastIndexOf('/');
//  String dev = yd.substring(0,sp1);
//  String type = yd.substring(sp1+1,sp2);
//  String ev = yd.substring(sp2+1,yd.length());
  payload[length] = '\0';
  // Add conditions
  if (yd.equals(cushionTopic)){
    printSomewhere("cushion:");
    workAsMassageChair(0, payload, length);
  }
  else {}

  const String msg = (char*)payload;
  float value = atof(msg.c_str());
  printSomewhere((int)(value*100));
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
//  Massage Chair Wave
//-------------------------------
struct PATTERN {
  int count;
  int msgStr;
};
//-------------------------------
int patternCount;
int patternOrder;
int nextCount;
int crntPtnNum;
//-------------------------------
//  ここに新しい動作パターンを追加
//-------------------------------
const PATTERN ptnA[] = 
{// *10msec, message
    { 0,   0 },
    { 50,  1 },
    { 100, 0 },  
    { 150, 1 }, 
    { 200, 0 }, 
    { 250, 1 }, 
    { 300, 0 }, 
    { 350, 1 }, 
    { 400, 0 },
    { 450, 1 },
    { 500, 0 },
    { 550, 1 },
    { 600, 0 },
    { 650, 0 }
};
//-------------------------------
const PATTERN ptnB[] = 
{// *10msec, message
    { 0,   0 },
    { 50,  0 },
    { 100, 0 },  
    { 150, 1 }, 
    { 200, 0 }, 
    { 250, 0 }, 
    { 300, 0 }, 
    { 350, 1 }, 
    { 400, 0 },
    { 450, 0 },
    { 500, 0 },
    { 550, 1 },
    { 600, 0 },
    { 650, 0 }
};
//-------------------------------
const PATTERN ptnC[] = 
{// *10msec, message
    { 0,   0 },
    { 50,  1 },
    { 100, 1 },  
    { 150, 1 }, 
    { 200, 0 }, 
    { 250, 0 }, 
    { 300, 0 }, 
    { 350, 0 }, 
    { 400, 1 },
    { 450, 1 },
    { 500, 1 },
    { 550, 1 },
    { 600, 1 },
    { 650, 0 }
};
//-------------------------------
const PATTERN* ptnPtr[MAX_PATTERN_NUM] =
{ // パターン名を追加
  ptnA, ptnB, ptnC
};
const int maxPtnNum[MAX_PATTERN_NUM] = 
{ // パターンの要素数を追加
  sizeof(ptnA)/sizeof(ptnA[0]),
  sizeof(ptnB)/sizeof(ptnB[0]),
  sizeof(ptnC)/sizeof(ptnC[0])
};
//-------------------------------
void initPattern(void)
{
  patternCount = 0;
  patternOrder = -1;
  nextCount = 0;
  crntPtnNum = 0;
}
void startPattern(int patternNum)
{
  if ( patternOrder != -1 ){ return;} // 動作中は受け付けない
  patternCount = 0;
  patternOrder = 0;
  crntPtnNum = patternNum;
  const PATTERN* ptr = ptnPtr[crntPtnNum];
  nextCount = ptr[patternOrder].count;
}
void periodicForChair(void)
{
  if (patternOrder == -1){ return;}

  if (patternCount >= nextCount){
    const PATTERN* ptr = ptnPtr[crntPtnNum];
    int mstr = ptr[patternOrder].msgStr;
    Serial1.println(mstr);
    digitalWrite(RELAY_PIN, mstr);
    printSomewhere(patternOrder);
    patternOrder++;
    if (patternOrder >= maxPtnNum[crntPtnNum]){
      stopPattern();
    }
    else {
      nextCount = ptr[patternOrder].count;      
    }
  }
  
  patternCount++; 
}
void stopPattern(void)
{
  patternOrder = -1;
  digitalWrite(RELAY_PIN, LOW);
}
//-------------------------------
//  Send message to Massage Chair
//-------------------------------
void workAsMassageChair(int ev, byte* payload, unsigned int length)
{
  const String msg = (char*)payload;
  int value = atof(msg.c_str());

  switch(value){
    case 0:startPattern(0);break;
    case 1:startPattern(1);break;
    case 2:startPattern(2);break;
    default: break;    
  }

  //  各脳波の数値によってパターンを変える
//  switch(ev){
//    case 0: if ( value > 100.0 ){ startPattern(0);} break;  //  0:Alpha
//    case 1: if ( value > 100.0 ){ startPattern(1);} break;  //  1:Beta
//    case 2: if ( value > 100.0 ){ startPattern(2);} break;  //  2:Delta
//    case 3: if ( value > 100.0 ){ startPattern(0);} break;  //  3:Theta
//
//  }
}
