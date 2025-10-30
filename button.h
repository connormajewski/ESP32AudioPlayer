#ifndef _BUTTON_H
#define _BUTTON_H

#include <Arduino.h>

#define BUTTON_PIN 32

#define BUTTON_1_VOLTAGE 4050
#define BUTTON_2_VOLTAGE 3500
#define BUTTON_3_VOLTAGE 3100
#define MAX_VOLTAGE 4095
#define VOLTAGE_RANGE 80
#define ADC_SAMPLES 5

typedef enum{

  NO_EVENT,
  SINGLE_PRESS

} buttonEvent;

typedef enum{

  NO_PRESS = 0,
  BUTTON_1,
  BUTTON_2,
  BUTTON_3,

} buttonType;

typedef struct {

  buttonEvent event;
  buttonType type;

} buttonReturn;

buttonReturn getButtonEvent();
buttonType getButtonType(int voltage);

#endif