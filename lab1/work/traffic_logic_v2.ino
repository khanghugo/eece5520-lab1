enum TrafficLightState {
  Boot = 0, // i was crazy debugging this stuffs so i use random enum just to verify that switch case works
  Failure = 4, // turns out, it is some compiler stuffs where if i declare variable inside a switch statement
  Solid = 8, // i will be in a world of pain
  Flashing = 16,
};

enum TrafficLightColor {
  Red,
  Green,
  Yellow,
};

#define TRAFFIC_BOOT_FLASH_TIME 1.0
#define TRAFFIC_FAILURE_FLASH_TIME 0.5
#define YELLOW_LIGHT_TIME 3.0
#define TRAFFIC_LAST_SECONDS 3.0 // how many seconds before the end to flash
#define TRAFFIC_LAST_SECOND_FLASH_TIME 0.5 // on and off time

bool CLEAR_KEYPAD_CODE = false;
bool CLEAR_KEYPAD_NUMBER = false;
bool SET_START = false;

extern "C"
{
  uint8_t calculate_cross_traffic_red_time(uint8_t green_time);
  uint8_t calculate_cross_traffic_green_time(uint8_t red_time);
}

class TrafficLight {
  public:
  TrafficLightState current_state;
  TrafficLightColor current_color;

  int red_pin, green_pin, yellow_pin;

  float traffic_timer = 0.0; // this variable is reused as an internal timer for the current light
  float flash_timer = 0.0; // secondary timer for flashing
  bool flash_high = false;
  bool is_cross_traffic = false;

  TrafficLight(int red_pin, int green_pin, int yellow_pin, bool cross_traffic) {
    this->red_pin = red_pin;
    this->green_pin = green_pin;
    this->yellow_pin = yellow_pin;
    this->is_cross_traffic = cross_traffic;

    current_state = TrafficLightState::Boot;
    current_color = TrafficLightColor::Red;

    // if (this->is_cross_traffic) {
    //   current_color = TrafficLightColor::Green;
    // } else {
    //   current_color = TrafficLightColor::Red;
    // }
  }

  void color_drive(bool is_high) {
    // let's try to make sense of the other station
    // s1: green -> s2 is red, no exception

    // s1: yellow -> s2 is still not moving, however, s2 is red before
    // so s1 yellow means s2 is flashing
    switch (current_color) {
      case TrafficLightColor::Red:
        digitalWrite(red_pin, is_high ? HIGH : LOW);
        break;
      case TrafficLightColor::Green:
        digitalWrite(green_pin, is_high ? HIGH : LOW);
        break;
      case TrafficLightColor::Yellow:
        digitalWrite(yellow_pin, is_high ? HIGH : LOW);
        break;
    }
  }

  void enter_failure_mode() {
    this->current_state = TrafficLightState::Failure;

    // turn off current color
    this->color_drive(false);

    // always red
    this->current_color = TrafficLightColor::Red;

    reset_timer();
  }

  void cycle_next_state() {
    // need to turn off current color
    // always have to do this
    this->color_drive(false);

    switch (this->current_state) {
      case TrafficLightState::Boot:
      case TrafficLightState::Failure:
        this->current_state = TrafficLightState::Solid;

        if (is_cross_traffic) {
          this->current_color = TrafficLightColor::Green;
        } else {
          this->current_color = TrafficLightColor::Red;
        }

        reset_timer();
        break;
      case TrafficLightState::Solid:
        switch (current_color) {
          case TrafficLightColor::Red: // solid and color X -> flashing and color X
          case TrafficLightColor::Green:
            // this is pretty interesting
            // if flashing starts with high, it means low is "offbeat"
            // meaning the color switch will be 2 ticks long
            // if this is to be "true", it looks very "sluggish"
            flash_high = false;
            current_state = TrafficLightState::Flashing;
            break;
          case TrafficLightColor::Yellow: // becomes red next, no flashing
            current_color = TrafficLightColor::Red;
            reset_timer();
            break;
        }
        break;
      case TrafficLightState::Flashing:
        switch (current_color) {
          case TrafficLightColor::Red:
            current_color = TrafficLightColor::Green;
            current_state = TrafficLightState::Solid;
            break;
          case TrafficLightColor::Green:
            current_color = TrafficLightColor::Yellow;
            current_state = TrafficLightState::Solid;
            break;
          case TrafficLightColor::Yellow: // unreachable
            Serial.println("Unreachable state of flashing yellow");
            current_color = TrafficLightColor::Red;
            current_state = TrafficLightState::Solid;
            break;
        }

        reset_timer();
        break;
    }
  }

  float get_curr_max_time() {
    switch (current_state) {
      case TrafficLightState::Boot:
        return TRAFFIC_BOOT_FLASH_TIME;
      case TrafficLightState::Failure:
        return TRAFFIC_FAILURE_FLASH_TIME;
      default:
        // this is where i control the other traffic light
        // let's list the cases
        // main = red -> cross = green
        // main = red with last 3 seconds -> cross = flashing green
        // main = flashing red -> cross = yellow
        // main = green -> cross = red
        // main = flashing green -> cross is still red
        // main = yellow -> cross is flashing red 
        // so, when main red is R seconds and green is G seconds
        // cross green is R-3 seconds and red is G+3 seconds

        // for some reasons, this piece of code is subtly different from the other piece of code
        // the flashing is very different
        // I think this is outside the scope of the project and I will not dig any deeper
        // switch (current_color) {
        //   case TrafficLightColor::Red:
        //     return is_cross_traffic ? (gGreenSetTime + 3.0) : gRedSetTime;
        //   case TrafficLightColor::Green:
        //     return is_cross_traffic ? (gRedSetTime - 3.0) : gGreenSetTime;
        //   case TrafficLightColor::Yellow:
        //     return YELLOW_LIGHT_TIME;
        // }
        switch (current_color) {
          case TrafficLightColor::Red:
            return (float) is_cross_traffic ? calculate_cross_traffic_red_time(gGreenSetTime) : gRedSetTime;
          case TrafficLightColor::Green:
            return (float) is_cross_traffic ? calculate_cross_traffic_green_time(gRedSetTime) : gGreenSetTime;
          case TrafficLightColor::Yellow:
            return YELLOW_LIGHT_TIME;
        }
    }
  }

