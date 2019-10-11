/*
 * File:   main.c
 * Author: muth.inc@free.fr
 * Created on 26 septembre 2019, 15:39
 */

// PIC16F19156 Configuration Bit Settings
// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config VBATEN = OFF     // VBAT Pin Enable bit (VBAT functionality is disabled)
#pragma config LCDPEN = ON      // LCD Charge Pump Mode bit (LCD Charge Pump is enabled)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = OFF      // Master Clear Enable bit (MCLR pin function is port defined function)
#pragma config PWRTE = OFF      // Power-up Timer selection bits (PWRT disable)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = SWDTEN    // WDT operating mode (WDT enabled/disabled by SWDTEN bit in WDTCON0)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = 512     // Boot Block Size Selection bits (Boot Block Size (Words) 512)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block NOT write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block NOT write-protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Words NOT write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM NOT write-protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF NOT write-protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// includes
#include <xc.h>
#include <stdint.h>

// CPU freq
#define _XTAL_FREQ  (32000000UL)

const uint8_t LCDdigit1[11] = {
    0b11110111, //0
    0b00010001, //1
    0b11101011, //2
    0b01111011, //3
    0b00111101, //4
    0b01111110, //5
    0b11111100, //6
    0b00110011, //7
    0b11111111, //8
    0b00111111, //9
    0b00001000  //-
};
const uint8_t LCDdigit2a[11] = {
    0b10100011, //0
    0b00000001, //1
    0b10000011, //2
    0b00000011, //3
    0b00100001, //4
    0b00100010, //5
    0b10100000, //6
    0b00000011, //7
    0b10100011, //8
    0b00100011, //9
    0b00000000  //-
};
const uint8_t LCDdigit2b[11] = {
    0b00001100, //0
    0b00001000, //1
    0b00010100, //2
    0b00011100, //3
    0b00011000, //4
    0b00011100, //5
    0b00011100, //6
    0b00001000, //7
    0b00011100, //8
    0b00011000, //9
    0b00010000  //-
};

void setDigit1(uint8_t digit);
void setDigit2(uint8_t digit);

int8_t rxByte = 0;

void interrupt isr(void) {
    if (PIR3bits.RC1IF) {
        rxByte = RC1REG;
        
//        setDigit2((rxByte & 0xF0) >> 4);
//        setDigit1(rxByte & 0x0F);
        if (rxByte < 100) {
            setDigit2(rxByte/10);
            setDigit1(rxByte%10);
        }
        if (rxByte == 100) {
            setDigit2(10);
            setDigit1(10);
        }
        if (rxByte > 100) {
            setDigit2(11);
            setDigit1(11);
        }
        
        PIR3bits.RC1IF = 0;
    }
}

void init() {
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0x00;
    
    // LCD
    LCDCONbits.CS = 0;
    LCDCONbits.LMUX = 0b0001;
    
    LCDPSbits.LP = 0b1111;
    LCDSE0 = 0b11111111;
    LCDSE1 = 0b10100011;
    LCDSE2 = 0b00011100;
    
    LCDVCON1bits.EN5V = 1;
    LCDVCON1bits.LPEN = 0;
    LCDVCON1bits.BIAS = 0b111;
    
    LCDVCON2bits.LCDVSRC = 0b0110;
    
    LCDCONbits.LCDEN = 1;
    
    // serial port
    TRISCbits.TRISC0 = 1; //RC0 as input
    RX1PPS = 0x10; // Rx1 on pin 11/RC0
    TX1STAbits.BRGH = 0;
    BAUD1CONbits.BRG16 = 0;
    SP1BRGH = 0;
    SP1BRGL = 51;
    RC1STAbits.CREN = 1;
    TX1STAbits.SYNC = 0;
    RC1STAbits.SPEN = 1;
    
    PIE3bits.RC1IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
}

void main(void) {
    
    init();
    
    setDigit2(10);
    setDigit1(10);
    
    while(1){
        __delay_ms(200);
    }
    
    
    return;
}

void setDigit1(uint8_t digit) {
    if (digit < 11) {
        LCDDATA0 = LCDdigit1[digit%11];
    } else {
        LCDDATA0 = 0x00;
    }
}

void setDigit2(uint8_t digit) {
    if (digit < 11) {
        LCDDATA1 = LCDdigit2a[digit%11];
        LCDDATA2 = LCDdigit2b[digit%11];
    } else {
        LCDDATA1 = 0x00;
        LCDDATA2 = 0x00;
    }
}
