#include <esp_now.h>
#include <WiFi.h>
#include "driver/rtc_io.h"

#define BUTTON_PIN_BITMASK 0x30  // GPIOs 4 and 5
#define LED_BUILTIN 15

RTC_DATA_ATTR int bootCount = 0;

int timeElapsed = 0;
int timeStartStopwatch = 0;
int timeEndStopwatch = 0;
int batteryVoltage = 0;

// Receiver MAC Address
// ESP Board MAC Address:  C8:F0:9E:F6:8C:FC (Green Arm)
uint8_t broadcastAddress[] = { 0xCC, 0xDB, 0xA7, 0x15, 0x01, 0x98 };

int btnPins[] = { 5, 4 };                                       // Two buttons
const int numBtnPins = (sizeof(btnPins) / sizeof(btnPins[0]));  // count the number of buttons, incase we want to add more
short btnValues[numBtnPins];
short prevBtnValues[numBtnPins];

// ESP NOW button data
typedef struct struct_message {
  float footButton;
  float buttonValue;
} struct_message;

// Create a struct_message called buttonData
struct_message buttonData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

/*
Method to print the GPIO that triggered the wakeup
*/
void print_GPIO_wake_up() {
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println((log(GPIO_reason)) / log(2), 0);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // To visually know the server is on

  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Set button pin modes
  for (int i = 0; i < numBtnPins; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
  }


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
}

void loop() {

  //Timer start for the sleep trigger
  timeStartStopwatch = millis();

  //Check the buttons presses and transmit
  processButtons();

  //Time check to see if a button has been pressed after the @param timeElapsed is above 20seconds
  timeElapsed = timeStartStopwatch - timeEndStopwatch;
  //Serial.println(timeElapsed);

  //Check sleep condition
  goToSleep();

  //Check Battery Voltage
  checkBattVoltage();
}

//Foot Buttons
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

    buttonData.footButton = i;
    buttonData.buttonValue = !btnValues[i];

    prevBtnValues[i] = btnValues[i];

    sendMessage();

    //Timer for the end of a button press and check for the sleep schedule event
    timeEndStopwatch = millis();
  }
}

void checkBattVoltage() {
  // read the analog / millivolts value for pin 2:
  int analogValue = analogRead(0);
  int analogVolts = analogReadMilliVolts(0);

  // print out the values you read:
  //Serial.print("ADC analog value = ");
  //Serial.println(analogValue);
  //Serial.print("ADC millivolts value = ");
  //Serial.println(analogVolts);
  //Serial.println("mV");
  // Please adjust the calculation coefficient according to the actual measurement.
  //Serial.print("BAT millivolts value = ");
  batteryVoltage = analogVolts * 2.1218 + 1000;
  //Serial.print(batteryVoltage);
  //Serial.println("mV");
  //Serial.println("--------------");
  //delay(500);
}

void goToSleep() {
  if (batteryVoltage < 5000) {
    if (timeElapsed > 20000) {

      Serial.println("Going to sleep");

      //Print the wakeup reason for ESP32
      print_wakeup_reason();

      //Print the GPIO used to wake up
      print_GPIO_wake_up();

      /*
  First we configure the wake up source
  We set our ESP32 to wake up for an external trigger.
  There are two types for ESP32, ext0 and ext1 .
  ext0 uses RTC_IO to wakeup thus requires RTC peripherals
  to be on while ext1 uses RTC Controller so doesnt need
  peripherals to be powered on.
  Note that using internal pullups/pulldowns also requires
  RTC peripherals to be turned on.
  */

      //esp_deep_sleep_enable_gpio_wakeup(5,ESP_GPIO_WAKEUP_GPIO_HIGH); //1 = High, 0 = Low

      //If you were to use ext1, you would use it like
      esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_LOW);

      rtc_gpio_isolate(GPIO_NUM_5);
      rtc_gpio_isolate(GPIO_NUM_4);
      esp_deep_sleep_start();
    }
  }
}

void sendMessage() {
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&buttonData, sizeof(buttonData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
}

