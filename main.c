#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "queue.h"
#include "DIO.h"


void vApplicationIdleHook( void ){
	GPIO_PORTF_DATA_R |= 0x04;	
}


int main(){


	vTaskStartScheduler();
  return 0;	
}
