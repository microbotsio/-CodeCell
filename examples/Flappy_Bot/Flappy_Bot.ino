/*
 * Overview:
 * This code brings your Flappy Bot to life! 
 * With this example the CodeCell controls two FlatFlaps that stops flapping when proximity is detected.
 * Their angle gets adjusted dynamically based on how close something gets.
 * Feel free to tweak the code with your own creative ideas, or add motion sensing for a new reaction! 
 */



#include <CodeCell.h>
#include <DriveCell.h>

#define IN1_pin1 2
#define IN1_pin2 3
#define IN2_pin1 5
#define IN2_pin2 6

DriveCell FlatFlap1(IN1_pin1, IN1_pin2);
DriveCell FlatFlap2(IN2_pin1, IN2_pin2);

CodeCell myCodeCell;

void setup() {
  Serial.begin(115200); /* Set Serial baud rate to 115200. Ensure Tools/USB_CDC_On_Boot is enabled if using Serial. */

  myCodeCell.Init(LIGHT); /*Initializes Light Sensing*/

  FlatFlap1.Init();
  FlatFlap2.Init();

  FlatFlap1.Tone();
  FlatFlap2.Tone();
}

void loop() {
  if (myCodeCell.Run()) {
    /*Runs  every 100ms*/
    uint16_t proximity = myCodeCell.Light_ProximityRead();
    if (proximity < 10) {
      FlatFlap1.Run(1, 100, 400);
      FlatFlap2.Run(1, 100, 400);
    } else {
      if (proximity > 100) {
        proximity = 100;
      }
      FlatFlap1.Drive(0, (proximity));
      FlatFlap2.Drive(0, (proximity));
    }
  }
}
