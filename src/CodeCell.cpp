#include "CodeCell.h"
#include "BNO085.h"

bool serial_flag = 0;
bool cc_timeflag = 1;
esp_sleep_wakeup_cause_t cc_wakeup_reason;
hw_timer_t *cctimer = NULL;

BNO085 Motion;


void IRAM_ATTR cc_Timer() {
  cc_timeflag = 1;
}

CodeCell::CodeCell() {
}

void CodeCell::Init(uint16_t sense_motion) {
  uint8_t light_timer = 0;

  delay(1400); /*Uart Setup delay*/

  if (Serial) {
    serial_flag = 1;
  }

  if (WakeUpCheck()) {
    Serial.println(">> Waking up..");
    _charge_state = POWER_INIT;
    _chrg_counter = 0;
  } else if (cc_wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
    Serial.println(">> Waking up..");
    _charge_state = POWER_INIT;
    _chrg_counter = 0;
  } else {
    Serial.println(" ");
    Serial.println(" ");
    Serial.print(">> CodeCell v" SW_VERSION);
    Serial.println(" Booting.. ");
  }

  _msense = sense_motion;

  Serial.print(">> Initializing Sensors: ");

  if ((_msense & LIGHT) == LIGHT) {
    while (Light_Init() == 1) {
      delay(10);
      LightReset();
      if (light_timer < 50U) {
        light_timer++;
      } else {
        Serial.print(" Light: Failed | ");
      }
    }
    Serial.print(" Light Activated");
  }

  if ((_msense & 0b0111111111111) != MOTION_DISABLE) {
    if ((_msense & LIGHT) == LIGHT) {
      Serial.print(" | ");
    }
    Motion_Init();
  } else {
    //skip
  }
  Serial.println();

  pinMode(LED_PIN, OUTPUT);   /*Set LED pin as output*/
  digitalWrite(LED_PIN, LOW); /*Init Set up to output low*/
  delay(1);
  rmtInit(LED_PIN, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000); /*Configure RMT to run the onboard addressable LED*/

  LED(0, 0, 0);
  delay(1);
  LED(0, 0, LED_SLEEP_BRIGHTNESS);
  delay(80);
  LED(LED_SLEEP_BRIGHTNESS, 0, 0);
  delay(80);
  LED(0, LED_SLEEP_BRIGHTNESS, 0);
  delay(80);
  LED(0, 0, 0);


  pinMode(0, INPUT); /*Set up Cable indication pin*/
  pinMode(4, INPUT); /*Set up Battery Voltage Read pin*/
  analogSetAttenuation(ADC_11db);

  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(1, LOW); /*Init Set up to output low*/
  digitalWrite(2, LOW); /*Init Set up to output low*/
  digitalWrite(3, LOW); /*Init Set up to output low*/
  digitalWrite(5, LOW); /*Init Set up to output low*/
  digitalWrite(6, LOW); /*Init Set up to output low*/
  digitalWrite(7, LOW); /*Init Set up to output low*/

  PrintSensors();
  cctimer = timerBegin(1000000);            /*Set timer frequency to 1Mhz*/
  timerAttachInterrupt(cctimer, &cc_Timer); /*Attach interrupt*/
  timerAlarm(cctimer, 100000, true, 0);     /*Set alarm to trigger every 100ms with auto-reload*/
}

void CodeCell::Test() {
  digitalWrite(1, HIGH); /*Init Set up to output HIGH*/
  digitalWrite(2, HIGH); /*Init Set up to output HIGH*/
  digitalWrite(3, HIGH); /*Init Set up to output HIGH*/
  digitalWrite(5, HIGH); /*Init Set up to output HIGH*/
  digitalWrite(6, HIGH); /*Init Set up to output HIGH*/
  digitalWrite(7, HIGH); /*Init Set up to output HIGH*/
  delay(500);
  digitalWrite(1, LOW); /*Init Set up to output low*/
  digitalWrite(2, LOW); /*Init Set up to output low*/
  digitalWrite(3, LOW); /*Init Set up to output low*/
  digitalWrite(5, LOW); /*Init Set up to output low*/
  digitalWrite(6, LOW); /*Init Set up to output low*/
  digitalWrite(7, LOW); /*Init Set up to output low*/
}

bool CodeCell::WakeUpCheck() {

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  cc_wakeup_reason = wakeup_reason;

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    return true;
  } else {
    return false;
  }
}

