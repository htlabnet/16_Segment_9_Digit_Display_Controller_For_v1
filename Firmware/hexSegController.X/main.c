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
#include <stdint.h>

#pragma config FOSC  = HS       // 20MHz Xtal(分周なし)
#pragma config MCLRE = ON       // リセットピンを利用する
#pragma config LVP   = OFF      // 低電圧プログラミング機能使用しない(OFF)
#pragma config WDT   = OFF      // ウォッチドッグタイマーを利用しない

#define _XTAL_FREQ 20000000


const char DEMO_MESSAGE[] =  "MAKER FAIRE TOKYO 2018 HTLABNET BOOTH! THIS IS 16 SEGMENT 9 DIGIT DISPLAY";
const int MESSAGE_LENGTH = (int)(sizeof(DEMO_MESSAGE)/sizeof(char));

uint8_t  led_stat;

//初期値"*********"
unsigned long segMap[9] = {
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111,
    0b110000000011111111
};

// 現在表示している桁数
short digitPtr = 0;

// (デバッグ用関数)
// 引数の数字を二進数でディスプレイに表示する
void showBinary(int input){
    for(int i = 0; i < 8; i++){
        if((input & (1 << i)) == 0){
            segMap[i] = ~(uint32_t)fontList[0x30];
        }else{
            segMap[i] = ~(uint32_t)fontList[0x31];
        }
    }
}

void setMsg(char input[]) {
    for (int i = 0; i < 9; i++) {
        segMap[i] = ~(uint32_t)(fontList[input[i]] | (1 << 16));
    }
}

int tmrIsr = 0;

void refreshShiftRegister(int ptr) {
    uint16_t ledSelector = 0b1 << ptr;

    uint32_t map =    ((segMap[ptr] & 0b11111111) << 24)
                    | ((segMap[ptr] & 0b1111111100000000) << 8)
                    | ((ledSelector & 0b0000000011) << 14)
                    | ((led_stat    & 0b00001111) << 10)
                    | ((segMap[ptr] & 0b110000000000000000) >> 8)
                    | ((ledSelector & 0b1111111100) >> 2);

    for (int i = 0; i < 32; i++) {
        LATBbits.LATB2 = (map >> i) & 1;
        LATBbits.LATB3 = 1;
        __delay_us(1);
        LATBbits.LATB3 = 0;
        __delay_us(1);
    }

    LATBbits.LATB4 = 1;
    __delay_us(1);
    LATBbits.LATB4 = 0;
    __delay_us(1);
}

void main(void) {

    ADCON1 = 0b00001111; //All Digital
    CMCON  = 0b00000111; //No Comparator
    TRISA  = 0b00000000;
    TRISB  = 0b00000000;
    TRISC  = 0b10000000; //Rxのみ入力
    TRISD  = 0b00000000;
    TRISE  = 0b00000000;

    LATCbits.LATC0 = 0; // OUTPUT ENABLE
    LATCbits.LATC1 = 1; // CLEAR

    LATCbits.LATC2 = 0; // PWM

    // 12VDCジャックアリのときはどっちも1
    LATAbits.LATA4 = 0; // USB_VCC -> VCC
    LATAbits.LATA5 = 0; // VCC -> LED
    
    /* MEMO
    A/D Result High Register (ADRESH)   
    A/D Result Low Register (ADRESL)    
    A/D Control Register 0 (ADCON0)     (-,-,CHS3,CHS2,CHS1,CHS0,GO/Done,ADON)
    A/D Control Register 1 (ADCON1)     (-,-,VCFG1,VCFG0,PCFG3,PCFG2,PCFG1,PCFG0)
    A/D Control Register 2 (ADCON2)     (ADFM,-,ACQT2,ACQT1,ADCS2,ADCS1,ADCS0)
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

    led_stat = 0b11110011;

    char RxData;            // 受信データ用バッファ
    short digitSelector;    // 書き換え桁数
    unsigned long dotflag;  // ドットをつけるかどうか
    
    
    setMsg("ABCDEFGHI");
    
    while (1){
        
        if (PIR1bits.RCIF) {
            PIR1bits.RCIF = 0;           //フラグを下げる
            RxData = RCREG;              // 受信データを取り込む
        
            //もし、先頭ビットが111であれば
            if ((RxData & 0b11100000) == 0b11100000) {
                digitSelector = (RxData & 0b00001111);
                dotflag = (RxData & 0b00010000) >> 4;
                while (!PIR1bits.RCIF);      // 受信するまで待つ
                PIR1bits.RCIF = 0;
                RxData = RCREG;               // 受信データを取り込む
                if (digitSelector > 8) continue;  // 無効な入力の処理
                if (RxData > 0b01111111) RxData = ~RxData;
                segMap[digitSelector] = ~(fontList[RxData] | (dotflag << 16)); // 値を実際にセット
            }
        }
        if (tmrIsr == 1) {
            tmrIsr = 0;
            //refreshShiftRegister(digitPtr);
            //digitPtr = (digitPtr+1)%9;      // digitPtrを次の値にセット
        }
        
    }
}

unsigned int divisor = 0;

void timerfunc() {
    if (divisor++ == 100) {
        
    }
}

//次の桁を表示する
void interrupt isr(void) {
    if (PIR1bits.TMR2IF) {
        PIR1bits.TMR2IF = 0;    // フラグを下げる
        tmrIsr = 1;
        timerfunc();
        refreshShiftRegister(digitPtr);
        digitPtr = (digitPtr+1)%9;      // digitPtrを次の値にセット
    }
}
