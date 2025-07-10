// // lm35 gnd to nanoGnd
// // lm35 Vs to nano5V
// // lm35 out to A0

// // LCD
// #include <LiquidCrystal.h> // کتابخانه استاندارد LCD
// // تعریف پین های LCD: (RS, E, DB4, DB5, DB6, DB7)
// // این پین ها باید با سیم کشی شما مطابقت داشته باشند
// const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// unsigned long fanOnStartTime = 0; // زمان شروع روشن بودن فن
// unsigned long totalFanOnTimeSeconds = 0; // کل زمان روشن بودن فن بر حسب ثانیه
// bool isFanOn = false; // وضعیت فعلی فن (روشن/خاموش)
// // Continue

// float temp, tempC;
// const int fanPin = 9;
// const int lm35Pin = A0;
// int pwmValue = 0;
// const int minTemp = 20;  // دمای حداقل برای شروع چرخش فن
// const int maxTemp = 55;


// void setup() {
//   pinMode(fanPin, OUTPUT);   // تنظیم پین فن به عنوان خروجی
//   analogReference(DEFAULT);  // fa'al sazi Vref 5v baraye deghat 0.5c
//   // analogReference(INTERNAL); // fa'al sazi Vref 1.1v baraye deghat 0.1c
//   analogWrite(fanPin, 0);
//   Serial.begin(9600);
//   // LCD
//   // مقداردهی اولیه LCD
//   lcd.begin(16, 2); // تنظیم LCD با 16 ستون و 2 سطر
//   lcd.print("Fan Control Sys");
//   lcd.setCursor(0, 1);
//   lcd.print("Starting...");
//   delay(2000);
//   lcd.clear();
// }

// void loop() {
//   temp = analogRead(lm35Pin);
//   tempC = 0.488 * temp;  // baraye Vref 5v
//   //  tempC = 1.0742 * temp / 10; // baraye Vref 1.1v
//   Serial.print("temperature: ");
//   Serial.print(tempC);
//   Serial.print(" pwm: ");
//   Serial.println(pwmValue);

//   bool currentFanState = false;
//   // تنظیم PWM بر اساس دما
//   if (tempC < minTemp) {
//     pwmValue = 0;  // خاموش کردن فن
//     currentFanState = false;
//   } else if (tempC >= minTemp && tempC <= maxTemp) {
//     // محاسبه مقدار PWM خطی بین 0 تا 255
//     pwmValue = map(tempC, minTemp, maxTemp, 0, 255);
//     pwmValue = constrain(pwmValue, 0, 255);  // محدود کردن مقدار PWM بین 0 تا 255
//     currentFanState = true;
//   } else {
//     pwmValue = 255;  // روشن کردن فن با حداکثر سرعت
//     currentFanState = true;
//   }
//   analogWrite(fanPin, pwmValue);  // تنظیم سرعت فن
//   delay(1000);
// }

#include <LiquidCrystal.h> // کتابخانه استاندارد LCD (برای LCD بدون I2C)

// تعریف پین های LCD: (RS, E, DB4, DB5, DB6, DB7)
const int rsPin = 12; 
const int enPin = 11;
const int d4Pin = 5;
const int d5Pin = 4;
const int d6Pin = 3;
const int d7Pin = 2;

LiquidCrystal lcd(rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin);

// تعریف پین ها و متغیرهای دیگر پروژه
const int lm35Pin = A0;      
const int fanPWMPin = 9;     

const int minTemp = 20;     
const int maxTemp = 50;     

// متغیرها برای ساعت شمار فن
unsigned long fanOnTimeSeconds = 0; // زمان روشن بودن فن بر حسب ثانیه
bool wasFanOnLastLoop = false;      // وضعیت فن در حلقه قبلی (برای تشخیص تغییر وضعیت)

// متغیر برای اندازه گیری زمان سپری شده در هر حلقه
unsigned long previousMillis = 0;

void setup() {
  pinMode(fanPWMPin, OUTPUT);
  analogWrite(fanPWMPin, 0);
  Serial.begin(9600);

  lcd.begin(16, 2); 
  lcd.print("Device is ON");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
  lcd.clear();

  // مقدار اولیه زمان سپری شده در حلقه
  previousMillis = millis();
}

void loop() {
  // 1. خواندن دما و کنترل فن
  float temperatureC = (analogRead(lm35Pin) * (5.0 / 1024.0)) * 100.0;
  int fanPWMValue = 0;
  bool isFanCurrentlyOn = false;

  if (temperatureC < minTemp) {
    fanPWMValue = 0;
    isFanCurrentlyOn = false;
  } else if (temperatureC >= minTemp && temperatureC <= maxTemp) {
    fanPWMValue = map(temperatureC, minTemp, maxTemp, 0, 255);
    isFanCurrentlyOn = true;
  } else { // temperatureC > maxTemp
    fanPWMValue = 255;
    isFanCurrentlyOn = true;
  }
  
  analogWrite(fanPWMPin, fanPWMValue);

  // 2. محاسبه زمان سپری شده در این حلقه
  unsigned long currentMillis = millis();
  unsigned long loopDuration = currentMillis - previousMillis;
  previousMillis = currentMillis;

  // 3. منطق ساعت شمار فن با شرط صفر شدن
  if (temperatureC < minTemp) {
    // دما کمتر از 20 درجه: ساعت شمار فن صفر شود
    fanOnTimeSeconds = 0;
  } else {
    // دما 20 درجه یا بیشتر: ساعت شمار فن افزایش یابد
    fanOnTimeSeconds += loopDuration / 1000;
  }
  
  // 4. محاسبه زمان کلی دستگاه از روی millis()
  unsigned long totalDeviceOnTimeSeconds = currentMillis / 1000;

  // 5. نمایش اطلاعات روی LCD
  // نمایش زمان روشن بودن دستگاه (ساعت اول)
  lcd.setCursor(0, 0); 
  lcd.print("Dev ON: ");
  lcd.print(totalDeviceOnTimeSeconds / 3600);         // ساعت
  lcd.print("h ");
  lcd.print((totalDeviceOnTimeSeconds % 3600) / 60);  // دقیقه
  lcd.print("m ");
  lcd.print(totalDeviceOnTimeSeconds % 60);           // ثانیه
  lcd.print("s ");

  // نمایش زمان روشن بودن فن (ساعت دوم)
  lcd.setCursor(0, 1);
  lcd.print("Fan ON: ");
  lcd.print(fanOnTimeSeconds / 3600);         // ساعت
  lcd.print("h ");
  lcd.print((fanOnTimeSeconds % 3600) / 60);  // دقیقه
  lcd.print("m ");
  lcd.print(fanOnTimeSeconds % 60);           // ثانیه
  lcd.print("s ");

  // 6. نمایش دما و PWM روی سریال مانیتور (برای اشکال زدایی)
  Serial.print("Temp: ");
  Serial.print(temperatureC);
  Serial.print("C | Fan PWM: ");
  Serial.print(fanPWMValue);
  Serial.print(" | Fan On Time: ");
  Serial.print(fanOnTimeSeconds);
  Serial.println("s");

  // تأخیر کوتاه برای جلوگیری از نوسانات شدید نمایش
  delay(100); 
}