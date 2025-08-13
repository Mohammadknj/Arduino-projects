#include <avr/sleep.h> // برای مدیریت حالت خواب
#include <LiquidCrystal.h>  // کتابخانه استاندارد LCD (برای LCD بدون I2C)

// تعریف پین های LCD: (RS, E, DB4, DB5, DB6, DB7)
const int rsPin = 10;
const int enPin = 11;
const int d4Pin = 5;
const int d5Pin = 4;
const int d6Pin = 3;
const int d7Pin = 2;

LiquidCrystal lcd(rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin);
// تعریف پین‌ها
#define PIR_PIN 14 //A0
#define BUZZER_PIN 7
#define LAMP_RELAY_PIN 6
#define MODE_SWITCH_PIN 12

// متغیرها
volatile boolean is_awake = false; // پرچم برای بیدار شدن از خواب
int cnt = 0;

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

  lcd.begin(16, 2);
  lcd.print("Device is ON");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(5000);
  lcd.clear();
}

void loop() {
  // بررسی وضعیت کلید
  if (digitalRead(MODE_SWITCH_PIN) == LOW && digitalRead(PIR_PIN) == HIGH) {
    // کلید روشن است، دزدگیر فعال است
      // بازر را روشن کنید
      digitalWrite(BUZZER_PIN, HIGH);
      
      // رله را فعال کنید (برای Active-Low باید LOW شود)
      digitalWrite(LAMP_RELAY_PIN, LOW);
      cnt++;
      // منتظر بمانید تا آلارم به مدت کافی روشن بماند (مثلاً 5 ثانیه)
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(cnt);
        
      delay(5000);

      // خاموش کردن آلارم و رله
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LAMP_RELAY_PIN, HIGH); // غیرفعال کردن رله
    
  }
}