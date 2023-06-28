#include <stdint.h>
#include <stdio.h>
#include "elevador.h"

#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/hw_memmap.h"



#define BUFFER_SIZE 14
// defines GPIOS constantes
#define GPIO_PA0_U0RX 0x00000001
#define GPIO_PA1_U0TX 0x00000401

uint8_t _buffer[BUFFER_SIZE];
uint8_t _buffer_i;


uint32_t SysClock;

uint16_t readSW1(){
  return !GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0);
}
uint16_t readSW2(){
  return !GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1);
}

// Send String by UART
void sendString(const uint8_t *String, uint32_t tamanho){
  while(tamanho--){
    UARTCharPut(UART0_BASE, *String++);
  }
}

void UART_Interruption_Handler(void) 
{
  //limpar IRQ exec
  UARTIntClear(UART0_BASE,UARTIntStatus(UART0_BASE,true));
  // Ler o próximo caractere na uart.
  unsigned char _char = (uint8_t)UARTCharGetNonBlocking(UART0_BASE);
  
  if(_char == '\n'){
    _buffer[_buffer_i]='\0';
    printf("%s\n", _buffer);
    
    decodificaComando(_buffer);
    _buffer_i = 0;
  }
  else
    _buffer[_buffer_i++]=_char;
}



void reset_buffer(){
  uint8_t i=0;
  for(i=0; i<BUFFER_SIZE; i++)
    _buffer[i] = 0;
  _buffer_i = 0;
}

//função para configurar e inicializar o periférico Uart a 115.2k,8,n,1
void SetupUart(void)
{
  //Habilitar porta serial a 115200 com interrupção seguindo sequencia de inicializações abaixo:
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
  UARTConfigSetExpClk(UART0_BASE, SysClock, 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  UARTFIFODisable(UART0_BASE);
  UARTIntEnable(UART0_BASE,UART_INT_RX);
  UARTIntRegister(UART0_BASE,UART_Interruption_Handler);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE,(GPIO_PIN_0|GPIO_PIN_1));
  
  // Reset Buffer
  reset_buffer();
}

volatile unsigned int SysTicks1ms;

void sleep(int milissegundos){
  SysTicks1ms=milissegundos;
  while(SysTicks1ms>0){}
}

void SysTickIntHandler(void)
{
  SysTicks1ms--;
}

void SetupSystick(void){
  SysTicks1ms=0;
  SysTickDisable();
  SysTickPeriodSet(120000-1-12-6);
  SysTickIntRegister(SysTickIntHandler);
  SysTickIntEnable();
  SysTickEnable();
}

void init(){
  //Inicializar clock principal a 120MHz
  SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240), 120000000);
  SetupSystick();
  SetupUart();
  
  /* configura SW1 (PJ0) e SW2 (PJ1) */
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)){};
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
}