void CodeCell::Sleep(uint16_t sleep_sec) {
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF3_MS_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x800 & 0xFF);           /*LSB*/
  _i2c_write_array[_i2c_write_size++] = ((0x800 >> 8) & 0xFF);    /*MSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    Serial.println(">> Error: Light Sensor not found");
  }
  _i2c_write_size = 0;
  _i2c_write_array[_i2c_write_size++] = VCNL4040_ALS_CONF_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x01 & 0xFF);         /*LSB*/
  _i2c_write_array[_i2c_write_size++] = ((0x01 >> 8) & 0xFF);  /*MSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    Serial.println(">> Error: Light Sensor not found");
  }
  _i2c_write_size = 0;
  _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF1_2_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x01 & 0xFF);           /*LSB*/
  _i2c_write_array[_i2c_write_size++] = ((0x01 >> 8) & 0xFF);    /*MSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    Serial.println(">> Error: Light Sensor not found");
  }
  _i2c_write_size = 0;

  if ((_msense & 0b0111111111111) != MOTION_DISABLE) {
    Motion.modeSleep();
  }

  digitalWrite(10, LOW);
  pinMode(10, INPUT);
  Wire.end();  // Stop I2C

  pinMode(8, INPUT);
  pinMode(9, INPUT);
  LED(0, 0, 0);

  esp_sleep_enable_timer_wakeup(sleep_sec * 1000000ULL);
  esp_deep_sleep_start();
}

void CodeCell::USBSleep(bool cable_polarity) {
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  if (cable_polarity) {
    Serial.println(" | Shutting Down"); /*Shut down but do not go to sleep to allow reprogramming*/
    delay(100);
    while (digitalRead(0) == 0) {
      delay(1);
    }
    Serial.println(">> Power Status: Battery is fully charged | Disconnect Cable"); 
    LED(0, LED_SLEEP_BRIGHTNESS, 0);/*Set LED to the minimum brightness Green*/
    delay(1000);
    while (BatteryRead() > USB_VOLTAGE) {
      delay(1);
    }
  } else {
    for (int er = 0; er < 10; er++) {
      LED(255U, 0, 0); /*Set LED to the minimum brightness Red*/
      delay(1000);
      LED(0, 0, 0); /*Set LED to the minimum brightness Red*/
      delay(1000);
      if (digitalRead(0) == 0) {
        /*USB Connected stop blink & Reset*/
        esp_restart();
      }
    }

    _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF3_MS_REG; /*Address*/
    _i2c_write_array[_i2c_write_size++] = (0x800 & 0xFF);           /*LSB*/
    _i2c_write_array[_i2c_write_size++] = ((0x800 >> 8) & 0xFF);    /*MSB*/
    if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
      Serial.println(">> Error: Light Sensor not found");
    }
    _i2c_write_size = 0;
    _i2c_write_array[_i2c_write_size++] = VCNL4040_ALS_CONF_REG; /*Address*/
    _i2c_write_array[_i2c_write_size++] = (0x01 & 0xFF);         /*LSB*/
    _i2c_write_array[_i2c_write_size++] = ((0x01 >> 8) & 0xFF);  /*MSB*/
    if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
      Serial.println(">> Error: Light Sensor not found");
    }
    _i2c_write_size = 0;
    _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF1_2_REG; /*Address*/
    _i2c_write_array[_i2c_write_size++] = (0x01 & 0xFF);           /*LSB*/
    _i2c_write_array[_i2c_write_size++] = ((0x01 >> 8) & 0xFF);    /*MSB*/
    if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
      Serial.println(">> Error: Light Sensor not found");
    }
    _i2c_write_size = 0;

    if ((_msense & 0b0111111111111) != MOTION_DISABLE) {
      Motion.modeSleep();
    }
    delay(1000);

    digitalWrite(10, LOW);
    pinMode(10, INPUT);
    Wire.end();  // Stop I2C

    pinMode(8, INPUT);
    pinMode(9, INPUT);

    esp_deep_sleep_enable_gpio_wakeup(1 << 0, ESP_GPIO_WAKEUP_GPIO_LOW); /*Set Gpio0 as wakeup pin*/
    esp_deep_sleep_start();
    /*Waiting to wake up*/
  }
}

