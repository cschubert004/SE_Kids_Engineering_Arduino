#include <Adafruit_GFX.h>
#include <gfxfont.h>

#include <Adafruit_CircuitPlayground.h>

/*
	Circuit Playground USB HID ScratchX interface firmware V1.6
	Copyright Embedit Electronics 2017
	Author - Robert Barron
*/
/**
 * You should have a LUFAConfig.h for this to work.
 */
#include "LUFAConfig.h"

/**
 * Include LUFA.h after LUFAConfig.h
 */
#include <LUFA.h>

/**
 * Finally include the LUFA device setup header
 */
//#include "DualVirtualSerial.h"

//#include "ArduinoSerial.h"

#include "CircuitPlaygroundUSB.h"

#include <Adafruit_CircuitPlayground.h>
#include <Adafruit_GFX.h>
#ifdef MATRIX
#include <Adafruit_NeoMatrix.h>
#endif
#include <Adafruit_NeoPixel.h>

#ifdef SERVO_ENABLED
#include <Servo.h>
#endif

#include "simon.hpp"

#define PIN 6

uint8_t usb_data = 1;
uint8_t tileX_c=1;
uint8_t tileY_c=1;

unsigned long time;//time how long it takes to respond to polling requests
//unsigned long time2;

#ifdef MATRIX
//setup the neopixel shield. 8x5 array on pin 6
Adafruit_NeoMatrix* matrix = new Adafruit_NeoMatrix(5, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);
#endif
//int x  = matrix->width();
//int pass = 0;
//extern "C" {struct HIDReportEcho;}
//extern "C" {struct OutGoingReport;}

//HIDReportEcho;
//extern struct OutGoingReport;

#ifdef SERVO_ENABLED
Servo servo9;  // create servo object to control servo 1 on pin #9
Servo servo10;  // create servo object to control servo 2 on pin #10
#endif

typedef enum
{
  MODE_GAME = 0,
  MODE_SCRATCH = 1
}mode_t;

static mode_t mode = MODE_SCRATCH;

void setup()
{
    CircuitPlayground.begin();

  if(CircuitPlayground.slideSwitch())
  {
    mode = MODE_GAME;
    simon_setup();
  }
  else
  {

        
    SetupHardware(); // ask LUFA to setup the hardware

    GlobalInterruptEnable(); // enable global interrupts
  	pinMode(9, INPUT); //ready for analog in on pin 9
  	pinMode(10, INPUT); //ready for analog in on pin 10
  	pinMode(12, INPUT); //ready for analog in on pin 12
  #ifdef MATRIX
  	matrix->begin();
  	matrix->setTextWrap(false);
  	matrix->setCursor(0,0);
  	matrix->setTextColor(matrix->Color(255,0,0));
  	//matrix->setTextSize(1);
  	matrix->setBrightness(30);//setting maximum neopixel brightness on the neomatrix. Not tested with higher brightness values.
  	matrix->fillScreen(0);
  #endif
  	CircuitPlayground.setPixelColor(0,0,0,0);
  }
}

#ifdef SERVO_ENABLED
//attach/detach servos on ports 9 and 10
void setupServo(uint8_t servo_num, uint8_t servo_setup)
{
	if(servo_num == 1)
	{
		if(servo_setup == 1)
		{
			servo9.attach(9);
		}
		else
		{
			servo9.detach();
			pinMode(9, INPUT);//make sure pin is back to analog read ready state
		}
	}
	else
	{
		if(servo_setup == 1)
		{
			servo10.attach(10);
		}
		else
		{
			servo10.detach();
			pinMode(10, INPUT);//make sure pin is back to analog read ready state
		}
	}
	
}

//set servo angle
void setServo(uint8_t servo_num, uint8_t servo_ang)
{	
	if(servo_num == 1)
	{
		servo9.write(servo_ang);
	}
	else
	{
		servo10.write(servo_ang);
	}
}

#endif

#ifdef MATRIX
//print text to the neomatrix
void matrixPrint(uint8_t message[])
{
	matrix->fillScreen(0);//erase neomatrix contents
	
	String str = (char*)message;
	matrix->print(str.substring(1)); //start from index 1 to skip command letter

	matrix->show(); 
}

