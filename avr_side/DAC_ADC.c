#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Defines.h"


union {
	uint32_t DWORD;
	uint8_t CHAR[4];
} spi_buffer;


uint8_t SPIpush(uint8_t arg){
	SPDR1 = arg;
	while (!(SPSR1 & 0b10000000)){_NOP;}
	arg = SPDR1;
	return arg;
}


void DAC_set(uint16_t arg){
	SPCR1 = 0b01010101; //FOR DAC
	
	if (arg >= 4096){
		arg = 4095;
	}
	spi_buffer.DWORD = (uint32_t)(arg << 2);

	cli();
	DAC_CSClear;
	for (int8_t i=1; i>=0; --i){
		spi_buffer.CHAR[i] = SPIpush(spi_buffer.CHAR[i]);
	}
	DAC_CSSet;
	sei();
}


uint16_t ADC_read_low_level(void){
	SPCR1 = 0b01010001; //FOR ADC
	_NOP;
	
	cli();
	ADC_CSClear;
	for (int8_t i=2; i>=0; --i){
		spi_buffer.CHAR[i] = SPIpush(spi_buffer.CHAR[i]);
	}
	ADC_CSSet;
	sei();
	
	return (uint16_t)((spi_buffer.DWORD >> 4) & 0x0000FFFF);
}


uint16_t ADC_read(){
	static uint8_t adc_buf_clk = 0;
	static uint16_t adc_buf[32];
	
	adc_buf[adc_buf_clk] = ADC_read_low_level();
	adc_buf_clk++;
	adc_buf_clk = adc_buf_clk & 0b00011111;
	
	uint32_t midvalue = 0;
	for (uint8_t i = 0; i<32; ++i)
	{
		midvalue += adc_buf[i];
	}
	return (uint16_t)(midvalue >> 5);
}
