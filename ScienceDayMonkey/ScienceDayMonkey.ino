#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#define BLYNK_DEBUG

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "ea0b3dd1a2ec434696158b5c6a7cba19";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DPS_Extreme";
char pass[] = "";


// Physical Pins:
#define BUTTON_PIN  5
#define LED_PIN     2     // GPIO13
#define MOTOR_EN    14    // Enable Pin for motor 1
#define MOTOR_IN1   12    // Control pin 1 for motor 1
#define MOTOR_IN2   13    // Control pin 2 for motor 1


WidgetBridge bridge(V32);
SimpleTimer timer;

int doorTriggered = LOW;
unsigned long lastButtonUpdate = 0; // millis()-tracking button update rate
#define BUTTON_UPDATE_RATE 100 // Button update rate in ms
int lastButtonState = -1; // Keeps track of last button state push
int doorBellTimerId = 0;


void buttonUpdate(void)
{
  if (millis() - lastButtonUpdate < BUTTON_UPDATE_RATE)
    return;
  if (millis() < lastButtonUpdate)
    lastButtonUpdate = millis();

  int state = digitalRead(BUTTON_PIN); // Read button state
  // Read 5 times to make sure it wasn't noise
  for(int i = 0; i < 5; i++)
  {
    delay(1);
    if (state != digitalRead(BUTTON_PIN))
      return;
  }
  
  if (state != lastButtonState) // If the state has changed
  {
    lastButtonState = state; // Update last state

    if (state == LOW)
    {
      BLYNK_LOG("Local alarm reset");
      resetAlarm();
    }

    lastButtonUpdate = millis();
  }
}

void resetDoorBell()
{
  digitalWrite(LED_PIN, LOW);
  doorBellTimerId = 0;
  Blynk.virtualWrite(V2, 0);
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass, IPAddress(192,168,0,5), 8442);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

  // Make pins HIGH by default
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // Setup the onboard blue LED
  pinMode(LED_PIN, OUTPUT);

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
}

void motorControl()
{
  analogWrite(MOTOR_EN, doorTriggered ? 1023 : 0);
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);

  Blynk.virtualWrite(V3, doorTriggered ? 1023 : 0);
}


void doorBell()
{
  BLYNK_LOG("Door bell");

  if (doorBellTimerId == 0)
  {
    digitalWrite(LED_PIN, HIGH);
    Blynk.virtualWrite(V2, 1023);
    doorBellTimerId = timer.setTimeout(2000L, resetDoorBell);
  }
}

void alarm()
{
  BLYNK_LOG("Door open");

  doorTriggered = HIGH;
  motorControl();
}

void resetAlarm()
{
  BLYNK_LOG("Reset alarm");
  
  doorTriggered = LOW;
  motorControl();
}


// Bridge control
BLYNK_WRITE(V10) {
  // Door bell
  int pinData = param.asInt();

  if (pinData != LOW)
    doorBell();
}

BLYNK_WRITE(V11) {
  // Door open
  int pinData = param.asInt();

  if (pinData != HIGH)
    alarm();
}


// App control
BLYNK_WRITE(V20) {
  // Door bell
  int pinData = param.asInt();

  if (pinData != LOW)
    doorBell();
}

BLYNK_WRITE(V21) {
  // Door open
  int pinData = param.asInt();

  if (pinData != LOW)
    alarm();
}

BLYNK_WRITE(V22) {
  // Reset alarm
  int pinData = param.asInt();

  if (pinData != LOW)
    resetAlarm();
}

void loop()
{
  Blynk.run();
  timer.run();

  buttonUpdate();
}

