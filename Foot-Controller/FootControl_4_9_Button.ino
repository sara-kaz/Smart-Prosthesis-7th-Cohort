/**
2023-24 Foot Control code 
Written by: Alfredo Gonzalez-Martinez, Sara Ali, Gerbert Funes

Button Pins:
Four toes: 8
Big toe: 9 

Depends on
https://github.com/Seeed-Studio/Seeed_Arduino_LSM6DS3
Intallation instuctions for SeeedBoards - https://wiki.seeedstudio.com/XIAO_BLE/
*/

#include <Arduino.h>
#include <SPI.h>
#include <ArduinoBLE.h>
#include "SeeedAcceloTrigger.h"
#include "BatteryCharger.h"

SeeedAcceloTrigger* acceloTrigger;

int btnPins[] = { D9, D8 };                                     // toe buttons
const int numBtnPins = (sizeof(btnPins) / sizeof(btnPins[0]));  // count the number of buttons, incase we want to add more
short btnValues[numBtnPins];
short prevBtnValues[numBtnPins];

bool systemActive = true;
bool debugMode = true;

// Define the structure to store the received data
struct payloadStruct {
  float footButton;
  float buttonValue;
  float footPitch;
  float pitchValue;
  float footYaw;
  float yawValue;
} payloadData;

BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic customCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214", BLENotify | BLEWrite | BLERead, sizeof(payloadStruct));

void setup() {

  if (debugMode) {
    Serial.begin(115200);
  }

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth Low Energy module failed!");
    while (1)
      ;
  }

  BLE.setLocalName("SEEED");
  BLE.setAdvertisedService(customService);

  customService.addCharacteristic(customCharacteristic);
  BLE.addService(customService);

  BLE.advertise();
  Serial.println("BLE Message");

  // Set button pin modes
  for (int i = 0; i < numBtnPins; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
  }

  //Setting up battery charger
  setupBatteryLevel();

  // Set IMU callbacks
  acceloTrigger = new SeeedAcceloTrigger();
  acceloTrigger->setOnPitchRestCallback(onPitchRestCallback);
  acceloTrigger->setOnPitchThresholdCallback(onPitchThresholdCallback);
  acceloTrigger->setOnYawRestCallback(onYawRestCallback);
  acceloTrigger->setOnYawThresholdCallback(onYawThresholdCallback);
  acceloTrigger->getYawOffset();


  delay(1000);

  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
  }

  delay(3000);
}

void loop() {
  BLEDevice central = BLE.central();

  monitor_battery_level();

  acceloTrigger->loop();
  systemActive = !acceloTrigger->getSleepState();
  if (!systemActive) return;  // nothing to do if system is off

  processButtons();
}

/************************************************************************
 * Foot Buttons 
 */
void processButtons() {
  // Read button values and transmit
  for (short i = 0; i < numBtnPins; i++) {
    btnValues[i] = digitalRead(btnPins[i]);
    if (prevBtnValues[i] == btnValues[i]) continue;  //if button is in same state as previously recorded, skip rest of function and restart

    // print for debugging
    Serial.print("button ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(!btnValues[i] ? "pressed" : "released");
    Serial.println(!btnValues[i]);

    payloadData.footButton = i;
    payloadData.buttonValue = !btnValues[i];

    customCharacteristic.writeValue(reinterpret_cast<uint8_t*>(&payloadData), sizeof(payloadData));

    prevBtnValues[i] = btnValues[i];
  }

  //Serial.print(digitalRead(btnPins[0]));
  //Serial.println(digitalRead(btnPins[1]));
}

/************************************************************************
 * IMU 
 */
///NOTE: Change these transmit payload functions. Either change type or index
void onPitchThresholdCallback(float offset) {
  Serial.println("*** ROTATE WRIST");
  //Serial.println(offset);
  payloadData.pitchValue = offset;
  customCharacteristic.writeValue(reinterpret_cast<uint8_t*>(&payloadData), sizeof(payloadData));
}
void onPitchRestCallback() {
  Serial.println("*** STOP ROTATE WRIST");
  //Serial.println("0");
  payloadData.pitchValue = 0;
  customCharacteristic.writeValue(reinterpret_cast<uint8_t*>(&payloadData), sizeof(payloadData));
}
void onYawThresholdCallback(float offset) {
  Serial.println("*** BEND WRIST");
  //Serial.println(offset);
  payloadData.yawValue = offset;
  customCharacteristic.writeValue(reinterpret_cast<uint8_t*>(&payloadData), sizeof(payloadData));
}
void onYawRestCallback() {
  Serial.println("*** STOP BEND WRIST");
  //Serial.println("0");
  payloadData.yawValue = 0;
  customCharacteristic.writeValue(reinterpret_cast<uint8_t*>(&payloadData), sizeof(payloadData));
}