//Set cursor position on the neomatrix. Allows you to scroll through printed text
void matrixSetCursor(uint8_t cursorX, uint8_t cursorY, uint8_t signX, uint8_t signY)
{
	int cX = cursorX;
	int cY = cursorY;
	if(signX)
	{
		cX *= -1;
	}
	if(signY)
	{	
		cY *= -1;
	}
	
	matrix->setCursor(cX, cY);
}

//setup neomatrix tiling. Works with up to 4 matrices in any layout, 1x2, 2x1, 2x2, etc.

void matrixSetup(uint8_t tileX, uint8_t tileY)
{
	if(tileX != tileX_c || tileY != tileY_c)
	{
		delete matrix;
		Adafruit_NeoMatrix* matrix = new Adafruit_NeoMatrix(5, 8, tileY, tileX, PIN,
		NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_COLUMNS   + NEO_TILE_PROGRESSIVE +
		NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
		NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
		NEO_GRB            + NEO_KHZ800);
		
		matrix->begin();
		matrix->setTextWrap(false);
		matrix->setTextSize(1);
		matrix->setCursor(0,0);
		matrix->setTextColor(matrix->Color(255,0,0));
		matrix->setBrightness(30);
		matrix->fillScreen(0);
	}
	tileX_c = tileX;
	tileY_c = tileY;
}

//draw rows or columns on the neopixel matrix
void drawNeoStrip(uint8_t strip, uint8_t pixel, uint16_t color)
{
  //draw row
  if(strip == 0)
  {
    matrix->drawFastVLine(pixel,0,matrix->height(),color);
  }
  else //draw column
  {
    matrix->drawFastHLine(0,pixel,matrix->width(),color);
  }
  matrix->show();
}


#endif
//set all neopixels on the circuit playground
void setNeoRing(uint8_t first, uint8_t last, uint8_t r, uint8_t g, uint8_t b)
{
	for(int i = first; i <= last; i++)
	{
		CircuitPlayground.setPixelColor(i,r,g,b);
	}
}


