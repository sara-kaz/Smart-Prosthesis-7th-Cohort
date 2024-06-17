/**
  2023-24 Wrist Movement Code
  Written By: Gerbert Funes
 */

// Initializing
short rotationDirection = 0;
short bendingDirection = 0;
short pos = 0;

// Wrist Rotation Servo
int rotationPin = 22; //22
Servo rotationServo;
int minRotationMotorPos = 30;
int maxRotationMotorPos = 130;
float rotationMotorPos = (minRotationMotorPos + maxRotationMotorPos) / 2;

// Wrist Bending Servo
int bendingPin = 21;
Servo bendingServo;
int minBendingMotorPos = 10;
int maxBendingMotorPos = 40;
float bendingMotorPos = (minBendingMotorPos + maxBendingMotorPos) / 2;
//float bendingMotorPos = 50; //Assigning a set value for testing

/**
 * Given a direction (-1 or 1), bend wrist down or up
 * @param direction -1 for down, 1 for up
 */
void moveWristBend(short direction) {

  if (direction == 0) {
    //Serial.println("Stop Bend");
    //WebSerial.println("Stop Bend");
    return;
  }

  if (direction == -1){
    Serial.println("Bend_Down");
    WebSerial.println("Bend Down");

          bendingServo.write(bendingMotorPos);

          bendingMotorPos = bendingMotorPos + 2; //Counter for the position movements

          if (bendingMotorPos > maxBendingMotorPos){

          //Locks the number of movements to max value in order to not cause overflow error 
            bendingMotorPos = maxBendingMotorPos; 
          }                  
      }
      
  if (direction == 1){
    Serial.println("Bend_Up");
    WebSerial.println("Bend Up");

          bendingServo.write(bendingMotorPos);
          //delay(20); // waits 15 ms for the servo to reach the position 

          bendingMotorPos = bendingMotorPos - 2; //Counter for the position movements

          if (bendingMotorPos <= minBendingMotorPos){

          //Locks the number of movements to max value in order to not cause overflow error 
            bendingMotorPos = minBendingMotorPos; 
          }                    
      }
}


/**
 * Given a direction (-1 or 1), rotate wrist left or right
 * @param direction -1 for left, 1 for right
 */
void moveWristRotation(short direction) {

  if (direction == 0) {
    //Serial.println("Stopped");
    //WebSerial.println("Stopped");
    return;
  }

  if (direction == 1){
    Serial.println("Rotate_Left");
    WebSerial.println("Rotate Left");

          rotationServo.write(rotationMotorPos);

          rotationMotorPos = rotationMotorPos + 2; //Counter for the position movements

          if (rotationMotorPos > maxRotationMotorPos){

          //Locks the number of movements to max value in order to not cause overflow error 
            rotationMotorPos = maxRotationMotorPos; 
          }               
      }
      
  if (direction == -1){
    Serial.println("Rotate_Right");
    WebSerial.println("Rotate Right");

          rotationServo.write(rotationMotorPos);

          rotationMotorPos = rotationMotorPos - 2; //Counter for the position movements

          if (rotationMotorPos <= minRotationMotorPos){

          //Locks the number of movements to max value in order to not cause overflow error 
            rotationMotorPos = minRotationMotorPos; 
          }
             
      }
}