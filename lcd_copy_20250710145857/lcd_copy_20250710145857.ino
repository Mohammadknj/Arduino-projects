// // lm35 gnd to nanoGnd
// // lm35 Vs to nano5V
// // lm35 out to A0
// float temp,tempC;
// int pwmPin = 9;
// void setup() {  
//   // analogRefrence(DEFAULT); // fa'al sazi Vref 5v baraye deghat 0.5c
// //  analogRefrence(INTERNAL); // fa'al sazi Vref 1.1v baraye deghat 0.1c
//   Serial.begin(9600);
// }

// void loop() {
//   temp = analogRead(A0);
//   tempC = 4.8 * temp / 10; // baraye Vref 5v
// //  tempC = 1.0742 * temp / 10; // baraye Vref 1.1v
// Serial.print("temperature: ");
// Serial.println(tempC);
// float voltage = temp * (5.0 / 1023.0); // تبدیل به ولتاژ
// float temperature = voltage * 100;
// int pwmValue = map(temperature, 0, 100, 0, 255); // تبدیل دما به مقدار PWM (0 تا 255)
//   pwmValue = constrain(pwmValue, 0, 255); // محدود کردن مقدار PWM بین 0 تا 255
//   analogWrite(pwmPin, pwmValue);
// delay(1000);
// }



#include <LiquidCrystal.h>

void setup(){
  LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //LiquidCrystal lcd(rs, enable, D4, D5, D6, D7)
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("HelloWorld");
}
void loop(){}