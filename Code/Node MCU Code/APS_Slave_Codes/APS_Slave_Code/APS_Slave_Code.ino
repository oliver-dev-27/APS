//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// APS Slave Code
// v0.0.1
// RFID Test Code - Can read RFID and display RFID to Serial
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// LIBRARIES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// RFID
#include <SPI.h>
#include <MFRC522.h>

// CONSTANTS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// DEFINES
#define TITLE "APS Slave Code"
#define VERSION "v0.0.1"
#define FEATURE "RFID Test Code - Can read RFID and display RFID to Serial"

// RFID
const int RFID_SS_PIN = 2;
const int RFID_RST_PIN = 0;

// VARIABLES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// RFID
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
String rfidString = "";

// SETUP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
  Serial.begin(115200);
  mcuShowVersion();
  rfidSetup();
}

// LOOP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  if (!rfidRead())
    return;

  rfidGet();
  rfidShowToSerial();
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
