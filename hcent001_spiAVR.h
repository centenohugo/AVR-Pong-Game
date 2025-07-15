#ifndef SPIAVR_H
#define SPIAVR_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//B5 should always be SCK(spi clock) and B3 should always be MOSI. If you are using an
//SPI peripheral that sends data back to the arduino, you will need to use B4 as the MISO pin.
//The SS pin can be any digital pin on the arduino. Right before sending an 8 bit value with
//the SPI_SEND() funtion, you will need to set your SS pin to low. If you have multiple SPI
//devices, they will share the SCK, MOSI and MISO pins but should have different SS pins.
//To send a value to a specific device, set it's SS pin to low and all other SS pins to high.

//Added
// Outputs, pin definitions
#define PIN_A0  PORTB0
#define PIN_RST PORTB1

// Commands (from the ST7735 datasheet)
#define SWRESET 0x01  // Software Reset
#define SLPOUT  0x11  // Sleep Out
#define COLMOD  0x3A  // Interface Pixel Format
#define DISPON  0x29  // Display On
#define CASET   0x2A  // Column Address Set
#define RASET   0x2B  // Row Address Set
#define RAMWR   0x2C  // Memory Write

//End Added

#define PIN_SCK                   PORTB5//SHOULD ALWAYS BE B5 ON THE ARDUINO
#define PIN_MOSI                  PORTB3//SHOULD ALWAYS BE B3 ON THE ARDUINO
#define PIN_CS                    PORTB2


//If SS is on a different port, make sure to change the init to take that into account.
void SPI_INIT(){
    DDRB |= (1 << PIN_SCK) | (1 << PIN_MOSI) | (1 << PIN_CS);//initialize your pins. 
    SPCR |= (1 << SPE) | (1 << MSTR); //initialize SPI coomunication
}


void SPI_SEND(char data)
{
    SPDR = data;//set data that you want to transmit
    while (!(SPSR & (1 << SPIF)));// wait until done transmitting
}

void Hardware_Reset() {
    PORTB &= ~(1 << PIN_RST);  // Set Reset pin to low
    _delay_ms(200);
    PORTB |=  (1 << PIN_RST);  // Set Reset pin to high
    _delay_ms(200);
}

void Send_Command(char cmd) {
    PORTB &= ~(1 << PIN_A0);   // A0 low for command
    PORTB &= ~(1 << PIN_CS);   // CS low to select TFT
    SPI_SEND(cmd);
    PORTB |=  (1 << PIN_CS);   // CS high to deselect
}

void Send_Data(char data) {
    PORTB |=  (1 << PIN_A0);   // A0 high for data
    PORTB &= ~(1 << PIN_CS);   // CS low to select TFT
    SPI_SEND(data);
    PORTB |=  (1 << PIN_CS);   // CS high to deselect
}

void ST7735_Init() {
    Hardware_Reset();

    Send_Command(SWRESET);
    _delay_ms(150);

    Send_Command(SLPOUT);
    _delay_ms(200);

    Send_Command(COLMOD);
    Send_Data(0x05); // sets 16 bit color mode
    _delay_ms(10);

    Send_Command(DISPON);
    _delay_ms(200);
}

#endif /* SPIAVR_H */