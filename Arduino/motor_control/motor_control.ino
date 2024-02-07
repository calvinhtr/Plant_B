int pump = 10;

void setup() {
  // put your setup code here, to run once:
  pinMode(10,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pump, HIGH);
  delay(2000);
  digitalWrite(pump,LOW);
  delay(2000);
}