void CodeCell::PrintSensors() {
  Serial.print(">> Reading Sensors: ");
  if ((_msense & LIGHT) == LIGHT) {
    Serial.print("Proximity: ");
    Serial.print(Light_ProximityRead());
    Serial.print(" cts | White: ");
    Serial.print(Light_WhiteRead());
    Serial.print(" lx | Ambient: ");
    Serial.print(Light_AmbientRead());
    Serial.print(" lx |");
  }

  if ((_msense & 0b0111111111111) != MOTION_DISABLE) {
    if ((_msense & MOTION_ACCELEROMETER) == MOTION_ACCELEROMETER) {
      Motion_AccelerometerRead(xx, yy, zz);
      Serial.printf(" Acc XYZ: %.2f, %.2f, %.2f m/s^2 |", xx, yy, zz);
    }
    if ((_msense & MOTION_GYRO) == MOTION_GYRO) {
      Motion_GyroRead(xx, yy, zz);
      Serial.printf(" Gyro XYZ: %.2f, %.2f, %.2f rad/sec |", xx, yy, zz);
    }
    if ((_msense & MOTION_MAGNETOMETER) == MOTION_MAGNETOMETER) {
      Motion_MagnetometerRead(xx, yy, zz);
      Serial.printf(" Magnetometer XYZ: %.2f, %.2f, %.2f uT |", xx, yy, zz);
    }
    if ((_msense & MOTION_LINEAR_ACC) == MOTION_LINEAR_ACC) {
      Motion_LinearAccRead(xx, yy, zz);
      Serial.printf(" Linear Acc XYZ: %.2f, %.2f, %.2f m/s^2 |", xx, yy, zz);
    }
    if ((_msense & MOTION_GRAVITY) == MOTION_GRAVITY) {
      Motion_GravityRead(xx, yy, zz);
      Serial.printf(" Gravity XYZ: %.2f, %.2f, %.2f m/s^2 |", xx, yy, zz);
    }
    if ((_msense & MOTION_ROTATION) == MOTION_ROTATION) {
      Motion_RotationRead(xx, yy, zz);
      Serial.printf(" Roll: %.2f°, Pitch: %.2f°, Yaw: %.2f° |", xx, yy, zz);
    }
    if ((_msense & MOTION_ROTATION_NO_MAG) == MOTION_ROTATION_NO_MAG) {
      Motion_RotationNoMagRead(xx, yy, zz);
      Serial.printf(" Roll: %.2f°, Pitch: %.2f°, Yaw: %.2f° |", xx, yy, zz);
    }
    if ((_msense & MOTION_STEP_COUNTER) == MOTION_STEP_COUNTER) {
      Motion_StepCounterRead(xxtemp);
      Serial.print(" Steps: ");
      Serial.print(xxtemp);
      Serial.print(" |");
    }
    if ((_msense & MOTION_TAP_DETECTOR) == MOTION_TAP_DETECTOR) {
      Serial.print(" Tap: ");
      Serial.print(Motion_TapRead());
      Serial.print(" |");
    }
    if ((_msense & MOTION_STATE) == MOTION_STATE) {
      Serial.print(" Current State: ");
      switch (Motion_StateRead()) {
        case MOTION_STATE_STABLE:
          Serial.print("Stable");
          break;
        case MOTION_STATE_ONTABLE:
          Serial.print("On Table");
          break;
        case MOTION_STATE_STATIONARY:
          Serial.print("Stationary");
          break;
        case MOTION_STATE_MOTION:
          Serial.print("Motion");
          break;
        default:
          Serial.print("Unkown");
          break;
      }
      Serial.print(" |");
    }
    if ((_msense & MOTION_ACTIVITY) == MOTION_ACTIVITY) {
      Serial.print(" Activity: ");
      switch (Motion_ActivityRead()) {
        case 1:
          Serial.print("Driving");
          break;
        case 2:
          Serial.print("Cycling");
          break;
        case 3:
        case 6:
          Serial.print("Walking");
          break;
        case 4:
          Serial.print("Still");
          break;
        case 5:
          Serial.print("Tilting");
          break;
        case 7:
          Serial.print("Running");
          break;
        case 8:
          Serial.print("Stairs");
          break;
        default:
          Serial.print("Reading..");
          break;
      }
      Serial.print(" |");
    }
  }
  Serial.println();
}

uint16_t CodeCell::BatteryRead() {
  double voltage = (double)analogRead(4) * 1.448;  //* 5930 / 4095
  uint16_t uvoltage = (uint16_t)voltage;
  return uvoltage;
}

