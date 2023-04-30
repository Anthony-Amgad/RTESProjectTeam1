#include "DIO.h"

void DIO_Init()
{
  SYSCTL_RCGCGPIO_R |= 0x20;
  while(( SYSCTL_PRGPIO_R & 0x20 ) == 0 );
  GPIO_PORTF_LOCK_R = 0x4C4F434B;
  GPIO_PORTF_CR_R |= 0x1F;
  GPIO_PORTF_DIR_R = 0x0E;  
  GPIO_PORTF_PUR_R = 0x11;
  GPIO_PORTF_DEN_R |= 0x1F;
  return ;
}
void DIO_WritePin(uint32_ptr_vol port, uint8 pin, uint8 value)
{
  if(value < 0 || value > 1) return;
  if(value == 0) *port &= ~(1<<pin);
  else *port |= (1<<pin);
}

void DIO_TogglePin(uint32_ptr_vol port, uint8 pin)
{
  Toggle_Bit(*port, pin);
}

void DIO_WritePort(uint32_ptr_vol port, uint8 value)
{
  *port = value;
}

uint8 DIO_ReadPin(uint32_ptr_vol port, uint8 pin)
{
  return ((*port>>pin) & 0x1);
}
uint32 DIO_ReadPort(uint32_ptr_vol port)
{
  return *port;
}
