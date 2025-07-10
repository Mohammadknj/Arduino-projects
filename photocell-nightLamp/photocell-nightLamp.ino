int sensor;
void setup() {
  Serial.begin(9600);
  pinMode(13,OUTPUT);
}

void loop() {
  sensor = analogRead(A0);
  Serial.print("Sensor value= ");
  Serial.println(sensor);
  if(sensor < 400)
    digitalWrite(13,HIGH);
  else
    digitalWrite(13,LOW);    
  delay(300);
}
