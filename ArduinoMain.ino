#include <Servo.h>
#include "IRremote.h"
// #include <Stepper.h>

// LATCH SERVO
 Servo latchServo;
const int PIN_LATCH = 13;
int latchState = 1; // 0 = Open Door; 1 = Close Door (for LED)

// SENSOR SERVO
 Servo sensorServo;
const int PIN_SENSORSERVO = 13;

// SENSOR
const int PIN_ECHO = 5; // Yellow = Echo
const int PIN_TRIG = 6; //Red = Trig
long distance = 0; // In centimeters

// IR Receiver

// DC MOTOR
const int PIN_ML_PWM  = 3;
const int PIN_ML_DIR1 = 2;
const int PIN_ML_DIR2 = 4;
// const int PIN_MR_PWM  = 9;
// const int PIN_MR_DIR1 = 7;
// const int PIN_MR_DIR2 = 8;
int motorSpeed = 200; // 0-255

// BASIC
int debugLevel = 1;
String debugMsg = "";
unsigned long logTimePrev = 0;
unsigned long logTimeDelay = 1000;
long distForward = 0;
int playState = false;



void setup() {
  // LATCH SERVO
  latchServo.attach(PIN_LATCH);
  latchServo.write(90);

  // SENSOR SERVO
  latchServo.attach(PIN_SENSORSERVO);
  latchServo.write(0);

  // SENSOR
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);

  // IR Receiver
  IrReceiver.begin(12,false);

  // DC MOTOR
  pinMode(PIN_ML_PWM,OUTPUT);
  pinMode(PIN_ML_DIR1,OUTPUT);
  pinMode(PIN_ML_DIR2,OUTPUT);
  analogWrite(PIN_ML_PWM,0); //half speed
  
  // pinMode(PIN_MR_PWM,OUTPUT);
  // pinMode(PIN_MR_DIR1,OUTPUT);
  // pinMode(PIN_MR_DIR2,OUTPUT);
  // analogWrite(PIN_MR_PWM,0); //half speed
  
  // BASIC
  pinMode(13, OUTPUT); // Turn warning LED off
  digitalWrite(13, LOW);
  Serial.begin(9600);
  delay(2000);
}




void loop() {
  delay(200); // Minimal Delay between Actions
  sensorServo.write(0);
  distForward = SenseDistance();
  IRReceiverCheck();
  IrReceiver.resume();
  if (playState == true) {
    if(distForward >= 15) {
        Move(1,1);
        delay(200);
      } else {
        Serial.println("STOPPING");
        Move(0,0);
        delay(200);
        Move(-1,-1);
        delay(200);
      }
  } else {
        Move(0,0);
  }

	if (millis() - logTimePrev > logTimeDelay) {
		debugLevel = 1;
		logTimePrev = millis();
	} else {
		debugLevel = 0;
	}

}




// APPARATUS FUNCTIONS
long SenseDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, LOW);
  distance = pulseIn(PIN_ECHO, HIGH) * 0.0175; // CM from target

  // Logging
  if (debugLevel >= 1) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println("cm");
  }

  return(distance);
}



void Move(int motorLeftDirection, int motorRightDirection) {
  // 0 = No Movement, 1 = CW, -1 = CCW
  // Left/Right Inverted to one another normally
  // Forward: Move(1,1); Left: Move(-1,1), etc.
  if (motorSpeed > 100) {
    digitalWrite(PIN_ML_PWM, HIGH);
  } else {
    digitalWrite(PIN_ML_PWM, LOW);
  }
  // analogWrite(PIN_ML_PWM, motorSpeed);
  if(motorLeftDirection < 0) {
    digitalWrite(PIN_ML_DIR1, 0);
    digitalWrite(PIN_ML_DIR2, -motorLeftDirection);
  } else {
    digitalWrite(PIN_ML_DIR1, motorLeftDirection);
    digitalWrite(PIN_ML_DIR2, 0);
  }
  // analogWrite(PIN_MR_PWM, motorSpeed);
  // digitalWrite(PIN_MR_DIR1, -motorRightDirection);
  // digitalWrite(PIN_MR_DIR2, motorRightDirection);

    // Logging
  if (debugLevel >= 1) {
    Serial.print("Motor: ");
    Serial.print(motorLeftDirection);
    Serial.print(", ");
    Serial.print(-motorLeftDirection);
    Serial.print(", ");
    Serial.print(motorSpeed);
    // Serial.print(", ");
    // Serial.print(motorRightDirection);
    Serial.println("");
  }

}



void ToggleLatch() {
  latchState = !latchState;
  latchServo.write(latchState*90);

//   if (debugLevel >= 1) {
    if (latchState == 0) {
      debugMsg = "OPEN";
    } else {
      debugMsg = "CLOSED";
    }
    Serial.print("SERVO: ");
    Serial.println(debugMsg);
//   }
  
}  

void IRReceiverCheck() {
	if(IrReceiver.decode()) {
		// IR Code Matching
		switch (IrReceiver.decodedIRData.decodedRawData) {
			case 0xBA45FF00: Serial.println("POWER"); break;
			case 0xB847FF00: Serial.println("FUNC/STOP"); break;
			case 0xB946FF00: Serial.println("VOL+");break;
			case 0xBB44FF00: Serial.println("FAST BACK"); if(motorSpeed>=0){motorSpeed-=20;}  break;
			case 0xBF40FF00: Serial.println("PAUSE");  playState = !playState; Serial.print("STATE: "); Serial.println(playState);  break;
			case 0xBC43FF00: Serial.println("FAST FORWARD"); if(motorSpeed<=220){motorSpeed+=20;} break;
			case 0xF807FF00: Serial.println("DOWN");  break;
			case 0xEA15FF00: Serial.println("VOL-");  break;
			case 0xF609FF00: Serial.println("UP");    break;
			case 0xE619FF00: Serial.println("EQ");    break;
			case 0xF20DFF00: Serial.println("ST/REPT");    break;
			case 0xE916FF00: Serial.println("0");   ToggleLatch(); break;
			case 0xF30CFF00: Serial.println("1");    break;
			case 0xE718FF00: Serial.println("2");    break;
			case 0xA15EFF00: Serial.println("3");    break;
			case 0xF708FF00: Serial.println("4");    break;
			case 0xE31CFF00: Serial.println("5");    break;
			case 0xA55AFF00: Serial.println("6");    break;
			case 0xBD42FF00: Serial.println("7");    break;
			case 0xAD52FF00: Serial.println("8");    break;
			case 0xB54AFF00: Serial.println("9");    break;
			default:
			Serial.println("UNKNOWN BUTTON");
		}// End Case
	}
	 // receive the next value

}
		// case 0xB946FF00: // VOL+ button pressed
		// 	small_stepper.setSpeed(500); //Max seems to be 500
		// 	Steps2Take  =  -2048;  // Rotate CW
		// 	small_stepper.step(Steps2Take);
		// 	delay(2000);
		// 	break;

		// case 0xEA15FF00: // VOL- button pressed
		// 	small_stepper.setSpeed(500);
		// 	Steps2Take  =  2048;  // Rotate CCW
		// 	small_stepper.step(Steps2Take);
		// 	delay(2000);
		// 	break;

		//store the last decodedRawData
		// last_decodedRawData = IrReceiver.decodedIRData.decodedRawData;