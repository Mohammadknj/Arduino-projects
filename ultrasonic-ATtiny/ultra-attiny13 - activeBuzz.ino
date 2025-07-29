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

#define NUM_READINGS 5 // تعداد خوانش‌ها برای میانگین‌گیری (مثلاً 3، 5 یا 7)
// هرچه این عدد بیشتر باشد، دقت بیشتر اما زمان و منابع بیشتری مصرف می‌شود.
// برای ATtiny13A، 3 یا 5 معقول است.

const int offset = 7; // فاصله از چشم تا لبه ماشین که نباید در فاصله سنجی محاسبه شود
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
unsigned int getRawDistanceCM() {
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

// تابع برای دریافت فاصله فیلتر شده (میانگین با حذف دورافتاده‌ها)
unsigned int getFilteredDistanceCM() {
  unsigned int readings[NUM_READINGS]; // آرایه برای ذخیره خوانش‌ها
  int validReadingsCount = 0;
  unsigned long sum = 0;
  unsigned int minVal = MAX_DISTANCE_CM + 1; // مقدار اولیه بزرگتر از حداکثر
  unsigned int maxVal = 0; // مقدار اولیه کوچکتر از حداقل

  for (int i = 0; i < NUM_READINGS; i++) {
    unsigned int dist = getRawDistanceCM();
    delay(10); // تأخیر کوتاه بین خوانش‌ها برای جلوگیری از تداخل Echo
    
    // فقط خوانش‌های معتبر و در محدوده را در نظر بگیریم
    if (dist > 0 && dist <= MAX_DISTANCE_CM) {
      readings[validReadingsCount] = dist; // ذخیره خوانش معتبر
      sum += dist; // جمع کل خوانش‌های معتبر
      if (dist < minVal) minVal = dist; // پیدا کردن حداقل
      if (dist > maxVal) maxVal = dist; // پیدا کردن حداکثر
      validReadingsCount++;
    }
  }

  // --- شروع منطق صحیح فیلتر ---
  if (validReadingsCount == 0) {
    return 0; // هیچ خوانش معتبری دریافت نشد
  } else if (validReadingsCount == 1) {
    return readings[0]; // فقط یک خوانش معتبر داشتیم
  } else if (validReadingsCount == 2) {
    // اگر فقط 2 خوانش معتبر داریم، نمی‌توانیم min/max را حذف کنیم (چون چیزی باقی نمی‌ماند).
    // پس میانگین ساده هر دو را بگیرید.
    return sum / 2; 
  } else { // validReadingsCount >= 3 (اکنون می‌توانیم حداقل و حداکثر را حذف کنیم)
    // حذف حداقل و حداکثر برای بهبود دقت (Truncated Mean)
    sum -= minVal; // حداقل را از مجموع کم کن
    sum -= maxVal; // حداکثر را از مجموع کم کن
    
    // میانگین با حذف 2 داده پرت
    return sum / (validReadingsCount - 2); 
  }
  // --- پایان منطق صحیح فیلتر ---
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW); // اطمینان از وضعیت اولیه LOW
}

void loop() {
  delay(100); // تأخیر بین اندازه‌گیری‌ها

  unsigned int distance_cm = getFilteredDistanceCM()-offset;

  // --- منطق بازر (برای بازر اکتیو) ---
  if (distance_cm <= 0 || distance_cm > MAX_DISTANCE_CM) { // اگر خارج از محدوده یا خطا بود
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