#define INTERRUPT_TIME 0.5 // in seconds
#define INTERRUPT_TICKRATE (int) (1 / INTERRUPT_TIME)

// #define DEBUG

// dont you tell me buzzer is pleasant to hear
#define BUZZER_ON
#define BUZZER 7

#define G1 2
#define Y1 3
#define R1 4

#define G2 11
#define Y2 12
#define R2 13

#define KEY_RESET_WINDOW 2 // x seconds before input buffer is cleared
#define KEY_BUFFER_SIZE 4 // if buffer is maxed then instantly collect inputs
uint8_t gKeyBuffer[KEY_BUFFER_SIZE];
uint8_t gKeyBufferIndex = 0;

// I love sepples can't handle namespace for enum
enum KeypadCode {
  KeypadNone,
  KeypadStar,
  KeypadFailure,
  KeypadSet,
  #ifdef DEBUG
  KeypadDebugPreset
  #endif
};

KeypadCode gKeypadCode = KeypadCode::KeypadNone;
uint8_t gKeypadRedTime = 0;
uint8_t gKeypadGreenTime = 0;

#define KEY_RED_CHAR 'A'
#define KEY_GREEN_CHAR 'B'

uint8_t gRedSetTime = 0;
uint8_t gGreenSetTime = 0;

uint8_t gDisplayNumber = 0;