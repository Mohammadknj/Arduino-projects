// lm35 gnd to nanoGnd
// lm35 Vs to nano5V
// lm35 out to A0
#include <LiquidCrystal.h>  // کتابخانه استاندارد LCD (برای LCD بدون I2C)

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
float temperatureC = 0;
int fanPWMValue = 0;
const int minTemp = 20;
const int maxTemp = 40;//////////
const int buzzerPin = 6;
// متغیرها برای ساعت شمار فن
unsigned long totalDeviceOnTimeSeconds = 0;  // زمان روشن بودن فن اول بر حسب ثانیه
unsigned long fanOnTimeSeconds = 0;          // زمان روشن بودن فن دوم بر حسب ثانیه
unsigned long fanNewOnTimeSeconds = 0;       // زمان روشن بودن فن اول بر حسب ثانیه
// متغیر برای اندازه گیری زمان سپری شده در هر حلقه
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long loopDuration = 0;

// متغیرهای جدید برای نمایش غیرمسدودکننده پیام
unsigned long messageDisplayStartTime = 0;
bool showMessage = false;
const unsigned long messageDuration = 2000;  // مدت زمان نمایش پیام (2 ثانیه)

const char* fanStatusMessages[] = {
  " Now it's coool   Here you are  ",  // این پیام با ایندکس 0 فراخوانی می شود
  "It's a bit hotttLet's cool down!",  // این پیام با ایندکس 1 فراخوانی می شود
  "Fan at MAX speed just god helps"    // این پیام با ایندکس 2 فراخوانی می شود
};

// متغیر جدید برای ذخیره وضعیت قبلی فن
int previousFanState = -1;  // -1 به معنی وضعیت نامشخص در ابتدا
int currentFanState = -1;

unsigned long lastStateChangeTime = 0;
unsigned long lastBeepTime = 0;
const unsigned long beepDuration = 200;  // مدت زمان هر بوق (200 میلی ثانیه)
int beepsToMake = 0;                     // تعداد بوق های باقی مانده
bool isWaitingForBeep = false;                  // پرچم برای فعال بودن بوق زدن
bool isBeeping = false;////
int stateAtStartOfWait = -1; // وضعیت فن در لحظه شروع انتظار
bool isBuzzerOn = false;

// تابع کمکی برای زمان بندی غیرمسدودکننده
bool isTimeUp(unsigned long& lastExecutionTime, unsigned long interval) {
  if (millis() - lastExecutionTime >= interval) {
    lastExecutionTime = millis();
    return true;
  }
  return false;
}

float findTemp(){
  int sum = 0;
  for(int i=0; i<200; i++){
    sum += analogRead(lm35Pin);
  }
  return sum / 200 * 0.488;
}

void findBeepNum(){
  float finalTemperatureC = findTemp();
    beepsToMake = 0;
    if (finalTemperatureC >= minTemp && finalTemperatureC < maxTemp) {
      beepsToMake = 1;
    } else if (finalTemperatureC >= maxTemp) {
      beepsToMake = 2;
    }
}
// متغیرهای جداگانه برای هر تایمر
unsigned long lastDisplayUpdateTime = 0;
unsigned long lastSerialPrintTime = 0;