//main program loop
void loop()
{

  if(mode == MODE_GAME)
  {
    simon_loop();
  }
  else
  {

  	//time = millis();//time before we run usb polling and sample sensors. uncomment to measure sampling time.
  	
  	// Necessary LUFA library calls that need be run periodically to check USB for data
  	HID_Device_USBTask(&Generic_HID_Interface);
  	USB_USBTask();
  	usb_data = 1;
  	
  		// HID Reports are 8 bytes long. The first byte specifies the function of that report; set leds, get light sensor values, etc.
  			switch(echoReportData[0]) {
  				// If O, set a circuitplayground RGB LED using bytes 1-4 of the HID report
  				case 'O':
  					CircuitPlayground.setPixelColor(echoReportData[1],echoReportData[2],echoReportData[3],echoReportData[4]);
  					break;
  				// If o, set all circuitplayground RGB LEDs using bytes 1-5 of the HID report
  				case 'o':
  					setNeoRing(echoReportData[1],echoReportData[2],echoReportData[3],echoReportData[4],echoReportData[5]);
  					break;
  #ifdef MATRIX
  				// If 'P', draw a pixel on the neopixel matrix
  				case 'P':
  					matrix->drawPixel(echoReportData[2], echoReportData[1], matrix->Color(echoReportData[3],echoReportData[4],echoReportData[5]));
  					matrix->show();
  					break;
          case 'F':
            matrix->fillScreen(matrix->Color(echoReportData[2], echoReportData[3], echoReportData[4]));
            matrix->show();
            break;
          //setup neopixel matrix tiling
          case 'D':
            matrixSetup(echoReportData[1],echoReportData[2]);
            break;
          //print string to neomatrix
          case 'p':
            matrixPrint(echoReportData);
            break;
          //set matrix cursor 
          case 'c':
            matrixSetCursor(echoReportData[1],echoReportData[2], echoReportData[3], echoReportData[4]);
            break;
          //set matrix text color
          case 'T':
            matrix->setTextColor(matrix->Color(echoReportData[1],echoReportData[2], echoReportData[3]));
            break;
          // If 'R', draw a row on the neopixel matrix
          case 'R':
            drawNeoStrip(1, echoReportData[1], matrix->Color(echoReportData[2], echoReportData[3], echoReportData[4]));
            break;
          // If 'C', draw a column on the neopixel matrix
          case 'C':
            drawNeoStrip(0, echoReportData[1], matrix->Color(echoReportData[2], echoReportData[3], echoReportData[4]));
            break;
          // If 'F', fill the neopixel matrix
  
  #endif
  				// If 'B', sound the buzzer
  				case 'B':
  					//outReportData[0] = CircuitPlayground.temperature();
  					break;
  				// If 'L', turn the circuitplayground led on/off
  				case 'L':
  					if(echoReportData[1] == 1)
  						CircuitPlayground.redLED(HIGH);
  					else
  						CircuitPlayground.redLED(LOW);
  					break;
  #ifdef SERVO_ENABLED
  				// If 'S', use bytes 1-2 to set servo position
  				case 'S':
  					//set_servo(HIDReportEcho.ReportData[1]-48, HIDReportEcho.ReportData[2]);
  					setServo(echoReportData[1],echoReportData[2]);					
  					break;
  				// If 's', use bytes 1-2 to attach/detach servos
  				case 's':
  					setupServo(echoReportData[1],echoReportData[2]);
  					break;
  #endif
  				case 'G':
  					//request for sensor readings
  					if(echoReportData[1] == '3') {
  						/*Capsense x4	0-3
  						Light			4
  						Microphone		5
  						Temperature		6
  						Pushbutton x2	7,8
  						Switch			9
  						Acc x3			10,11,12
  						Analog #9		13
  						Analog #10		14
  						Analog A11 #12  15
  						Note: must respond to this polling request within ~20 milliseconds.
  						*/
  						//time = millis();//time before we sample sensors. uncomment to measure sampling time.
  						outReportData[0] = CircuitPlayground.readCap(0);
  						outReportData[1] = CircuitPlayground.readCap(1);
  						outReportData[2] = CircuitPlayground.readCap(2);
  						outReportData[3] = CircuitPlayground.readCap(3);
  						outReportData[4] = map(CircuitPlayground.lightSensor(), 0, 1023, 0, 255);
  						outReportData[5] = map(CircuitPlayground.soundSensor(), 0, 1023, 0, 255);
  						outReportData[6] = CircuitPlayground.temperatureF();
  						outReportData[7] = CircuitPlayground.leftButton();
  						outReportData[8] = CircuitPlayground.rightButton();
  						outReportData[9] = CircuitPlayground.slideSwitch();
  						outReportData[10] = map(constrain(CircuitPlayground.motionX(),-9.8,9.8), -9.8,9.8, 0, 255);
  						outReportData[11] = map(constrain(CircuitPlayground.motionY(),-9.8,9.8), -9.8,9.8, 0, 255);
  						outReportData[12] = map(constrain(CircuitPlayground.motionZ(),-20.0,20.0), -20.0, 20.0, 0, 255);
  #ifdef SERVO_ENABLED
  						if(!servo9.attached()) outReportData[13] = map(analogRead(9), 0, 1023, 0, 255);
  						if(!servo10.attached()) outReportData[14] = map(analogRead(10), 0, 1023, 0, 255); 
   #endif
  						outReportData[15] = map(analogRead(A11), 0, 1023, 0, 255); 						
  						//You can add outgoing data from additional sensors here, using outReportData[16-17]
  						
  						//outReportData[16] = millis() - time;//time to sample sensors. uncomment to measure sampling time.
  						outReportData[18] = 0x06; //firmware version
  						break;
  					}
  					//request for board type
  					if(echoReportData[1] == '4') {
  						outReportData[18] = 0x06; //firmware version
  					}
  					
  					break;
  					
  				// Returns an incrementing counter - used to measure cycle time and as a keep-alive.
  				case 'z':
  					/*OutGoingReport.ReportData[0] = count;
  					count++;
  					if(count > 255) {
  						count = 0;
  					}
  					break;*/
  				default:
  					usb_data = 0;
  					break;
  			}
  			// Only if there was valid data, set the last byte of the outgoing report, and reset the exit_count, max_count things
  			if(usb_data == 1) {/*
  				// Reset idle mode
  				if(HIDReportEcho.ReportData[0] == 'R')
  				{
  					//activity_state = 0;
  					exit_count = max_count+5;
  				}
  				else
  					//activity_state = 1;
  				*/
  				echoReportData[0] = 0x00;
  				// Sets last byte of outgoing report to last byte of incoming report so an outgoing report can be matched to its incoming request
  				//outReportData[13] = echoReportData[13];
  				outReportData[19] = echoReportData[19];
  				//exit_count = 0;
  				//max_count = 500000;
  			}
    }
}
