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

const String yourDevice("HMMT_chair");        //  #### Your Device
const String swTopic("M5SW");

const String alphaTopic("alpha");
const String betaTopic("beta");
const String deltaTopic("delta");
const String thetaTopic("theta");


// Global Variables
unsigned long lastUpdateTime = 0;
unsigned long lastUpdateTime10ms = 0;


//-------------------------------
//  Arduino Functions
//-------------------------------
void setup() {

  int wifiCheckCount = 0;
  initPattern();

  M5.begin();
  initPrintSomewhere();
  WiFi.begin(ssid, password);
  Serial.begin(115200);

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
    const String topicStr = yourDevice+"/"+swTopic+"/B";
    const char* const topic = topicStr.c_str();
    const char* const msg = "Pressed";
    printSomewhere(topic);
    printSomewhere(msg);
    mqttClient.publish(topic, msg);
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
    startPattern(0);
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
//  int sp1 = yd.indexOf('/');
//  int sp2 = yd.lastIndexOf('/');
//  String dev = yd.substring(0,sp1);
//  String type = yd.substring(sp1+1,sp2);
//  String ev = yd.substring(sp2+1,yd.length());

  printSomewhere("**Message has come!**");

  // Add conditions
  if (yd.equals(alphaTopic)){
    workAsMassageChair(0, payload, length);
  }
  else if (yd.equals(betaTopic)){
    workAsMassageChair(1, payload, length);
  }
  else if (yd.equals(deltaTopic)){
    workAsMassageChair(2, payload, length);
  }
  else if (yd.equals(thetaTopic)){
    workAsMassageChair(3, payload, length);
  }
  else {}
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
#define MAX_PATTERN_NUM   3
struct PATTERN {
  int count;
  char* msgStr;
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
    { 0,   "moter;fssss" },
    { 50,  "moter;ffsss" },
    { 100, "moter;fffss" },  
    { 150, "moter;ffffs" }, 
    { 200, "moter;rffff" }, 
    { 250, "moter;rrfff" }, 
    { 300, "moter;rrrff" }, 
    { 350, "moter;rrrrf" }, 
    { 400, "moter;rrrrr" },
    { 450, "moter;srrrr" },
    { 500, "moter;ssrrr" },
    { 550, "moter;sssrr" },
    { 600, "moter;ssssr" },
    { 650, "moter;sssss" }
};
//-------------------------------
const PATTERN ptnB[] = 
{// *10msec, message
    { 0,    "moter;fssss" },
    { 100,  "moter;ffsss" },
    { 200,  "moter;fffss" },  
    { 300,  "moter;rfffs" }, 
    { 400,  "moter;rrfff" }, 
    { 500,  "moter;rrrff" }, 
    { 600,  "moter;rrrrf" }, 
    { 700,  "moter;srrrr" }, 
    { 800,  "moter;ssrrr" },
    { 900,  "moter;sssrr" },
    { 1000, "moter;ssssr" },
    { 1100, "moter;sssss" }
};
//-------------------------------
const PATTERN ptnC[] = 
{// *10msec, message
    { 0,    "moter;fssss" },
    { 100,  "moter;rssss" },
    { 200,  "moter;rfsss" },  
    { 300,  "moter;srsss" }, 
    { 400,  "moter;srfss" }, 
    { 500,  "moter;ssrss" }, 
    { 600,  "moter;ssrfs" }, 
    { 700,  "moter;sssrs" }, 
    { 800,  "moter;sssrf" },
    { 900,  "moter;ssssr" },
    { 1000, "moter;ssssr" },
    { 1100, "moter;sssss" }
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
  nextCount = ptnPtr[patternNum]->count;
}
void periodicForChair(void)
{
  if (patternOrder == -1){ return;}

  if (patternCount >= nextCount){
    const PATTERN* ptr = ptnPtr[crntPtnNum];
    char* mstr = ptr[patternOrder].msgStr;
    Serial.println(mstr);
    printSomewhere(patternOrder);
    nextCount = ptr[patternOrder].count;
    patternOrder++;
    if (patternOrder >= maxPtnNum[crntPtnNum]){
      stopPattern();
    }
  }
  
  patternCount++; 
}
void stopPattern(void)
{
  patternOrder = -1;
}
//-------------------------------
//  Send message to Massage Chair
//-------------------------------
void workAsMassageChair(int ev, byte* payload, unsigned int length)
{
  const String msg = (char*)payload;
  float value = atof(msg.c_str());

  //  各脳波の数値によってパターンを変える
  switch(ev){
    case 0: if ( value > 100.0 ){ startPattern(0);} break;  //  Alpha
    case 1: if ( value > 100.0 ){ startPattern(1);} break;  //  Beta
    case 2: if ( value > 100.0 ){ startPattern(2);} break;  //  Delta
    case 3: if ( value > 100.0 ){ startPattern(0);} break;  //  Theta
    default: break;
  }
}
