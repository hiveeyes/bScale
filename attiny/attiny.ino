#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int const pinGate = 0; //PIN for MOSFET GATE
long const waitTime = 90000; //How long the mosfet is open
volatile boolean f_wdt=1; //Flag to open MOSFET 1 open 0 closed
volatile int count; //count the circles
int const circles = 225; // Amount of circle till gate will be opened, Watchdog = 8 sec per circle 

void setup(){
  pinMode(pinGate,INPUT); // set all used port to intput to save power
  count = circles - 1; //Init first circle on power up
  
  // CPU Sleep Modes 
  // SM2 SM1 SM0 Sleep Mode
  // 0    0  0 Idle
  // 0    0  1 ADC Noise Reduction
  // 0    1  0 Power-down
  // 0    1  1 Power-save
  // 1    0  0 Reserved
  // 1    0  1 Reserved
  // 1    1  0 Standby(1)

  /*cbi( SMCR,SE );      // sleep enable, power down mode
  cbi( SMCR,SM0 );     // power down mode
  sbi( SMCR,SM1 );     // power down mode
  cbi( SMCR,SM2 );     // power down mode*/

  setup_watchdog(9);
}


void loop(){
  if (f_wdt==1) {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    f_wdt=0;       // reset flag
    count++;
    if(circles == count){
      pinMode(pinGate,OUTPUT); //Set Gate as Output
      count = 0;
      openGate();
      pinMode(pinGate,INPUT); // set all used port to intput to save power
    }
    system_sleep();
  }
}


// set system into the sleep state 
// system wakes up when watchdog is timed out
void system_sleep() {

  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();

  sleep_mode();                        // System sleeps here
  sleep_disable();                     // System continues execution here when watchdog timed out 
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON

}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR  |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR  = bb;
  WDTCR  |= _BV(WDIE);


}

// Watchdog Interrupt Service / is executed when  watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}

// Simple PIN 0 push to open mosfet
void openGate()
{
  digitalWrite(pinGate, HIGH);
  delay(waitTime);
  digitalWrite(pinGate, LOW);
}

