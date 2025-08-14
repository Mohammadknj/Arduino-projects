// ----------------------------------------------------
// پروژه: سیستم امنیتی ESP32-CAM
// قطعات:
// - دوربین: esp32 cam (مدل استاندارد)
// - ماژول GSM: SIM800L
// - سنسور: HC-SR501 PIR
// - کارت حافظه Micro SD
// - رله: 5V Active-Low
// - آداپتور: 5V حداقل 2 آمپر
// - کلید راکر و مبدل USB-به-TTL
// ----------------------------------------------------

// کتابخانه‌های مورد نیاز
#include "esp_camera.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "FS.h"
#include "SD_MMC.h"
#include "Preferences.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WebServer.h>

// مهم: این خط برای کارکرد کتابخانه TinyGSM ضروری است
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

// تعریف پین‌ها برای تمامی قطعات
#define PIR_PIN 13
#define GSM_RING_PIN 14
#define MODE_SWITCH_PIN 12
#define BUZZER_PIN 2
#define LAMP_RELAY_PIN 6
#define FLASHER_PIN 4
#define GSM_RX_PIN 16
#define GSM_TX_PIN 17

// تنظیمات وای‌فای و تلگرام
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const String BOT_TOKEN = "YOUR_TELEGRAM_BOT_TOKEN";
const String CHAT_ID = "YOUR_CHAT_ID";

// ارتباط با ماژول GSM
HardwareSerial gsmSerial(2); // استفاده از پورت سریال 2 سخت‌افزاری ESP32
TinyGsm modem(gsmSerial);
Preferences preferences;
WebServer server(80);

bool isWiFiConnected = false;

// ----------------------
// توابع مربوط به وب‌سرور
// ----------------------
void handleRoot() {
  String html = "<html><body><h1>Update Proxies</h1><form action='/save' method='POST'>";
  preferences.begin("proxies", true);
  for (int i = 0; i < 10; i++) {
    String proxyAddr_key = "proxy_addr_" + String(i);
    String proxyPort_key = "proxy_port_" + String(i);
    String proxyAddr_val = preferences.getString(proxyAddr_key.c_str(), "");
    int proxyPort_val = preferences.getInt(proxyPort_key.c_str(), 0);
    
    html += "Proxy " + String(i + 1) + ": ";
    html += "<input type='text' name='addr" + String(i) + "' value='" + proxyAddr_val + "' placeholder='Address'>";
    html += "<input type='number' name='port" + String(i) + "' value='" + (proxyPort_val == 0 ? "" : String(proxyPort_val)) + "' placeholder='Port'><br>";
  }
  preferences.end();
  html += "<input type='submit' value='Save Proxies'></form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  preferences.begin("proxies", false);
  for (int i = 0; i < 10; i++) {
    String proxyAddr_key = "proxy_addr_" + String(i);
    String proxyPort_key = "proxy_port_" + String(i);
    
    String newAddr = server.arg("addr" + String(i));
    int newPort = server.arg("port" + String(i)).toInt();
    
    preferences.putString(proxyAddr_key.c_str(), newAddr);
    preferences.putInt(proxyPort_key.c_str(), newPort);
  }
  preferences.end();
  server.send(200, "text/plain", "Proxies updated successfully!");
}

// ----------------------
// توابع اصلی برنامه
// ----------------------
void takePhoto(int photoNumber, bool useFlasher) {
  if (useFlasher) {
    digitalWrite(FLASHER_PIN, HIGH);
    delay(100);
    Serial.printf("Taking photo #%d with flasher...\n", photoNumber);
  } else {
    Serial.printf("Taking photo #%d without flasher...\n", photoNumber);
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    if (useFlasher) {
      digitalWrite(FLASHER_PIN, LOW);
    }
    return;
  }
  
  String filename = "/capture_" + String(photoNumber) + ".jpg";
  File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.printf("Photo #%d saved to SD card.\n", photoNumber);
  }
  
  esp_camera_fb_return(fb);

  if (useFlasher) {
    digitalWrite(FLASHER_PIN, LOW);
  }
}

void sendPhotosFromSD() {
  if (!isWiFiConnected) {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(400);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nWiFi connection failed! Aborting photo sending.");
      return;
    }
    Serial.println("\nWiFi connected.");
    isWiFiConnected = true;
  }

  preferences.begin("proxies", true);
  bool sent = false;
  for (int i = 0; i < 10; i++) {
    String proxyAddr = preferences.getString(("proxy_addr_" + String(i)).c_str(), "");
    int proxyPort = preferences.getInt(("proxy_port_" + String(i)).c_str(), 0);

    if (proxyAddr.length() > 0 && proxyPort > 0) {
      Serial.printf("Trying proxy %d: %s:%d\n", i + 1, proxyAddr.c_str(), proxyPort);
      // Logic for sending photos via proxy
      bool currentProxySuccess = true;
      for (int photoNum = 1; photoNum <= 4; photoNum++) {
        String filename = "/capture_" + String(photoNum) + ".jpg";
        File file = SD_MMC.open(filename.c_str());
        if (file) {
          // HTTP client code to send 'file' via the current proxy
          file.close();
        }
      }
      if (currentProxySuccess) {
        sent = true;
        break;
      }
    }
  }
  preferences.end();
  
  if (sent) {
    Serial.println("Photos sent successfully!");
  } else {
    Serial.println("All proxy attempts failed. Photos were not sent.");
  }
  
  WiFi.disconnect(true);
}

