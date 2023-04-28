#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"


void vApplicationIdleHook( void ){
	GPIO_PORTF_DATA_R |= 0x04;	
}

void TaskRed(){
	for(;;){
    GPIO_PORTF_DATA_R ^= 0x02;
		GPIO_PORTF_DATA_R &= ~0x04;
		vTaskDelay(1000/ portTICK_RATE_MS);		
	}
}

void TaskGreen(){
	for(;;){
    GPIO_PORTF_DATA_R ^= 0x08;
		GPIO_PORTF_DATA_R &= ~0x04;		
		vTaskDelay(2000/ portTICK_RATE_MS);
	}
}


int main(){
SYSCTL_RCGCGPIO_R |= 0x20;
  while(( SYSCTL_PRGPIO_R & 0x20 ) == 0 );
  
  GPIO_PORTF_CR_R |= 0x0E;
  GPIO_PORTF_DIR_R |= 0x0E;  
  GPIO_PORTF_DEN_R |= 0x0E;
	xTaskCreate(TaskRed,
						"Red",
						40,
						NULL,
						1,
						NULL);
	xTaskCreate(TaskGreen,
						"Green",
						40,
						NULL,
						1,
						NULL);
	
	GPIO_PORTF_DATA_R |= 0x0E;	

	vTaskStartScheduler();
  return 0;	
}
