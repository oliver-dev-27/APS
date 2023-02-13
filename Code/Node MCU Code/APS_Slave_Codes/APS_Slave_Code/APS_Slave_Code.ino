//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// APS Slave Code
// v0.0.9
// Ultrasonic Test Code - Can read Ultrasonic and display Distance to Serial
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// LIBRARIES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// CONSTANTS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// TITLE
#define TITLE "APS Slave Code"
#define VERSION "v0.0.9"
#define FEATURE "Can read Ultrasonic and display Distance to Serial"

// Ultrasonic
const int ULTRASONIC_TRIG_PIN = 16;
const int ULTRASONIC_ECHO_PIN = 16;

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.0341

float distanceCm;

// VARIABLES
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// SETUP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup() {
  ultrasonicSetup();
  Serial.begin(115200);
  mcuShowVersion();
}

// LOOP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  getDistance();
  showDistanceToSerial();
  delay(1000);
}

// FUNCTIONS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// MAIN
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void getDistance() {
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  // Clears the ULTRASONIC_TRIG_PIN
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the ULTRASONIC_TRIG_PIN on HIGH state for 10 micro seconds
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  // Reads the ULTRASONIC_ECHO_PIN, returns the sound wave travel time in microseconds
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;
}

void showDistanceToSerial() {
  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
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

// Ultrasonic
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ultrasonicSetup() {
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);  // Sets the ULTRASONIC_TRIG_PIN as an Output
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);   // Sets the ULTRASONIC_ECHO_PIN as an Input
}