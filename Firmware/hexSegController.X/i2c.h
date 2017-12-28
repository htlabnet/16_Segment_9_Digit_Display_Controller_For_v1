/* 
 * File:   i2c.h
 * Author: thotgamma
 *
 * Created on 2017/12/28, 20:27
 */

#ifndef I2C_H
#define	I2C_H

#include <stdint.h>
#include <xc.h>
#include <p18f4553.h>

void    i2c_wait(void);
void    i2c_enable_master(int baud_divisor);
void    i2c_disable_master(void);
void    i2c_enable_slave(uint8_t address);
void    i2c_enable_isr(void);
void    i2c_start(void);
void    i2c_repeat_start(void);
void    i2c_stop(void);
void    i2c_send_byte(unsigned char data);
uint8_t i2c_read_byte(unsigned int ack);

#endif	/* I2C_H */

