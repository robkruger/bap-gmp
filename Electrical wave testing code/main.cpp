#include <Arduino.h>
#include <pwm.h>

#define PWMpin D9
PwmOut pwmsig(9); // Use any PWM-capable pin (3,5,6,9,10,11)

void setup() {
  Serial.begin(9600);
  pinMode(PWMpin, OUTPUT);
  pwmsig.begin(50.14f, 25.0f); // 1 kHz frequency, 25 % duty cycle
}

void loop() {
}