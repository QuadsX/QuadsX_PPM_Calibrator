# QuadsX_PPM_Calibrator
Radio-Control Transmitter PPM Calibrator for Arduino - Serial Monitor, Quick, Easy, Accurate

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
