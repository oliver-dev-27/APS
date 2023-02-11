//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// APS Slave Code
// v0.0.4
// Firebase & RFID Test Code - Merge Firebase Code and RFID Code
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
#define VERSION "v0.0.4"
#define FEATURE "Firebase & RFID Test Code - Merge Firebase Code and RFID Code"

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
const int RFID_SS_PIN = 2;
const int RFID_RST_PIN = 0;

// VARIABLES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// RFID
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
String rfidString = "";

// SETUP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
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
    if (!rfidRead())
      return;

    rfidGet();
    rfidShowToSerial();
  }
}

// FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

    if (strstr(devID, DEV_ID)) {
      Serial.println(DEV_ID " FOUND! No need to register.");
    } else if (strstr(devID, "path not exist")) {
      Serial.println(DEV_ID " NOT FOUND! Registration in progress...");
      FirebaseJson json;

      json.set("id", F(DEV_ID));
      json.set("availability", 0);
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

bool rfidRead() {
  if (!rfid.PICC_IsNewCardPresent())
    return false;

  if (!rfid.PICC_ReadCardSerial())
    return false;

  return true;
}

void rfidGet() {
  rfidString = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidString.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    rfidString.concat(String(rfid.uid.uidByte[i], HEX));
  }

  rfidString.toUpperCase();
}

void rfidShowToSerial() {
  Serial.println();
  Serial.print(F(" UID tag :"));
  Serial.print(rfidString);
  Serial.println();
}
