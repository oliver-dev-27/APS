//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// APS Slave Code
// v0.0.8
// Firebase & RFID Test Code - Check RFID on Firebase and Buzz if unverified
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// LIBRARIES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// WIFI
#include <Arduino.h>
#if defined(ESP32) || defined(PICO_RP2040)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

// Firebase
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// RFID
#include <SPI.h>
#include <MFRC522.h>

// CONSTANTS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// TITLE
#define TITLE "APS Slave Code"
#define VERSION "v0.0.8"
#define FEATURE "Check RFID on Firebase and Buzz if unverified"

// WIFI
#define WIFI_SSID "********"
#define WIFI_PASSWORD "********"

// Firebase
#define API_KEY "********"
#define DATABASE_URL "********"
#define USER_EMAIL "********"
#define USER_PASSWORD "********"

#define DEV_ID "S000001"

// RFID
const int RFID_SS_PIN = 2;   // D4
const int RFID_RST_PIN = 0;  // D3

// RGB
const int RGB_G_PIN = 5;  // D1
const int RGB_R_PIN = 4;  // D2

// Buzzer
const int BUZZER_PIN = 15;  // D8

// VARIABLES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// RFID
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
String rfidDataContent = "";
unsigned long rfidAccessTime = 0;
bool rfidIsRemoved = false;
bool rfidIsVerified = false;
bool rfidDetectedOnce = false;

// SETUP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
  rgbSetup();
  buzzerSetup();
  Serial.begin(115200);
  mcuShowVersion();
  rfidSetup();
  wifiConnect();
  firebaseSetup();
  poleRegistration();
}

// LOOP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  if (Firebase.ready()) {

    detectRfidRemovalFromReader();
    lightGreenLED();
    sendUnoccupied();
    resetBuzzer();

    if (!rfidDetectedFromReader())
      return;
    else
      rfidAccessTime = millis();

    if (!rfidDetectedOnce) {
      rfidDetectedOnce = true;
      readRfidContent();
      showRfidTagToSerial();
      verifyRfid();
      lightRedLED();
      buzzBuzzer();
      sendOccupied();
    }
  }
}

// FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// MAIN
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void detectRfidRemovalFromReader() {
  if (rfidDataContent != "" && (millis() - rfidAccessTime > 10)) {
    rfidIsRemoved = true;
    Serial.println("RFID Card Removed!");

    rfidDataContent = "";
    rfidDetectedOnce = false;
  } else {
    rfidIsRemoved = false;
  }
}

void lightGreenLED() {
  if (rfidIsRemoved) {
    Serial.println("Light Green LED!");
    rgbGreenOn();
  }
}

void sendUnoccupied() {
  if (rfidIsRemoved && rfidIsVerified) {
    Serial.println("Availability = 1");
    FirebaseJson json;
    json.set("availability", 1);
    Serial.printf("Set " DEV_ID " availability... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/poles/" DEV_ID), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}

void resetBuzzer() {
  if (rfidIsRemoved && !rfidIsVerified) {
    Serial.println("Buzzer Off!");
    buzzerOff();
  }
}

bool rfidDetectedFromReader() {
  if (!rfid.PICC_IsNewCardPresent())
    return false;

  if (!rfid.PICC_ReadCardSerial())
    return false;

  return true;
}

void readRfidContent() {
  rfidDataContent = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidDataContent.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    rfidDataContent.concat(String(rfid.uid.uidByte[i], HEX));
  }

  rfidDataContent.toUpperCase();
}

void showRfidTagToSerial() {
  Serial.println();
  Serial.print(F("UID Tag:"));
  Serial.print(rfidDataContent);
  Serial.println();
}

void verifyRfid() {
  char rfidCharArray[16] = "";

  String path = "/vehicles/" + rfidDataContent + "/rfid";
  strcpy(rfidCharArray, Firebase.RTDB.getString(&fbdo, path.c_str()) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

  Serial.print("UID From Firebase:");
  Serial.println(rfidCharArray);

  if (strcmp(rfidCharArray, rfidDataContent.c_str()) == 0) {
    rfidIsVerified = true;
    Serial.println(rfidDataContent + " RFID found!");
  } else {
    rfidIsVerified = false;
    Serial.println(rfidDataContent + " RFID not found!");
  }
}

void lightRedLED() {
  Serial.println("Light Red LED!");
  rgbRedOn();
}

void buzzBuzzer() {
  if (!rfidIsVerified) {
    Serial.println("Buzzer On!");
    buzzerOn();
  }
}

void sendOccupied() {
  if (rfidIsVerified) {
    Serial.println("Availability = 0");
    FirebaseJson json;
    json.set("availability", 0);
    Serial.printf("Set " DEV_ID " availability... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/poles/" DEV_ID), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}

// VERSION
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void mcuShowVersion() {
  Serial.println();
  Serial.println(F(TITLE));
  Serial.println(F(VERSION));
  Serial.println(F(FEATURE));
  Serial.println();
}

// WIFI
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void wifiConnect() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Firebase
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void firebaseSetup() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

#if defined(ESP8266)
  fbdo.setBSSLBufferSize(2048, 2048);
#endif

  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;
}

void poleRegistration() {
  if (Firebase.ready()) {
    char devID[24] = "";
    strcpy(devID, Firebase.RTDB.getString(&fbdo, F("/poles/" DEV_ID "/id")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

    Serial.println(devID);

    if (strcmp(devID, DEV_ID) == 0) {
      Serial.println(DEV_ID " FOUND! No need to register.");
    } else if (strstr(devID, "path not exist")) {
      Serial.println(DEV_ID " NOT FOUND! Registration in progress...");
      FirebaseJson json;

      json.set("id", F(DEV_ID));
      json.set("availability", 1);
      json.set("location/lat", F("00.000000"));
      json.set("location/lon", F("000.000000"));
      Serial.printf("Set " DEV_ID "... %s\n", Firebase.RTDB.set(&fbdo, F("/poles/" DEV_ID), &json) ? "ok" : fbdo.errorReason().c_str());
    }
  }
}

// RFID
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void rfidSetup() {
  SPI.begin();
  rfid.PCD_Init();
}

// LED
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void rgbSetup() {
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_R_PIN, OUTPUT);

  rgbGreenOn();
}

void rgbGreenOn() {
  digitalWrite(RGB_G_PIN, HIGH);
  digitalWrite(RGB_R_PIN, LOW);
}

void rgbRedOn() {
  digitalWrite(RGB_R_PIN, HIGH);
  digitalWrite(RGB_G_PIN, LOW);
}

// Buzzer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void buzzerSetup() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void buzzerOn() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerOff() {
  digitalWrite(BUZZER_PIN, LOW);
}
