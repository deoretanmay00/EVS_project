#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi settings
const char* ssid = "Student s";
const char* password = "244466666";

// Server URL
const char* serverName = "http://10.232.102.202:3000/update";

// Dustbin ID
int dustbinID = 1;

// Pins
#define trig D5
#define echo D6
#define buzzer D7

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
long duration;
float distance;
String statusBin;
int isFull;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  lcd.init();
  lcd.backlight();

  connectWiFi();   // 🔥 Clean WiFi connect function
}

// ---------------- WIFI CONNECT FUNCTION ----------------
void connectWiFi() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  int retry = 0;

  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  lcd.clear();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());

    lcd.setCursor(0,0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());

  } else {
    Serial.println("\nWiFi Failed!");

    lcd.setCursor(0,0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0,1);
    lcd.print("Retrying...");
  }

  delay(2000);
}

// ---------------- LOOP ----------------
void loop() {

  // -------- WiFi Auto Reconnect --------
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Lost!");

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WiFi Lost!");
    lcd.setCursor(0,1);
    lcd.print("Reconnecting");

    connectWiFi();
    return;
  }

  // -------- Ultrasonic Sensor --------
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH, 20000);

  if (duration == 0) {
    Serial.println("No Echo");
    return;
  }

  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.println(distance);

  // -------- Status Logic --------
  if (distance < 5) {
    statusBin = "FULL";
    isFull = 1;
  } 
  else if (distance <= 12) {
    statusBin = "HALF";
    isFull = 0;
  } 
  else {
    statusBin = "EMPTY";
    isFull = 0;
  }

  // -------- LCD Display --------
  lcd.setCursor(0,0);
  lcd.print("Dist:");
  lcd.print(distance);
  lcd.print("cm   ");

  lcd.setCursor(0,1);
  lcd.print("Status:");
  lcd.print(statusBin);
  lcd.print("   ");

  // -------- Buzzer --------
  digitalWrite(buzzer, (isFull == 1) ? HIGH : LOW);

  // -------- Send Data to Server --------
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{";
  jsonData += "\"id\":" + String(dustbinID) + ",";
  jsonData += "\"is_full\":" + String(isFull);
  jsonData += "}";

  Serial.println("Sending:");
  Serial.println(jsonData);

  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
    Serial.print("Response: ");
    Serial.println(httpCode);
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();

  delay(3000);
}