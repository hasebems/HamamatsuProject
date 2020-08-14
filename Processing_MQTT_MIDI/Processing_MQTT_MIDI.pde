//
//  Hamamatsu Project
//    Data Transfer System by MQTT for M5StickC 
//
//    Aug.13 , 2020 FabLab Hamamatsu
//
import themidibus.*;   // MidiBus
import mqtt.*;         // Processing MQTT

String mqttServer = "tailor.cloudmqtt.com";
String mqttUserName = "ovjirwrr";
String mqttPassword = "S0f6ZlAbEwqp";
String mqttPort = "11333";
String mqttClientID = "Processing(hasebe)";

String topicNtOn = "HMMT_hasebe/MIDI/note_on";  //  ####
String topicNtOf = "HMMT_hasebe/MIDI/note_off"; //  ####

String myMIDIIn = "midiglue";                  //  ####
String myMIDIOut = "WebMIDI";                  //  ####

MQTTClient client;
MidiBus myBus;

void setup() {
  MidiBus.list();
  myBus = new MidiBus(this, myMIDIIn, myMIDIOut);

  client = new MQTTClient(this);
  client.connect("mqtt://" + mqttUserName + ":" + mqttPassword + "@" + mqttServer + ":" + mqttPort, mqttClientID);
}

void draw() {
}

void keyPressed() {
  client.publish("/keyPressed", str(char(key)));

  int channel = 0;
  int pitch = 78;
  int velocity = 127;
  myBus.sendNoteOn(channel, pitch, velocity); // Send a Midi noteOn
}
void keyReleased() {
  client.publish("/keyReleased", str(char(key)));

  int channel = 0;
  int pitch = 78;
  int velocity = 127;
  myBus.sendNoteOff(channel, pitch, velocity); // Send a Midi nodeOff
}

void noteOn(int channel, int pitch, int velocity) {
  println();
  println("Note On:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
  String dataStr = str(channel)+str(pitch)+str(velocity);
  println(dataStr);
  client.publish("/NoteOn:", dataStr);
}

void noteOff(int channel, int pitch, int velocity) {
  println();
  println("Note Off:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
  String dataStr = str(channel)+str(pitch)+str(velocity);
  println(dataStr);
  client.publish("/NoteOff:", dataStr);
}

void controllerChange(int channel, int number, int value) {
  println();
  println("Controller Change:");
  println("--------");
  println("Channel:"+channel);
  println("Number:"+number);
  println("Value:"+value);
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
  if (topic.equals(topicNtOn)) {
    String[] params = s.split("-");
    if (params.length == 3) {
      int channel = int(params[0]);
      int pitch = int(params[1]);
      int velocity = int(params[2]);
      myBus.sendNoteOn(channel, pitch, velocity); // Send a Midi noteOn
    }
  } else if (topic.equals(topicNtOf)) {
    String[] params = s.split("-");
    if (params.length == 3) {
      int channel = int(params[0]);
      int pitch = int(params[1]);
      int velocity = int(params[2]);
      myBus.sendNoteOff(channel, pitch, velocity); // Send a Midi noteOff
    }
  }
}

void connectionLost() {
  println("connection lost");
}
