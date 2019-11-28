#include "ads12xx.h"

volatile int DRDY_state = HIGH;
volatile byte state = LOW;

// Waits untill DRDY Pin is falling (see Interrupt setup). 
// Some commands like WREG, RREG need the DRDY to be low.
bool waitforDRDY() {
	uint16_t read_count = 0;
	while (digitalRead(7) == 1){// Need to fix this for knowing what DRDY pin is. Maybe store in a static variable?
		read_count++;
		// if (read_count == 40000){
		// 	return 0;
		// }
	}
	return 1;
}

bool waitforDRDY_int() {
	int timeout=0;
	timeout = millis();
	while (DRDY_state) {
		// Serial.println("Wait for DRDY");
		// if(millis()-timeout >10000){
		// 	return 0;
		// }
		// continue;
	}
	noInterrupts();
	DRDY_state = HIGH;
	interrupts();
	return 1;
}

//Interrupt function
void DRDY_Interuppt() {
	DRDY_state = LOW;
#ifdef heartbeat
	state = !state; //Heartbeat
	digitalWrite(13, state);
#endif
}


// ads12xx setup
ads12xx::ads12xx() {}

void ads12xx::begin(int CS, int START, int DRDY, int _RESET) {
	pinMode(_RESET, OUTPUT);
	digitalWrite(_RESET,HIGH);
	delay(20);

	pinMode(START, OUTPUT);
	digitalWrite(START, HIGH);
	delay(20);
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
	SPI.begin();

	pinMode(CS, OUTPUT);
	pinMode(DRDY, INPUT);
	digitalWrite(CS,HIGH);
	delay(100);
	_CS = CS;
	_DRDY = DRDY;
	delay(200);

	attachInterrupt(digitalPinToInterrupt(_DRDY), DRDY_Interuppt, FALLING); //This, tested seperately works for pin 2 and 7
}


// function to get a 3byte conversion result from the adc
long ads12xx::GetConversion() {
	int32_t regData; 
	// waitforDRDY();
	waitforDRDY_int();
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
	digitalWrite(_CS, LOW); //Pull SS Low to Enable Communications with ADS1247	
	regData |= SPI.transfer(NOP);
	regData <<= 8;
	regData |= SPI.transfer(NOP);
	regData <<= 8;
	regData |= SPI.transfer(NOP);
	regData = regData << 8;
	regData = regData >> 8;
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
	return regData;;
}

// function to get a 3byte conversion result from the adc but also change the input
long ads12xx::GetConversion_Mux(uint8_t regValue) {
	int32_t regData; 
	// waitforDRDY();
	waitforDRDY_int();
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
	digitalWrite(_CS, LOW); //Pull SS Low to Enable Communications with ADS1247	
	regData |= SPI.transfer(WREG | MUX0);
	regData <<= 8;
	regData |= SPI.transfer(0x00);
	regData <<= 8;
	regData |= SPI.transfer(regValue);
	regData = regData << 8;
	regData = regData >> 8;
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
	return regData;;
}


// function to write a register value to the adc
// argumen: adress for the register to write into, value to write
void ads12xx::SetRegisterValue(uint8_t regAdress, uint8_t regValue) {
#ifdef ADS1248
	if (regAdress == IDAC0) {
		regValue = regValue | IDAC0_ID;	  // add non 0 non-write register value IDAC0_ID
	}
#endif
	uint8_t regValuePre = ads12xx::GetRegisterValue(regAdress);
	if (regValue != regValuePre) {
		delay(10);
		SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with SPI_SPEED, MSB first, SPI Mode1
		digitalWrite(_CS, LOW);
		SPI.transfer(WREG | regAdress); // send 1st command byte, address of the register
		SPI.transfer(0x00);		// send 2nd command byte, write only one register
		SPI.transfer(regValue);         // write data (1 Byte) for the register
		digitalWrite(_CS, HIGH);
		if (regValue != ads12xx::GetRegisterValue(regAdress)) {   //Check if write was succesfull
			Serial.print("Write to Register 0x");
			Serial.print(regAdress, HEX);
			Serial.println(" failed!");
		}
		SPI.endTransaction();
		
	}

}


//function to read a register value from the adc
//argument: adress for the register to read
unsigned long ads12xx::GetRegisterValue(uint8_t regAdress) {
	uint8_t bufr;
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with 4Mhz clock, MSB first, SPI Mode0
	digitalWrite(_CS, LOW);
	SPI.transfer(RREG | regAdress); // send 1st command byte, address of the register
	SPI.transfer(0x00);			// send 2nd command byte, read only one register
	bufr = SPI.transfer(NOP);	// read data of the register;
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
	return bufr;
}

/*
Sends a Command to the ADC
Like SELFCAL, GAIN, SYNC, WAKEUP
*/
void ads12xx::SendCMD(uint8_t cmd) {
	waitforDRDY();
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with 4Mhz clock, MSB first, SPI Mode0
	digitalWrite(_CS, LOW);
	delay(10);
	SPI.transfer(cmd);
	delay(10);
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
}


// function to reset the adc
void ads12xx::Reset() {
	SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1
	digitalWrite(_CS, LOW);
	SPI.transfer(RESET); //Reset
	delay(2); //Minimum 0.6ms required for Reset to finish.
	digitalWrite(_CS, HIGH);
	SPI.endTransaction();
}