bool CodeCell::Run(uint8_t run_frequency) {
  bool tt_flag = 0;
  if (cc_timeflag) {
    tt_flag = 1;
    cc_timeflag = 0;

    if (run_frequency > 100) {
      run_frequency = 100;
    } else if (run_frequency < 10) {
      run_frequency = 10;
    } else {
      //skip
    }
    if (run_frequency != _run_frequency_last) {
      Serial.print(">> Sampling Rate: ");
      Serial.print(run_frequency);
      Serial.println("Hz");
      timerStop(cctimer);
      _run_frequency_last = run_frequency;
      timerAlarm(cctimer, ((1000000) / _run_frequency_last), true, 0); /*Set alarm to trigger to new frequency*/
      timerStart(cctimer);
    }

    _power_counter++;
    if (_power_counter >= (run_frequency / 10)) {
      ///Check Power every 1Hz
      _power_counter = 0;

      uint16_t battery_voltage = BatteryRead();

      if ((battery_voltage > USB_VOLTAGE) || (digitalRead(0) == 0)) {
        _lowvoltage_counter = 0;
        _chrg_counter = 0;
        if (_charge_state == POWER_BAT_CHRG) {
          esp_restart(); /*Restart the CodeCell*/
        }
        if (digitalRead(0) == 0) {
          Serial.print(">> Power Status: Battery is now charging");
          LED(0, 0, LED_SLEEP_BRIGHTNESS); /*Set LED to the minimum Blue*/
          USBSleep(1);
          battery_voltage = BatteryRead();
          if (battery_voltage > USB_VOLTAGE) {
            _charge_state = POWER_INIT;
          } else {
            _charge_state = POWER_BAT_CHRG_FULL;
          }
        } else if (_charge_state == POWER_INIT) {
          /*Battery First Time Check*/
          Serial.println(">> Power Status: Running from USB Power");
          _charge_state = POWER_USB;
        } else {
          /*USB Cabel connected without battery - Run application*/
          LED_Breathing(LED_COLOR_BLUE);
          if ((!Serial) && (serial_flag == 1)) {
            Serial.end();         /*Close the current Serial connection*/
            delay(500);           /*Short delay before reinitializing*/
            Serial.begin(115200); /*Reinitialize the Serial connection*/
            Serial.println(">> Serial connection re-established!");
            delay(100);    /*Short delay before reinitializing*/
            esp_restart(); /*Restart the CodeCell*/
          }
        }
      } else {
        if ((_charge_state == POWER_BAT_CHRG_FULL) || (_charge_state == POWER_INIT)) {
          /*Battery has been charged or USB is disconnected*/
          Serial.println(">> Power Status: Running from Battery Power");
          _charge_state = POWER_BAT_CHRG; /*Battery is at an operating voltage level*/
          _chrg_counter = 0;
          _lowvoltage_counter = 0;
        } else {
          if (battery_voltage < MIN_BATTERY_VOLTAGE) {
            /*Voltage is less than 3.3V or higher than 4.3V*/
            if (_lowvoltage_counter < 10) {
              _lowvoltage_counter++;
            } else {
              Serial.println(">> Error: Battery Volotage");
              _chrg_counter = 0;
              USBSleep(0);
            }
          } else {
            _lowvoltage_counter = 0;
            if (_chrg_counter < 10) {
              _chrg_counter++;
            } else {
              _charge_state = POWER_BAT_CHRG; /*Battery is at an operating voltage level*/
              LED_Breathing(LED_COLOR_GREEN);
            }
          }
        }
      }
    }
  } else {
    /*wait*/
  }
  return tt_flag;
}

void CodeCell::LED(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(LED_PIN, r, g, b); /*RMT ESP32 function for addressable LEDs*/
}

void CodeCell::LED_Breathing(uint32_t rgb_color_24bit) {
  uint32_t r_counter = (rgb_color_24bit >> 16) & 0xFFU;
  uint32_t g_counter = (rgb_color_24bit >> 8) & 0xFFU;
  uint32_t b_counter = rgb_color_24bit & 0xFFU;
  uint16_t getProx = Light_ProximityRead();

  if (getProx > 2000U) {
    getProx = 2000U;
  }

  getProx = (getProx / 100) + 2;
  if (getProx > LED_DEFAULT_BRIGHTNESS) {
    getProx = LED_DEFAULT_BRIGHTNESS;
  }
  getProx = getProx << 1U;

  if (_LED_Breathing_flag == 1) {
    if (_LED_Breathing_counter <= getProx) {
      _LED_Breathing_counter++;
    } else {
      _LED_Breathing_flag = 0;
    }
  } else {
    if (_LED_Breathing_counter >= 1) {
      _LED_Breathing_counter--;
    } else {
      _LED_Breathing_flag = 1;
    }
  }

  r_counter = (r_counter * _LED_Breathing_counter) / 22U;
  g_counter = (g_counter * _LED_Breathing_counter) / 22U;
  b_counter = (b_counter * _LED_Breathing_counter) / 22U;

  LED(r_counter, g_counter, b_counter); /*Set LED to the default Blue*/
}

