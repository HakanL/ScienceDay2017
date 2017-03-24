#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
//#define BLYNK_DEBUG

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "151dab6870d34b5aa5e58ac9b25178a4";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "DPS_Extreme";
char pass[] = "";


#define BUTTON_PIN  0
#define CONTACT_PIN 16

WidgetBridge bridge(V32);

SimpleTimer timer;

unsigned long lastButtonUpdate = 0; // millis()-tracking button update rate
unsigned long lastContactUpdate = 0; // millis()-tracking button update rate
#define BUTTON_UPDATE_RATE 100 // Button update rate in ms
int lastButtonState = -1; // Keeps track of last button state push
int lastContactState = -1; // Keeps track of last button state push


void buttonUpdate(void)
{
  if (millis() - lastButtonUpdate < BUTTON_UPDATE_RATE)
    return;
  if (millis() < lastButtonUpdate)
    lastButtonUpdate = millis();

  int state = digitalRead(BUTTON_PIN); // Read button state
  if (state != lastButtonState) // If the state has changed
  {
    lastButtonState = state; // Update last state

    if (state)
      Blynk.virtualWrite(V1, 0);
    else
      Blynk.virtualWrite(V1, 1023);

    bridge.virtualWrite(V10, !state);

    lastButtonUpdate = millis();
  }
}

void contactUpdate(void) 
{
  if (millis() - lastContactUpdate < BUTTON_UPDATE_RATE)
    return;
  if (millis() < lastContactUpdate)
    lastContactUpdate = millis();

  int state = digitalRead(CONTACT_PIN); // Read button state
  if (state != lastContactState) // If the state has changed
  {
    lastContactState = state; // Update last state

    if (state)
      Blynk.virtualWrite(V2, 1023);
    else
      Blynk.virtualWrite(V2, 0);

    bridge.virtualWrite(V11, state);

    lastContactUpdate = millis();
  }
}

void test()
{
  BLYNK_LOG("Test");
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass, IPAddress(192,168,0,5), 8442);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

  // Make pins inputs and with pull up/down enabled
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CONTACT_PIN, INPUT_PULLDOWN_16);

  timer.setInterval(1000L, test);

  while (Blynk.connect() == false) {
    // Wait until Blynk is connected
  }

  bridge.digitalWrite(V10, LOW);
  bridge.digitalWrite(V11, LOW);
}

BLYNK_CONNECTED() {
  bridge.setAuthToken("ea0b3dd1a2ec434696158b5c6a7cba19");
}

// App control
BLYNK_WRITE(V20) {
  // Door bell
  int pinData = param.asInt();

  Blynk.virtualWrite(V1, pinData ? 1023 : 0);
  bridge.virtualWrite(V10, pinData);
}


void loop()
{
  Blynk.run();
  timer.run();

  buttonUpdate();
  contactUpdate();
}
