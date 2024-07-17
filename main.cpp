/* Author: Rigoberto Acosta-González
   TP_1_Embedded
 */

#include "arm_book_lib.h"
#include "mbed.h"


//=====[Defines]===============================================================
#define NUMBER_OF_SAMPLES_TX 5000 // TOTAL SAMPLES TO BE TRANSMITTED
#define DEBOUNCE_BUTTON_TIME_MS 40 
#define TIME_INCREMENT_MS                       10

//=====[Declaration of public data types]======================================
typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_FALLING,
    BUTTON_RISING
} buttonState_t;

//=====[Declaration and initialization of public global objects]===============
DigitalIn permissionButton(D7);
DigitalOut recordLed(D3);
DigitalOut permissionLed(D4);
DigitalIn sensorDigitalOut(D2);

AnalogIn sensorMic(A0);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

//=====[Declaration and initialization of public global variables]=============
bool recordState = false;
bool permissionState = false;
bool sensorState = OFF;
float arrayDataMic[NUMBER_OF_SAMPLES_TX];

int accumulatedDebounceButtonTime     = 0;
int numberOfPermissionButtonReleasedEvents = 0;
buttonState_t permissionButtonState;

//=====[Declarations (prototypes) of public functions]=========================
void inputsInit();
void outputsInit();
void recordPermissionUpdate();
void testSensor();
void uartTask();
void saveDataMic();

void debounceButtonInit();
bool debounceButtonUpdate();

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
    debounceButtonInit();
}

void outputsInit() {
    recordLed = OFF;     // Yellow
    permissionLed = OFF; // Red
}

void recordPermissionUpdate() {
    bool permissionButtonReleasedEvent = debounceButtonUpdate();
    if (permissionButtonReleasedEvent) {
        permissionLed = !permissionLed;
        permissionState = !permissionState;
    }
}

void testSensor() {
    if (sensorDigitalOut && permissionState && !recordState) {  
    recordState = true; // Mientras recordState == true solo se transmite
    recordLed = ON;
    saveDataMic(); //Bloqueante durante Grabación
    recordLed = OFF; //Apago LED de Grabación
    uartTask();  //Bloqueante durante Transmisión
    recordState = false;

    }
}

void saveDataMic(){
    float total = 0;
    for (int i = 0; i < NUMBER_OF_SAMPLES_TX; i++) {
        arrayDataMic[i] = sensorMic.read() * 3.3;
    }
}

void uartTask() {
    char str[100];
    int stringLength;
    for (int i = 0; i < NUMBER_OF_SAMPLES_TX; i++) {
        sprintf(str, "%.2f\r\n", arrayDataMic[i]); 
        stringLength = strlen(str);
        uartUsb.write(str, stringLength);
    }
}

void debounceButtonInit()
{
    if( permissionButton == 1) {
        permissionButtonState = BUTTON_UP;
    } else {
        permissionButtonState = BUTTON_DOWN;
    }
}

bool debounceButtonUpdate()
{
    bool permissionButtonReleasedEvent = false;
    switch( permissionButtonState ) {

    case BUTTON_UP:
        if( permissionButton ) {
            permissionButtonState = BUTTON_FALLING;
            accumulatedDebounceButtonTime = 0;
        }
        break;

    case BUTTON_FALLING:
        if( accumulatedDebounceButtonTime >= DEBOUNCE_BUTTON_TIME_MS ) {
            if( permissionButton ) {
                permissionButtonState = BUTTON_DOWN;
            } else {
                permissionButtonState = BUTTON_UP;
            }
        }
        accumulatedDebounceButtonTime = accumulatedDebounceButtonTime +
                                        TIME_INCREMENT_MS;
        break;

    case BUTTON_DOWN:
        if( !permissionButton ) {
            permissionButtonState = BUTTON_RISING;
            accumulatedDebounceButtonTime = 0;
        }
        break;

    case BUTTON_RISING:
        if( accumulatedDebounceButtonTime >= DEBOUNCE_BUTTON_TIME_MS ) {
            if( !permissionButton ) {
                permissionButtonState = BUTTON_UP;
                permissionButtonReleasedEvent = true;
            } else {
                permissionButtonState = BUTTON_DOWN;
            }
        }
        accumulatedDebounceButtonTime = accumulatedDebounceButtonTime +
                                        TIME_INCREMENT_MS;
        break;

    default:
        debounceButtonInit();
        break;
    }
    return permissionButtonReleasedEvent;
}