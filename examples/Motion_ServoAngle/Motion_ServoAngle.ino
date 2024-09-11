/*
 * Overview:
 * This code demonstrates the CodeCell's Rotation Sensing features.
 * In this example, the CodeCell continuously reads the angular rotations 
 * Every 100ms it prints the Roll, Pitch, and Yaw angles on the Serial Monitor
 * Feel free to tweak the code with your own creative ideas!
 */

#include <CodeCell.h>
#include <ESP32Servo.h>

CodeCell myCodeCell;
Servo myservo;  // create servo object to control a servo

float Roll = 0.0;
float Pitch = 0.0;
float Yaw = 0.0;
int servo_angle = 0;

void setup() {
  Serial.begin(115200); /* Set Serial baud rate to 115200. Ensure Tools/USB_CDC_On_Boot is enabled if using Serial. */

  myCodeCell.Init(MOTION_ROTATION); /*Initializes Rotation Sensing*/
  myservo.attach(1);  // attaches the servo on pin 9 to the servo object
}

void loop() {
  if (myCodeCell.Run()) {
    myCodeCell.Motion_RotationRead(Roll, Pitch, Yaw);
    servo_angle = abs((int)Pitch);
    servo_angle = (180 - servo_angle);

    if(servo_angle> 60){
      servo_angle = 60;
    }
    else if (servo_angle < 0){
      servo_angle = 0;
    }
    else{
      /*skip*/
    }

    Serial.println(servo_angle);
    myservo.write(servo_angle);
  }
}
