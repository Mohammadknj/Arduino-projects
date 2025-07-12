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
const int maxTemp = 50;
const int buzzerPin = 6;
// متغیرها برای ساعت شمار فن
unsigned long totalDeviceOnTimeSeconds = 0;  // زمان روشن بودن فن اول بر حسب ثانیه
unsigned long fanOnTimeSeconds = 0;          // زمان روشن بودن فن دوم بر حسب ثانیه
unsigned long fanNewOnTimeSeconds = 0;  // زمان روشن بودن فن اول بر حسب ثانیه
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

unsigned long lastBeepTime = 0;
const unsigned long beepDuration = 200;  // مدت زمان هر بوق (200 میلی ثانیه)
int beepsToMake = 0;                     // تعداد بوق های باقی مانده
bool isBeeping = false;                  // پرچم برای فعال بودن بوق زدن

// تابع کمکی برای زمان بندی غیرمسدودکننده
bool isTimeUp(unsigned long& lastExecutionTime, unsigned long interval) {
  if (millis() - lastExecutionTime >= interval) {
    lastExecutionTime = millis();
    return true;
  }
  return false;
}

// متغیرهای جداگانه برای هر تایمر
unsigned long lastDisplayUpdateTime = 0;
unsigned long lastSerialPrintTime = 0;

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
  temperatureC = analogRead(lm35Pin) * 0.488;
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
// if((previousFanState == 0 || previousFanState == -1) & (currentFanState == 1 || currentFanState == 2)){
// fanOnTimeSeconds = messageDuration;

// }
if(previousFanState == -1 & (currentFanState == 1 || currentFanState == 2)){
fanOnTimeSeconds = currentMillis;
}else if(previousFanState == 0 & (currentFanState == 1 || currentFanState == 2)){
fanOnTimeSeconds = messageDuration;
}
    if (currentFanState > 0) {        // بوق زدن فقط برای حالت های ON
      beepsToMake = currentFanState;  // تعداد بوق ها برابر با وضعیت فعلی (1 یا 2)
      isBeeping = true;
    }else{
      fanOnTimeSeconds = 0;
    }
  }

  // به روزرسانی وضعیت قبلی برای حلقه بعدی
  previousFanState = currentFanState;

  //  منطق ساعت شمار فن با شرط صفر شدن
  // if(temperatureC < minTemp)
  // temperatureC=/1;
  // if (fanPWMValue == 0) {
  //   // دما کمتر از 20 درجه: ساعت شمار فن صفر شود
  //   // lcd.print("s");
  //   fanOnTimeSeconds = 0;
  // } else {
  //   // دما 20 درجه یا بیشتر: ساعت شمار فن افزایش یابد
  //   fanOnTimeSeconds += loopDuration / 1000;
  // }
  if(currentFanState == 1 || currentFanState == 2){
    fanOnTimeSeconds += loopDuration;
  }

  if (isBeeping) {
    if (isTimeUp(lastBeepTime, beepDuration)) {
      if (digitalRead(buzzerPin) == LOW) {
        // اگر بازر خاموش است، آن را روشن کن
        digitalWrite(buzzerPin, HIGH);
      } else {
        // اگر بازر روشن است، آن را خاموش کن و تعداد بوق ها را کم کن
        digitalWrite(buzzerPin, LOW);
        beepsToMake--;
        if (beepsToMake <= 0) {
          // اگر بوق ها تمام شدند، فرآیند را متوقف کن
          isBeeping = false;
        }
      }
    }
  }
  if (isTimeUp(lastDisplayUpdateTime, 100)) {
    if (showMessage && (currentMillis - messageDisplayStartTime < messageDuration)) {
      // اگر هنوز در بازه 2 ثانیه نمایش پیام هستیم
      lcd.setCursor(0, 0);
      // lcd.print(fanStatusMessages[currentFanState]);
      // if the 2nd half didn't print:
      // for (int i = 0; i < 32; i++) {
      //   if (i == 16)
      //     lcd.setCursor(0, 1);
      //   lcd.print(fanStatusMessages[currentFanState][i]);
      // }
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
      lcd.print("C");              // برای پاک کردن هر چیز قبلی

      // 4. محاسبه زمان کلی دستگاه از روی millis()
      totalDeviceOnTimeSeconds = currentMillis / 1000;
      fanNewOnTimeSeconds=fanOnTimeSeconds / 1000;
      // if(millis() == 10000){
      // fanNewOnTimeSeconds=fanOnTimeSeconds / 1000;

      // }
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
      // lcd.print(loopDuration);
    }
  }
  // 6. نمایش دما و PWM روی سریال مانیتور (برای اشکال زدایی)
  if (isTimeUp(lastSerialPrintTime, 1000)) {
    Serial.print("Temp: ");
    Serial.print(temperatureC);
    Serial.print("C | Fan PWM: ");
    Serial.print(fanPWMValue);
    Serial.print(" | Fan On Time: ");
    Serial.print(fanNewOnTimeSeconds);
    Serial.println("s");
  }
  // تأخیر کوتاه برای جلوگیری از نوسانات شدید نمایش
  // delay(100);
}

void printLooongStrings1(const char* str, int index) {
  for (int i = 0; i < 32; i++) {
    if (i == 16)
      lcd.setCursor(0, 1);
    // lcd.print(*str[index][i]);
  }
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
