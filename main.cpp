/* Author: Rigoberto Acosta-González
   TP_1_Embedded
 */

#include "arm_book_lib.h"
#include "mbed.h"


//=====[Defines]===============================================================
#define NUMBER_OF_SAMPLES_TX 1000 // TOTAL SAMPLES TO BE TRANSMITTED

//=====[Declaration and initialization of public global objects]===============

DigitalIn permissionButton(D7);
DigitalOut recordLed(D3);
DigitalOut permissionLed(D4);
DigitalIn sensorActivition(D2);

AnalogIn sensorMic(A0);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=============

bool recordState = false;
bool permissionState = false;
bool sensorState = OFF;
float sensorMicReading = 0;
//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
void outputsInit();
void recordPermissionUpdate();
void testSensor();
void uartTask();

//=====[Main function, the program entry point after power on or reset]========

int main() {
  inputsInit();
  outputsInit();
  while (true) {
    recordPermissionUpdate();
    testSensor();
  }
}

//=====[Implementations of public functions]===================================

void inputsInit() {
  permissionButton.mode(PullDown); // Button mode PullDown
}

void outputsInit() {
  recordLed = OFF;     // Yellow
  permissionLed = OFF; // Red
}

void recordPermissionUpdate() {

  if (permissionButton) {
    permissionLed = !permissionLed;
    permissionState = !permissionState;
  }
}

void testSensor() {

  if (sensorActivition && permissionState &&
      !recordState) {   // Acá hay código bloqueante
    recordState = true; // Mientras recordState == true solo se transmite
    recordLed = ON;
    uartTask();
    recordState = false;
    recordLed = OFF;
  }
}

void uartTask() {
  char str[100];
  int stringLength;
  for (int i = 0; i < NUMBER_OF_SAMPLES_TX; i++) {
    sensorMicReading = sensorMic.read();
    sprintf(str, "%.2f\r\n", sensorMicReading); //Unsigned pues voy hasta 50000. 
    // thread_sleep_for(10);
    stringLength = strlen(str);
    uartUsb.write(str, stringLength);
  }
}