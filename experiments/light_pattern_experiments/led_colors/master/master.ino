// Chuan-Che Huang & Evan Kerrigan, chuanche@umich.edu
// Purpose: Led Light Pattern Experiments
// Title: Color Experiment
// This one is *master*

#include <stdint.h>
#include <ffft.h>
#include <NcrcViz.h>
#include <ledcontroller.h>
#include <Wire.h>

volatile  byte  position = 0;
volatile  long  zero = 0;

int16_t capture[FFT_N];			/* Wave captureing buffer */
complex_t bfly_buff[FFT_N];		/* FFT buffer */
uint16_t spektrum[FFT_N/2];		/* Spectrum output buffer */

//*****
SoundHandler soundHandler = SoundHandler();
boolean humanVoiceHasBeenDetected;
unsigned long timeWhenHumanVoiceIsDetected;
boolean debug = DEBUG_FLAG;
boolean ledAnimationBegin = false;
unsigned int ledAnimationFrameCounter = 0;
unsigned int ledAnimationNumOfFrames = 30;
unsigned int bgAnimationFrameCounter = 0;
unsigned int bgAnimationNumOfFrames = 100;

//I2C Communication
char incomingByte = 0;

// Using Classes from ledcontroller library
using LedController::Color; 
using LedController::LedStrip;
using LedController::MovingPeak;
using LedController::Interval;

Color red(0xFFFF00);
Color prettyblue(0x6FBAFC);
Color white(0xFFFFFF);
Color dye(0x6F6F10);

LedStrip ledStrips[] = {LedStrip(PIN_LED1_OUT_SDI, PIN_LED1_OUT_CKI),
                       LedStrip(PIN_LED2_OUT_SDI, PIN_LED2_OUT_CKI),
                       LedStrip(PIN_LED3_OUT_SDI, PIN_LED3_OUT_CKI),
                       LedStrip(PIN_LED4_OUT_SDI, PIN_LED4_OUT_CKI),
                       LedStrip(PIN_LED5_OUT_SDI, PIN_LED5_OUT_CKI),                     
};

//Moving Peak Pattern
Interval i = Interval(20);
  
#define NUM_COLOR_TEST_IN_EACH_CHANNEL 5
byte i_colorChangingCounter = 0;
byte j_colorChangingCounter = 0;
byte k_colorChangingCounter = 0;
byte R[NUM_COLOR_TEST_IN_EACH_CHANNEL] = {0, 64, 128, 192, 255};
byte G[NUM_COLOR_TEST_IN_EACH_CHANNEL] = {0, 64, 128, 192, 255};
byte B[NUM_COLOR_TEST_IN_EACH_CHANNEL] = {0, 64, 128, 192, 255};

void setup()
{
  //I2C Communication
  Wire.begin();
  
  pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_MIC_IN, INPUT);
  
  // Initialize ADC
  Serial.begin(9600);
  adcInit();
  adcCalb();
  
  // Initialize LED strip
  for(byte i=0; i < NUM_LED_STRIPS_MASTER; i++){
  ledStrips[i].setup();
  ledStrips[i].clear();
  ledStrips[i].send();
  }
  
  delay(2000);
  
  //establishContact();  // send a byte to establish contact until Processing respon
}

void loop()
{    
  // Clear the background first 
  for(byte i=0; i < NUM_LED_STRIPS_MASTER; i++){
    ledStrips[i].clear();
  }
  
  if(i.update()){
    // Change color for LED strips
    k_colorChangingCounter++;
    if(k_colorChangingCounter == NUM_COLOR_TEST_IN_EACH_CHANNEL){
      k_colorChangingCounter = 0;
      j_colorChangingCounter++;
      if(j_colorChangingCounter == NUM_COLOR_TEST_IN_EACH_CHANNEL){
        j_colorChangingCounter = 0;
        i_colorChangingCounter++;
        if(i_colorChangingCounter == NUM_COLOR_TEST_IN_EACH_CHANNEL){
          i_colorChangingCounter = 0;
        }
      }
    }
    i.clearExpired();
  }

  for(byte i=0; i < NUM_LED_STRIPS_MASTER; i++){
    byte r = R[i_colorChangingCounter];
    byte g = G[j_colorChangingCounter];
    byte b = B[k_colorChangingCounter];
    for(byte j=0; j < NUM_LEDS; j++){
      ledStrips[i].getColors()[j].setChannelValues(r, g, b);
    }
  }
  
//  For Animation  
//  bgAnimationFrameCounter++;
//  if(ledAnimationFrameCounter >= bgAnimationNumOfFrames){
//    bgAnimationFrameCounter = 0;
//  }
  
  // Draw the LEDs!
  for(byte i=0; i < NUM_LED_STRIPS_MASTER; i++){  
    ledStrips[i].send();
  }
  
  // Audio Signal Processing
  if (position == FFT_N)
  {
    
    /* Audio Signal Processor Begin*/
    // 1. FFT    
    fft_input(capture, bfly_buff);
    fft_execute(bfly_buff);
    fft_output(bfly_buff, spektrum);
    
    // 2. Audio Recognizer
    boolean detectHumanVoice = soundHandler.containHumanVoice(spektrum, 64);
    
    
    // 3. State Machine Code, Also a Central Controller 
    // Controller should set different rendering flag in order to run different 
    // led animations.
//    if(detectHumanVoice == true){
//     if(humanVoiceHasBeenDetected == true){
//        //...
//      } else {
//        // Send signal to slave to activate the LED
//        sendEventToSlave(EVENT_DETECT_HUMAN_VOICE);
//        
//        // Change the control flag and begin render the special animation
//        humanVoiceHasBeenDetected = true;
//        timeWhenHumanVoiceIsDetected = millis();
//        
//        if(ledAnimationBegin == false){
//          ledAnimationBegin = true;
//        }
//      }
//  
//    }
  
   position = 0;
  }//end position==FFT_N

}


/* Facility Functions Begin */
void establishContact() {
 while (Serial.available() <= 0) {
      Serial.write('A');   // send a capital A
      delay(300);
  }
}

// free running ADC fills capture buffer
ISR(ADC_vect)
{
  if (position >= FFT_N)
    return;
  
  capture[position] = ADC + zero;
  if (capture[position] == -1 || capture[position] == 1)
    capture[position] = 0;

  position++;
}
void adcInit(){
  /*  REFS0 : VCC use as a ref, IR_AUDIO : channel selection, ADEN : ADC Enable, ADSC : ADC Start, ADATE : ADC Auto Trigger Enable, ADIE : ADC Interrupt Enable,  ADPS : ADC Prescaler  */
  // free running ADC mode, f = ( 16MHz / prescaler ) / 13 cycles per conversion 
  ADMUX = _BV(REFS0) | IR_AUDIO; // | _BV(ADLAR); 
//  ADCSRA = _BV(ADSC) | _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) //prescaler 64 : 19231 Hz - 300Hz per 64 divisions
  ADCSRA = _BV(ADSC) | _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // prescaler 128 : 9615 Hz - 150 Hz per 64 divisions, better for most music
  sei();
}
void adcCalb(){
  Serial.println("Start to calc zero");
  long midl = 0;
  // get 2 meashurment at 2 sec
  // on ADC input must be NO SIGNAL!!!
  for (byte i = 0; i < 2; i++)
  {
    position = 0;
    delay(100);
    midl += capture[0];
    delay(900);
  }
  zero = -midl/2;
  Serial.println("Done.");
}

//I2C Communication
void sendEventToSlave(byte event)
{
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(event);
  Wire.endTransmission();
}

