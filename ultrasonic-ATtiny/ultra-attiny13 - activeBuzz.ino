// شماره پین‌ها را بر اساس نحوه مپ شدن در هسته ATtiny13A شما تنظیم کنید.
// در بسیاری از هسته‌ها:
// PB0 = Pin 0 (یا 5)
// PB1 = Pin 1 (یا 6)
// PB2 = Pin 2 (یا 7)
// PB3 = Pin 3 (یا 2)
// PB4 = Pin 4 (یا 3)
// PB5 = Pin 5 (یا 1)

#define TRIG_PIN PB0 // پین تریگر HC-SR04
#define ECHO_PIN PB1 // پین اکو HC-SR04
#define BUZZER_PIN PB2 // پین بازر اکتیو

#define MAX_DISTANCE_CM 200 // حداکثر فاصله مورد نظر برای اندازه‌گیری (سانتی‌متر)
#define MAX_ECHO_DURATION (MAX_DISTANCE_CM * 58) // معادل میکروثانیه برای Timeout

// تابع دستی برای خواندن پالس (جایگزین pulseIn)
// این تابع با Polling کار می کند و ممکن است در محیط های شلوغ دقیق نباشد
unsigned long manualPulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
  unsigned long startMicros = micros();
  
  // منتظر تغییر وضعیت به مخالف حالت مورد نظر
  while (digitalRead(pin) == state) {
    if (micros() - startMicros > timeout) return 0; // Timeout
  }

  // منتظر تغییر وضعیت به حالت مورد نظر (شروع پالس)
  while (digitalRead(pin) != state) {
    if (micros() - startMicros > timeout) return 0; // Timeout
  }

  unsigned long pulseStart = micros(); // زمان شروع پالس

  // منتظر تغییر وضعیت به مخالف حالت مورد نظر (پایان پالس)
  while (digitalRead(pin) == state) {
    if (micros() - startMicros > timeout) return 0; // Timeout
  }

  return micros() - pulseStart; // مدت زمان پالس
}

// تابع اصلی برای دریافت فاصله بر حسب سانتی‌متر (معادل sonar.ping_cm())
unsigned int getDistanceCM() {
  // ارسال پالس ماشه
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10); // پالس 10 میکروثانیه
  digitalWrite(TRIG_PIN, LOW);

  // اندازه‌گیری مدت زمان پالس بازگشتی
  unsigned long duration = manualPulseIn(ECHO_PIN, HIGH, MAX_ECHO_DURATION);

  if (duration == 0 || duration > MAX_ECHO_DURATION) {
    return 0; // عدم تشخیص یا خارج از محدوده
  } else {
    // تبدیل میکروثانیه به سانتی‌متر
    return duration / 58;
  }
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW); // اطمینان از وضعیت اولیه LOW
}

void loop() {
  delay(50); // تأخیر بین اندازه‌گیری‌ها

  unsigned int distance_cm = getDistanceCM();

  // --- منطق بازر (برای بازر اکتیو) ---
  if (distance_cm == 0 || distance_cm > MAX_DISTANCE_CM) { // اگر خارج از محدوده یا خطا بود
    digitalWrite(BUZZER_PIN, LOW); // بازر خاموش
  } else if (distance_cm > 100) { // فواصل دورتر از 100
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1250); // 1 ثانیه بوق
    digitalWrite(BUZZER_PIN, LOW);
    delay(1250); // 1 ثانیه سکوت
  } else if (distance_cm > 50) { // فواصل 51 تا 100
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);
  } else if (distance_cm > 25) { // فواصل 26 تا 50
    digitalWrite(BUZZER_PIN, HIGH);
    delay(250);
    digitalWrite(BUZZER_PIN, LOW);
    delay(250);
  } else if (distance_cm > 10) { // فواصل 11 تا 25
    digitalWrite(BUZZER_PIN, HIGH);
    delay(125);
    digitalWrite(BUZZER_PIN, LOW);
    delay(125);
  } else { // فواصل 1 تا 10 سانتی‌متر (خیلی نزدیک)
    digitalWrite(BUZZER_PIN, HIGH); // بوق ممتد
  }
}