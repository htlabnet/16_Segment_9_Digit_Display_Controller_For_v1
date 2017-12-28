//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//	main.c
//	16 segment 9 digit display Controller 実体プログラム
//
//	Created by thotgamma. (https://gammalab.net)
//
//	16Segment9DigitDisplay公式:
//		https://htlab.net/products/electronics/16-segment-9-digit-display-board-1/
//		https://htlab.net/products/electronics/16-segment-9-digit-display-controller-1/
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


#include <xc.h>
#include <p18f4553.h>

#include "segFonts.h"

#pragma config FOSC  = HS       // 20MHz Xtal(分周なし)
#pragma config MCLRE = ON       // リセットピンを利用する
#pragma config LVP   = OFF      // 低電圧プログラミング機能使用しない(OFF)
#pragma config WDT   = OFF      // ウォッチドッグタイマーを利用しない

#define _XTAL_FREQ 20000000

//文字を指定する
void setSegPin(unsigned long input){
    input = ~input;
	PORTC = (input & 0b10000000000000000) >> 16;
	PORTD = (input & 0b01111111100000000) >> 8;
	PORTB =  input & 0b00000000011111111;
}

//桁を指定する
void setDigitPin(unsigned int input){
	PORTA = (input & 0b111111000) >> 3;
	PORTE =  input & 0b000000111;
}

//初期値"*********"
unsigned long segMap[9] = {
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

// 現在表示している桁数
short digitPtr = 0;

// (デバッグ用関数)
// 引数の数字を二進数でディスプレイに表示する
void showBinary(int input){
    for(int i = 0; i < 8; i++){
        if((input & (1 << i)) == 0){
            segMap[i] = fontList[0x30];
        }else{
            segMap[i] = fontList[0x31];
        }
    }
}

void main(void) {

	ADCON1 = 0b00001111; //All Digital
	CMCON  = 0b00000111; //No Comparator
	TRISA  = 0b00000000;
	TRISB  = 0b00000000;
	TRISC  = 0b10000000; //Rxのみ入力
	TRISD  = 0b00000000;
	TRISE  = 0b00000000;


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

    
	char RxData;            // 受信データ用バッファ
	short digitSelector;    // 書き換え桁数
    unsigned long dotflag;  // ドットをつけるかどうか
    
    

	while(1){
		while (!PIR1bits.RCIF);      // 受信するまで待つ
        PIR1bits.RCIF = 0;           //フラグを下げる
        RxData = RCREG;               // 受信データを取り込む
        
        //もし、先頭ビットが111であれば
		if ((RxData & 0b11100000) == 0b11100000){
            digitSelector = (RxData & 0b00001111);
            dotflag = (RxData & 0b00010000) >> 4;
			while (!PIR1bits.RCIF);      // 受信するまで待つ
            PIR1bits.RCIF = 0;
			RxData = RCREG;               // 受信データを取り込む
            if(digitSelector > 8)continue;  // 無効な入力の処理
            if(RxData > 0b01111111) RxData = ~RxData;
            segMap[digitSelector] = fontList[RxData] | (dotflag << 16); // 値を実際にセット
            
        }
	}
}

//次の桁を表示する
void interrupt isr(void){
	if(PIR1bits.TMR2IF){
		PIR1bits.TMR2IF = 0;    // フラグを下げる

		setSegPin(0b00000000000000000); // 移行する前に消灯する(でないと次の桁に引きずるため)
		setDigitPin(1<<digitPtr);       // 桁の移行
		setSegPin(segMap[digitPtr]);    // 値をセット
		digitPtr = (digitPtr+1)%9;      // digitPtrを次の値にセット
	}
}