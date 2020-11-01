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
const char* mqttClientID = "HMMT_MassageChair1101";  //  #### Your ClientID
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
    { 0,   "motor:fssss" },
    { 50,  "motor:ffsss" },
    { 100, "motor:fffss" },  
    { 150, "motor:ffffs" }, 
    { 200, "motor:rffff" }, 
    { 250, "motor:rrfff" }, 
    { 300, "motor:rrrff" }, 
    { 350, "motor:rrrrf" }, 
    { 400, "motor:rrrrr" },
    { 450, "motor:srrrr" },
    { 500, "motor:ssrrr" },
    { 550, "motor:sssrr" },
    { 600, "motor:ssssr" },
    { 650, "motor:sssss" },
    { 700, "motor:fssss" },
    { 750,  "motor:ffsss" },
    { 800, "motor:fffss" },  
    { 850, "motor:ffffs" }, 
    { 900, "motor:rffff" }, 
    { 950, "motor:rrfff" }, 
    { 1000, "motor:rrrff" }, 
    { 1050, "motor:rrrrf" }, 
    { 1100, "motor:rrrrr" },
    { 1150, "motor:srrrr" },
    { 1200, "motor:ssrrr" },
    { 1250, "motor:sssrr" },
    { 1300, "motor:ssssr" },
    { 1350, "motor:sssss" },
    { 1400, "motor:fssss" },
    { 1450, "motor:ffsss" },
    { 1500, "motor:fffss" },  
    { 1550, "motor:ffffs" }, 
    { 1600, "motor:rffff" }, 
    { 1650, "motor:rrfff" }, 
    { 1700, "motor:rrrff" }, 
    { 1750, "motor:rrrrf" }, 
    { 1800, "motor:rrrrr" },
    { 1850, "motor:srrrr" },
    { 1900, "motor:ssrrr" },
    { 1950, "motor:sssrr" },
    { 2000, "motor:ssssr" },
    { 2050, "motor:sssss" },
    { 2100, "motor:fssss" },
    { 2150, "motor:ffsss" },
    { 2200, "motor:fffss" },  
    { 2250, "motor:ffffs" }, 
    { 2300, "motor:rffff" }, 
    { 2350, "motor:rrfff" }, 
    { 2400, "motor:rrrff" }, 
    { 2450, "motor:rrrrf" }, 
    { 2500, "motor:rrrrr" },
    { 2550, "motor:srrrr" },
    { 2600, "motor:ssrrr" },
    { 2650, "motor:sssrr" },
    { 2700, "motor:ssssr" },
    { 2750, "motor:sssss" }
};
//-------------------------------
const PATTERN ptnB[] = 
{// *10msec, message    <Focus>
    { 0,    "motor:fssss" },  
    { 300,  "motor:rssss" }, 
    { 500,  "motor:rfsss" }, 
    { 700,  "motor:sfsss" }, 
    { 800,  "motor:srsss" },
    { 1000, "motor:srfss" },
    { 1200, "motor:ssfss" },
    { 1300, "motor:ssrss" },
    { 1500, "motor:ssrfs" },
    { 1700, "motor:sssfs" },
    { 1800, "motor:sssrs" },
    { 2000, "motor:sssrf" },
    { 2200, "motor:ssssf" },
    { 2300, "motor:ssssr" },
    { 2700, "motor:sssss" }
};
//-------------------------------
const PATTERN ptnC[] = 
{// *10msec, message    <Meditation>
    { 0,    "motor:fsfsf" },
    { 50,   "motor:rfrfr" },
    { 100,  "motor:rrrrr" },
    { 150,  "motor:srsrs" },
    { 200,  "motor:sssss" },
    { 250,  "motor:fsfsf" },
    { 300,  "motor:rfrfr" },
    { 350,  "motor:rrrrr" },
    { 400,  "motor:srsrs" },    
    { 450,  "motor:sssss" },
    { 500,  "motor:fsfsf" },
    { 550,  "motor:rfrfr" },
    { 600,  "motor:rrrrr" },
    { 650,  "motor:srsrs" },    
    { 700,  "motor:sssss" },
    { 750,  "motor:fsfsf" },
    { 800,  "motor:rfrfr" },
    { 850,  "motor:rrrrr" },
    { 900,  "motor:srsrs" },    
    { 950,  "motor:sssss" },
    { 1000,  "motor:fsfsf" },
    { 1050,  "motor:rfrfr" },
    { 1100,  "motor:rrrrr" },
    { 1150,  "motor:srsrs" },    
    { 1200,  "motor:sssss" },
    { 1250,  "motor:fsfsf" },
    { 1300,  "motor:rfrfr" },
    { 1350,  "motor:rrrrr" },
    { 1400,  "motor:srsrs" },    
    { 1450,  "motor:sssss" },
    { 1500,  "motor:fsfsf" },
    { 1550,  "motor:rfrfr" },
    { 1600,  "motor:rrrrr" },
    { 1650,  "motor:srsrs" },    
    { 1700,  "motor:sssss" },
    { 1750,  "motor:fsfsf" },
    { 1800,  "motor:rfrfr" },
    { 1850,  "motor:rrrrr" },
    { 1900,  "motor:srsrs" },    
    { 1950,  "motor:sssss" },
    { 2000,  "motor:fsfsf" },
    { 2050,  "motor:rfrfr" },
    { 2100,  "motor:rrrrr" },
    { 2150,  "motor:srsrs" },    
    { 2200,  "motor:sssss" },
    { 2250,  "motor:fsfsf" },
    { 2300,  "motor:rfrfr" },
    { 2350,  "motor:rrrrr" },
    { 2400,  "motor:srsrs" },    
    { 2450,  "motor:sssss" },
    { 2500,  "motor:fsfsf" },
    { 2550,  "motor:rfrfr" },
    { 2600,  "motor:rrrrr" },
    { 2650,  "motor:srsrs" },    
    { 2700,  "motor:sssss" },
};\
//-------------------------------
const PATTERN ptnD[] = 
{// *10msec, message    <Deep Sleep>
    { 0,    "motor:fffff" },
    { 100,  "motor:rrrrr" },
    { 300,  "motor:fffff" },
    { 400,  "motor:rrrrr" },
    { 600,  "motor:fffff" },
    { 700,  "motor:rrrrr" },
    { 900,  "motor:fffff" },
    { 1000,  "motor:rrrrr" },
    { 1200,  "motor:fffff" },
    { 1300,  "motor:rrrrr" },
    { 1500,  "motor:fffff" },
    { 1600,  "motor:rrrrr" },
    { 1800,  "motor:fffff" },
    { 1900,  "motor:rrrrr" },
    { 2100,  "motor:fffff" },
    { 2200,  "motor:rrrrr" },
    { 2400,  "motor:fffff" },
    { 2500,  "motor:rrrrr" },
    { 2700,  "motor:sssss" },
};
//-------------------------------
const PATTERN ptnE[] = 
{// *10msec, message    <Sleep>
    { 0,    "motor:fffff" },
    { 300,  "motor:rrrrr" },   
    { 700,  "motor:fffff" },
    { 1000,  "motor:rrrrr" },
    { 1400,  "motor:fffff" },
    { 1700,  "motor:rrrrr" },
    { 2100,  "motor:fffff" },
    { 2400,  "motor:rrrrr" },
    { 2800,  "motor:sssss" }
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
