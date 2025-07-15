#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include <util/delay.h>
#include "SPIAVR.h"

//MODIFICATIONS:
//Created an array defining a 5x7 ASCII font for use with the LCD display.
//Created functions to draw shapes and characters on the LCD display.


#ifndef PERIPH_H
#define PERIPH_H

#define screenWidth 128
#define screenHeight 128
#define X_OFFSET 2
#define Y_OFFSET 1

//ASCII 5x7 font array
const uint8_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,  // 32  
    0x00, 0x00, 0x5F, 0x00, 0x00,  // 33 !
    0x00, 0x07, 0x00, 0x07, 0x00,  // 34 "
    0x14, 0x7F, 0x14, 0x7F, 0x14,  // 35 #
    0x24, 0x2A, 0x7F, 0x2A, 0x12,  // 36 $
    0x23, 0x13, 0x08, 0x64, 0x62,  // 37 %
    0x36, 0x49, 0x55, 0x22, 0x50,  // 38 &
    0x00, 0x05, 0x03, 0x00, 0x00,  // 39 '
    0x00, 0x1C, 0x22, 0x41, 0x00,  // 40 (
    0x00, 0x41, 0x22, 0x1C, 0x00,  // 41 )
    0x14, 0x08, 0x3E, 0x08, 0x14,  // 42 *
    0x08, 0x08, 0x3E, 0x08, 0x08,  // 43 +
    0x00, 0x50, 0x30, 0x00, 0x00,  // 44 ,
    0x08, 0x08, 0x08, 0x08, 0x08,  // 45 -
    0x00, 0x60, 0x60, 0x00, 0x00,  // 46 .
    0x20, 0x10, 0x08, 0x04, 0x02,  // 47 /
    0x3E, 0x51, 0x49, 0x45, 0x3E,  // 48 0
    0x00, 0x42, 0x7F, 0x40, 0x00,  // 49 1
    0x42, 0x61, 0x51, 0x49, 0x46,  // 50 2
    0x21, 0x41, 0x45, 0x4B, 0x31,  // 51 3
    0x18, 0x14, 0x12, 0x7F, 0x10,  // 52 4
    0x27, 0x45, 0x45, 0x45, 0x39,  // 53 5
    0x3C, 0x4A, 0x49, 0x49, 0x30,  // 54 6
    0x01, 0x71, 0x09, 0x05, 0x03,  // 55 7
    0x36, 0x49, 0x49, 0x49, 0x36,  // 56 8
    0x06, 0x49, 0x49, 0x29, 0x1E,  // 57 9
    0x00, 0x36, 0x36, 0x00, 0x00,  // 58 :
    0x00, 0x56, 0x36, 0x00, 0x00,  // 59 ;
    0x08, 0x14, 0x22, 0x41, 0x00,  // 60 <
    0x14, 0x14, 0x14, 0x14, 0x14,  // 61 =
    0x00, 0x41, 0x22, 0x14, 0x08,  // 62 >
    0x02, 0x01, 0x51, 0x09, 0x06,  // 63 ?
    0x32, 0x49, 0x79, 0x41, 0x3E,  // 64 @
    0x7E, 0x11, 0x11, 0x11, 0x7E,  // 65 A
    0x7F, 0x49, 0x49, 0x49, 0x36,  // 66 B
    0x3E, 0x41, 0x41, 0x41, 0x22,  // 67 C
    0x7F, 0x41, 0x41, 0x22, 0x1C,  // 68 D
    0x7F, 0x49, 0x49, 0x49, 0x41,  // 69 E
    0x7F, 0x09, 0x09, 0x09, 0x01,  // 70 F
    0x3E, 0x41, 0x49, 0x49, 0x7A,  // 71 G
    0x7F, 0x08, 0x08, 0x08, 0x7F,  // 72 H
    0x00, 0x41, 0x7F, 0x41, 0x00,  // 73 I
    0x20, 0x40, 0x41, 0x3F, 0x01,  // 74 J
    0x7F, 0x08, 0x14, 0x22, 0x41,  // 75 K
    0x7F, 0x40, 0x40, 0x40, 0x40,  // 76 L
    0x7F, 0x02, 0x0C, 0x02, 0x7F,  // 77 M
    0x7F, 0x04, 0x08, 0x10, 0x7F,  // 78 N
    0x3E, 0x41, 0x41, 0x41, 0x3E,  // 79 O
    0x7F, 0x09, 0x09, 0x09, 0x06,  // 80 P
    0x3E, 0x41, 0x51, 0x21, 0x5E,  // 81 Q
    0x7F, 0x09, 0x19, 0x29, 0x46,  // 82 R
    0x46, 0x49, 0x49, 0x49, 0x31,  // 83 S
    0x01, 0x01, 0x7F, 0x01, 0x01,  // 84 T
    0x3F, 0x40, 0x40, 0x40, 0x3F,  // 85 U
    0x1F, 0x20, 0x40, 0x20, 0x1F,  // 86 V
    0x3F, 0x40, 0x38, 0x40, 0x3F,  // 87 W
    0x63, 0x14, 0x08, 0x14, 0x63,  // 88 X
    0x07, 0x08, 0x70, 0x08, 0x07,  // 89 Y
    0x61, 0x51, 0x49, 0x45, 0x43,  // 90 Z
    0x00, 0x7F, 0x41, 0x41, 0x00,  // 91 [
    0x02, 0x04, 0x08, 0x10, 0x20,  // 92 '\'
    0x00, 0x41, 0x41, 0x7F, 0x00,  // 93 ]
    0x04, 0x02, 0x01, 0x02, 0x04,  // 94 ^
    0x40, 0x40, 0x40, 0x40, 0x40,  // 95 _
    0x00, 0x01, 0x02, 0x04, 0x00,  // 96 `
    0x20, 0x54, 0x54, 0x54, 0x78,  // 97 a
    0x7F, 0x48, 0x44, 0x44, 0x38,  // 98 b
    0x38, 0x44, 0x44, 0x44, 0x20,  // 99 c
    0x38, 0x44, 0x44, 0x48, 0x7F,  // 100 d
    0x38, 0x54, 0x54, 0x54, 0x18,  // 101 e
    0x08, 0x7E, 0x09, 0x01, 0x02,  // 102 f
    0x0C, 0x52, 0x52, 0x52, 0x3E,  // 103 g
    0x7F, 0x08, 0x04, 0x04, 0x78,  // 104 h
    0x00, 0x44, 0x7D, 0x40, 0x00,  // 105 i
    0x20, 0x40, 0x44, 0x3D, 0x00,  // 106 j
    0x7F, 0x10, 0x28, 0x44, 0x00,  // 107 k
    0x00, 0x41, 0x7F, 0x40, 0x00,  // 108 l
    0x7C, 0x04, 0x18, 0x04, 0x78,  // 109 m
    0x7C, 0x08, 0x04, 0x04, 0x78,  // 110 n
    0x38, 0x44, 0x44, 0x44, 0x38,  // 111 o
    0x7C, 0x14, 0x14, 0x14, 0x08,  // 112 p
    0x08, 0x14, 0x14, 0x18, 0x7C,  // 113 q
    0x7C, 0x08, 0x04, 0x04, 0x08,  // 114 r
    0x48, 0x54, 0x54, 0x54, 0x20,  // 115 s
    0x04, 0x3F, 0x44, 0x40, 0x20,  // 116 t
    0x3C, 0x40, 0x40, 0x20, 0x7C,  // 117 u
    0x1C, 0x20, 0x40, 0x20, 0x1C,  // 118 v
    0x3C, 0x40, 0x30, 0x40, 0x3C,  // 119 w
    0x44, 0x28, 0x10, 0x28, 0x44,  // 120 x
    0x0C, 0x50, 0x50, 0x50, 0x3C,  // 121 y
    0x44, 0x64, 0x54, 0x4C, 0x44,  // 122 z
    0x00, 0x08, 0x36, 0x41, 0x00,  // 123 {
    0x00, 0x00, 0x7F, 0x00, 0x00,  // 124 |
    0x00, 0x41, 0x36, 0x08, 0x00,  // 125 }
    0x10, 0x08, 0x08, 0x10, 0x08,  // 126 ~
    0x78, 0x46, 0x41, 0x46, 0x78   // 127 DEL (box)
};




