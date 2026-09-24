#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "time.h"

// ตัวช่วยของ Firebase
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ==========================================
// 1. ตั้งค่า Wi-Fi และ Firebase
// ==========================================
const char* ssid       = ""; 
const char* password   = "";      

#define API_KEY ""
#define DATABASE_URL "" 

const char* ntpServer  = "";
const long  gmtOffset_sec = 7 * 3600; 
const int   daylightOffset_sec = 0;

// Object สำหรับ Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ==========================================
// 2. Hardware Pin Mapping 
// ==========================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_PIN 32     
#define RAIN_PIN 33     
#define LDR_PIN 34      
#define MQ2_PIN 35      
#define BUZZER_PIN 19   

const int WARNING_THRESHOLD = 800;  
const int DANGER_THRESHOLD = 1500;  
const int RAIN_THRESHOLD = 3000; 

// ==========================================
// 3. Global Variables 
// ==========================================
float currentTemp = 0.0;
float currentHum = 0.0;
int currentSoilPercent = 0;
int currentSmokeValue = 0;

bool isDaytime = true;
bool isRaining = false;
bool isSensorError = false;
bool isAlarmActive = false;
bool forceRender = true; 

int currentScreen = 0; 
unsigned long previousScreenMillis = 0;
const long screenInterval = 5000; 

unsigned long previousSensorMillis = 0;
const long sensorInterval = 2000; 

unsigned long previousFirebaseMillis = 0;
const long firebaseInterval = 10000; 
// ==========================================
// 4. System Setup
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("ERROR: OLED failed!"));
    for(;;); 
  }

  // หน้าจอตอนบูต
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Connecting Wi-Fi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Connected!");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // โชว์สถานะกำลังต่อ Firebase
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Connecting Cloud...");
  display.display();

  // --- ตั้งค่า Firebase ---
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  // โหมดลงทะเบียนแบบไม่ระบุตัวตน (Anonymous) สำหรับทดสอบ
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Auth Success");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  delay(2000);
}

// ==========================================
// 5. Main Loop 
// ==========================================
void loop() {
  unsigned long currentMillis = millis();

  // Task 1: อ่านเซนเซอร์อัปเดตหน้าจอทุก 2 วินาที
  if (currentMillis - previousSensorMillis >= sensorInterval) {
    previousSensorMillis = currentMillis;
    fetchSensorData();
    checkSecurity(); 
    forceRender = true; 
  }

  // Task 2: สลับหน้าจอ OLED
  if (!isAlarmActive) {
    if (currentMillis - previousScreenMillis >= screenInterval) {
      previousScreenMillis = currentMillis;
      currentScreen++;
      if (currentScreen > 2) currentScreen = 0; 
      forceRender = true; 
    }
  }

  // Task 3: ส่งข้อมูลขึ้น Firebase ทุกๆ 5 นาที (และเช็คว่าเน็ตต่ออยู่)
  if (Firebase.ready() && (currentMillis - previousFirebaseMillis >= firebaseInterval || previousFirebaseMillis == 0)) {
    previousFirebaseMillis = currentMillis;
    sendDataToFirebase();
  }

  // วาดหน้าจอ
  if (forceRender) {
    renderUI();
    forceRender = false; 
  }
}

// ==========================================
// 6. Functions 
// ==========================================
void sendDataToFirebase() {
  Serial.println("------------------------------------");
  Serial.println("Uploading Data to Firebase...");

  // สร้างเส้นทางข้อมูลใน Database เช่น /SmartTree/CurrentData
  String basePath = "/SmartTree/CurrentData";

  // ส่งข้อมูลแบบแยกทีละตัว (เพื่อความง่ายในการดึงไปทำเว็บ)
  Firebase.RTDB.setFloat(&fbdo, basePath + "/temperature", currentTemp);
  Firebase.RTDB.setFloat(&fbdo, basePath + "/humidity", currentHum);
  Firebase.RTDB.setInt(&fbdo, basePath + "/soil_moisture", currentSoilPercent);
  Firebase.RTDB.setInt(&fbdo, basePath + "/gas_level", currentSmokeValue);
  Firebase.RTDB.setBool(&fbdo, basePath + "/is_daytime", isDaytime);
  Firebase.RTDB.setBool(&fbdo, basePath + "/is_raining", isRaining);
  
  // เช็คสถานะการแจ้งเตือนเพื่อบันทึกลงฐานข้อมูลด้วย
  String statusMsg = "SAFE";
  if (isAlarmActive) statusMsg = "DANGER";
  else if (currentSmokeValue >= WARNING_THRESHOLD) statusMsg = "WARNING";
  
  Firebase.RTDB.setString(&fbdo, basePath + "/status", statusMsg);

  Serial.println("Upload Complete!");
  Serial.println("------------------------------------");
}

void fetchSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    isSensorError = true;
  } else {
    isSensorError = false;
    currentHum = h;
    currentTemp = t;
  }
  currentSoilPercent = constrain(map(analogRead(SOIL_PIN), 4095, 1500, 0, 100), 0, 100);
  isDaytime = (analogRead(LDR_PIN) < 2000) ? true : false;
  currentSmokeValue = analogRead(MQ2_PIN);
  isRaining = (analogRead(RAIN_PIN) < RAIN_THRESHOLD) ? true : false;
}

void checkSecurity() {
  if (currentSmokeValue >= DANGER_THRESHOLD) {
    isAlarmActive = true;
    digitalWrite(BUZZER_PIN, HIGH); 
  } else {
    isAlarmActive = false;
    digitalWrite(BUZZER_PIN, LOW);  
  }
}

void renderUI() {
  display.clearDisplay();
  if (isAlarmActive) {
    display.setTextSize(2); display.setCursor(0, 10); display.println("DANGER!");
    display.setTextSize(1); display.setCursor(0, 35); display.print("Smoke Level: "); display.println(currentSmokeValue);
    display.display(); return; 
  }
  if (isSensorError) {
    display.setTextSize(2); display.setCursor(0, 20); display.println("DHT ERROR!"); display.display(); return;
  }

  if (currentScreen == 0) {
    struct tm timeinfo; display.setTextSize(1); display.setCursor(0, 0);
    if(getLocalTime(&timeinfo)) display.printf("Time: %02d:%02d", timeinfo.tm_hour, timeinfo.tm_min); else display.print("Time: --:--");
    display.setCursor(85, 0); if(isDaytime) display.print("[DAY]"); else display.print("[NIGHT]");
    display.setTextSize(2); display.setCursor(0, 20); display.print("T:"); display.print(currentTemp, 1); display.println("C");
    display.setCursor(0, 45); display.print("H:"); display.print(currentHum, 1); display.println("%");
  } else if (currentScreen == 1) {
    display.setTextSize(1); display.setCursor(0, 0); display.println("Soil & Rain (2/3)");
    display.setTextSize(3); display.setCursor(20, 18); display.print(currentSoilPercent); display.println("%");
    display.setTextSize(1); display.setCursor(0, 50); if (isRaining) display.println("Status: RAINING"); else display.println("Status: NOT RAINING");
  } else if (currentScreen == 2) {
    display.setTextSize(1); display.setCursor(0, 0); display.println("Air Quality (3/3)");
    display.setTextSize(1); display.setCursor(0, 15); display.print("Raw Gas: "); display.println(currentSmokeValue);
    display.setTextSize(2); display.setCursor(0, 35);
    if (currentSmokeValue >= WARNING_THRESHOLD) display.println("WARNING"); else display.println("SAFE");
  }
  display.display(); 
}