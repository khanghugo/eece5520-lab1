#include "shared.h"

// code from the lab1 repo
void setup(){
  buzzer_setup();
  keypad_setup();
  led_setup();
  traffic_light_setup();
  display_setup();

  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  // DO NOT TOUCH ANYTHING HERE
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  /**
  KL: I modify the timer here from 1*1024 to 2 * 1024.
  So, the value changes from 15624 to 7811.5
  */ 
  OCR1A = 7812;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts

}//end setup

ISR(TIMER1_COMPA_vect){
  buzzer_int_loop();
  keypad_int_loop();
  traffic_light_int_loop();

  // gDisplayNumber = (gDisplayNumber + 1) % 100;
}


void loop() {
  keypad_loop();
  display_loop();
}