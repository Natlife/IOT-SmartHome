#include "DHT.h"
//LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Khai báo chân kết nối
#define DHT11Pin 2
#define DHTType DHT11
#define PIR_PIN 3
#define LED_PIN1 7
#define LED_PIN2 8
#define LED_PIN3 9
#define buzzer 6

DHT dht(DHT11Pin, DHT11);
LiquidCrystal_I2C lcd(0X27,16,2); 
SoftwareSerial BT(10, 11);
// Biến lưu nhiệt độ & độ ẩm
float humidity, tempC;
const int mq02Pin = A0;
// Cảm biến PIR
int calibrationTime = 10;  // Giảm thời gian hiệu chỉnh từ 30s xuống 10s
long unsigned int lowIn;
long unsigned int pause = 5000;  
boolean lockLow = true;
boolean takeLowTime;
String inputString = "";  

void setup() {
    Serial.begin(9600);
    BT.begin(9600);     
    lcd.init();
    lcd.backlight();

    dht.begin();
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
    pinMode(LED_PIN3, OUTPUT);
    pinMode(buzzer, OUTPUT);
    digitalWrite(PIR_PIN, LOW);

    Serial.print("Đang hiệu chỉnh cảm biến...");
    for(int i = 0; i < calibrationTime; i++) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nCảm biến đã sẵn sàng!");
     Serial.println("Nhap lenh AT:");
}

void loop() {
    readSensors();
//    updateDisplay();
    handleBluetooth();
    checkMotion();
    delay(500); // Reduce CPU load
}

void readSensors() {
    humidity = dht.readHumidity();
    tempC = dht.readTemperature();
    int mq02Value = analogRead(mq02Pin);
    Serial.print("Độ ẩm: "); Serial.print(humidity);
    Serial.print("% - Nhiệt độ: "); Serial.print(tempC);
    Serial.println("°C");
    Serial.print("Giá trị MQ-02: ");
    Serial.println(mq02Value);
    if (mq02Value >= 200) {
        digitalWrite(LED_PIN1, LOW);
        digitalWrite(LED_PIN2, LOW);
        digitalWrite(LED_PIN3, LOW);
        lcd.clear();
        digitalWrite(buzzer, HIGH);
        delay(300);
        digitalWrite(buzzer, LOW);
        readSensors();

    } else {
        digitalWrite(buzzer, LOW);
        updateDisplay();
        
    }
}

void updateDisplay() {
    lcd.setCursor(2, 0);
    lcd.print("Humi:" + String(humidity) + "%");
    lcd.setCursor(0, 1);
    lcd.print("TempC:" + String(tempC) + "*C");
}

void handleBluetooth() {
    if (BT.available()) {
        char c = BT.read();
        Serial.print("From BT: ");
        Serial.println(c);
    }
    if (Serial.available()) {
        char c = Serial.read();
        BT.write(c);
        Serial.print("To BT: ");
        Serial.println(c);
    }
    while (BT.available()) {
        char c = BT.read();
        if (c == '\n') {
            processCommand(inputString);
            inputString = "";
        } else {
            inputString += c;
        }
    }
}

void checkMotion() {
    if (digitalRead(PIR_PIN) == HIGH) {
        digitalWrite(LED_PIN1, HIGH);
        digitalWrite(LED_PIN2, HIGH);
        digitalWrite(LED_PIN3, HIGH);
        if (lockLow) {
            lockLow = false;
            Serial.println(" Phát hiện chuyển động!");
            delay(50);
        }
        takeLowTime = true;
    } else {
        if (takeLowTime) {
            lowIn = millis();
            takeLowTime = false;
        }
        if (!lockLow && millis() - lowIn > pause) {
            lockLow = true;
            digitalWrite(LED_PIN1, LOW);
            digitalWrite(LED_PIN2, LOW);
            digitalWrite(LED_PIN3, LOW);
            Serial.println(" Chuyển động kết thúc.");
            delay(50);
        }
    }
}
void processCommand(String cmd) {
    if (cmd == "1-1") digitalWrite(LED_PIN1, HIGH);
    else if (cmd == "1-0") digitalWrite(LED_PIN1, LOW);
    else if (cmd == "2-1") digitalWrite(LED_PIN2, HIGH);
    else if (cmd == "2-0") digitalWrite(LED_PIN2, LOW);
    else if (cmd == "3-1") digitalWrite(LED_PIN3, HIGH);
    else if (cmd == "3-0") digitalWrite(LED_PIN3, LOW);
}
