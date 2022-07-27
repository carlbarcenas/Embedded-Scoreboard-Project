/*----------------------------------------------------------------------------
 * Cris: this file is developed starting from the one I give in lab#3
 *
 * Name:    Serial.c
 * Purpose: MCB1700 Low level serial functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC17xx.H> 
#include "Serial.h"

//----------------------------------------------------------------------------
//  Initialize UART pins, Baudrate
//----------------------------------------------------------------------------
void SERIAL_init (int uart_port, int baudrate) 
{
	uint32_t Fdiv;
  uint32_t pclkdiv, pclk;
  LPC_UART_TypeDef *pUart;

  // (1) initialize either port UART0 or UART2 or UART3
	// UART0 or UART2 are the two UART peripherals connected to the 
	// DB9 connectors on the LantTiger board;
	// UART3 (but also UART2) can be connected to using the female connectors
	// along side teh LCD display;
  if (uart_port == 0) {                          // UART0 
		LPC_PINCON->PINSEL0 |= (1 << 4);             // Pin P0.2 used as TXD0 (Com0) 
    LPC_PINCON->PINSEL0 |= (1 << 6);             // Pin P0.3 used as RXD0 (Com0) 
		// Bits 7:6 of LPC_SC->PCLKSEL0: "A pair of bits in a Peripheral Clock 
		// Selection register controls the rate of the clock signal that will be 
		// supplied to the corresponding peripheral"; see page 57 of the user manual!
    pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;    // Bits 7:6 are for UART0;    
    pUart = (LPC_UART_TypeDef *)LPC_UART0;
  } else if (uart_port == 2) {                   // UART2 
    // power up the UART2 peripheral;
    // Note: this is the only difference compared to the other branch of
    // this if, i.e., the case of UART0; here we need to power-up the UART2
    // because (as described on page 307 of the user manual) only UART0 is
    // powered by default! so, to be able to use UART2, we need to set to
    // "1" the bit index 24 of the PCONP register, as discussed on page
    // 64 of the user manual!
    LPC_SC->PCONP |= (1 << 24);    
    LPC_PINCON->PINSEL0 |= (1 << 20);            // Pin P0.10 used as TXD2 (Com2) 
    LPC_PINCON->PINSEL0 |= (1 << 22);            // Pin P0.11 used as RXD2 (Com2) 
    pclkdiv = (LPC_SC->PCLKSEL1 >> 16) & 0x03;   // Bits 17:16 of PCLKSEL1 are for UART2; see page 58 of the user manual!
    pUart = (LPC_UART_TypeDef *)LPC_UART2;
  } else {                                       // UART3
    // power up the UART3 peripheral;
		// Note: looking at the schematic diagram of the LandTiger board we see that
		// UART3 RX and TX are available as pin functions at the pins P0.0, P0.1
		// and pins P4.29, P4.28; also note that P0.0, P0.1 are wired on the board's
		// PCB to the pins CAN1_RX, CAN1_TX that go thru a small IC to the output connector
		// CAN 1; on the other hand, P4.29, P4.28 are wired on the board's PCB to the pins 
		// 485_RX, 485_TX that go thru a small IC to the output connector RS485;
		// I guess, we could  use any of these two sets of pins to make direct connections 
		// to UART3; let's use P4.29, P4.28;
    LPC_SC->PCONP |= (1 << 25);
		LPC_PINCON->PINSEL9 |= (3 << 24);             // Pin P4.28 used as TXD3 (Com3); see page 120 of the user manual!
    LPC_PINCON->PINSEL9 |= (3 << 26);             // Pin P4.29 used as RXD3 (Com3) 
    pclkdiv = (LPC_SC->PCLKSEL1 >> 18) & 0x03;    // Bits 19:18 of PCLKSEL1 are for UART3; see page 58 of the user manual!
    pUart = (LPC_UART_TypeDef *)LPC_UART3;
  }

  // (2)
  switch ( pclkdiv )
  {
    case 0x00:
    default:
    pclk = SystemCoreClock/4;
    break;
    case 0x01:
    pclk = SystemCoreClock;
    break; 
    case 0x02:
    pclk = SystemCoreClock/2;
    break; 
    case 0x03:
    pclk = SystemCoreClock/8;
    break;
  }
  
  // (3)
  pUart->LCR    = 0x83;                          // 8 bits, no Parity, 1 Stop bit
  Fdiv = ( pclk / 16 ) / baudrate;               // baud rate 
  pUart->DLM = Fdiv / 256;
  pUart->DLL = Fdiv % 256;
  pUart->LCR = 0x03;                             // DLAB = 0
  pUart->FCR = 0x07;                             // Enable and reset TX and RX FIFO
}

//----------------------------------------------------------------------------
//  Write character to Serial Port
//----------------------------------------------------------------------------
int SERIAL_putChar (int uart, int c) 
{
  LPC_UART_TypeDef *pUart;
	if (uart == 0) {
		pUart =(LPC_UART_TypeDef *)LPC_UART0;
	} else if (uart == 2) {
		pUart =(LPC_UART_TypeDef *)LPC_UART2;
	} else {
		pUart =(LPC_UART_TypeDef *)LPC_UART3;
	}
  while (!(pUart->LSR & 0x20));
  return (pUart->THR = c);
}

//----------------------------------------------------------------------------
//  Read character from Serial Port (blocking read)
//----------------------------------------------------------------------------
int SERIAL_getChar (int uart) 
{
  LPC_UART_TypeDef *pUart;
  if (uart == 0) {
		pUart =(LPC_UART_TypeDef *)LPC_UART0;
	} else if (uart == 2) {
		pUart =(LPC_UART_TypeDef *)LPC_UART2;
	} else {
		pUart =(LPC_UART_TypeDef *)LPC_UART3;
	}
  while (!(pUart->LSR & 0x01)); // this is the "blocking" aspect of it; it waits/blocks here;
  return (pUart->RBR);
}

//----------------------------------------------------------------------------
//  Read character from Serial Port (non-blocking read)
//----------------------------------------------------------------------------
int SERIAL_getChar_nb (int uart) 
{
  LPC_UART_TypeDef *pUart;
  if (uart == 0) {
		pUart =(LPC_UART_TypeDef *)LPC_UART0;
	} else if (uart == 2) {
		pUart =(LPC_UART_TypeDef *)LPC_UART2;
	} else {
		pUart =(LPC_UART_TypeDef *)LPC_UART3;
	}
  if (pUart->LSR & 0x01)
    return (pUart->RBR);
  else
    return 0;
}

//----------------------------------------------------------------------------
//  Write character to Serial Port
//----------------------------------------------------------------------------
void SERIAL_putString (int uart, char *s) 
{
  while (*s != 0) {
		SERIAL_putChar(uart, *s++);
  }
}
