#include "button.h"

static buttonType lastButtonPressed = NO_PRESS;
static uint32_t lastChangeTime = 0;
static const uint32_t DEBOUNCE_MILLIS = 50;
static bool waitingForRelease = false;
static buttonType buttonDownType = NO_PRESS;

static int readSmoothedADC() {
  long sum = 0;
  for (int i = 0; i < ADC_SAMPLES; ++i) {
    sum += analogRead(BUTTON_PIN);
  }
  return (int)(sum / ADC_SAMPLES);
}

buttonType getButtonType(int voltage) {

  if (abs(voltage - BUTTON_1_VOLTAGE) < VOLTAGE_RANGE) return BUTTON_1;
  if (abs(voltage - BUTTON_2_VOLTAGE) < VOLTAGE_RANGE) return BUTTON_2;
  if (abs(voltage - BUTTON_3_VOLTAGE) < VOLTAGE_RANGE) return BUTTON_3;

  return NO_PRESS;

}

buttonReturn getButtonEvent(){

  buttonReturn returnButton = {

    .event = NO_EVENT,
    .type = NO_PRESS

  };

  uint32_t now = millis();

  int voltage = readSmoothedADC();

  buttonType current = getButtonType(voltage);

  if(current != lastButtonPressed){

    if(now - lastChangeTime >= DEBOUNCE_MILLIS){

      lastChangeTime = now;
      lastButtonPressed = current;

      if(current != NO_PRESS){

        if(!waitingForRelease){

          buttonDownType = current;
          waitingForRelease = true;

        }

      }

      else{

        if(waitingForRelease){

          returnButton.event = SINGLE_PRESS;
          returnButton.type = buttonDownType;

          waitingForRelease = false;
          buttonDownType = NO_PRESS;

        }

      }

    }

  }

  return returnButton;

}