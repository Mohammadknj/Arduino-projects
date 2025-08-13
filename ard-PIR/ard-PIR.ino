#include <avr/sleep.h> // برای مدیریت حالت خواب

// تعریف پین‌ها
#define PIR_PIN 13
#define BUZZER_PIN 4
#define LAMP_RELAY_PIN 6
#define MODE_SWITCH_PIN 12

// متغیرها
volatile boolean is_awake = false; // پرچم برای بیدار شدن از خواب

void setup() {
  // تنظیم پین‌ها
  pinMode(PIR_PIN, INPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LAMP_RELAY_PIN, OUTPUT);
  
  // رله را در حالت غیرفعال نگه دارید (برای Active-Low باید HIGH شود)
  digitalWrite(LAMP_RELAY_PIN, HIGH);
  
  // بازر را خاموش کنید
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
  // بررسی وضعیت کلید
  if (digitalRead(MODE_SWITCH_PIN) == LOW) {
    // کلید روشن است، دزدگیر فعال است
    if (is_awake) {
      // بازر را روشن کنید
      digitalWrite(BUZZER_PIN, HIGH);
      
      // رله را فعال کنید (برای Active-Low باید LOW شود)
      digitalWrite(LAMP_RELAY_PIN, LOW);
      
      // منتظر بمانید تا آلارم به مدت کافی روشن بماند (مثلاً 5 ثانیه)
      delay(10000);
      
      // خاموش کردن آلارم و رله
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LAMP_RELAY_PIN, HIGH); // غیرفعال کردن رله
      is_awake = false; // پرچم را برای خواب بعدی ریست کنید
    }
    
    // اگر حرکت تشخیص داده نشده، به حالت خواب بروید
    if (!is_awake) {
      goToSleep();
    }
  } else {
    // مطمئن شوید که بازر و رله خاموش هستند
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LAMP_RELAY_PIN, HIGH); // غیرفعال کردن رله
    is_awake = false; // پرچم را ریست کنید
    
    delay(1000); // برای جلوگیری از اجرای سریع حلقه
  }
}

void goToSleep() {
  // تنظیم وقفه برای بیدار شدن از خواب
  // PIN 13 به نام "PCINT" در میکروکنترلر شناخته می‌شود.
  // در آردوینو نانو، PCINT0 تا PCINT7 روی پین‌های D8 تا D13 قرار دارند.
  // ما از PCINT5 (برای پین D13) استفاده می‌کنیم.
  EIMSK &= ~(1 << INT0); // غیرفعال کردن وقفه خارجی INT0
  PCMSK0 |= (1 << PCINT5); // فعال کردن وقفه PCINT برای پین 13 (PCINT5)
  PCICR |= (1 << PCIE0); // فعال کردن وقفه PCIE0
  
  // انتخاب حالت خواب: SLEEP_MODE_PWR_DOWN
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  // تنظیم میکروکنترلر برای خواب
  sleep_enable();
  
  // رفتن به خواب
  sleep_cpu();
  
  // بعد از بیدار شدن از خواب، این دستورات اجرا می‌شوند
  sleep_disable();
  
  // غیرفعال کردن وقفه برای جلوگیری از اجرای مجدد
  PCMSK0 &= ~(1 << PCINT5);
  PCICR &= ~(1 << PCIE0);
}

// تابع وقفه برای بیدار شدن از خواب
// این تابع فقط پرچم is_awake را تغییر می‌دهد و بلافاصله برمی‌گردد.
// کارهای اصلی در تابع loop انجام می‌شود.
ISR(PCINT0_vect) {
  is_awake = true;
}