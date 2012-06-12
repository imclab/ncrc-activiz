/*
This Example acquire analog signal from A0 of Arduino, and Serial out to Processing application to visualize.
Tested with preamplified audio data. Take a look at http://www.youtube.com/watch?v=drYWullBWcI

Analog signal is captured at 9.6 KHz, 64 spectrum bands each 150Hz which can be change from adcInit()
Load the this file to Arduio, run Processing application.

Original Fixed point FFT library is from ELM Chan, http://elm-chan.org/works/akilcd/report_e.html
A way to port it to the Arduino library and most demo codes are from AMurchick http://arduino.cc/forum/index.php/topic,37751.0.html
Processing app is based on codes from boolscott http://boolscott.wordpress.com/2010/02/04/arduino-processing-analogue-bar-graph-2/
*/


#include <stdint.h>
#include <ffft.h>
#include <NcrcViz.h>
#include <ledcontroller.h>
#define  IR_AUDIO  0 // ADC channel to capture


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
unsigned int bgAnimationNumOfFrames = 200;

// Using Classes from ledcontroller library
using LedController::Color; 
using LedController::LedStrip;

Color red(0xFFFF00);
Color prettyblue(0x6FBAFC);

LedStrip ledStrips[] = {LedStrip(PIN_LED1_OUT_SDI, PIN_LED1_OUT_CKI),
                       LedStrip(PIN_LED2_OUT_SDI, PIN_LED2_OUT_CKI),
                       LedStrip(PIN_LED3_OUT_SDI, PIN_LED3_OUT_CKI),
                       LedStrip(PIN_LED4_OUT_SDI, PIN_LED4_OUT_CKI),
                       LedStrip(PIN_LED5_OUT_SDI, PIN_LED5_OUT_CKI),                     
};

void setup()
{
  pinMode(PIN_LED1_OUT_SDI, OUTPUT);
  pinMode(PIN_LED1_OUT_CKI, OUTPUT);
  pinMode(PIN_LED2_OUT_SDI, OUTPUT);
  pinMode(PIN_LED2_OUT_CKI, OUTPUT);
  pinMode(PIN_LED3_OUT_SDI, OUTPUT);
  pinMode(PIN_LED3_OUT_CKI, OUTPUT);
  pinMode(PIN_LED4_OUT_SDI, OUTPUT);
  pinMode(PIN_LED4_OUT_CKI, OUTPUT);
  pinMode(PIN_LED5_OUT_SDI, OUTPUT);
  pinMode(PIN_LED5_OUT_CKI, OUTPUT);
  
  pinMode(PIN_IR_IN, INPUT);
  pinMode(PIN_MIC_IN, INPUT);
  
  // Initialize ADC
  Serial.begin(9600);
  adcInit();
  adcCalb();
  
  // Initialize LED strip
  for(byte i=0; i < NUM_LED_STRIPS; i++){
  ledStrips[i].setup();
  ledStrips[i].clear();
  ledStrips[i].send();
  }
  
  delay(2000);
  
  //establishContact();  // send a byte to establish contact until Processing respon
}

void loop()
{
 
  
  
  if (position == FFT_N)
  {
    
    //draw background
    for(byte i=0; i < NUM_LED_STRIPS; i++){
      ledStrips[i].clear();
    }
    float scale = 0.45 + 0.35*sin( ( float(bgAnimationFrameCounter)/float(bgAnimationNumOfFrames)) * 2*PI + PI/2 );
    for(byte i=0; i < 32; i++){
      for(byte j=0; j < NUM_LED_STRIPS; j++){
        ledStrips[j].getColors()[i].add(prettyblue.scaled(scale));
      }
    }
      
    bgAnimationFrameCounter++;
    if(ledAnimationFrameCounter >= bgAnimationNumOfFrames){
      bgAnimationFrameCounter = 0;
    }

    
    
    
    fft_input(capture, bfly_buff);
    fft_execute(bfly_buff);
    fft_output(bfly_buff, spektrum);

//    debug && Serial.println("Raw spectrum");
//    for(byte i = 0; i < 20; i++){
//      Serial.println(spektrum[i]);
//    }
    
//    Serial.println("--end--");
    boolean detectHumanVoice = soundHandler.containHumanVoice(spektrum, 64);
    if(detectHumanVoice == true){
//      debug && Serial.println("**********************HUMAN VOICE***************");
      if(humanVoiceHasBeenDetected == true){
        //...
      } else {
        humanVoiceHasBeenDetected = true;
        timeWhenHumanVoiceIsDetected = millis();
        if(ledAnimationBegin == false){
          ledAnimationBegin = true;
        }
      }
  
    }
    if(humanVoiceHasBeenDetected == true){
      unsigned long now = millis();
      if(now - timeWhenHumanVoiceIsDetected > 3000){
        humanVoiceHasBeenDetected = false;
      }
    } 
   
     
  if(ledAnimationBegin == true){
      for(byte i=0; i < NUM_LED_STRIPS; i++){
        ledStrips[i].clear();
      }
      float scale = 0.45 + 0.35*sin( ( float(ledAnimationFrameCounter)/float(ledAnimationNumOfFrames)) * 2*PI + PI/2 );
      for(byte i=0; i < 32; i++){
        for(byte j=0; j < NUM_LED_STRIPS; j++){
          ledStrips[j].getColors()[i].add(red.scaled(scale));
        }
      }
      
      ledAnimationFrameCounter++;
      if(ledAnimationFrameCounter >= ledAnimationNumOfFrames){
        ledAnimationBegin = false;
        ledAnimationFrameCounter = 0;
      }
      
  }
  
  for(byte i=0; i < NUM_LED_STRIPS; i++){  
    ledStrips[i].send();
  }
  
   position = 0;
  }//end position==FFT_N

}

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




