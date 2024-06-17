/**
  2023-24 Foot Button Code 
  Written by: Gerbert Funes

  Depends on
  https://github.com/poelstra/arduino-multi-button  
 */

#include <MultiButton.h>

MultiButton bigToe;
MultiButton smallToe;

// Toes States
bool lastBigToeState = false;
bool lastSmallToeState = false;
bool lastSmallSeedToeState = false;
bool bigToePressed = false;
bool smallToePressed = false;
int bigToeValue = 0;
int smallToeValue = 0;
int buttonValueCounter = 0;

// Used to handle clicking one button after another to trigger event.
bool bigToeClicked = false;
bool smallToeClicked = false;
unsigned long lastBigToeClick;
unsigned long lastSmallToeClick;
int nextClickMSThreshold = 2000;

// Finger Servo Motor calibration
float fingerPos = 160;
float ringPinkPos = 0;
int minFingerMotorPos = 0;
int maxFingerMotorPos = 160;
int minThumbMotorPos = 80;

//Thumb Movement 
int thumbBaseMovement = 0;
int thumbBaseDefault = 30;
int thumbMovement = 0;
int thumbBaseGripMax = 0;
int thumbBasePinchMax = 40;
int thumbBaseTripodMax = 60;
int thumbBasePointMax = 90;

// Finger Pins
  // Thumb
  int thumbPin = 32;
  //int thumbPin = 15;
  Servo thumbServo;

  // Thumb
  int thumbBasePin = 5;
  Servo thumbBaseServo;

  // Index
  int indexPin = 25;
  Servo indexServo;

  // Middle
  int middlePin = 26;
  Servo middleServo;

  // Ring
  int ringPin = 23;
  Servo ringServo;

  // Pinky
  int pinkPin = 27;
  Servo pinkServo;

// Maximum number of finger poses
int fingerType = 0;
int maxFingerTypes = 3;

