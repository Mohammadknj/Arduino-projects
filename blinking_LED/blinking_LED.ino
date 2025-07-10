int white = 13;
int yellow = 12;
int blue = 11;
void setup() {
  pinMode(white, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(blue, OUTPUT);
}

void loop() {
  for(int i=13; i>=11; i--){
    digitalWrite(i, HIGH);
    delay(1000);
    digitalWrite(i, LOW);
    delay(1000);
  }
}
