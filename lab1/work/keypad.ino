// code from elegoo
// i just delete comments and change the pins
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
const byte rowPins[ROWS] = {53, 51, 49, 47}; //connect to the row pinouts of the keypad
const byte colPins[COLS] = {45, 43, 41, 39}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

float seconds_since_last_input = 0.0;

void keypad_setup(){
  // Serial.begin(9600);
}

void keypad_loop() {
  char customKey = customKeypad.getKey();

  if (customKey) {
    #ifdef DEBUG
    Serial.print("Received key ");
    Serial.println(customKey);
    #endif

    gKeyBuffer[gKeyBufferIndex] = customKey;
    ++gKeyBufferIndex;

    if (gKeyBufferIndex == KEY_BUFFER_SIZE) {
      keypad_collect_input();
    }

    seconds_since_last_input = 0.0;
  }
}

#ifdef DEBUG
char keypad_read_last_key() {
  char index = (gKeyBufferIndex - 1) % KEY_BUFFER_SIZE;
  char res = gKeyBuffer[index];

  // then reset the entire buffer because read = consumes
  keypad_reset();

  return res;
}
#endif

void keypad_collect_input() {
  // no key
  if (gKeyBufferIndex == 0) {
    keypad_reset();
    return;
  }

  // at least 1 key input
  if (gKeyBuffer[gKeyBufferIndex - 1] == '*') {
    gKeypadCode = KeypadCode::KeypadStar;

    #ifdef DEBUG
    Serial.println("Set input Set mode");
    #endif

    buzz();
    keypad_reset();
    return;
  }

  // need at lesat 2 keys to enter failure mode
  if (gKeyBufferIndex < 2) {
    keypad_reset();
    return;
  }

  // fast debug without me wasting 30 seconds to input every time i want to test
  #ifdef DEBUG
  if (gKeyBufferIndex >= 3 && gKeyBuffer[gKeyBufferIndex - 1] == '#' && gKeyBuffer[gKeyBufferIndex - 2] == '#' && gKeyBuffer[gKeyBufferIndex - 3] == '#') {
    gKeypadCode = KeypadCode::KeypadDebugPreset;

    Serial.println("Set input Debug mode");

    buzz();
    keypad_reset();
    return;
  }
  #endif

  if (gKeyBuffer[gKeyBufferIndex - 1] == '#' && gKeyBuffer[gKeyBufferIndex - 2] == '#') {
    gKeypadCode = KeypadCode::KeypadFailure;

    #ifdef DEBUG
    Serial.println("Set input Failure mode");
    #endif

    buzz();
    keypad_reset();
    return;
  }

  // need at least 4 keys to set things
  if (gKeyBufferIndex < 4) {
    keypad_reset();
    return;
  }

  // only works if the last key is pound
  if (gKeyBuffer[gKeyBufferIndex - 1] == '#') {    
    char ten_e0 = gKeyBuffer[gKeyBufferIndex - 2] - '0';
    char ten_e1 = gKeyBuffer[gKeyBufferIndex - 3] - '0';

    // something wrong
    if (ten_e1 > 9) {
      keypad_reset();
      return;
    }

    char set_time = ten_e1*10 + ten_e0;

    if (gKeyBuffer[gKeyBufferIndex - 4] == 'A') {
      gKeypadRedTime = set_time; 
    } else if (gKeyBuffer[gKeyBufferIndex - 4] == 'B') {
      gKeypadGreenTime = set_time; 
    }

    gKeypadCode = KeypadCode::KeypadSet;

    #ifdef DEBUG
    Serial.println("Set input");
    Serial.print("Red ");
    Serial.println((int) gKeypadRedTime);
    Serial.print("Green ");
    Serial.println((int) gKeypadGreenTime);
    #endif

    buzz();
    keypad_reset();
    return;
  }
}

void keypad_reset() {
  // lazy remove
  gKeyBufferIndex = 0;

  seconds_since_last_input = 0.0;
}

void keypad_int_loop() {
  seconds_since_last_input += INTERRUPT_TIME;

  if (seconds_since_last_input >= KEY_RESET_WINDOW) {
    keypad_collect_input();
  }
}