// int fanStateInLastBeep =
void setup() {
  pinMode(fanPWMPin, OUTPUT);
  analogWrite(fanPWMPin, 0);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
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

  //  محاسبه زمان سپری شده در این حلقه
  currentMillis = millis();
  loopDuration = currentMillis - previousMillis;
  previousMillis = currentMillis;

  //  خواندن دما و کنترل فن
  /////////
  temperatureC = findTemp();
  // temperatureC = analogRead(lm35Pin) * 0.488;
  fanPWMValue = 0;

  if (temperatureC < minTemp) {
    fanPWMValue = 0;
    currentFanState = 0;
  } else if (temperatureC >= minTemp && temperatureC <= maxTemp) {
    fanPWMValue = map(temperatureC, minTemp, maxTemp, 0, 255);
    currentFanState = 1;
  } else {  // temperatureC > maxTemp
    fanPWMValue = 255;
    currentFanState = 2;
  }

  analogWrite(fanPWMPin, fanPWMValue);

  if (currentFanState != previousFanState) {
    showMessage = true;
    messageDisplayStartTime = currentMillis;
    lcd.clear();  // پاک کردن صفحه برای پیام جدید

    if (previousFanState == -1 & (currentFanState == 1 || currentFanState == 2)) {
      fanOnTimeSeconds = currentMillis;
    } else if (previousFanState == 0 & (currentFanState == 1 || currentFanState == 2)) {
      fanOnTimeSeconds = 0;
    }
    if (currentFanState > 0) {        // بوق زدن فقط برای حالت های ON
      isWaitingForBeep = true;
      lastStateChangeTime = currentMillis;
      stateAtStartOfWait = currentFanState;
    } else {
      fanOnTimeSeconds = 0;
    }
  }

  // به روزرسانی وضعیت قبلی برای حلقه بعدی

  if (currentFanState == 1 || currentFanState == 2) {
    fanOnTimeSeconds += loopDuration;
  }

// تابع منتظر برای ثابت شدن وضعیت بوق 
  if (isWaitingForBeep) {
    if (currentFanState != stateAtStartOfWait) {
      lastStateChangeTime = currentMillis;
      stateAtStartOfWait = currentFanState;
    }
    
    if (isTimeUp(lastStateChangeTime, 2000)) {
      // 3. پس از 2 ثانیه پایداری، تعداد بوق‌ها را تعیین کن
      switch (currentFanState) {
        case 0:
          beepsToMake = 0; // بوق نزن
          break;
        case 1:
          beepsToMake = 1; // یک بوق بزن
          break;
        case 2:
          beepsToMake = 2; // دو بوق بزن
          break;
      }
      isBeeping = true; // شروع به بوق زدن
      isWaitingForBeep = false;
    }
  }
  if (isBeeping) {
    if (isTimeUp(lastBeepTime, 200)) { // هر 200 میلی ثانیه
      if (isBuzzerOn) {
        // بازر روشن است، خاموشش کن و تعداد بوق را کم کن
        noTone(buzzerPin);
        // digitalWrite(buzzerPin, LOW);
        isBuzzerOn = false;
        beepsToMake--;
      } else {
        // بازر خاموش است، روشنش کن
        tone(buzzerPin, 1000);
        // digitalWrite(buzzerPin, HIGH);
        isBuzzerOn = true;
      }
    }
    // اگر تعداد بوق‌ها تمام شد، بوق زدن را متوقف کن
    if (beepsToMake <= 0 && !isBuzzerOn) {
      isBeeping = false;
    }
  }


  previousFanState = currentFanState;


  if (isTimeUp(lastDisplayUpdateTime, 100)) {
    if (showMessage && (currentMillis - messageDisplayStartTime < messageDuration)) {
      // اگر هنوز در بازه 2 ثانیه نمایش پیام هستیم
      lcd.setCursor(0, 0);
      printLooongStrings(fanStatusMessages, currentFanState);

    } else {
      // اگر زمان نمایش پیام به پایان رسیده یا پیامی برای نمایش نیست
      if (showMessage) {  // فقط یک بار پس از اتمام زمان پیام
        lcd.clear();
        showMessage = false;
      }
      // نمایش دما روی LCD
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperatureC, 2);  // نمایش دما با یک رقم اعشار
      lcd.print((char)223);        // کاراکتر درجه (°)
      lcd.print("C  ");              // برای پاک کردن هر چیز قبلی

      totalDeviceOnTimeSeconds = currentMillis / 1000;
      fanNewOnTimeSeconds = fanOnTimeSeconds / 1000;

      // 5. نمایش اطلاعات روی LCD
      // نمایش زمان روشن بودن دستگاه (ساعت اول)
      lcd.setCursor(0, 1);
      lcd.print(totalDeviceOnTimeSeconds / 3600);  // ساعت
      lcd.print(":");
      lcd.print((totalDeviceOnTimeSeconds % 3600) / 60);  // دقیقه
      lcd.print(":");
      lcd.print(totalDeviceOnTimeSeconds % 60);  // ثانیه

      // setting distance between 2 clocks
      if ((totalDeviceOnTimeSeconds / 3600) < 10 && (fanNewOnTimeSeconds / 3600) < 10)
        lcd.print("  ");
      else if ((totalDeviceOnTimeSeconds / 3600) >= 10 && (fanNewOnTimeSeconds / 3600) < 10)
        lcd.print(" ");
      // نمایش زمان روشن بودن فن (ساعت دوم)
      lcd.print(fanNewOnTimeSeconds / 3600);  // ساعت
      lcd.print(":");
      lcd.print((fanNewOnTimeSeconds % 3600) / 60);  // دقیقه
      lcd.print(":");
      lcd.print(fanNewOnTimeSeconds % 60);  // ثانیه
      // برای اینکه در نوشتن دقیقه جدید به باگ نخوریم
      lcd.print("    ");
    }
  }
  // 6. نمایش دما و PWM روی سریال مانیتور (برای اشکال زدایی)
  // if (isTimeUp(lastSerialPrintTime, 1000)) {
  //   Serial.print("Temp: ");
  //   Serial.print(temperatureC);
  //   Serial.print("C | Fan PWM: ");
  //   Serial.print(fanPWMValue);
  //   Serial.print(" | Fan On Time: ");
  //   Serial.print(fanNewOnTimeSeconds);
  //   Serial.println("s");
  // }
}

void printLooongStrings(const char* strings[], int index) {
  // رشته مورد نظر را از آرایه انتخاب می کنیم
  const char* str = strings[index];

  for (int i = 0; i < 32; i++) {
    // بررسی می کنیم که به انتهای رشته نرسیده باشیم
    if (str[i] == '\0') {
      break;
    }

    // موقعیت مکان نما را برای خط دوم تنظیم می کنیم
    if (i == 16) {
      lcd.setCursor(0, 1);
    }

    // هر کاراکتر را به صورت جداگانه چاپ می کنیم
    lcd.print(str[i]);
  }
}
