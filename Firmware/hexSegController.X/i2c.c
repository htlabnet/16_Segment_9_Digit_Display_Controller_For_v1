/*
 * File:   i2c.c
 * Author: thotgamma
 *
 * Created on 2017/12/28, 20:27
 */

#include "i2c.h"

void i2c_wait(void) {
	while ( ( SSPCON2 & 0x1F ) || ( SSPSTATbits.R_nW ) );
}

void i2c_enable_master(int baud_divisor) {
    SSPSTAT = 0b00000000;		// reset i2c status
	SSPADD = baud_divisor;      // Baud rate: XTAL/((SSPADD + 1)*4) = 400kHz
	SSPCON1 = 0b00101000;		// Enable I2C Master Mode
}


void i2c_disable_master(void) {
	SSPCON1 = 0b00001000;		// Disable I2C Master Mode
}

void i2c_enable_slave(uint8_t address) {
    SSPADD = address << 1;
}

void i2c_enable_isr(void) {
    SSPIF = 0; // 割り込みフラグの初期化
    SSPIE = 1; // 割り込み許可
}

void i2c_start(void) {
	SSPCON2bits.SEN = 1;	//  Start Condition Enabled bit
	i2c_wait();
}


void i2c_repeat_start(void) {
	SSPCON2bits.RSEN = 1;	//  Start Condition Enabled bit
	i2c_wait();
}


void i2c_stop(void) {
	SSPCON2bits.PEN = 1;	// Stop Condition Enable bit
	i2c_wait();
}


void i2c_send_byte(unsigned char data) {
	SSPBUF = data;
	i2c_wait();
}


unsigned char i2c_read_byte(unsigned int ack) {
	SSPCON2bits.RCEN = 1;
	i2c_wait();
	unsigned char data = SSPBUF;
	i2c_wait();
 
	if(ack) SSPCON2bits.ACKDT = 0;		// ACK
	else SSPCON2bits.ACKDT = 1;		// NO_ACK
 
	SSPCON2bits.ACKEN = 1;
 
	i2c_wait();
	return data;
}