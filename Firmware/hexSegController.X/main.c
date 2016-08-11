/*
 * File:   main.c
 * Author: gamma
 *
 * Created on 2016/07/19, 13:39
 */


#include <xc.h>
#include <p18f4553.h>

#include "segFonts.h"

#include "system.h"

#include "usart.h"
#include "usb/usb.h"
#include "usb/usb_device.h"
#include "usb/usb_device_cdc.h"

#pragma config FOSC = HS //20MHz Xtal(No PLL)
#pragma config MCLRE = ON
#pragma config LVP = OFF        // 低電圧プログラミング機能使用しない(OFF)

#define _XTAL_FREQ 20000000


void setSegPin(unsigned long input){
    input = ~input;
	PORTC = (input & 0b10000000000000000) >> 16;
	PORTD = (input & 0b01111111100000000) >> 8;
	PORTB =  input & 0b00000000011111111;
}

void setDigitPin(unsigned int input){
	PORTA = (input & 0b111111000) >> 3;
	PORTE =  input & 0b000000111;
}

unsigned long segMap[9] = {
    /*
	~0b00110111000001100, //B
	~0b01111110000001100, //E
	~0b01111110000110000, //A
	~0b01011011111111100, //T
	~0b01110101100110011, //M
	~0b01111110000110000, //A
	~0b00111101100110011, //N
	~0b01011011111001100, //I
	~0b01111110000110000, //A
     */
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000,
    0b01111111100000000
};


short digitPtr = 0;

/*
void showBinary(int input){
    for(int i = 0; i < 8; i++){
        if((input & (1 << i)) == 0){
            segMap[i] = fontList[0x30];
        }else{
            segMap[i] = fontList[0x31];
        }
    }
    //segMap[8] = fontList[input];
}
*/

void main(void) {

	ADCON1 = 0b00001111; //All Digital
	CMCON  = 0b00000111; //No Comparator
	TRISA  = 0b00000000;
	TRISB  = 0b00000000;
	TRISC  = 0b10000000;
	TRISD  = 0b00000000;
	TRISE  = 0b00000000;
    /*
	LATA   = 0b00000000;
	LATB   = 0b00000000;
	LATC   = 0b00000000;
	LATD   = 0b00000000;
	LATE   = 0b00000000;
     */

	//タイマー設定。比較機が使えるTimer2を使う
	T2CON = 0;
	TMR2 = 0;
	PR2 = 125;
	T2CON = 0b01111101;
	//各種割り込み許可
	PIE1bits.TMR2IE = 1;
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;

	// UART設定
    
	RCSTA   = 0b10010000;
	BAUDCON = 0b00001000;
	SPBRGH  = 0;
    BRGH = 0;
	SPBRG   = 129;
    
    // USB 設定
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);

    USBDeviceInit();
    USBDeviceAttach();



	char RxData;
	short digitSelector;
    
    
    
    segMap[0] = fontList['H']; //H
    segMap[1] = fontList['T']; //T
    segMap[2] = fontList['L']; //L
    segMap[3] = fontList['A']; //A
    segMap[4] = fontList['B'] | 0b10000000000000000; //B.
    segMap[5] = fontList['N']; //N
    segMap[6] = fontList['E']; //E
    segMap[7] = fontList['T']; //T
    segMap[8] = fontList['!']; //!
    
    unsigned char buff;
    while(1){
        buff = USART_getcUSART();
        if(buff != 0){
            USART_putcUSART(buff);
            buff = 0;
        }
    }
    

	while(1){
		while (!PIR1bits.RCIF);      // 受信するまで待つ
        PIR1bits.RCIF = 0;
        RxData = RCREG;               // 受信データを取り込む
        //showBinary(RxData);
		if ((RxData & 0b00000111) == 0b111){
            digitSelector = (RxData & 0b11111000) >> 3;
			while (!PIR1bits.RCIF);      // 受信するまで待つ
            PIR1bits.RCIF = 0;
			RxData = RCREG;               // 受信データを取り込む
			switch(digitSelector){
				case 0b00111: //1
                    segMap[0] = fontList[RxData];
					break;
				case 0b01000: //2
                    segMap[1] = fontList[RxData];
					break;
				case 0b01001: //3
                    segMap[2] = fontList[RxData];
					break;
				case 0b01010: //4
                    segMap[3] = fontList[RxData];
					break;
				case 0b01011: //5
                    segMap[4] = fontList[RxData];
					break;
				case 0b01100: //6
                    segMap[5] = fontList[RxData];
					break;
				case 0b01101: //7
                    segMap[6] = fontList[RxData];
					break;
				case 0b01110: //8
                    segMap[7] = fontList[RxData];
					break;
				case 0b01111: //9
                    segMap[8] = fontList[RxData];
					break;
				default:
                    //showBinary(RxData);
                    segMap[8] = fontList[0x45];
					break;
			}
		}else{
        }
	}
}

void interrupt isr(void){
	if(PIR1bits.TMR2IF){
		PIR1bits.TMR2IF = 0;

		setSegPin(0b00000000000000000);
		setDigitPin(1<<digitPtr);
		setSegPin(segMap[digitPtr]);
		digitPtr = (digitPtr+1)%9;
	}
}