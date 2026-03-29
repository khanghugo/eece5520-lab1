//www.elegoo.com
//2016.12.12

int latch=24;  //74HC595  pin 9 STCP
int clock=26; //74HC595  pin 10 SHCP
int data=22;   //74HC595  pin 8 DS

#define DIGIT_UNIT 32
#define DIGIT_TENS 34
#define SWITCH_TIME 12

unsigned char table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c
,0x39,0x5e,0x79,0x71,0x00};

void display_setup() {
  pinMode(latch,OUTPUT);
  pinMode(clock,OUTPUT);
  pinMode(data,OUTPUT);

  pinMode(DIGIT_UNIT, OUTPUT); // digit2 - left to right
  pinMode(DIGIT_TENS, OUTPUT); // digit1
}

void Display(unsigned char num, unsigned char pos)
{
  // every thing off
  digitalWrite(DIGIT_UNIT, HIGH);
  digitalWrite(DIGIT_TENS, HIGH);

  digitalWrite(latch,LOW);
  digitalWrite(pos, LOW);
  shiftOut(data,clock,MSBFIRST,table[num]);
  digitalWrite(latch,HIGH);
  
}
void display_loop() {
  // 32 = unit
  // 34 = tens

  char unit = gDisplayNumber % 10;
  char tens = gDisplayNumber / 10;

  Display(unit, DIGIT_UNIT);
  delay(SWITCH_TIME);
  Display(tens, DIGIT_TENS);
  delay(SWITCH_TIME);
}
