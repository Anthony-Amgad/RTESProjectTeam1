#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "queue.h"
#include "time.h"

#define mainSW_INTURRUPT_PortA ((IRQn_Type)0)

xTaskHandle xISRTASK;

xQueueHandle xWindowQueue;
xSemaphoreHandle xMovingSema;
xSemaphoreHandle xUpSema;
xSemaphoreHandle xDownSema;
xSemaphoreHandle xStuckSema;
xSemaphoreHandle xStuckMutex;

void Stuck_Handler(){
	while(1){
		if(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2) == GPIO_PIN_2){
			xSemaphoreGive(xStuckSema);
			while(GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2) == GPIO_PIN_2);
			for(int i = 0; i < 100000; i++);
		}
	}
}

void MSISR(){
	xSemaphoreTake(xStuckSema,0);
	while(1){
		xSemaphoreTake(xStuckSema, portMAX_DELAY);
		xSemaphoreTake(xStuckMutex, portMAX_DELAY);
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);
		vTaskDelay(500/ portTICK_RATE_MS);
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);
		xSemaphoreGive(xStuckMutex);
		GPIOIntEnable(GPIO_PORTA_BASE,GPIO_INT_PIN_2);  
	}
}

void UpLimit(){
	while(1){
		xSemaphoreTake(xUpSema, portMAX_DELAY);
		uint32_t cnt = 0;
		while(GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) == 0 && cnt++<10000000);
		if(GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) == GPIO_PIN_4){
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);
		}
	}
}

void DownLimit(){
	while(1){
		xSemaphoreTake(xDownSema, portMAX_DELAY);
		uint32_t cnt = 0;
		while(GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5) == 0 && cnt++<10000000);
		if(GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5) == GPIO_PIN_5){
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);
		}
	}
}

void DriverListen(){
	int8_t Sup = 1;
	int8_t Lup = 2;
	int8_t Sdown = -1;
	int8_t Ldown = -2;
	while(1)
	{
		if(xSemaphoreGetMutexHolder(xStuckMutex) != xISRTASK){
			if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) == GPIO_PIN_2){
				xSemaphoreTake(xMovingSema,portMAX_DELAY);
				int cnt = 0;
				while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2) == GPIO_PIN_2){
					if(xSemaphoreGetMutexHolder(xStuckMutex) != xISRTASK){
						if(cnt != 0 && (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) == 0))
							xQueueSendToBack(xWindowQueue, &Sup, 0);
						cnt++;
						if(cnt == 1)
							for(int i = 0; i < 200000; i++);
					}
				}
				if(cnt == 1)
					xQueueSendToBack(xWindowQueue, &Lup, 0);
				xSemaphoreGive(xMovingSema);
		}
		else if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3) == GPIO_PIN_3)
		{
			xSemaphoreTake(xMovingSema,portMAX_DELAY);
			int cnt = 0;
			while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3) == GPIO_PIN_3){
				if(xSemaphoreGetMutexHolder(xStuckMutex) != xISRTASK){
					if(cnt != 0 && (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5) == 0))
						xQueueSendToBack(xWindowQueue, &Sdown, 0);
					cnt++;
					if(cnt == 1)
						for(int i = 0; i < 200000; i++);
				}
			}
			if(cnt == 1)
				xQueueSendToBack(xWindowQueue, &Ldown, 0);
			xSemaphoreGive(xMovingSema);
		}
		}
		taskYIELD();
	}
}

void PassListen(){
	int8_t Sup = 1;
	int8_t Lup = 2;
	int8_t Sdown = -1;
	int8_t Ldown = -2;
	while(1)
	{
		if(xSemaphoreGetMutexHolder(xStuckMutex) != xISRTASK){
			if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) == GPIO_PIN_1){
				xSemaphoreTake(xMovingSema,portMAX_DELAY);
				if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1) == 0){
					int cnt = 0;
					while(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1) == GPIO_PIN_1){
						if(xSemaphoreGetMutexHolder(xStuckMutex) != xISRTASK){
							if(cnt != 0 && (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_4) == 0) && GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1) == 0)
								xQueueSendToBack(xWindowQueue, &Sup, 0);
							cnt++;
							if(cnt == 1)
								for(int i = 0; i < 200000; i++);
						}
					}
					if(cnt == 1)
						xQueueSendToBack(xWindowQueue, &Lup, 0);
			}
			xSemaphoreGive(xMovingSema);
		}
		else if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2) == GPIO_PIN_2)
		{
			xSemaphoreTake(xMovingSema,portMAX_DELAY);
			if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1) == 0){
				int cnt = 0;
				while(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2) == GPIO_PIN_2){
					if(xSemaphoreGetMutexHolder(xStuckMutex) != xISRTASK){
						if(cnt != 0 && (GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5) == 0) && GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_1) == 0)
							xQueueSendToBack(xWindowQueue, &Sdown, 0);
						cnt++;
						if(cnt == 1)
							for(int i = 0; i < 200000; i++);
					}
				}
				if(cnt == 1)
					xQueueSendToBack(xWindowQueue, &Ldown, 0);
			}
			xSemaphoreGive(xMovingSema);
		}
		}
		taskYIELD();
	}
}


void WindowMove(){
	int8_t data = 0;
	while(1){
		xQueueReceive(xWindowQueue, &data, portMAX_DELAY);
		switch(data){
			case 1:
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);			
				for(int i = 0; i < 200000; i++);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
				break;
			case 2:
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);
				xSemaphoreGive(xUpSema);
				break;
			case -1:
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);			
				for(int i = 0; i < 200000; i++);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);
				break;
			case -2:
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
				GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);
				xSemaphoreGive(xDownSema);
				break;
		}
	
	}
}


int main(){
	
	xWindowQueue = xQueueCreate(5, sizeof(int8_t));
	xMovingSema = xSemaphoreCreateMutex();
	xUpSema = xSemaphoreCreateBinary();
	xDownSema = xSemaphoreCreateBinary();
	xStuckMutex = xSemaphoreCreateMutex();
	xStuckSema = xSemaphoreCreateBinary();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
	
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPD);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_2);
  GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPD);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5);
  GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPD);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3);
	
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);
  GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPD);
	
	
	xTaskCreate(DriverListen,
						"DriverListen",
						40,
						NULL,
						1,
						NULL);
	xTaskCreate(PassListen,
						"PassengerListen",
						40,
						NULL,
						1,
						NULL);
	xTaskCreate(WindowMove,
						"WindowMover",
						40,
						NULL,
						2,
						NULL);		
	xTaskCreate(UpLimit,
						"UpLimit",
						40,
						NULL,
						1,
						NULL);
	xTaskCreate(DownLimit,
						"DownLimit",
						40,
						NULL,
						1,
						NULL);
	xTaskCreate(Stuck_Handler,
						"StuckInt",
						40,
						NULL,
						1,
						&xISRTASK);
	xTaskCreate(MSISR,
						"ISRTASK",
						40,
						NULL,
						3,
						&xISRTASK);
	vTaskStartScheduler();

	for( ;; ); 
  
  return 0;	
}