void processToeButtons() {
  /**
 Changing Modes:
  - You can change mode by either pressing the big toe first and then the small toes or you could press the small toes first and then the big toe. This combination can be in any order, the mode will still change to the one thats next in the order no matter which one you chose to do
*/
  bigToe.update(lastBigToeState);
  smallToe.update(lastSmallToeState);

  if (bigToe.isSingleClick()) {
    lastBigToeClick = millis();
    bigToeClicked = true;
  }
  if (smallToe.isSingleClick()) {
    lastSmallToeClick = millis();
    smallToeClicked = true;
  }

  if (bigToeClicked) {
    if ((millis() - lastBigToeClick) < nextClickMSThreshold) {
      if (smallToe.isSingleClick()) {
        fingerType = fingerType + 1;
        Serial.print("Finger Mode = ");
        WebSerial.print("Finger Mode = ");
        WebSerial.println(fingerType);
        Serial.println(fingerType);
        
        if (fingerType > maxFingerTypes) {
          fingerType = 0;
          Serial.print("Finger Mode = ");
          WebSerial.print("Finger Mode = ");
          WebSerial.println(fingerType);
          Serial.println(fingerType);
        }
        bigToeClicked = false;
        smallToeClicked = false;
        return;
      }
    }
  }

  if (smallToeClicked) {
    if ((millis() - lastSmallToeClick) < nextClickMSThreshold) {
      if (bigToe.isSingleClick()) {
        fingerType = fingerType + 1;
        Serial.print("Finger Mode = ");
        WebSerial.print("Finger Mode = ");
        WebSerial.println(fingerType);
        Serial.println(fingerType);
        if (fingerType > maxFingerTypes) {
          fingerType = 0;
          Serial.print("Finger Mode = ");
          WebSerial.print("Finger Mode = ");
          WebSerial.println(fingerType);
          Serial.println(fingerType);
        }
        bigToeClicked = false;
        smallToeClicked = false;
        return;
      }
    }
  }

  /**
 Gripping:
    - The gripping is done by pressing and holding the big toe button. Each different gripping pose will only actuate the respective fingers. Adding a new pose is simple. Increase the value of @param maxFingerTypes at the beginning and then add your new pose to the new fingerType number. Ideally, for each new grip pose added, you want to add a release pose.

    - For controlling the thumb. @param thumbMovement controls how much it closes onto itself. Adjust the @param thumbBase_POSE_Max value in order to change how much the thumb moves inwards towards the palm. While @param thumbBaseMovement controls the speed of the last parameter mentioned. The thumb was given a default starting position in order to combat transmission lag. @param thumbBaseDefault is where you can adjust starting position. 
*/
  // Full Grip
  if (bigToeValue == 1 && fingerType == 0) {
    Serial.println("Grip");
    WebSerial.println("Grip");

    if (thumbBaseMovement <= thumbBaseGripMax){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBaseGripMax;
    }

    // Finger Movement
    indexServo.write(fingerPos);
    middleServo.write(fingerPos);
    pinkServo.write(ringPinkPos);
    ringServo.write(ringPinkPos);
    fingerPos = fingerPos - 5;
    ringPinkPos = ringPinkPos + 5;

    // Thumb Movement
    thumbServo.write(thumbMovement);
    //thumbBaseServo.write(thumbBaseMovement);
    thumbBaseMovement = thumbBaseMovement - 1;
    thumbMovement = thumbMovement - 2;
    if (fingerPos < minFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = minFingerMotorPos;
      ringPinkPos = maxFingerMotorPos;
      thumbMovement = minThumbMotorPos;
    }
  }

  // Pinch
  if (bigToeValue == 1 && fingerType == 1) {
    Serial.println("Grip_Pinch");
    WebSerial.println("Grip_Pinch");

    if (thumbBaseMovement >= thumbBasePinchMax){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBasePinchMax;
    }

    if (thumbBaseMovement <= thumbBaseDefault){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBaseDefault;
    }

    indexServo.write(fingerPos);
    //middleServo.write(fingerPos);
    //ringPinkServo.write(fingerPos);
    
    // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);
    
    // Counters
    fingerPos = fingerPos - 5;
    ringPinkPos = ringPinkPos + 5;
    thumbBaseMovement = thumbBaseMovement + 1;
    thumbMovement = thumbMovement - 2;
    Serial.println(thumbBaseMovement);

    if (fingerPos < minFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = minFingerMotorPos;
      ringPinkPos = maxFingerMotorPos;
      thumbMovement = minThumbMotorPos;
    }
  }

  // Tripod
  if (bigToeValue == 1 && fingerType == 2) {
    Serial.println("Grip_Tripod");
    WebSerial.println("Grip_Tripod");

    if (thumbBaseMovement >= thumbBaseTripodMax){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBaseTripodMax;
    }

    if (thumbBaseMovement <= thumbBaseDefault){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBaseDefault;
    }

    indexServo.write(fingerPos);
    middleServo.write(fingerPos);
    //ringPinkServo.write(fingerPos);

    // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);
    
    // Counters
    fingerPos = fingerPos - 5;
    ringPinkPos = ringPinkPos + 5;
    thumbBaseMovement = thumbBaseMovement + 1;
    thumbMovement = thumbMovement - 2;
    Serial.println(thumbBaseMovement);

    if (fingerPos < minFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = minFingerMotorPos;
      ringPinkPos = maxFingerMotorPos;
      thumbMovement = minThumbMotorPos;
    }
  }

  // Point
  if (bigToeValue == 1 && fingerType == 3) {
    Serial.println("Grip_Point");
    WebSerial.println("Grip_Point");

    if (thumbBaseMovement >= thumbBasePointMax){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBasePointMax;
    }

    if (thumbBaseMovement <= thumbBaseDefault){

      //Locks the number of movements to max value in order to not cause overflow error
      thumbBaseMovement = thumbBaseDefault;
    }

    //indexServo.write(fingerPos);
    middleServo.write(fingerPos);
    pinkServo.write(ringPinkPos);
    ringServo.write(ringPinkPos);

        // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);
    
    // Counters
    fingerPos = fingerPos - 5;
    ringPinkPos = ringPinkPos + 5;
    thumbBaseMovement = thumbBaseMovement + 1;
    thumbMovement = thumbMovement - 2;
    Serial.println(thumbBaseMovement);

    if (fingerPos < minFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = minFingerMotorPos;
      ringPinkPos = maxFingerMotorPos;
      thumbMovement = minThumbMotorPos;
    }
  }

  /**
  Releases:
    - The release is done by pressing and holding the small toe button. Each different release pose will only actuate the respective fingers. Look at how the gripping poses work in order to find out how to add new release poses. 

    - Thumb movement has nothing special here. Only that it moves back to its original starting position. 
*/

  // Release Full Grip
  if (smallToeValue == 1 && fingerType == 0) {
    Serial.println("unGrip");
    WebSerial.println("unGrip");
    
    // Finger Movement
    indexServo.write(fingerPos);
    middleServo.write(fingerPos);
    pinkServo.write(ringPinkPos);
    ringServo.write(ringPinkPos);
    fingerPos = fingerPos + 5;
    ringPinkPos = ringPinkPos - 5;

    // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);
    thumbBaseMovement = thumbBaseMovement - 1;
    thumbMovement = thumbMovement + 2;

    if (fingerPos > maxFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = maxFingerMotorPos;
      ringPinkPos = minFingerMotorPos;
      thumbMovement = maxFingerMotorPos;
    }
  }

  // Release Pinch
  if (smallToeValue == 1 && fingerType == 1) {
    Serial.println("unGrip_Pinch");
    WebSerial.println("unGrip_Pinch");

    indexServo.write(fingerPos);
    //middleServo.write(fingerPos);
    //ringPinkServo.write(fingerPos);

    // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);

    //Counters
    fingerPos = fingerPos + 5;
    ringPinkPos = ringPinkPos - 5;
    thumbBaseMovement = thumbBaseMovement - 1;
    thumbMovement = thumbMovement + 2;
    

    if (fingerPos > maxFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = maxFingerMotorPos;
      ringPinkPos = minFingerMotorPos;
      thumbMovement = maxFingerMotorPos;
    }
  }

  // Release Tripod
  if (smallToeValue == 1 && fingerType == 2) {
    Serial.println("unGrip_Tripod");
    WebSerial.println("unGrip_Tripod");

    indexServo.write(fingerPos);
    thumbServo.write(fingerPos);
    middleServo.write(fingerPos);
    //ringPinkServo.write(fingerPos);

    // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);

    //Counters
    fingerPos = fingerPos + 5;
    ringPinkPos = ringPinkPos - 5;
    thumbBaseMovement = thumbBaseMovement - 1;
    thumbMovement = thumbMovement + 2;

    if (fingerPos > maxFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = maxFingerMotorPos;
      ringPinkPos = minFingerMotorPos;
      thumbMovement = maxFingerMotorPos;
    }
  }

  // Release Point
  if (smallToeValue == 1 && fingerType == 3) {
    Serial.println("unGrip_Point");
    WebSerial.println("unGrip_Point");

    //indexServo.write(fingerPos);
    middleServo.write(fingerPos);
    pinkServo.write(ringPinkPos);
    ringServo.write(ringPinkPos);

    // Thumb Movement
    thumbServo.write(thumbMovement);
    thumbBaseServo.write(thumbBaseMovement);

    //Counters
    fingerPos = fingerPos + 5;
    ringPinkPos = ringPinkPos - 5;
    thumbBaseMovement = thumbBaseMovement - 1;
    thumbMovement = thumbMovement + 2;

    if (fingerPos > maxFingerMotorPos) {

      //Locks the number of movements to max value in order to not cause overflow error
      fingerPos = maxFingerMotorPos;
      ringPinkPos = minFingerMotorPos;
      thumbMovement = maxFingerMotorPos;
    }
  }
}