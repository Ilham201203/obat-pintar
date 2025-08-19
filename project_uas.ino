#include <HX711.h>

#define IR_SENSOR_PIN 4 
#define BUZZER_PIN 26   
#define LOAD_CELL_DOUT 27
#define LOAD_CELL_SCK 14

HX711 scale;

float initialWeight = 0.0;
float pillWeight = 0.5; 
bool bottleOpened = false;
unsigned long lastOpenedTime = 0;
const unsigned long reminderInterval = 5 * 60 * 1000; 

void setup() {
  Serial.begin(115200);

  scale.begin(LOAD_CELL_DOUT, LOAD_CELL_SCK);
  scale.set_scale(); 
  scale.tare();      
  initialWeight = scale.get_units(); 

  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("Setup selesai!");
}

void loop() {
  int irState = digitalRead(IR_SENSOR_PIN);

  if (irState == HIGH && !bottleOpened) {
    bottleOpened = true;
    lastOpenedTime = millis();
    Serial.println("Botol dibuka!");

    delay(1000); 
    float currentWeight = scale.get_units();
    float weightDiff = initialWeight - currentWeight;

    if (weightDiff >= pillWeight) {
      Serial.println("Pil telah diambil.");
      initialWeight = currentWeight; 
    } else {
      Serial.println("Tidak ada pil yang diambil.");
    }
  }
  if (bottleOpened && millis() - lastOpenedTime > reminderInterval) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Pengingat: Tutup botol atau ambil pil!");
    bottleOpened = false; 
  }

  delay(500); 
}