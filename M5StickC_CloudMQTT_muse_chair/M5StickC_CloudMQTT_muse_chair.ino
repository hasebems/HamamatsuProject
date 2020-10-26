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
const int mqttPort = ;
const char* mqttClientID = "";  //  #### Your ClientID
PubSubClient mqttClient(wifiClient);

const String yourDevice("HMMT_chair");        //  #### Your Device
const String swTopic("M5SW");

const String alphaTopic("/alpha");
const String betaTopic("/beta");
const String gammaTopic("/gamma");
const String deltaTopic("/delta");
const String thetaTopic("/theta");
const String patternTopic("/massage_pattern");


// Global Variables
unsigned long lastUpdateTime = 0;
unsigned long lastUpdateTime10ms = 0;
int ptnToggle = 0;

#define MAX_PATTERN_NUM   5

//-------------------------------
//  Arduino Functions
//-------------------------------
void setup() {

  int wifiCheckCount = 0;
  initPattern();

  M5.begin();
  initPrintSomewhere();
  WiFi.begin(ssid, password);
  Serial2.begin(9600, SERIAL_8N1, 32, 33);

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

    startPattern(ptnToggle);
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
  }
}

//-------------------------------
//  MQTT
//-------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  String yd = topic;

  payload[length] = '\0';

  // Add conditions
  if (yd.equals(alphaTopic)){
    printSomewhere("a:");
    workAsMassageChair(0, payload, length);
  }
  else if (yd.equals(betaTopic)){
    printSomewhere("b:");
    workAsMassageChair(1, payload, length);
  }
  else if (yd.equals(gammaTopic)){
    printSomewhere("g:");
    workAsMassageChair(2, payload, length);
  }
  else if (yd.equals(deltaTopic)){
    printSomewhere("d:");
    workAsMassageChair(3, payload, length);
  }
  else if (yd.equals(thetaTopic)){
    printSomewhere("c:");
    workAsMassageChair(4, payload, length);
  }
  else if (yd.equals(patternTopic)){
    const String msg = (char*)payload;
    int value = atoi(msg.c_str());
    printSomewhere("ptn:");
    startPattern(value);
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
{// *10msec, message    <Relax>
    { 0,   "moter:fssss" },
    { 50,  "moter:ffsss" },
    { 100, "moter:fffss" },  
    { 150, "moter:ffffs" }, 
    { 200, "moter:rffff" }, 
    { 250, "moter:rrfff" }, 
    { 300, "moter:rrrff" }, 
    { 350, "moter:rrrrf" }, 
    { 400, "moter:rrrrr" },
    { 450, "moter:srrrr" },
    { 500, "moter:ssrrr" },
    { 550, "moter:sssrr" },
    { 600, "moter:ssssr" },
    { 650, "moter:sssss" }
};
//-------------------------------
const PATTERN ptnB[] = 
{// *10msec, message    <Focus>
    { 0,    "moter:fssss" },  
    { 300,  "moter:rssss" }, 
    { 500,  "moter:rfsss" }, 
    { 700,  "moter:sfsss" }, 
    { 800,  "moter:srsss" },
    { 1000, "moter:srfss" },
    { 1200, "moter:ssfss" },
    { 1300, "moter:ssrss" },
    { 1500, "moter:ssrfs" },
    { 1700, "moter:sssfs" },
    { 1800, "moter:sssrs" },
    { 2000, "moter:sssrf" },
    { 2200, "moter:ssssf" },
    { 2300, "moter:ssssr" },
    { 2700, "moter:sssss" }
};
//-------------------------------
const PATTERN ptnC[] = 
{// *10msec, message    <Meditation>
    { 0,    "moter:fsfsf" },
    { 50,   "moter:rfrfr" },
    { 100,  "moter:rrrrr" },
    { 150,  "moter:srsrs" },
    { 200,  "moter:fsfsf" },
    { 250,  "moter:rfrsr" },
    { 300,  "moter:rrrsr" },
    { 350,  "moter:sssss" },
    { 400,  "moter:fsfsf" },
    { 450,  "moter:rfrsr" },
    { 500,  "moter:rrrsr" },
    { 550,  "moter:sssss" },
};
//-------------------------------
const PATTERN ptnD[] = 
{// *10msec, message    <Deep Sleep>
    { 0,    "moter:fffff" },
    { 100,  "moter:rrrrr" },
    { 300,  "moter:fffff" },
    { 400,  "moter:rrrrr" },
    { 600,  "moter:fffff" },
    { 700,  "moter:rrrrr" },
    { 900,  "moter:sssss" }
};
//-------------------------------
const PATTERN ptnE[] = 
{// *10msec, message    <Sleep>
    { 0,    "moter:fffff" },
    { 300,  "moter:rrrrr" },   
    { 700,  "moter:sssss" }
};
//-------------------------------
const PATTERN* ptnPtr[MAX_PATTERN_NUM] =
{ // パターン名を追加
  ptnA, ptnB, ptnC, ptnD, ptnE
};
const int maxPtnNum[MAX_PATTERN_NUM] = 
{ // パターンの要素数を追加
  sizeof(ptnA)/sizeof(ptnA[0]),
  sizeof(ptnB)/sizeof(ptnB[0]),
  sizeof(ptnC)/sizeof(ptnC[0]),
  sizeof(ptnD)/sizeof(ptnD[0]),
  sizeof(ptnE)/sizeof(ptnE[0])
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
    char* mstr = ptr[patternOrder].msgStr;
    Serial2.println(mstr);
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
    case 2: if ( value > 100.0 ){ startPattern(2);} break;  //  Gamma
    case 3: if ( value > 100.0 ){ startPattern(3);} break;  //  Delta
    case 4: if ( value > 100.0 ){ startPattern(4);} break;  //  Theta
    default: break;
  }
}
