//
//  Hamamatsu Project
//    Data Transfer System by MQTT for Processing
//
//    Nov.3 , 2020 FabLab Hamamatsu
//
import mqtt.*;         // Processing MQTT

String mqttServer = "";
String mqttUserName = "";
String mqttPassword = "";
String mqttPort = "";
String mqttClientID = "";

MQTTClient client;

void setup() {
  client = new MQTTClient(this);
  client.connect("mqtt://" + mqttUserName + ":" + mqttPassword + "@" + mqttServer + ":" + mqttPort, mqttClientID);
}

void draw() {
}

void keyPressed() {
  client.publish("/keyPressed", str(char(key)));
}
void keyReleased() {
  client.publish("/keyReleased", str(char(key)));
}

void delay(int time) {
  int current = millis();
  while (millis () < current+time) Thread.yield();
}

void clientConnected() {
  println("client connected");
  client.subscribe("#");
}

void messageReceived(String topic, byte[] payload) {
  String s = new String(payload);
  println("new message: ***" + topic + "***");
  println(s);
}

void connectionLost() {
  println("connection lost");
}
