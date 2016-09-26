/*

  09/17/2016 - QuadsX.com - this Open-Source Release of the "PPM Pulse Timing Calibrator for Arduino" is made available on github 
  
  Version 1.0.1 - QuadsX_PPM_Pulse_Timing_Calibrator.ino
  
  
  --------------
  
  You will need to open the Arduino->Tools->Serial Monitor display to view the output from this sketch
  
  --------------
  
  With the popularity of RC and especially quadcopters and multirotor devices which rely on PID controllers, the calibration of
  timing signals for remote control is more critical than ever.
  
  Most of the lesser expensive TX units do not have endpoint or travel adjustments that display actual microseconds output which is unfortunate.
  
  This Arduino Sketch will allow any TX that produces PPM output to become accurately-helpful in setting up the TX and Flight Controller
  
  Please search and read about 'PPM, timing, end-point (travel-) adjustment, and subtrim'
  
  NOTE:
     Travel Adjust and End-Points are the SAME THING, although slightly confusing until you read more,,,
     
  Search on YouTube for video content for PPM Travel Subtrim Adjust for yor RC Transmitter 
  If you have a Spektrum DX6i Transmitter, then you would enter soething like...
  
  - i.e. Search For ---->   'Spektrum DX6i PPM Travel Subtrim Adjust'
  
  Proper use of the setup methods and setting your TX end-points and subtrim properly will mean that your flight experience will become more/very predictable and your need to use
  front-panel TX sub-trim controls will be minimal and maybe not necessary at all
  
  Starting off by calibrating your TX timing will make setting PID much easier and more predictable.
  
  No flight controller can become properly stable if the range of timing values it receives is outside of the range of values that the flight 
  controller expects.
  
  Software in flight controllers may actually discard erroneous values from a TX in order to compensate for bad values, but this is actually
  working around the useful control range that may otherwise be avaiable from gimbals and other control elements.
  
  Flight controllers depend on 1000-1500-2000 (low-mid-high) microsecond timing ranges to make predictable navigation possible.
  
  The Arduino UNO uses a 16mhz clock source and this provides a stable timebase to derive these pulse measurements.
  
  Timing values have been validated on several oscilloscopes and will be accurate with no code modifications when run on a 16mhz ATmega328P.
  
  This sketch has been used to calibrate RC Transmitters from Spektrum, Futaba, Flysky, and others by creating a connector cable as described below.
  
  ---------------------------

  >>>>>>>>>   this sketch uses Arduino Pin D4 for PPM input from the RC Transmitter's PPM / Trainer Port   <<<<<<<<<
  
  NOTE: 
  Pin 4 was selected to allow further development and modification such as color display addition, etc without relocating the connection pin
  - 
  NOTE:
  signal polarity of the PPM signal should not be an issue as this code is triggered by timing changes and not level of voltage in the PPM signal
  -
  NOTE:
  it is highly recommended that a 270 ohm resistor be connected between the transmitter output and Arduino pin D4 input - this will limit the current 
  which might flow from the TX into the Arduino to a reasonable maximum

  ------------
  
  do not expect the code to operate if there is no PPM signal on Arduino pin D4
  
  output is displayed on the Serial Monitor at 115200 BPS
  
  PPM channel data is displayed from left to right - left-most data is from PPM channel #1, and so on
  
  RC expects to 'see' 1000 for the minimum, 1500 for the mid-point, and 2000 for the maximum on any channel
  
  values displayed are in microseconds (us) i.e. 1000 = 1000 microseconds (= 1000 us)
  
  setting channel values to lower than 1000 us or higher than 2000 us may cause problems with your flight or rc controller and would be unusual
  
  
  ARDUINO BOARD DIAGRAM (Pin 0 - 13 side of Arduino pcb)
  
  ------
       |
       |
   GND |--------------------------<   GND FROM TX PPM / SIMULATOR OUTPUT CONNECTOR 
       |     270 ohm
    D4 |----/\/\/\/\--------------<   PPM INPUT FROM TX PPM/SIMULATOR OUTPUT CONNECTOR
       |
       |
  ------
  
  NOTE: see the TX user manual or search online for your RC Transmitter/Model Number for PPM / Trainer output wiring connections
  
        each manufacturer may have one or more different connectors to allow the PPM output connection
        
  NOTE: It is possible to change the input connection to different pins using alternate pin/register programming ans isr interrupt routines
        code possibilities are inluded in comments - see the ATmega328P Specification PDF for more detailed information

*/


#include <Arduino.h>

int update_interval = 2000;     // this will help update the Serial Monitor display every so often when there is no PPM data input

#define MAX_PPM_CHANNELS 10

#define MID_PULSE    600        // the sync pulse within each PPM pulse width will be less than this value in microseconds
#define OVER_PULSE  2100        // this is past valid PPM pulse width and this code does assume it is now waiting for the next sequence to begin

volatile int channel_count = 0;  // this allows the dynamic handling of PPM data - up to 9 channels handled in this version of the sketch

volatile int which_channel = (int)-1;                                   // the heart of the ISR update and performance
volatile unsigned long ppm_sync_time = (unsigned long)0;
volatile unsigned long p_sync_time = (unsigned long)0;

#define LED_PIN 13                                                     // this is the LED pin - could be changed to your needs

volatile int receiver_input[MAX_PPM_CHANNELS];  
volatile unsigned long timer[MAX_PPM_CHANNELS];
volatile unsigned long current_time;

static unsigned long loop_timer;


// set up the necessary hardware items

