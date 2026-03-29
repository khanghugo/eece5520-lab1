enum BuzzState {
  Idle,
  WantToBuzz,
  Buzzing,
};

BuzzState buzz_state = BuzzState::Idle;

void buzz() {
  buzz_state = BuzzState::WantToBuzz;
}

void buzzer_setup() {
  pinMode(BUZZER, OUTPUT);
}

void buzzer_int_loop() {
  switch (buzz_state) {
    case BuzzState::WantToBuzz:
      #ifdef BUZZER_ON
      digitalWrite(BUZZER, HIGH);
      #endif
      buzz_state = BuzzState::Buzzing;
      break;
    // this has crazy assumption
    // we only buzz for 1 tick (0.5s or 2 ticks per second)
    // so, we know for sure that this buzz persists for exactly 0.5 second
    // meaning we stop buzzing when the loop is called
    case BuzzState::Buzzing:
      digitalWrite(BUZZER, LOW);
      break;
    case BuzzState::Idle:
    default:
      break;
  }
}