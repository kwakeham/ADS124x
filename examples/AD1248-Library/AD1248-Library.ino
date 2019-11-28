#include <Arduino.h>
#include <SPI.h>
#include "ads12xx.h"

/*
For Grimsel Pass 1.1
Start = 5
CS = 6
DRDY = 2
Reset = 9
*/

int  START = 5;
int  CS = 6;
int  DRDY = 2;
int _RESET = 9;

//For multi channel read, this is the read next
int channel = 0;

//Define the class so we can access it
ads12xx ADS;

void setup()
{

  Serial.begin(115200);
  while (!Serial) {

  }
  Serial.println("Serial online");
  ADS.begin(CS, START, DRDY, _RESET); //initialize ADS as object of the ads12xx class

  ADS.Reset();
  Serial.println("ADS1248 Reset");

  ADS.SetRegisterValue(MUX1, BCS1_1 | MUX_SP2_AIN0 | MUX_SN2_AIN1);
  ADS.SetRegisterValue(MUX1, MUXCAL2_NORMAL | VREFCON1_ON | REFSELT1_ON_REF0);
  ADS.SetRegisterValue(SYS0, DOR3_40 | PGA2_128);
  Serial.println("Default Values are Pin = Ain0, Nin = Ain1, 2.048 Ref on and internally connected to Ref 0, Ref 0 selected (external), datarate of 40sps and gain of 128");

}


void loop() {

  if (Serial.available()) {
    char cin = Serial.read();
    char  check_ser = 'y';
	//While not used define the register values
    uint8_t cmd, FSC_0, FSC_1, FSC_2;
    uint8_t cin1, OFC_0, OFC_1, OFC_2;

    switch (cin) {
      case 'r':
        Serial.println("Which Register to read?");
        while (!Serial.available());
        Serial.print("Register Value for: ");
        cin1 = Serial.parseInt();
        Serial.println(cin1);
        Serial.println(ADS.GetRegisterValue(cin1));
        break;
      case 'w':
        Serial.println("Which Register to write?");
        while (!Serial.available());
        cin1 = Serial.parseInt();
        Serial.println("Which Value to write?");
        while (!Serial.available());
        ADS.SetRegisterValue(cin1, Serial.parseInt());
        break;
      case 'c':
        Serial.println("Conversion Result");
        while (true){
          Serial.println(ADS.GetConversion());
        }
        break;
      case 'm':
        Serial.println("Conversion Result");
		/* This loops through the conversions. Each time it reads
		the mux register can be written simultaneously.
		So we set the channel as the next channel to be written
		In this example 5 channels are read, default was 40 sps so 40sps/5
		gives 8 sps per channel, or 8sps total
		 */
        while (true){
          if (channel == 0){
            Serial.print(ADS.GetConversion_Mux(BCS1_1 | MUX_SP2_AIN7 | MUX_SN2_AIN2));
            channel = 1;
          } else if (channel == 1){
            Serial.print(",");
            Serial.print(ADS.GetConversion_Mux(BCS1_1 | MUX_SP2_AIN7 | MUX_SN2_AIN4));
            channel = 2;
          } else if (channel == 2){
            Serial.print(",");
            Serial.print(ADS.GetConversion_Mux(BCS1_1 | MUX_SP2_AIN7 | MUX_SN2_AIN5));
            channel = 3;
          } else if (channel == 3){
            Serial.print(",");
            Serial.print(ADS.GetConversion_Mux(BCS1_1 | MUX_SP2_AIN7 | MUX_SN2_AIN6));
            channel = 4;
          } else if (channel == 4){
            Serial.print(",");
            Serial.println(ADS.GetConversion_Mux(BCS1_1 | MUX_SP2_AIN0 | MUX_SN2_AIN1));
            channel = 0; //sets the next channel
          }
        }
        break;
      case 'x':
        Serial.println("Stop SDATAC");
        ADS.Reset();
        break;
      case 'o':
        Serial.println("Writing predefind Registers to defaults");
    		ADS.SetRegisterValue(MUX1, BCS1_1 | MUX_SP2_AIN0 | MUX_SN2_AIN1);
    		ADS.SetRegisterValue(MUX1, MUXCAL2_NORMAL | VREFCON1_ON | REFSELT1_ON_REF0);
	    	ADS.SetRegisterValue(SYS0, DOR3_40 | PGA2_128);       Serial.println("Writing sucessfull");
        break;
      case '?':
        Serial.println("Commands for testing:");
        Serial.println("'r' to read Register");
        Serial.println("'w' to write Register");
		    Serial.println("'m' to read continously multiple conversions from different inputs");
        Serial.println("'c' to get a Conversion Result continously");
        Serial.println("'x' to stop ADC read (reset, loss register)");
        Serial.println("'o' to write Pre Predefinde Registers");
        Serial.println("'?' for this help");
        break;

      default:
        break;
    }
  }
}