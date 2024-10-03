# CodeCell

<img src="https://microbots.io/cdn/shop/files/penny_600x.png?v=1719926378" alt="CodeCell" width="300" align="right" style="margin-left: 20px;">

CodeCell is a penny-sized board that helps you miniaturize your DIY robots, wearables, and IoT projects with ease! Designed for all makers, it features an Arduino-friendly ESP32-C3 Wi-Fi & BLE module, programmable through a standard USB-C cable ~ which can also charge a LiPo battery that easily plugs into the onboard connector. That’s not all! We've created a CodeCell library to make it easier to interact with the onboard sensors ~ its Light Sensor can detect darkness and measure proximity up to 20 cm! While the optional Motion Sensor features a 9-axis sensor-fusion IMU to measure angular rotations, step counts, personal activity, tapping, gravity, acceleration, rate of rotation, magnetic fields, and more!

## Getting started with the CodeCell Library:

### Init()
- To initialize the CodeCell, use the `myCodeCell.Init()` function with one or more of the predefined macros. Each macro corresponds to a specific sensing function. Here are the available macros:
  
  - `LIGHT`                          // Enables Light Sensing
  - `MOTION_ACCELEROMETER`           // Enables Accelerometer Sensing
  - `MOTION_GYRO`                    // Enables Gyroscope Sensing
  - `MOTION_MAGNETOMETER`            // Enables Magnetometer Sensing
  - `MOTION_LINEAR_ACC`              // Enables Linear Acceleration Sensing
  - `MOTION_GRAVITY`                 // Enables Gravity Sensing
  - `MOTION_ROTATION`                // Enables Rotation Sensing
  - `MOTION_ROTATION_NO_MAG`         // Enables Rotation Sensing without Magnetometer
  - `MOTION_STEP_COUNTER`            // Enables Walking Step Counter
  - `MOTION_STATE`                   // Enables Motion State Detection
  - `MOTION_TAP_DETECTOR`            // Enables Tap Detector
  - `MOTION_ACTIVITY`                // Enables Motion Activity Recognition

- Example Usage:
  - `myCodeCell.Init(LIGHT);`                                      // Initializes Light Sensing
  - `myCodeCell.Init(LIGHT + MOTION_ROTATION);`                    // Initializes Light Sensing and Angular Rotation Sensing
  - `myCodeCell.Init(LIGHT + MOTION_ROTATION + MOTION_STATE);`     // Initializes Light Sensing, Angular Rotation Sensing, and State Detection

Note: You can combine multiple macros using the `+` operator to initialize multiple sensors.

### Run()
- Call the `myCodeCell.Run()` function in the `loop()` to manage battery and power. This function returns true every 100ms and also handles the onboard LED to indicate power status. When the battery voltage falls below 3.3V, the LED will blink red 10 times and then go into Sleep Mode until the USB cable is connected for charging. While charging, the CodeCell will shut down the application, light the LED blue, and wait until the battery is fully charged. Once fully charged, it will start a breathing-light animation with a speed corresponding to the proximity distance. The LED will shine green when powered by the battery and blue when powered via USB.


### Read

- After initializing the sensors, you can use the following functions to read data from them:

  Sensor Read Functions:
  
  - `Light_ProximityRead()`                                            // Reads the proximity value from the light sensor
  - `Light_WhiteRead()`                                                // Reads the white light intensity from the light sensor
  - `Light_AmbientRead()`                                              // Reads the ambient light intensity from the light sensor
  - `Motion_TapRead()`                                                  // Reads the number of taps detected (tap = 1, no tap = 0)
  - `Motion_StateRead()`                                               // Reads the current state (On Table = 1, Stationary = 2, Stable = 3, Motion = 4)
  - `Motion_ActivityRead()`                                            // Reads the current activity (Driving = 1, Cycling = 2, Walking = 3/6, Still = 4, Tilting = 5, Running = 7, Climbing Stairs = 8)
  - `Motion_AccelerometerRead(float &x, float &y, float &z)`           // Reads acceleration data along the x, y, and z axes
  - `Motion_GyroRead(float &x, float &y, float &z)`                    // Reads rotational velocity data along the x, y, and z axes
  - `Motion_MagnetometerRead(float &x, float &y, float &z)`            // Reads magnetic field strength data along the x, y, and z axes
  - `Motion_GravityRead(float &x, float &y, float &z)`                 // Reads gravity vector data along the x, y, and z axes
  - `Motion_LinearAccRead(float &x, float &y, float &z)`               // Reads linear acceleration data along the x, y, and z axes
  - `Motion_StepCounterRead(uint16_t &x)`                              // Reads the number of steps counted
  - `Motion_RotationRead(float &roll, float &pitch, float &yaw)`       // Reads angular rotational data (roll, pitch, yaw)
  - `Motion_RotationNoMagRead(float &roll, float &pitch, float &yaw)`  // Reads angular rotational data without magnetometer

- Example Usage:
  - `uint16_t proximity = myCodeCell.Light_ProximityRead();`
  - `myCodeCell.Motion_AccelerometerRead(myX, myY, myZ);`
  - `myCodeCell.Motion_RotationRead(myRoll, myPitch, myYaw);`

Note: You can use `myCodeCell.PrintSensors()` to print the values of all enabled sensors on the Serial Monitor.

## Attribution
This 'CodeCell' library contains various features, including IMU sensor functionality and a custom light sensor implementation. The VCNL4040 light sensor code does not rely on any external libraries. But some of the BNO085 Motion-sensor functions were adapted from the [SparkFun BNO08x Arduino Library](https://github.com/sparkfun/SparkFun_BNO08x_Arduino_Library) and the official library provided by [CEVA](https://github.com/ceva-dsp/sh2/tree/main) for the SH2 sensor hub.

The SparkFun BNO08x library, originally written by Nathan Seidle and adjusted by Pete Lewis at SparkFun Electronics, is released under the MIT License. Significant modifications were made to integrate it with the 'CodeCell' library.

Additionally, this project incorporates the official [CEVA SH2 sensor hub library](https://github.com/ceva-dsp/sh2/tree/main), which is licensed under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

CEVA’s notice is as follows:
This software is licensed from CEVA, Inc.  
Copyright (c) CEVA, Inc. and its licensors. All rights reserved.  
CEVA and the CEVA logo are trademarks of CEVA, Inc.  
For more information, visit [CEVA's website](https://www.ceva-dsp.com/app/motion-sensing/).

