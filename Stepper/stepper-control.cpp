// ======================================================
// Simple Stepper Motor Control with AccelStepper Library
// ======================================================

#include <AccelStepper.h>

const int dirPin = 2;
const int stepPin = 3;

const float microsteps = 16; 

const int steps = 1;

int direction = 1; // Direction of rotation (1 for clockwise, -1 for counterclockwise)

#define motorInterfaceType 1

AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

void setup() {
	myStepper.setMaxSpeed(200*microsteps);
	myStepper.setAcceleration(10*microsteps);
	myStepper.setSpeed(200*microsteps);
	myStepper.moveTo(steps*microsteps);
}

void loop() {
	// Change direction once the motor reaches target position
	if (myStepper.distanceToGo() == 0) 
  {
    if (direction == 1) 
    {
      myStepper.moveTo(0);
      direction = -1; // Change to counterclockwise
    } 
    else 
    {
      myStepper.moveTo(steps*microsteps);
      direction = 1; // Change to clockwise
    }
  }

	// Move the motor one step
	myStepper.run();
}

// ===================================================
// Simple Stepper Motor Control without libraries
// ===================================================

// #include <Arduino.h>

// // Define pin connections & motor's steps per revolution
// const int dirPin = 2;
// const int stepPin = 3;
// const int stepsPerRevolution = 2;

// void setup()
// {
// 	// Declare pins as Outputs
// 	pinMode(stepPin, OUTPUT);
// 	pinMode(dirPin, OUTPUT);
// }
// void loop()
// {
// 	// Set motor direction clockwise
// 	digitalWrite(dirPin, HIGH);

//   // delay(2000);

//   // digitalWrite(stepPin, HIGH); // Set step pin high
//   // delay(10);
//   // digitalWrite(stepPin, LOW); // Set step pin low

//   // delay(10);

//   // digitalWrite(dirPin, LOW); // Set motor direction counterclockwise

//   // delay(100);

//   // digitalWrite(stepPin, HIGH); // Set step pin high
//   // delay(10);
//   // digitalWrite(stepPin, LOW); // Set step pin low

//   // delay(2000);

// 	// Spin motor slowly
// 	for(int x = 0; x < stepsPerRevolution; x++)
// 	{
// 		digitalWrite(stepPin, HIGH);
// 		delayMicroseconds(2000);
// 		digitalWrite(stepPin, LOW);
// 		delayMicroseconds(2000);
// 	}
// 	delay(1000); // Wait a second
// }