#ifndef _BUTTON_H
#define _BUTTON_H

#include <Arduino.h>

/*

  This is used to capture button input to a single ADC input pin.

  Buttons are attatched to single pin with resistors. 

  If using different resistors, BUTTON_X_VOLTAGE values need to be recalculated.

  ADC_SAMPLES and VOLTAGE_RANGE is used to account for fluctuating voltage values.

  buttonReturn struct contains:

    buttonEvent - Button state.
    buttonType - BUtton type.

*/

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