uint16_t CodeCell::Light_AmbientRead() {
  uint16_t value = 0;
  Wire.beginTransmission(VCNL4040_ADDRESS);
  Wire.write(VCNL4040_AMBIENT_REG);
  Wire.endTransmission(false); /*Don't release the bus*/
  I2CRead(VCNL4040_ADDRESS, _i2c_read_array, 2);
  Wire.endTransmission();
  value = _i2c_read_array[0];         /*LSB*/
  value |= (_i2c_read_array[1] << 8); /*MSB*/
  return value;
}

uint16_t CodeCell::Light_WhiteRead() {
  uint16_t value = 0;
  Wire.beginTransmission(VCNL4040_ADDRESS);
  Wire.write(VCNL4040_WHITE_REG);
  Wire.endTransmission(false); /*Don't release the bus*/
  I2CRead(VCNL4040_ADDRESS, _i2c_read_array, 2);
  Wire.endTransmission();
  value = _i2c_read_array[0];         /*LSB*/
  value |= (_i2c_read_array[1] << 8); /*MSB*/
  return value;
}

uint16_t CodeCell::Light_ProximityRead() {
  uint16_t value = 0;
  Wire.beginTransmission(VCNL4040_ADDRESS);
  Wire.write(VCNL4040_PROX_REG);
  Wire.endTransmission(false); /*Don't release the bus*/
  I2CRead(VCNL4040_ADDRESS, _i2c_read_array, 2);
  Wire.endTransmission();
  value = _i2c_read_array[0];         /*LSB*/
  value |= (_i2c_read_array[1] << 8); /*MSB*/
  return value;
}

void CodeCell::Motion_AccelerometerRead(float &x, float &y, float &z) {
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_ACCELEROMETER) {
    x = Motion.getAccelX();
    y = Motion.getAccelY();
    z = Motion.getAccelZ();
  }
  Wire.endTransmission();
}

void CodeCell::Motion_GyroRead(float &x, float &y, float &z) {
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_GYROSCOPE_CALIBRATED) {
    x = Motion.getGyroX();
    y = Motion.getGyroY();
    z = Motion.getGyroZ();
  }
  Wire.endTransmission();
}

void CodeCell::Motion_MagnetometerRead(float &x, float &y, float &z) {
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_MAGNETIC_FIELD) {
    x = Motion.getMagX();
    y = Motion.getMagY();
    z = Motion.getMagZ();
  }
  Wire.endTransmission();
}

void CodeCell::Motion_GravityRead(float &x, float &y, float &z) {
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_GRAVITY) {
    x = Motion.getGravityX();
    y = Motion.getGravityY();
    z = Motion.getGravityZ();
  }
  Wire.endTransmission();
}

void CodeCell::Motion_LinearAccRead(float &x, float &y, float &z) {
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_LINEAR_ACCELERATION) {
    x = Motion.getLinAccelX();
    y = Motion.getLinAccelY();
    z = Motion.getLinAccelZ();
  }
  Wire.endTransmission();
}

bool CodeCell::Motion_TapRead() {
  bool x = false;
  if (Motion.getSensorEvent() == true) {
    if (Motion.getSensorEventID() == SENSOR_REPORTID_TAP_DETECTOR) {
      x = true;
    }
  }
  Wire.endTransmission();
  return x;
}

void CodeCell::Motion_StepCounterRead(uint16_t &x) {
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_STEP_COUNTER) {
    x = (uint16_t)Motion.getStepCount();
  }
  Wire.endTransmission();
}

uint16_t CodeCell::Motion_StateRead() {
  uint16_t x = 0;
  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_STABILITY_CLASSIFIER) {
    x = Motion.getStabilityClassifier();
  }
  Wire.endTransmission();
  return x;
}

