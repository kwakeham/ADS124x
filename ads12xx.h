#ifndef ads12xx_H
#define ads12xx_H

#define ADS1248

#ifdef ADS1248
#include "ads1248.h"
#endif

#include "SPI.h"
class ads12xx {
public:
	ads12xx();
	void begin(
		int CS,
		int START,
		int DRDY,
		int _RESET
		);

	void Reset(
		
		);

	unsigned long  GetRegisterValue(
		uint8_t regAdress
		);
	
	void SendCMD(
		uint8_t cmd
		);

	void SetRegisterValue(
		uint8_t regAdress,
		uint8_t regValue
		);

	struct regValues_t
	{
		uint8_t MUX0_val;
		uint8_t VBIAS_val;
		uint8_t MUX1_val;
		uint8_t SYS0_val;
		uint8_t IDAC0_val;
		uint8_t IDAC1_val;
	};


	long GetConversion(
		);

	long GetConversion_Mux(
		uint8_t regValue
		);

	private:
	int _CS;
	int _DRDY;
	int ;
	volatile int DRDY_state;
};
#endif
