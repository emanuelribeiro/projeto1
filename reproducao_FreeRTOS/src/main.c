/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/**
  * @brief  Main program.
  * @param  None
  * @retval None
*/

void vPlayTask( void *pvParameters )
{
	for( ; ; )
	{
		WavePlayBack(I2S_AudioFreq_48k); 
	}
}

void vMaisUmaTask( void *pvParameters )
{
	int i = 0;
	
	for( ; ; )
	{
		i++;
	}
}

int main(void)
{
	int i = 0;
	
	xTaskCreate( vPlayTask, ( signed portCHAR * ) "vPlayTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
	
	for ( i = 0; i < 30; i++ ) {
		xTaskCreate( vMaisUmaTask, ( signed portCHAR * ) "vMaisUmaTask", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
	}
	
	vTaskStartScheduler();
  while (1);
}