uint16_t CodeCell::Motion_ActivityRead() {
  uint16_t x = 0;
  //Motion_Read();
  Wire.beginTransmission(BNO085_ADDRESS);
  if (Motion.getSensorEvent() == true) {
    if (Motion.getSensorEventID() == SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER) {
      x = Motion.getActivityClassifier();
    }
  }
  Wire.endTransmission();
  return x;
}


void CodeCell::Motion_RotationRead(float &roll, float &pitch, float &yaw) {
  float Rr = 0;
  float Ri = 0;
  float Rj = 0;
  float Rk = 0;

  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR) {
    Rr = Motion.getRot_R();
    Ri = Motion.getRot_I();
    Rj = Motion.getRot_J();
    Rk = Motion.getRot_K();

    roll = atan2(2.0 * (Rr * Ri + Rj * Rk), 1.0 - 2.0 * (Ri * Ri + Rj * Rj)) * RAD_TO_DEG;  /* ROLL */
    pitch = atan2(2.0 * (Rr * Rj - Rk * Ri), 1.0 - 2.0 * (Rj * Rj + Ri * Ri)) * RAD_TO_DEG; /* PITCH */
    yaw = atan2(2.0 * (Rr * Rk + Ri * Rj), 1.0 - 2.0 * (Rj * Rj + Rk * Rk)) * RAD_TO_DEG;   /* YAW */
  }
  Wire.endTransmission();
}

void CodeCell::Motion_RotationNoMagRead(float &roll, float &pitch, float &yaw) {
  float Rr = 0;
  float Ri = 0;
  float Rj = 0;
  float Rk = 0;

  Motion_Read();
  if (Motion.getSensorEventID() == SENSOR_REPORTID_GAME_ROTATION_VECTOR) {
    Rr = Motion.getGameReal();
    Ri = Motion.getGameI();
    Rj = Motion.getGameJ();
    Rk = Motion.getGameK();

    roll = atan2(2.0 * (Rr * Ri + Rj * Rk), 1.0 - 2.0 * (Ri * Ri + Rj * Rj)) * RAD_TO_DEG;  /* ROLL */
    pitch = atan2(2.0 * (Rr * Rj - Rk * Ri), 1.0 - 2.0 * (Rj * Rj + Ri * Ri)) * RAD_TO_DEG; /* PITCH */
    yaw = atan2(2.0 * (Rr * Rk + Ri * Rj), 1.0 - 2.0 * (Rj * Rj + Rk * Rk)) * RAD_TO_DEG;   /* YAW */
  }
  Wire.endTransmission();
}

void CodeCell::Motion_Read() {
  uint16_t imu_read_timer = 0U;
  Wire.beginTransmission(BNO085_ADDRESS);
  while (Motion.getSensorEvent() == false) {
    if (imu_read_timer > 20000U) {
      Serial.println(">> Error: Motion Sensor not found");
      Serial.println(">> Reseting");
      delay(100);
      esp_restart();
    } else {
      imu_read_timer++;
    }
    delayMicroseconds(1);
  }
}

bool CodeCell::Light_Init() {
  bool light_error = 0;

  Wire.begin(8, 9, 400000);  // SDA on GPIO8, SCL on GPIO9, 400kHz speed

  Wire.beginTransmission(VCNL4040_ADDRESS);
  /*Configure - Continuous conversion mode, high dynamic range, integration time of 80 ms*/
  _i2c_write_array[_i2c_write_size++] = VCNL4040_ALS_CONF_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x000 & 0xFF);        /*LSB*/
  _i2c_write_array[_i2c_write_size++] = ((0x000 >> 8) & 0xFF); /*MSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    light_error = 1;
  }
  _i2c_write_size = 0;
  /*Configure - duty cycle to 1/40, 16-bit resolution*/
  _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF1_2_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x80E & 0xFF);          /*LSB*/
  _i2c_write_array[_i2c_write_size++] = ((0x80E >> 8) & 0xFF);   /*MSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    light_error = 1;
  }
  _i2c_write_size = 0;
  /*Configure - Set LED current to 200 mA, No interrupt settings*/
  _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF3_MS_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x4710 & 0xFF);          /*LSB*/
  _i2c_write_array[_i2c_write_size++] = ((0x4710 >> 8) & 0xFF);   /*MSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    light_error = 1;
  }
  _i2c_write_size = 0;
  Wire.endTransmission();

  return light_error;
}

void CodeCell::LightReset() {
  _i2c_write_array[_i2c_write_size++] = VCNL4040_ALS_CONF_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x01 & 0xFF);         /*LSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    Serial.println(">> Error: Light Sensor not found");
  }
  _i2c_write_size = 0;
  _i2c_write_array[_i2c_write_size++] = VCNL4040_PS_CONF1_2_REG; /*Address*/
  _i2c_write_array[_i2c_write_size++] = (0x01 & 0xFF);           /*LSB*/
  if (!I2CWrite(VCNL4040_ADDRESS, _i2c_write_array, _i2c_write_size)) {
    Serial.println(">> Error: Light Sensor not found");
  }
  _i2c_write_size = 0;
}