void setup() {

  DDRB |= B00100000;                                                    // Configure digital port PB5 - Arduino D13 - as output (LED)
 
  digitalWrite((int)LED_PIN, HIGH);                                     //Turn on the LED.
  
  //PCICR |= (1 << PCIE0);                                              //turn on Port B - Set PCIE0 to enable PCMSK0 scan.
  //PCICR |= (1 << PCIE1);                                              //turn on Port C - Set PCIE1 to enable PCMSK1 scan.
  PCICR |= (1 << PCIE2);                                              //turn on Port D - Set PCIE2 to enable PCMSK2 scan.

  //PCMSK0 |= (1 << PCINT0);                                  //Set PCINT0 (Arduino pin D4) to trigger an interrupt on state change.
  
  //PCMSK0 |= 0b00000000;    // set Port B Bits to 1 to enable the Pin Change Mask for the pins/bits desired, use appropriate ISR
  //PCMSK1 |= 0b00000000;    // set Port C Bits to 1 to enable the Pin Change Mask for the pins/bits desired, use appropriate ISR
  PCMSK2 |= 0b00010000;    // turn on Bit 4 of Port D - Arduino Pin D4 - ATmega328P Pin 6

  Serial.begin(115200);                                                     // setup the harware uart port to display ppm data

  loop_timer = micros();                                                    //Set the timer for the next loop.
 
   //When everything is done, turn off the led.
  digitalWrite((int)LED_PIN,LOW);      
  
}

// here is where we do the display to the SERIAL MONITOR at 115200 BPS and we should see the individual PPM channels displayed line by line
// use the endpoint / travel, and subtrim adjustments in your transmitter to change the display of the calibrated values in microseconds

void loop() {
  
  loop_timer = micros();

  if(loop_timer % update_interval == 0) 
  {  
    digitalWrite((int)LED_PIN, HIGH);        //Turn on the LED.
    
    Serial.print(receiver_input[0]);
    Serial.print('\t');
    Serial.print(receiver_input[1]);
    Serial.print('\t');
    Serial.print(receiver_input[2]);
    Serial.print('\t');
    Serial.print(receiver_input[3]);
    Serial.print('\t');
    Serial.print(receiver_input[4]);
    Serial.print('\t');
    Serial.print(receiver_input[5]);
    Serial.print('\t');
    if(channel_count > 6)
    {
       Serial.print(receiver_input[6]);
       Serial.print('\t');   
    }
    if(channel_count > 7)
    {
       Serial.print(receiver_input[7]);
       Serial.print('\t');    
    }
    if(channel_count > 8)
    {
       Serial.print(receiver_input[8]);
       Serial.print('\t');    
    }
    
    Serial.print('\n'); 
    
    digitalWrite((int)LED_PIN, LOW);       // turn OFF the LED
  }
  
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//This INTERRUPT ROUTINE is called every time Arduino pin (D4) changes state and is used to read the incoming PPM receiver signals. 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//ISR(PCINT0_vect){  // Port B Interrupt Vector
//ISR(PCINT1_vect){  // Port C Interrupt Vector
ISR(PCINT2_vect){  // Port D Interrupt Vector

  current_time = micros();
  
  if(ppm_sync_time > 0)
  {
     p_sync_time = current_time - ppm_sync_time;

     if(p_sync_time < (unsigned long)MID_PULSE)      // throw away the middle-position pulse (this is PCINT0) in each ppm data pulse, keep actual timing
        return;
     if(p_sync_time >  (unsigned long)OVER_PULSE)    // this is a sync pulse time event
        which_channel = 0;                           // next channel input is from tx channel 0
  }

  ppm_sync_time = current_time;                      // make a time entry for comparison

     switch(which_channel)                  
     {
        case -1:
           return;
        break;
       
        case 0:
        
           which_channel = (int)1;
           timer[0] = current_time;
           
           channel_count = (int)0;
        
           return;
     
        break;

    
        case 1:
        
           receiver_input[0] = current_time - timer[0];                             //PPM Channel 1 
           timer[1] = current_time;
           which_channel = (int)2;
           channel_count = (int)1;
           
           return;
 
        break;
    
        case 2:
        
           receiver_input[1] = current_time - timer[1];                             //PPM Channel 2 
           timer[2] = current_time;
           which_channel = (int)3;
           channel_count = (int)2;
                      
           return;
 
        break;
    
        case 3:
        
           receiver_input[2] = current_time - timer[2];                             //PPM Channel 3 
           timer[3] = current_time;
           which_channel = (int)4;
           channel_count = (int)3;
                      
           return;

        break;
    
        case 4:
        
           receiver_input[3] = current_time - timer[3];                             //PPM Channel 4 
           timer[4] = current_time;
           which_channel = (int)5;
           channel_count = (int)4;
                      
           return;

        break;
    
        case 5:
        
           receiver_input[4] = current_time - timer[4];                             //PPM Channel 5 
           timer[5] = current_time;
           which_channel = (int)6;           
           channel_count = (int)5;
                      
           return;

        break;
    
        case 6:
        
           receiver_input[5] = current_time - timer[5];                             //PPM Channel 6 
           timer[6] = current_time;
           which_channel = (int)7;         
           channel_count = (int)6;
           
           return;

        break; 

        case 7:
        
           receiver_input[6] = current_time - timer[6];                             //PPM Channel 7 
           timer[7] = current_time;
           which_channel = (int)8;
           channel_count = (int)7;
          
           return;

        break;     
        
        case 8:
        
           receiver_input[7] = current_time - timer[7];                             //PPM Channel 8
           timer[8] = current_time;
           which_channel = (int)9;
           channel_count = (int)8;
           
           return;

        break;   

        case 9:
        
           receiver_input[8] = current_time - timer[8];                             //PPM Channel 9
           which_channel = (int)-1;                             // set this to -1 on the last channel that you want to see
           channel_count = (int)9;
                      
           return;

        break;   

     }

}


