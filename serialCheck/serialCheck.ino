char moji[10];
int strindex;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial1.setTimeout(30);

  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);

  strindex = 0;
  for (int i=0; i<10; i++ ){ moji[i] = 0;}
}

void loop() {
  char *buffer;
  size_t length;
  
  // put your main code here, to run repeatedly:
  if (Serial1.available() > 0) {
    // read the incoming byte:
    String str = Serial1.readString();
    Serial.print('*');
    Serial.println(str);
  }
}