void CodeCell::Motion_Init() {
  uint8_t imu_timer = 0U;
  Wire.begin(8, 9, 400000);  // SDA on GPIO8, SCL on GPIO9, 400kHz speed

  Wire.beginTransmission(BNO085_ADDRESS);
  while (Motion.begin() == false) {
    if (imu_timer > 100U) {
      Serial.println(">> Error: Motion Sensor not found");
      Serial.println(">> Reseting");
      delay(100);
      esp_restart();
    } else {
      imu_timer++;
    }
    delay(10);
    Motion.softReset();
  }
  if ((_msense & MOTION_ACCELEROMETER) == MOTION_ACCELEROMETER) {
    if (Motion.enableAccelerometer() == true) {
      Serial.print("Accelerometer Activated | ");
    } else {
      Serial.print("Accelerometer: Failed | ");
    }
  }
  if ((_msense & MOTION_GYRO) == MOTION_GYRO) {
    if (Motion.enableGyro() == true) {
      Serial.print("Gyro Activated | ");
    } else {
      Serial.print("Gyro: Failed | ");
    }
  }
  if ((_msense & MOTION_MAGNETOMETER) == MOTION_MAGNETOMETER) {
    if (Motion.enableMagnetometer() == true) {
      Serial.print("Magnetometer Activated | ");
    } else {
      Serial.print("Magnetometer: Failed | ");
    }
  }
  if ((_msense & MOTION_LINEAR_ACC) == MOTION_LINEAR_ACC) {
    if (Motion.enableLinearAccelerometer() == true) {
      Serial.print("Linear Accelerometer Activated | ");
    } else {
      Serial.print("Linear Accelerometer Motion: Failed | ");
    }
  }
  if ((_msense & MOTION_GRAVITY) == MOTION_GRAVITY) {
    if (Motion.enableGravity() == true) {
      Serial.print("Gravity Sensing Activated | ");
    } else {
      Serial.print("Gravity Sensing: Failed | ");
    }
  }
  if ((_msense & MOTION_ROTATION) == MOTION_ROTATION) {
    if (Motion.enableRotationVector() == true) {
      Serial.print("Rotation Sensing Activated | ");
    } else {
      Serial.print("Rotation Sening: Failed | ");
    }
  }
  if ((_msense & MOTION_ROTATION_NO_MAG) == MOTION_ROTATION_NO_MAG) {
    if (Motion.enableGameRotationVector() == true) {
      Serial.print("Compass-Free Rotation Sensing Activated | ");
    } else {
      Serial.print("Compass-Free Rotation Sensing: Failed | ");
    }
  }
  if ((_msense & MOTION_STEP_COUNTER) == MOTION_STEP_COUNTER) {
    if (Motion.enableStepCounter() == true) {
      Serial.print("Step Counter Activated | ");
    } else {
      Serial.print("Step Counter: Failed | ");
    }
  }
  if ((_msense & MOTION_TAP_DETECTOR) == MOTION_TAP_DETECTOR) {
    if (Motion.enableTapDetector() == true) {
      Serial.print("Tap Detector Activated | ");
    } else {
      Serial.print("Tap Detector: Failed | ");
    }
  }
  if ((_msense & MOTION_STATE) == MOTION_STATE) {
    if (Motion.enableStabilityClassifier() == true) {
      Serial.print("Motion State Activated | ");
    } else {
      Serial.print("Motion State: Failed | ");
    }
  }
  if ((_msense & MOTION_ACTIVITY) == MOTION_ACTIVITY) {
    if (Motion.enableActivityClassifier(1000, 0x1F) == true) {
      Serial.print("Motion Activity Activated | ");
    } else {
      Serial.print("Motion Activity: Failed | ");
    }
  }
  _i2c_write_size = 0;
  Wire.endTransmission();
}
