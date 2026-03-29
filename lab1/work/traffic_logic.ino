// this is the first version i have and it doesn't have cross traffic capability
// so i have to redo it
// the reason why i even did it like this is because it saves me from repeating the code
// it works for single traffic light but nto cross traffic light
#if false

enum TrafficLightState {
  Boot,
  Failure,
  Solid,
  Flashing,
};

enum TrafficLightColor {
  Red,
  Green,
  Yellow,
};

TrafficLightState CURRENT_STATE = TrafficLightState::Solid;
TrafficLightColor CURRENT_COLOR = TrafficLightColor::Red;

#define TRAFFIC_BOOT_FLASH_TIME 1.0
#define TRAFFIC_FAILURE_FLASH_TIME 0.5
#define YELLOW_LIGHT_TIME 3.0
#define TRAFFIC_LAST_SECONDS 3.0 // how many seconds before the end to flash
#define TRAFFIC_LAST_SECOND_FLASH_TIME 0.5 // on and off time

float TRAFFIC_TIMER = 0.0; // this variable is reused as an internal timer for the current light
float FLASH_TIMER = 0.0; // secondary timer for flashing
bool FLASH_HIGH = false;

void color_drive(bool is_high) {
  // let's try to make sense of the other station
  // s1: green -> s2 is red, no exception

  // s1: yellow -> s2 is still not moving, however, s2 is red before
  // so s1 yellow means s2 is flashing
  switch (CURRENT_COLOR) {
    case TrafficLightColor::Red:
      digitalWrite(R1, is_high ? HIGH : LOW);

      // now drive the other station
      digitalWrite(G2, is_high ? HIGH : LOW);
      break;
    case TrafficLightColor::Green:
      digitalWrite(G1, is_high ? HIGH : LOW);
      digitalWrite(R2, is_high ? HIGH : LOW);
      break;
    case TrafficLightColor::Yellow:
      digitalWrite(Y1, is_high ? HIGH : LOW);

      digitalWrite(Y2, is_high ? HIGH : LOW);
      break;
  }
}

void cycle_next_state() {
  switch (CURRENT_STATE) {
    case TrafficLightState::Boot:
    case TrafficLightState::Failure:
      CURRENT_STATE = TrafficLightState::Solid;
      CURRENT_COLOR = TrafficLightColor::Red;
      break;
    case TrafficLightState::Solid:
      switch (CURRENT_COLOR) {
        case TrafficLightColor::Red: // solid and color X -> flashing and color X
        case TrafficLightColor::Green:
          FLASH_HIGH = false; // start with off so it is definitely flashing
          CURRENT_STATE = TrafficLightState::Flashing;
          break;
        case TrafficLightColor::Yellow: // becomes red next, no flashing
          CURRENT_COLOR = TrafficLightColor::Red;
          reset_timer();
          break;
      }
      break;
    case TrafficLightState::Flashing:
      switch (CURRENT_COLOR) {
        case TrafficLightColor::Red:
          CURRENT_COLOR = TrafficLightColor::Green;
          CURRENT_STATE = TrafficLightState::Solid;
          reset_timer();
          break;
        case TrafficLightColor::Green:
          CURRENT_COLOR = TrafficLightColor::Yellow;
          CURRENT_STATE = TrafficLightState::Solid;
          reset_timer();
          break;
        case TrafficLightColor::Yellow: // unreachable
          Serial.println("Unreachable state of flashing yellow");
          CURRENT_COLOR = TrafficLightColor::Red;
          CURRENT_STATE = TrafficLightState::Solid;
          reset_timer();
          break;
      }
      break;
  }
}

float get_curr_max_time() {
  switch (CURRENT_STATE) {
    case TrafficLightState::Boot:
      return TRAFFIC_BOOT_FLASH_TIME;
    case TrafficLightState::Failure:
      return TRAFFIC_FAILURE_FLASH_TIME;
    default:
      switch (CURRENT_COLOR) {
        case TrafficLightColor::Red:
          return gRedSetTime;
        case TrafficLightColor::Green:
          return gGreenSetTime;
        case TrafficLightColor::Yellow:
          return YELLOW_LIGHT_TIME;
      }
  }
}

bool should_flash_last_seconds() {
  return CURRENT_COLOR != TrafficLightColor::Yellow;
}

// traffic light logic only updates on interrupt
void traffic_light_int_loop() {
  update_timer();

  float max_time = get_curr_max_time();
  bool overtime = TRAFFIC_TIMER >= max_time;
  bool at_last_seconds = TRAFFIC_TIMER >= max_time - TRAFFIC_LAST_SECONDS;
  bool switch_flash;

  switch (CURRENT_STATE) {
    case TrafficLightState::Boot:
    case TrafficLightState::Failure:
      switch_flash = TRAFFIC_TIMER >= max_time;

      if (switch_flash) {
        FLASH_HIGH = !FLASH_HIGH;
        reset_timer();
      }

      color_drive(FLASH_HIGH);

      break;
    case TrafficLightState::Solid:
      if ((should_flash_last_seconds() && at_last_seconds) || (!should_flash_last_seconds() && overtime)) {
        color_drive(false);
        cycle_next_state();
      }

      // write high even when it is already high
      // i guess this is fine
      color_drive(true);
      break;
    case TrafficLightState::Flashing:
      switch_flash = FLASH_TIMER >= TRAFFIC_LAST_SECOND_FLASH_TIME;

      // flashing during operational period = will switch to other lights anyway
      if (overtime) {
        color_drive(false); // just off
        cycle_next_state();
      }

      if (switch_flash) {
        FLASH_HIGH = !FLASH_HIGH;
      }

      color_drive(FLASH_HIGH);

      if (switch_flash) {
        FLASH_TIMER = 0.0;
      }

      break;
  }
}

void update_timer() {
  FLASH_TIMER += INTERRUPT_TIME;
  TRAFFIC_TIMER += INTERRUPT_TIME;
}

void reset_timer() {
  FLASH_TIMER = 0.0;
  TRAFFIC_TIMER = 0.0;
}

#endif