////////// ADC UTILITY FUNCTIONS ///////////

void ADC_init() {
  ADMUX = (1<<REFS0);
	ADCSRA|= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

unsigned int ADC_read(unsigned char chnl){
  	uint8_t low, high;

  	ADMUX  = (ADMUX & 0xF8) | (chnl & 7);
  	ADCSRA |= 1 << ADSC ;
  	while((ADCSRA >> ADSC)&0x01){}
  
  	low  = ADCL;
	high = ADCH;

	return ((high << 8) | low) ;
}

////////// LCD SCREEN UTILITY FUNCTIONS ///////////
#define X_OFFSET 2
#define Y_OFFSET 1

void drawPixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x >= 128 || y >= 128) return; // Out of bounds check

    // Set column address (CASET)
    Send_Command(CASET);
    Send_Data(0x00);
    Send_Data(x);       // Start and end x = x
    Send_Data(0x00);
    Send_Data(x);

    // Set row address (RASET)
    Send_Command(RASET);
    Send_Data(0x00);
    Send_Data(y);       // Start and end y = y
    Send_Data(0x00);
    Send_Data(y);

    // Write pixel color
    Send_Command(RAMWR);
    Send_Data(color >> 8);     // High byte
    Send_Data(color & 0xFF);   // Low byte
}

