#include "DHT.h"
//LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Khai báo chân kết nối
#define DHT11Pin 2
#define DHTType DHT11
#define PIR_PIN 3
#define LED_PIN 13

DHT dht(DHT11Pin, DHT11);
LiquidCrystal_I2C lcd(0X27,16,2); 
// Biến lưu nhiệt độ & độ ẩm
float humidity, tempC;

// Cảm biến PIR
int calibrationTime = 10;  // Giảm thời gian hiệu chỉnh từ 30s xuống 10s
long unsigned int lowIn;
long unsigned int pause = 5000;  
boolean lockLow = true;
boolean takeLowTime;  

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();

    dht.begin();
    pinMode(PIR_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(PIR_PIN, LOW);

    Serial.print("Đang hiệu chỉnh cảm biến...");
    for(int i = 0; i < calibrationTime; i++) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nCảm biến đã sẵn sàng!");
}

void loop() {
    // Đọc dữ liệu từ DHT11
    humidity = dht.readHumidity();
    tempC = dht.readTemperature(); 
    lcd.setCursor(2,0);
    lcd.print("Humi:"+String(humidity)+"%");
    lcd.setCursor(0,1);
    lcd.print("TempC:"+String(tempC)+"*C");

    // Kiểm tra dữ liệu hợp lệ trước khi in ra
    if (!isnan(humidity) && !isnan(tempC)) {
        Serial.print("Độ ẩm: "); Serial.print(humidity);
        Serial.print("% - Nhiệt độ: "); Serial.print(tempC);
        Serial.println("°C");
    } else {
        Serial.println("Lỗi đọc DHT11!");
    }
    delay(700);
    // Kiểm tra chuyển động PIR
    if (digitalRead(PIR_PIN) == HIGH) {
        digitalWrite(LED_PIN, HIGH);
        if (lockLow) {  
            lockLow = false;
            Serial.println(" Phát hiện chuyển động!");
            delay(50);
        }         
        takeLowTime = true;
    } 
    else {  // Không có chuyển động
        digitalWrite(LED_PIN, LOW);
        if (takeLowTime) {
            lowIn = millis();
            takeLowTime = false;
        }
        if (!lockLow && millis() - lowIn > pause) {
            lockLow = true;
            Serial.println(" Chuyển động kết thúc.");
            delay(50);
        }
    }
    
    delay(500); // Giảm tải CPU
}
