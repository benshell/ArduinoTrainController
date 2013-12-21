
/*
 * MegaMoto Setup
 */
int EnablePin = 8;
int duty;
int PWMPin1 = 9;
int PWMPin2 = 3;
const byte CPin = 0;  // analog input channel
int CRaw;      // raw A/D value
float CVal;    // adjusted Amps value


/**
 * Other variables
 */
byte reverseVoltage;
byte advancedAnalog = 1;
byte increment = 10;
byte val;
 
struct commands
{
  byte speed: 8;
  byte direction: 8;
  byte whistle: 8;
  byte bell: 8;
  byte maxSpeed: 8;
  byte minSpeed: 8;
};

commands target;
commands current;

void setup()
{
  Serial.begin(9600);

  // Initialize the MegaMoto
  pinMode(EnablePin, OUTPUT);     
  pinMode(PWMPin1, OUTPUT);
  pinMode(PWMPin2, OUTPUT);
  setPwmFrequency(PWMPin1, 8);  // change Timer2 divisor to 8 gives 3.9kHz PWM freq
  setPwmFrequency(PWMPin2, 8);  // change Timer2 divisor to 8 gives 3.9kHz PWM freq

  // Initialize the motor variables
  current.speed = 0;
  target.speed = 0;
  target.direction = 1;
  reverseVoltage = 0;
  target.maxSpeed = 255;
  target.minSpeed = 0;
}

void loop()
{
  // Enable the MegaMoto
  digitalWrite(EnablePin, HIGH);
  
  // Bluetooth
  if (Serial.available()) {
    val = Serial.read();
    //Serial.print("Bluetooth command: ");
    
    // 1: Faster
    if (val == 49) {
      Serial.println("Faster");
      if (target.speed < 246) {
        target.speed = target.speed + 10;
      }
      else {
        target.speed = 255;
      }
    }
    
    // 2: Slower
    else if (val == 50) {
      Serial.println("Slower");
      if (target.speed > 9) {
        target.speed = target.speed - 10;
      }
      else {
        target.speed = 0;
      }
    }
    
    // 3: Change Directions
    else if (val == 51) {
      Serial.println("Change Directions");
      if (target.direction == 1) {
        target.direction = 2;
      }
      else {
        target.direction = 1;
      }
    }
    
    // 0: Emergency Stop
    else if (val == 48) { 
      Serial.println("Emergency Stop");
      target.speed = 0;
    }
    
    // 4: Whistle toggle
    else if (val == 52) {
      Serial.println("Toggle Whistle");
      target.whistle = target.whistle ^ 1;
    }
    
    // 5: Bell toggle
    else if (val == 53) {
      Serial.println("Toggle Bell");
      target.bell = target.bell ^ 1;
    }
    
    // 6: Set maximum speed
    else if (val == 54) {
      Serial.println("Set maximum speed");
      target.maxSpeed = current.speed;
    }
    
    // 7: Remove maximum speed
    else if (val == 55) {
      Serial.println("Unset maximum speed");
      target.maxSpeed = 255;
    }
    
    // 8: Set min speed
    else if (val == 56) {
      Serial.println("Set minimum speed");
      target.minSpeed = current.speed;
    }
    
    // 9: Remove min speed
    else if (val == 57) {
      Serial.println("Unset minimum speed");
      target.minSpeed = 0;
    }
    
    // ":" Enable QSI Advanced Advanced Analog
    else if (val == 58) {
      Serial.println("Enable advanced analog");
      advancedAnalog = 1;
    }
    
    // ";" Disable QSI Advanced Advanced Analog
    else if (val == 59) {
      Serial.println("Disable advanced analog");
      advancedAnalog = 0;
    }
    
  }
  
  
  if (advancedAnalog) {
    if (target.whistle != current.whistle) {
      whistle();
      current.whistle = target.whistle;
    }
    if (target.bell != current.bell) {
      bell();
      current.bell = target.bell;
    }
  }
  
  updateSpeed();
  
  delay(100);
}

void whistle()
{
  Serial.println("Whistle");
  setReverseVoltage();
  writeSpeed();
}

void bell()
{
  Serial.println("Bell");
  setReverseVoltage();
  writeSpeed();
  setReverseVoltage();
  writeSpeed();
}

void setReverseVoltage()
{
  if (reverseVoltage == 0) {
    reverseVoltage = 1;
  }
  else {
    reverseVoltage = 0;
  }
}

void updateSpeed()
{
  // Conform max speed to limit (default: 255)
  if (target.speed > target.maxSpeed) {
    target.speed = target.maxSpeed;
  }

  // Conform min speed to limit (default: 0)
  if (target.speed < target.minSpeed) {
    target.speed = target.minSpeed;
  }

  if ((target.speed == current.speed) && (target.direction == current.direction)) {
    //Serial.println("Nothing to do");
    return;
  }
  
  // If the speed is below the minSpeed (probably zero), the direction should be set to the target direction immediately (nothing to slow down first)
  if (current.speed <= target.minSpeed) {
    current.speed = target.minSpeed; // enforce minSpeed
    current.direction = target.direction;
  }
  
  // If the target direction is different than the current direction, slow down
  if (target.direction != current.direction) {
    // It may be safe to slow to the min speed immediately
    if ((current.speed - target.minSpeed) < increment) {
      current.speed = target.minSpeed;
    }
    else {
      current.speed = current.speed - increment;
    }
  }
  
  // Speeding up?
  else if (target.speed > current.speed) {
    if ((target.speed - current.speed) < increment) {
      current.speed = target.speed;
    }
    else {
      current.speed = current.speed + increment;
    }
  }

  // Slowing down?
  else if (target.speed < current.speed) {
    if ((current.speed - target.speed) < increment) {
      current.speed = target.speed;
    }
    else {
      current.speed = current.speed - increment;
    }
  }
  
  writeSpeed();
}

void writeSpeed() {
  Serial.print("Writing speed: ");
  Serial.print(current.speed);
  
  if (current.direction == 1) {
    Serial.println(" Forward");
    // Normal?
    if (reverseVoltage == 0) {
      analogWrite(PWMPin1, current.speed);
      analogWrite(PWMPin2, 0);
    }
    // Reversed? (after triggering QSI Advanced Analog commands)
    else {
      analogWrite(PWMPin1, 0);
      analogWrite(PWMPin2, current.speed);
    }
  }
  else {
    Serial.println(" Reverse");
    // Normal?
    if (reverseVoltage == 0) {
      analogWrite(PWMPin1, 0);
      analogWrite(PWMPin2, current.speed);
    }
    // Reversed? (after triggering QSI whistle)
    else {
      analogWrite(PWMPin1, current.speed);
      analogWrite(PWMPin2, 0);
    }
  }
  
  // Long pause if changing directions
  if ((target.direction != current.direction) && (current.speed == 0) && (target.speed != 0)) {
    Serial.println("Pausing");
    delay(2000);
  }
  
  // Brief pause anytime the speed is updated, just to slow the program down a little
  else {
    delay(100);
  }
}

/*
 * Divides a given PWM pin frequency by a divisor.
 * 
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 * 
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired (Timer0)
 *   - Pins 9 and 10 are paired (Timer1)
 *   - Pins 3 and 11 are paired (Timer2)
 * 
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 5, 6 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 * 
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
 
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) { // Timer0 or Timer1
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) { 
      TCCR0B = TCCR0B & 0b11111000 | mode; // Timer0
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode; // Timer1
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode; // Timer2
  }
}

