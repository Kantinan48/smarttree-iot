# 🌳 Smart Tree IoT & Real-time Web Dashboard

![Project Status](https://img.shields.io/badge/Status-Completed-success)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![Database](https://img.shields.io/badge/Database-Firebase-FFCA28)
![Frontend](https://img.shields.io/badge/Frontend-TailwindCSS-38B2AC)

โปรเจกต์ระบบตรวจสอบสภาพแวดล้อมและดูแลต้นไม้อัจฉริยะ (Smart Tree) ที่ทำงานบนบอร์ด **ESP32** ส่งข้อมูลผ่าน Wi-Fi ขึ้นระบบ Cloud ของ **Firebase Realtime Database** และแสดงผลผ่านหน้าเว็บ Dashboard ที่สวยงามแบบ **Real-time** พร้อมระบบแจ้งเตือนอันตรายจากแก๊สหรือไฟไหม้

🔗 **Live Demo (Web Dashboard):** [https://smarttree-iot.web.app](https://smarttree-iot.web.app)

---

## ✨ Features (ความสามารถของระบบ)

- **Real-time Monitoring:** ติดตามค่าอุณหภูมิ, ความชื้นในอากาศ, และความชื้นในดิน ผ่านหน้าเว็บได้ทันที
- **Safety Alert System:** มีเซนเซอร์ตรวจจับแก๊ส/ควันไฟ หากพบค่าผิดปกติจะแจ้งเตือน (DANGER) ขึ้นหน้าเว็บพร้อมเสียงร้องเตือนจาก Buzzer ทันที
- **Weather API Integration:** หน้าเว็บเชื่อมต่อกับ Open-Meteo API เพื่อแสดงผลสภาพอากาศจริงภายนอก (อิงพิกัดจังหวัดเชียงใหม่)
- **Power Efficient:** โค้ดถูกออกแบบมาให้ส่งข้อมูลทุกๆ 10 วินาที (ปรับตั้งค่าได้) เพื่อประหยัดพลังงานบอร์ดและลดการใช้แบนด์วิดท์
- **Serverless Architecture:** หน้าเว็บโฮสต์บน Firebase Hosting ดึงข้อมูลตรงจาก Database โดยไม่ต้องตั้งเซิร์ฟเวอร์ Backend

---

## 🛠️ Hardware Requirements (รายการอุปกรณ์)

*   **บอร์ด ESP32 + สาย USB เชื่อมต่อคอม:** ไมโครคอนโทรลเลอร์หลักประมวลผลและส่งข้อมูล[span_1](start_span)[span_1](end_span)
*   **Breadboard:** แผงต่อวงจรต้นแบบ[span_2](start_span)[span_2](end_span)
*   **สายไฟ Jumper:** สำหรับเชื่อมต่ออุปกรณ์ต่างๆ[span_3](start_span)[span_3](end_span)
*   **จอ OLED 0.96" (I2C):** แสดงผลที่ตัวเครื่อง (มี 4 ขา: VCC, GND, SCL, SDA)[span_4](start_span)[span_4](end_span)
*   **เซนเซอร์ตรวจจับมือ (Ultrasonic HC-SR04):** ตรวจจับระยะห่างหรือการเคลื่อนไหว[span_5](start_span)[span_5](end_span)
*   **DHT11 Sensor:** ตรวจวัดอุณหภูมิ/ความชื้น (ใช้งานจริงแทน DHT22 ในแผนงาน)[span_6](start_span)[span_6](end_span)
*   **Resistive Soil Moisture Sensor + บอร์ด HD-38:** ตรวจวัดความชื้นดิน[span_7](start_span)[span_7](end_span)
*   **Raindrop Sensor:** ตรวจจับน้ำฝน[span_8](start_span)[span_8](end_span)
*   **LDR Module:** ตรวจวัดแสง[span_9](start_span)[span_9](end_span)
*   **MQ-2 (ตรวจควัน/แก๊ส) + Active Buzzer:** ระบบตรวจจับความปลอดภัยและส่งเสียงเตือน[span_10](start_span)[span_10](end_span)

---

## 💻 Tech Stack (เทคโนโลยีที่ใช้)

**Hardware / Backend:**
- C++ (Arduino IDE)
- [Firebase ESP Client by Mobizt](https://github.com/mobizt/Firebase-ESP-Client)
- Firebase Realtime Database
- Firebase Authentication (Anonymous Login)

**Frontend / Web Dashboard:**
- HTML5 / Vanilla JavaScript
- [Tailwind CSS](https://tailwindcss.com/) (ผ่าน CDN)
- Firebase Web SDK v9 (Modular)
- Firebase Hosting
- Open-Meteo API (พยากรณ์อากาศ)

---

## 🚀 Installation & Setup (วิธีติดตั้งและใช้งาน)

### Phase 1: Hardware & Arduino Setup
1. โคลน Repository นี้ลงในเครื่องของคุณ:
   ```bash
   git clone https://github.com/your-username/smarttree-iot.git
   ```
2. เปิดโปรแกรม Arduino IDE และติดตั้งไลบรารีที่จำเป็น:
   - `Firebase ESP Client` โดย Mobizt
   - `DHT sensor library` โดย Adafruit
3. เปิดไฟล์โค้ดของ ESP32 (`.ino`) และแก้ไขข้อมูลต่อไปนี้ให้ตรงกับของคุณ:
   ```cpp
   #define WIFI_SSID "YOUR_WIFI_NAME"
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
   #define API_KEY "YOUR_FIREBASE_API_KEY"
   #define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL"
   ```
4. กดอัปโหลดโค้ดลงบอร์ด ESP32 

### Phase 2: Firebase Configuration
1. ไปที่ [Firebase Console](https://console.firebase.google.com/) สร้างโปรเจกต์ใหม่
2. เปิดใช้งาน **Realtime Database** และตั้งค่า Rules ดังนี้:
   ```json
   {
     "rules": {
       ".read": true,
       ".write": "auth != null"
     }
   }
   ```
3. เปิดใช้งาน **Authentication** -> เลือกแท็บ Sign-in method -> เปิด **Anonymous**
4. นำ API Key และ Database URL ไปใส่ในโค้ด Arduino (Phase 1) และไฟล์ `index.html` (Phase 3)

### Phase 3: Web Dashboard Deployment
หน้าเว็บนี้พร้อมใช้งานผ่านเซิร์ฟเวอร์ท้องถิ่น (เปิดไฟล์ `index.html` โดยตรง) หรือนำขึ้นออนไลน์ผ่าน Firebase Hosting:
1. ติดตั้ง Firebase CLI ในคอมพิวเตอร์:
   ```bash
   npm install -g firebase-tools
   ```
2. ล็อกอินและตั้งค่าโปรเจกต์:
   ```bash
   firebase login
   firebase init hosting
   ```
   *(เลือก Use an existing project -> เลือกโปรเจกต์ของคุณ -> Public directory ปล่อยว่างไว้ -> ไม่ใช้ Single-page app -> ไม่เชื่อม GitHub)*
3. นำไฟล์ `index.html` ไปใส่ในโฟลเดอร์ `public` ที่ระบบสร้างขึ้น
4. สั่ง Deploy ขึ้นเว็บ:
   ```bash
   firebase deploy
   ```

---

## 📌 Wiring Diagram (การต่อสายไฟเบื้องต้น)
*(ปรับเปลี่ยนเลขขา Pin ได้ตามความเหมาะสมในโค้ด)*
- **DHT11:** Data pin -> ESP32 GPIO 4
- **MQ-2 Gas:** Analog pin -> ESP32 GPIO 34
- **Soil Moisture:** Analog pin -> ESP32 GPIO 35
- **LDR:** Analog pin -> ESP32 GPIO 32
- **Buzzer:** Signal pin -> ESP32 GPIO 5

---

## 📝 License
This project is open-source and available under the [MIT License](LICENSE).
