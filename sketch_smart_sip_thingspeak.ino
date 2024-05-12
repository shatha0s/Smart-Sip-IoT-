//SmartSip
//Libraries
#include <ESP8266WiFi.h>
#include "HX711.h"
#include <ThingSpeak.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <SimpleDHT.h>
#include <OneWire.h>
#include "DHT.h"
#include <Time.h>
#include <TimeLib.h>
#define DHTPIN D5      // what digital pin the DHT22 is conected to
#define DHTTYPE DHT11  // there are multiple kinds of DHT sensors
#define Tilt D7

// definitions
const char *ssid = "IoT_LAB";
const char *pass = "IoT123321";
unsigned long counterChannelNumber = 2240605;          // Channel ID
const char *myCounterReadAPIKey = "MPF00N9V6XSMS6TL";  // Read API Key
const int FieldNumber1 = 3;                            // The field you wish to read

WiFiClient client;
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);  // temp sensor

//for weight scale
float Tare = 0;
const int LOADCELL_DOUT_PIN = D6;
const int LOADCELL_SCK_PIN = D4;
HX711 scale;

void wifi_setup() {
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
}

int timeSinceLastRead = 0;

void setup() {

    pinMode(Tilt, INPUT);  //tilt sesnsor pin

  //LCD
  lcd.init();
  lcd.begin(16, 2);
  LCD_print(0, 0, "Welcom to");
  LCD_print(2, 1, "Smart Sip");

  Serial.begin(115200);
  //Wire.begin(D1, D2);

  //lcd.backlight();
  WiFi.begin(ssid, pass);
  //lcd.clear();
  Serial.println("WiFi connected");
    LCD_print(0, 0, "hi15");

  // temp sensor
  Serial.setTimeout(500);
  dht.begin();

  //Weight scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println(scale.read());            // print a raw reading from the ADC
  Serial.println(scale.read_average(20));  // print the average of 20 readings from the ADC
  Serial.println(scale.get_value(5));
  Serial.println(scale.get_units(5), 1);
  scale.set_scale(1455.9766);  //(Reading/khnown weight ) (500/-531217)= -1062.434
  scale.tare(0);               // reset the scale to 0
  Serial.print("read: \t\t");
  Serial.println(scale.read());
  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));
  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));
  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(), 1);
  Serial.println("Readings:");


  LCD_print(0, 0, "hi23");

  ThingSpeak.begin(client);
}


void loop() {

  LCD_print(0, 0, "1.12002556aaaaaaa");

  //Weight scale
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 5);

  delay(500);


  //Reminder's code
  static byte lastSecond = 0;
  uint32_t currentSeconds = now();
  if (lastSecond != second(currentSeconds)) {
    lastSecond = second(currentSeconds);
    char elapsedTime[32] = "";
    int hrs = currentSeconds / 3600;
    int minutes = currentSeconds % 3600 / 60;
    sprintf(elapsedTime, "%3d:%02d:%02d",
            hrs,
            minutes,
            (currentSeconds % 60));
    Serial.println(elapsedTime);
  }



  //Tilt sensor code
  int val_digital = digitalRead(Tilt);
  if (val_digital) {
    scale.read();
    Serial.println(val_digital);
    Serial.print("\t");
    LCD_print(6, 0, "Yes!");
    LCD_print(1, 1, "keep Hydrated!");
    //lcd.clear();
  }

  //temp sensor
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (timeSinceLastRead > 2000) {
    if (isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }
    float hif = dht.computeHeatIndex(f);
    float hic = dht.computeHeatIndex(t, false);
    Serial.print("Temperature: ");
    timeSinceLastRead = 0;
  }
  timeSinceLastRead += 100;
  int C = (f - 32) * 0.5556;
  Serial.print(C);
  Serial.println(" °C\t");
  if (C > 15) {
    LCD_print(1, 0, "Temperature is :");
    // itoa(C);
    //LCD_print(C);



    // lcd.clear();
    LCD_print(3, 0, "High temp");
    LCD_print(1, 1, "drink water!");
    // lcd.clear();
  }
  ThingSpeak.setField(2, scale.get_units(10));
  ThingSpeak.setField(1, dht.readTemperature());
  ThingSpeak.setField(3, digitalRead(Tilt));
  ThingSpeak.writeFields(counterChannelNumber, myCounterReadAPIKey);
}


void LCD_print(int x, int y, char *str) {

  lcd.backlight();
  lcd.setCursor(x, y);
  lcd.print(str);
  delay(3000);
  lcd.clear();
}