#include <toneAC.h>

#include "NewPing.h"

#define TRIGGER_PIN 4  // پین تریگر سنسور HC-SR04 به آردوینو پین 9
#define ECHO_PIN 3    // پین اکو سنسور HC-SR04 به آردوینو پین 10
#define MAX_DISTANCE 400 // حداکثر فاصله ای که می خواهیم سنسور پینگ کند (بر حسب سانتی متر).

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // تنظیم NewPing با پین ها و حداکثر فاصله
int buzzerPin = 6; // پین بازر (به عنوان مثال پین 4)
const int frequency = 1000; // فرکانس مناسب برای بازر

void setup() {
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  delay(50); // تأخیر 50 میلی‌ثانیه بین پینگ‌ها برای جلوگیری از تداخل بازتاب‌ها

  // NewPing::ping_cm() فاصله را بر حسب سانتی‌متر برمی‌گرداند.
  // اگر هیچ بازتابی در MAX_DISTANCE دریافت نشود، 0 برمی‌گرداند.
  unsigned int distance = sonar.ping_cm();

  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");

  // --- منطق بازر با NewPing و مدیریت خطای بهبود یافته ---
  if (distance == 0) { // اگر NewPing 0 برگرداند، یعنی بازتابی دریافت نشد (شیء خیلی دور/خیلی نزدیک/نبودن شیء)
    digitalWrite(buzzerPin, LOW);
    Serial.println("--- شیء تشخیص داده نشد یا خارج از محدوده است ---");
  } else if (distance > 200) {
    digitalWrite(buzzerPin, LOW);
  } else if (distance > 100) {
    digitalWrite(buzzerPin, HIGH);
    delay(1250); // 1 ثانیه بوق
    digitalWrite(buzzerPin, LOW);
    delay(1250); // 1 ثانیه سکوت
  } else if (distance > 50) {
    digitalWrite(buzzerPin, HIGH);
    delay(500); // 0.5 ثانیه بوق
    digitalWrite(buzzerPin, LOW);
    delay(500); // 0.5 ثانیه سکوت
  } else if (distance > 25) {
    digitalWrite(buzzerPin, HIGH);
    delay(250); // 0.25 ثانیه بوق
    digitalWrite(buzzerPin, LOW);
    delay(250); // 0.25 ثانیه سکوت
  } else if (distance > 10) {
    digitalWrite(buzzerPin, HIGH);
    delay(125); // 0.125 ثانیه بوق
    digitalWrite(buzzerPin, LOW);
    delay(125); // 0.125 ثانیه سکوت
  } else { // این قسمت فواصل 1 تا 10 سانتی‌متر را شامل می‌شود، از جمله محدوده حداقل دشوار
    // هر چیزی کمتر یا مساوی 10 سانتی‌متر را به عنوان "بسیار نزدیک" در نظر می‌گیریم
    digitalWrite(buzzerPin, HIGH);
    // نیازی به noTone() یا delay() اینجا نیست اگر می‌خواهید بوق تا زمانی که شیء نزدیک است پیوسته باشد.
  }
}