  bool should_flash_last_seconds() {
    return current_color != TrafficLightColor::Yellow;
  }

  bool in_set_mode() {
    return current_state == TrafficLightState::Boot || current_state == TrafficLightState::Failure;
  }

  // traffic light logic only updates on interrupt
  void int_loop() {
    float max_time = get_curr_max_time();
    bool overtime = traffic_timer >= max_time;
    bool at_last_seconds = traffic_timer >= (max_time - TRAFFIC_LAST_SECONDS);
    bool switch_flash;

    switch (current_state) {
      case TrafficLightState::Boot:
      case TrafficLightState::Failure:
        switch_flash = traffic_timer >= max_time;

        if (switch_flash) {
          flash_high = !flash_high;
          reset_timer();
        }

        color_drive(flash_high);

        break;
      case TrafficLightState::Solid:
        if ((should_flash_last_seconds() && at_last_seconds) || (!should_flash_last_seconds() && overtime)) {
          // buzz during overtime if is red or is green
          // also cross traffic should not buzz
          if (should_flash_last_seconds() && !is_cross_traffic) {
            buzz();
          }

          cycle_next_state();
          break;
        }

        // write high even when it is already high
        // i guess this is fine
        color_drive(true);
        break;
      case TrafficLightState::Flashing:
        switch_flash = flash_timer >= TRAFFIC_LAST_SECOND_FLASH_TIME;

        // flashing during operational period = will switch to other lights anyway
        if (overtime) {
          cycle_next_state();
          break;
        }

        // always buzzes when it is flashing
        // only buzzes when it is not cross traffic also
        // condition stays after this so it doesnt buzz when overtime
        if (!is_cross_traffic)
          buzz();

        if (switch_flash) {
          flash_high = !flash_high;
        }

        color_drive(flash_high);

        if (switch_flash) {
          flash_timer = 0.0;
        }

        break;
    }
  
    update_timer();
  }

  void update_timer() {
    flash_timer += INTERRUPT_TIME;
    traffic_timer += INTERRUPT_TIME;
  }

  void reset_timer() {
    flash_timer = 0.0;
    traffic_timer = 0.0;
  }
};

TrafficLight main_light(R1, G1, Y1, false);
TrafficLight cross_traffic(R2, G2, Y2, true);

void traffic_light_input_polling() {
  bool is_star_code = gKeypadCode == KeypadCode::KeypadStar;
  bool is_failure_code = gKeypadCode == KeypadCode::KeypadFailure;
  bool in_set_mode = main_light.in_set_mode();

  #ifdef DEBUG
  if (gKeypadCode == KeypadCode::KeypadDebugPreset) {
    CLEAR_KEYPAD_CODE = true;
    gRedSetTime = gGreenSetTime = 8;
    Serial.println("Debug fast track into operational mode");
    main_light.cycle_next_state();
    cross_traffic.cycle_next_state();
  }
  #endif

  // set mode
  if (is_star_code && !SET_START && in_set_mode) {
    // start of the set mode, nothing special
    // this scope is after the other scope
    // otherwise, i am in a very weird loop limbo
    SET_START = true;
    CLEAR_KEYPAD_CODE = true;
    CLEAR_KEYPAD_NUMBER = true;

    #ifdef DEBUG
    Serial.println("Set mode start");
    #endif
  } else if (is_star_code && SET_START && in_set_mode) {
    // end of the set mode
    // verify data is correct
    SET_START = false;
    CLEAR_KEYPAD_CODE = true;

    // all ok, go next
    if (gKeypadRedTime != 0 && gKeypadGreenTime != 0) {
      gRedSetTime = gKeypadRedTime;
      gGreenSetTime = gKeypadGreenTime;
      CLEAR_KEYPAD_NUMBER = true;

      main_light.cycle_next_state();
      cross_traffic.cycle_next_state();
    }

    #ifdef DEBUG
    Serial.println("Set mode stop");
    #endif
  }

  // failure mode
  if (!in_set_mode && is_failure_code) {
    main_light.enter_failure_mode();
    cross_traffic.enter_failure_mode();

    // failure mode is automatic set start true
    SET_START = true;

    CLEAR_KEYPAD_NUMBER = true;
    CLEAR_KEYPAD_CODE = true;
  }

  // reset states
  if (CLEAR_KEYPAD_CODE) {
    gKeypadCode = KeypadCode::KeypadNone;
    CLEAR_KEYPAD_CODE = false;
  }

  if (CLEAR_KEYPAD_NUMBER) {
    gKeypadRedTime = 0;
    gKeypadGreenTime = 0;
    CLEAR_KEYPAD_NUMBER = false;
  }
}

void traffic_light_display() {
  char max_time = main_light.get_curr_max_time();
  gDisplayNumber = max_time - main_light.traffic_timer;
}

void traffic_light_int_loop() {
  traffic_light_input_polling();
  traffic_light_display();

  main_light.int_loop();
  cross_traffic.int_loop();
}

void traffic_light_setup() {

}