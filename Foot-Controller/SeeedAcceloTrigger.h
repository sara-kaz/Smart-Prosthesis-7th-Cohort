/*****************************************************************************/
//  SeeedAcceloTrigger
//  Hardware:      Seeeduino Xiao Sense 6-Axis Accelerometer&Gyroscope
//	Author:	       Alfredo Gonzalez-Martinez
//	Date: 	       March,2023
//	Version:       v1.0
//
//  Description:
//  This library allows the Seeduino to be used like a "smart" trigger when 
//  its pitch or yaw is rotated past a given threshold. Device orientation is 
//  accounted for internally. If the device is moved too quickly, this class will
//  enter sleep mode for two seconds until the device is no longer moving quickly.
//  The rest position is reset whenever the device exits sleep mode.
//
//  Available callbacks:
//  - onPitchThresholdCallback,
//  - onPitchRestCallback,
//  - onYawThresholdCallback,
//  - onYawRestCallback
/*******************************************************************************/

#include "LSM6DS3.h"
#include "Wire.h"

class SeeedAcceloTrigger {
  private:
    LSM6DS3 imu;
    float restX, restY, restZ;
    float offsetX, offsetY, offsetZ;
    
    float pitchOffsetThreshold = 0.32;
    float pitchRestThreshold = 0.15;
    bool pitchTresholdCrossed = false;

    float yawOffsetThreshold = 0.18;
    float yawRestThreshold = 0.15;
    bool yawTresholdCrossed = false;

    typedef void (*threshold_callback_t)(float offset); // Define the callback function pointer type
    typedef void (*rest_callback_t)(); // Define the callback function pointer type

    threshold_callback_t onPitchThresholdCallback; // called when pitch threshold passed
    threshold_callback_t onYawThresholdCallback; // called when yaw threshold passed
    rest_callback_t onPitchRestCallback; // called when pitch returns to rest
    rest_callback_t onYawRestCallback; // called when yaw returns to rest
    
    //bool walking;
    bool sleep;
    unsigned long sleepStart;
    unsigned long sleepLength = 30; //Time before it wakes up
    
    //The measurement to when it defines that someone is walking (AKA goes to sleep)
    float sleepGyroThreshold = 50;

    /*
     * Start sleeping if user is moving too fast; Wake up if enough time has passed
     */
    void checkSleepConditions () {
      while (
        ((abs(imu.readFloatGyroX()) > sleepGyroThreshold || abs(imu.readFloatAccelX()) > sleepGyroThreshold) && (abs(imu.readFloatGyroY()) > sleepGyroThreshold || abs(imu.readFloatAccelY()) > sleepGyroThreshold)) ||
        ((abs(imu.readFloatGyroX()) > sleepGyroThreshold || abs(imu.readFloatAccelX()) > sleepGyroThreshold) && (abs(imu.readFloatGyroZ()) > sleepGyroThreshold || abs(imu.readFloatAccelZ()) > sleepGyroThreshold)) ||
        ((abs(imu.readFloatGyroY()) > sleepGyroThreshold || abs(imu.readFloatAccelY()) > sleepGyroThreshold) && (abs(imu.readFloatGyroZ()) > sleepGyroThreshold || abs(imu.readFloatAccelZ()) > sleepGyroThreshold))
      ) {
        //Serial.println("Walking Now");
        //walking = 1;
        sleep = true;
        sleepStart = millis();
        offsetX, offsetY, offsetZ = 0;
        delay(1000);
        return;
      }

      // wake up
      if (millis() - sleepStart  >= sleepLength) {
        if (sleep) {
          Serial.println("waking up...");
          delay(250);
          setRestOrientation();
        }
        sleep = false;
        //walking = 0;
      }
    }

  public:
    SeeedAcceloTrigger() : imu(I2C_MODE, 0x6A) {
      if (imu.begin() != 0) {
        Serial.println("Device error");
      } else {
        Serial.println("Device OK!");
      }      

      delay(500);
      setRestOrientation();
    }

    /*
    * Should be called in your code's loop function
    */
    void loop() {
      checkSleepConditions();
      if (sleep) return;

      offsetX = imu.readFloatAccelX() - restX;
      offsetY = imu.readFloatAccelY() - restY;
      offsetZ = imu.readFloatAccelZ() - restZ;

      // handle callbacks
      float pitchOffset = getPitchOffset();
      if (abs(pitchOffset) < pitchRestThreshold && pitchTresholdCrossed && onPitchRestCallback) { onPitchRestCallback(); pitchTresholdCrossed = false; }
      if (abs(pitchOffset) > pitchOffsetThreshold && !pitchTresholdCrossed && onPitchThresholdCallback) { onPitchThresholdCallback(pitchOffset); pitchTresholdCrossed = true; }

      float yawOffset = getYawOffset();
      if (abs(yawOffset) < yawRestThreshold && yawTresholdCrossed  && onYawRestCallback) { onYawRestCallback(); yawTresholdCrossed = false; }
      if (abs(yawOffset) > yawOffsetThreshold && !yawTresholdCrossed  && onYawThresholdCallback) { onYawThresholdCallback(yawOffset); yawTresholdCrossed = true; }
    }

    /*
    * Use the current orientation of the device as the new rest position   
    */
    void setRestOrientation() {
      restX = imu.readFloatAccelX();
      restY = imu.readFloatAccelY();
      restZ = imu.readFloatAccelZ();
    }

    /*
     * Get the current axis that is facing upwards, the effective roll axis.
     * @returns char 'x' || 'y' || 'z'
     */
    char getUpAxis () {
      if (abs(restX) > abs(restY) && abs(restX) > abs(restZ)) return 'x';
      if (abs(restY) > abs(restX) && abs(restY) > abs(restZ)) return 'y';
      if (abs(restZ) > abs(restX) && abs(restZ) > abs(restY))return 'z';
    }

    /*
     * Get the current pitch offset from the rest position, taking into account the current up (roll) angle.
     */
    float getPitchOffset () {
      char upAxis = getUpAxis();
      if (upAxis == 'x') return offsetZ;
      if (upAxis == 'y') return offsetX;
      return offsetY;
    }

    /*
     * Get the current yaw offset from the rest position, taking into account the current up (roll) angle.
     */
    float getYawOffset () {
      char upAxis = getUpAxis();
      if (upAxis == 'x') return offsetY;
      if (upAxis == 'y') return offsetZ;
      return offsetX;
    }

    // Callback Setters
    void setOnPitchThresholdCallback(threshold_callback_t callback) { onPitchThresholdCallback = callback; }
    void setOnPitchRestCallback(rest_callback_t callback) { onPitchRestCallback = callback; }
    void setOnYawThresholdCallback(threshold_callback_t callback) { onYawThresholdCallback = callback; }
    void setOnYawRestCallback(rest_callback_t callback) { onYawRestCallback = callback; }
    // Treshold Setters
    void setPitchOffsetThreshold (float threshold) { pitchOffsetThreshold = threshold; }
    void setYawOffsetThreshold (float threshold) { yawOffsetThreshold = threshold; }
    void setPitchRestThreshold (float threshold) { pitchRestThreshold = threshold; }
    void setYawRestThreshold (float threshold) { yawRestThreshold = threshold; }

    // SLEEP STATE GETTER
    bool getSleepState () { return sleep; }
    
    // REST GETTERS
    float getRestX () { return restX; }
    float getRestY () { return restY; }
    float getRestZ () { return restZ; }

    // OFFSET GETTERS
    float getOffsetX () { return offsetX; }
    float getOffsetY () { return offsetY; }
    float getOffsetZ () { return offsetZ; }
};