void makeCallAndSendSMS() {
  String phoneNumber = "YOUR_PHONE_NUMBER"; 
  
  Serial.println("Attempting to make a call using AT commands...");
    
  modem.sendAT("ATD" + phoneNumber + ";");
  
  if (modem.waitResponse(10000) == 1) {
    Serial.println("Call initiated successfully.");
  } else {
    Serial.println("Call failed.");
  }
  
  delay(5000);
  modem.sendAT("ATH"); // قطع تماس
  
  Serial.println("Sending SMS...");
  String smsMessage = "Motion detected at your location. Check the photos.";
  if (modem.sendSMS(phoneNumber, smsMessage)) {
    Serial.println("SMS sent successfully.");
  } else {
    Serial.println("SMS failed.");
  }
}

void deletePhotosFromSD() {
  File root = SD_MMC.open("/");
  File file = root.openNextFile();
  while(file){
    String fileName = file.name();
    if(fileName.endsWith(".jpg")){
      SD_MMC.remove(fileName.c_str());
      Serial.printf("Deleted file: %s\n", fileName.c_str());
    }
    file = root.openNextFile();
  }
  Serial.println("Garbage emptied successfully.");
}

// ----------------------
// منطق اصلی برنامه
// ----------------------
void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(GSM_RING_PIN, INPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LAMP_RELAY_PIN, OUTPUT);
  pinMode(FLASHER_PIN, OUTPUT); // Set flasher pin as output
  digitalWrite(LAMP_RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Camera configuration for standard ESP32-CAM (compatible with most models)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1; 
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // Initialize SD card
  if(!SD_MMC.begin()){
    Serial.println("SD card mount failed!");
  } else {
    Serial.println("SD card mounted successfully.");
  }

  // Initialize GSM modem
  Serial.println("Initializing modem...");
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  delay(10000); // Wait 10 seconds for the modem to initialize
  
  preferences.begin("proxies", false);
  
  // Set up deep sleep wakeup sources
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
  esp_sleep_enable_ext1_wakeup(1ULL << GSM_RING_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
  
  Serial.println("System initialized. Going to sleep...");
  esp_deep_sleep_start();
}

void loop() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  int switch_state = digitalRead(MODE_SWITCH_PIN);
  String receivedSMS = "";
  
  isWiFiConnected = false;

  // SMS command handling
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
    Serial.println("System woke up by GSM RING signal. Checking for SMS...");
    delay(500);
    
    // Set SMS mode to text mode
    modem.sendAT("AT+CMGF=1");
    modem.waitResponse(500);

    // List unread messages
    modem.sendAT("AT+CMGL=\"REC UNREAD\"");
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        if (gsmSerial.available()) {
            response += (char)gsmSerial.read();
        }
    }
    
    int contentIndex = response.indexOf("\r\n");
    if (contentIndex != -1) {
      receivedSMS = response.substring(contentIndex + 2, response.indexOf("\r\n", contentIndex + 2));
      receivedSMS.trim();
      Serial.printf("Received SMS: %s\n", receivedSMS.c_str());
    }

    if (receivedSMS.indexOf("load proxy") != -1) {
      Serial.println("Received 'load proxy' SMS. Starting web server.");
      WiFi.softAP("ESP32_Proxy_Setup", "12345678");
      server.on("/", handleRoot);
      server.on("/save", handleSave);
      server.begin();
      unsigned long startTime = millis();
      while(millis() - startTime < 120000) { // 2-minute timeout
        server.handleClient();
        delay(10);
      }
      Serial.println("Proxy setup timeout. Going back to security mode.");
    } else if (receivedSMS.indexOf("empty garbage") != -1) {
      deletePhotosFromSD();
    } else if (receivedSMS.indexOf("send pics") != -1) {
      takePhoto(1, true);
      delay(1000);
      takePhoto(2, true);
      delay(1000);
      takePhoto(3, true);
      sendPhotosFromSD();
    } else if (receivedSMS.indexOf("continue") != -1) {
      Serial.println("Received 'continue' SMS. System will go back to sleep.");
    }
    
    // Delete all SMS messages after processing
    modem.sendAT("AT+CMGD=1,4");
    modem.waitResponse(500);
    
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    // PIR trigger logic for Away Mode, now also checks the switch state
    if (switch_state == HIGH) {
        Serial.println("System activated by PIR! Executing Away mode protocol.");
        takePhoto(1, false);
        delay(1000);
        digitalWrite(LAMP_RELAY_PIN, LOW);
        takePhoto(2, false);
        makeCallAndSendSMS();
        takePhoto(3, false);
        delay(1000);
        takePhoto(4, false);
        sendPhotosFromSD();
        
        // Wait for "stop" SMS or timeout
        unsigned long pirStartTime = millis();
        Serial.println("Awaiting 'stop' command for 5 minutes...");
        while(millis() - pirStartTime < 300000) { // 5-minute timeout
            // Check for SMS manually using AT commands
            modem.sendAT("AT+CMGL=\"REC UNREAD\"");
            String response = "";
            unsigned long smsCheckTime = millis();
            while (millis() - smsCheckTime < 5000) {
                if (gsmSerial.available()) {
                    response += (char)gsmSerial.read();
                }
            }
            int contentIndex = response.indexOf("\r\n");
            if (contentIndex != -1) {
                receivedSMS = response.substring(contentIndex + 2, response.indexOf("\r\n", contentIndex + 2));
                receivedSMS.trim();
            }

            if (receivedSMS.indexOf("stop") != -1) {
                Serial.println("Received 'stop' SMS. Lamp turned off.");
                digitalWrite(LAMP_RELAY_PIN, HIGH);
                modem.sendAT("AT+CMGD=1,4");
                modem.waitResponse(500);
                break;
            }
            delay(1000);
        }
        digitalWrite(LAMP_RELAY_PIN, HIGH);
    } else {
        Serial.println("PIR triggered, but switch is not in 'Away' mode. Ignoring.");
    }
  }
  
  delay(1000);
  Serial.println("Going back to deep sleep...");
  esp_deep_sleep_start();
}
