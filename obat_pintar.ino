#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

const int irSensorPin = 2;      
const int vibrationSensorPin = 3; 
const int mp3RxPin = 17;         
const int mp3TxPin = 18;         

RTC_DS1307 rtc;
SoftwareSerial mp3Serial(mp3RxPin, mp3TxPin);

int alarmHour = 0; 
int alarmMinute = 1; 
bool medicineTaken = false;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

bool irSensorState = HIGH;
bool lastIrSensorState = HIGH;

void setup() {
  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("RTC tidak terdeteksi!");
    while (1);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC tidak berjalan, mengatur waktu...");
    rtc.adjust(DateTime(2024, 12, 12, 17, 30, 0)); 
  }

  pinMode(irSensorPin, INPUT_PULLUP);  
  pinMode(vibrationSensorPin, INPUT);

  mp3Serial.begin(9600);
  delay(500);
  sendCommandToMP3(0x3F, 0, 0); 
  delay(500);
  sendCommandToMP3(0x06, 0, 15); 

  Serial.println("Sistem siap!");
}

void loop() {
  DateTime now = rtc.now();

  int hourWIB = now.hour() + 7; /
  if (hourWIB >= 24) hourWIB -= 24; 

  Serial.print("Waktu saat ini (WIB): ");
  if (hourWIB < 10) Serial.print("0");
  Serial.print(hourWIB);
  Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.println(now.minute());

  if (hourWIB == alarmHour && now.minute() == alarmMinute && !medicineTaken) {
    Serial.println("[ALARM] Waktunya minum obat!");
    playReminder();
    delay(60000);
  }

  int reading = digitalRead(irSensorPin);
 if (reading != lastIrSensorState) {
    lastDebounceTime = millis(); 
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != irSensorState) {
      irSensorState = reading;

      if (irSensorState == LOW) {
        Serial.println("[INFO] Botol dibuka.");
        delay(500);

        if (digitalRead(vibrationSensorPin) == HIGH) {
          Serial.println("[INFO] Pil diambil.");
          medicineTaken = true;
          delay(5000); 
        } else {
          Serial.println("[WARNING] Botol terbuka tapi pil tidak diambil.");
          delay(300000); 
        }
      } else {
        if (!medicineTaken) {
          Serial.println("[INFO] Botol tertutup dan pil belum diambil.");
        }
      }
    }
  }
  lastIrSensorState = reading;

  delay(1000); 
}

void playReminder() {
  Serial.println("[ALARM] Memutar pengingat...");
  sendCommandToMP3(0x12, 0, 1); 
}

void sendCommandToMP3(byte command, byte param1, byte param2) {
  byte checksum = 0 - (0x7E + 0xFF + 0x06 + command + 0x00 + param1 + param2);
  byte buffer[] = {0x7E, 0xFF, 0x06, command, 0x00, param1, param2, checksum, 0xEF};
  for (byte i = 0; i < sizeof(buffer); i++) {
    mp3Serial.write(buffer[i]);
  }
}