void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    if (x >= 128 + X_OFFSET|| y >= 128) return;
    if ((x + w - 1) >= 128) w = 128 - x;
    if ((y + h - 1) >= 128) h = 128 - y;

    // Apply screen-specific offset
    uint8_t x_start = x + X_OFFSET;
    uint8_t x_end   = x + w - 1 + X_OFFSET;
    uint8_t y_start = y + Y_OFFSET;
    uint8_t y_end   = y + h - 1 + Y_OFFSET;

    // Set column window
    Send_Command(CASET);
    Send_Data(0x00); Send_Data(x_start);
    Send_Data(0x00); Send_Data(x_end);

    // Set row window
    Send_Command(RASET);
    Send_Data(0x00); Send_Data(y_start);
    Send_Data(0x00); Send_Data(y_end);

    // Fill with color
    Send_Command(RAMWR);
    for (uint16_t i = 0; i < w * h; i++) {
        Send_Data(color >> 8);
        Send_Data(color & 0xFF);
    }
}

void fillScreen(uint16_t color) {
    // Set column address (x: 0–127 maps to 2–129 on hardware)
    Send_Command(CASET);
    Send_Data(0x00); Send_Data(X_OFFSET);             // x_start = 2
    Send_Data(0x00); Send_Data(X_OFFSET + 127);       // x_end = 2 + 127 = 129

    // Set row address (y: 0–127 maps to 1–128 on hardware)
    Send_Command(RASET);
    Send_Data(0x00); Send_Data(Y_OFFSET);             // y_start = 1
    Send_Data(0x00); Send_Data(Y_OFFSET + 127);       // y_end = 1 + 127 = 128

    Send_Command(RAMWR);
    for (int i = 0; i < 128 * 128; i++) {
        Send_Data(color >> 8);
        Send_Data(color & 0xFF);
    }
}

void drawFastVLine(int x, int y, int h, uint16_t color) {
    for (int i = 0; i < h; i++) {
        drawPixel(x, y + i, color);
    }
}

void drawFilledCircle(int x0, int y0, int r, uint16_t color) {
    int x = 0;
    int y = r;
    int dp = 1 - r;

    drawFastVLine(x0, y0 - r, 2 * r + 1, color); // center vertical line

    while (x < y) {
        if (dp < 0) {
            dp = dp + 2 * x + 3;
        } else {
            dp = dp + 2 * (x - y) + 5;
            y--;
        }
        x++;

        // draw vertical lines across each horizontal layer
        drawFastVLine(x0 + x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 - x, y0 - y, 2 * y + 1, color);
        drawFastVLine(x0 + y, y0 - x, 2 * x + 1, color);
        drawFastVLine(x0 - y, y0 - x, 2 * x + 1, color);
    }
}



// Function to draw a single 5x7 character
void drawChar(int x, int y, char c, uint16_t color) {
    if (c < 32 || c > 127) return;
    uint16_t index = (c - 32) * 5;

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = font5x7[index + col];
        for (uint8_t row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                // Flip both Y and X direction
                drawPixel(x + (4 - col), y + (6 - row), color);
            }
        }
    }
}

void drawCharScaled(int x, int y, char c, uint16_t color, uint8_t scale) {
    if (c < 32 || c > 127) return;
    uint16_t index = (c - 32) * 5;

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = font5x7[index + col];
        for (uint8_t row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                drawRect(x + (4 - col) * scale, y + (6 - row) * scale, scale, scale, color);
            }
        }
    }
}





// Draw a full string at a given position
void drawString(int x, int y, const char* str, uint16_t color) {
    // First, calculate string length
    int len = 0;
    const char* tmp = str;
    while (*tmp++) len++;

    // Start drawing from right to left
    x += (len - 1) * 6;

    for (int i = 0; i < len; i++) {
        drawChar(x, y, str[i], color);
        x -= 6;
    }
}

void drawStringScaled(int x, int y, const char* str, uint16_t color, uint8_t scale) {
    int len = 0;
    const char* tmp = str;
    while (*tmp++) len++;

    x += (len - 1) * (6 * scale); // back up so we print left to right visually

    for (int i = 0; i < len; i++) {
        drawCharScaled(x, y, str[i], color, scale);
        x -= 6 * scale;
    }
}



// Centered string
void drawStringCentered(const char* str, int center_x, int center_y, uint16_t color) {
    int len = 0;
    const uint8_t CHAR_WIDTH = 6;
    const uint8_t CHAR_HEIGHT = 7;

    while (str[len] != '\0') len++;

    int total_width = len * CHAR_WIDTH;
    int start_x = center_x - total_width / 2;
    int start_y = center_y - CHAR_HEIGHT / 2;

    drawString(start_x, start_y, str, color);
}

void drawStringScaledCentered(const char* str, int center_x, int center_y, uint16_t color, uint8_t scale) {
    int len = 0;
    while (str[len] != '\0') len++;

    int total_width = len * 6 * scale;
    int total_height = 7 * scale;

    int start_x = center_x - total_width / 2;
    int start_y = center_y - total_height / 2;

    drawStringScaled(start_x, start_y, str, color, scale);
}

#endif /* PERIPH_H */