/*
  2023-24 Arm Code
  Written by: Gerbert Funes, Sara Ali

  Depends on
  https://github.com/Goober56/ESP32Servo
  https://www.arduino.cc/reference/en/libraries/arduinoble/
  */

#include <SPI.h>
#include <Wire.h>
#include <esp_now.h>
#include <ESP32Servo.h>
#include <ArduinoBLE.h>
#include "armServer.h"
#include "WebSerial.h"
#include "processToeButtons.h"
#include "wristRotations.h"
#include "Arduino.h"

#define LED_BUILTIN 2

/**
* Reset Function to restart arm through serial
*/
void (*resetFunc)(void) = 0;

float gyroState[3];  // x, y, z

bool systemActive = false;

// Define the structure to store the received data
struct payloadStruct {
  float footButton;
  float buttonValue;
  float footPitch;
  float pitchValue;
  float footYaw;
  float yawValue;
} receivedData;

// Define the UUID of the characteristic that carries the message
// Original characteristic id: "19b10001-e8f2-537e-4f6c-d104768a1214"
const char *messageCharacteristicUUID = "19b10001-e8f2-537e-4f6c-d104768a1214";

// ESP NOW button data
typedef struct StructMessage {
  float footButton;
  float buttonValue;
} StructMessage;

// Create a struct_message called buttonData
StructMessage buttonData;

/*
   Ingest button from Foot Controller Sleeve
 */
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  memcpy(&buttonData, data, sizeof(buttonData));
  //Button Message
  ButtonAssign(buttonData.footButton, buttonData.buttonValue);
}

void setup() {
  /** 
* Setting up ESP Port
*/
  Serial.begin(115200);
  Serial.println("Setup begun");

  /**
* Setting up the server
* Always setup the server before ESPNOW
*/
  pinMode(LED_BUILTIN, OUTPUT);
  esp32ServerStart();

  /**
* ESP NOW
*/

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  // Register the callback function for received data
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  /**
* BLE
*/

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1)
      ;
  } else {
    Serial.println("Bluetooth® Low Energy Central - Receive Message");
  }

  BLE.scan();

  /**
*Servos
*/
  // Finger Servos
  thumbServo.attach(thumbPin);
  indexServo.attach(indexPin);
  middleServo.attach(middlePin);
  pinkServo.attach(pinkPin);
  thumbBaseServo.attach(thumbBasePin);
  ringServo.attach(ringPin);

  // Rotation Servos
  rotationServo.attach(rotationPin);
  rotationServo.write(rotationMotorPos);

  // Bending Servos
  bendingServo.attach(bendingPin);
  bendingServo.write(bendingMotorPos);
}

void loop() {

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() == "SEEED") {
      BLE.stopScan();
      explorePeripheral(peripheral);
    }
  }
}

void explorePeripheral(BLEDevice peripheral) {
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Discovering attributes ...");

  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    BLE.scan();
    loop();
    return;
  }

  Serial.println();
  Serial.print("Device name: ");
  Serial.println(peripheral.deviceName());
  Serial.print("Server Name: ");
  Serial.println(peripheral.localName());

  for (int i = 0; i < peripheral.serviceCount(); i++) {
    BLEService service = peripheral.service(i);
    exploreService(service);
  }
}

void exploreService(BLEService service) {
  Serial.println();
  Serial.print("Service ");
  Serial.println(service.uuid());

  for (int i = 0; i < service.characteristicCount(); i++) {
    BLECharacteristic characteristic = service.characteristic(i);
    exploreCharacteristic(characteristic);
  }
}

void exploreCharacteristic(BLECharacteristic characteristic) {
  String character = characteristic.uuid();
  Serial.print("\tCharacteristic ");
  Serial.println(character);

  if (character == messageCharacteristicUUID) {
    Serial.println("Match!");
    Serial.println("ConnectedController: NoTapeController");
    delay(20);
    while (characteristic.canRead() & BLE.connected()) {
      characteristic.read();
      if (characteristic.valueLength() == sizeof(payloadStruct)) {
        const uint8_t *receivedDataBytes = characteristic.value();
        memcpy(&receivedData, receivedDataBytes, sizeof(payloadStruct));
        readBleMessages(receivedData);
        processToeButtons();
        ElegantOTA.loop();
      }
    }
    Serial.println("Cannot Read :(");
    BLE.scan();
    loop();
  } else {
    Serial.println("No Match");
  }
}

/**
  In order to combine both the USB-C insole and the wireless insole into one system this function was created. Both the foot sleeve and insole send their button data to this function. This functions job is to get rid of conflicting data between both controllers. It will allow the arm to switch between the insole and the wireless foot sleeve without having to reflash the arm. 
 */
void ButtonAssign(int toeButton, int toeButtonValue){
  if (toeButton == 0){
    lastBigToeState = toeButtonValue;
    bigToeValue = toeButtonValue;
    //Serial.print("Big Toe");
    //Serial.println(toeButtonValue);
  }

  if (toeButton == 1){
    lastSmallToeState = toeButtonValue;
    smallToeValue = toeButtonValue;
    //Serial.println("Small Toe");
    //Serial.print(toeButtonValue);
  }
}

/**
  Ingest button and axis messages from Foot Controller Unit
 */
void readBleMessages(payloadStruct data) {

  //Button Message
  ButtonAssign(data.footButton,data.buttonValue);

  //Rotation Message
  if (data.pitchValue < 0) {
    //Serial.println("Rotate 1, Pitch Value: ");
    //Serial.println(data.pitchValue);
    rotationDirection = -1;
  } else if (data.pitchValue > 0) {
    //Serial.println("Rotate -1, Pitch Value: ");
    //Serial.println(data.pitchValue);
    rotationDirection = 1;
  } else {
    //Serial.println(0);
    rotationDirection = 0;
  }

  //Bending Message
  if (data.yawValue < 0) {
    //Serial.println("Bend -1, Yaw  Value: ");
    //Serial.println(data.yawValue);
    bendingDirection = -1;
  } else if (data.yawValue > 0) {
    //Serial.println("Bend 1, Yaw  Value: ");
    //Serial.println(data.yawValue);
    bendingDirection = 1;
  } else {
    //Serial.println("Bend 0 ");
    bendingDirection = 0;
  }

  moveWristRotation(rotationDirection);
  moveWristBend(bendingDirection);
}

/**
  Setting up the server
    ToDo: Add a way to turn on and off the server on command
     -WiFi.softAPdisconnect(wifioff)
     -https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
*/

void esp32ServerStart(void) {
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }

  // Configures static IP address
  WiFi.softAPConfig(ESP32IP, gateway, subnet);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  digitalWrite(LED_BUILTIN, HIGH);  // To visually know the server is on

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
    //Serial.println("Root Exists");
  });

  ElegantOTA.begin(&server);
  WebSerial.begin(&server);
  WebSerial.msgCallback(webSerialMessage);
  server.begin();
  Serial.println("HTTP server started");
}

/**
  WebSerial Commands
   How it works: Assign a text that you are going to input into the WebSerial  page. Then assign the specific command that is going to be ran when the text is processed
*/

void webSerialMessage(uint8_t *data, size_t len) {
  WebSerial.println("Data Received!");
  String Data = "";
  for (int i = 0; i < len; i++) {
    Data += char(data[i]);
  }
  //Add Commands here to control from the web serial server
  WebSerial.println(Data);
  if (Data == "LED ON") digitalWrite(LED_BUILTIN, HIGH);
  if (Data == "LED OFF") digitalWrite(LED_BUILTIN, LOW);
  if (Data == "Restart Arm") resetFunc();
}
