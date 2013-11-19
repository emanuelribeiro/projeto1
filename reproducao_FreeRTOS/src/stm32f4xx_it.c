/* Includes ------------------------------------------------------------------*/
#include "main.h"

__IO uint8_t PauseResumeStatus = 2, Count = 0;

/**
  * @brief  This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
	static int anterior = 1;
	
  /* Check the clic on the accelerometer to Pause/Resume Playing */
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    if( !anterior ) {
			anterior = 1;
      PauseResumeStatus = 1;
    } else {
			anterior = 0;
      PauseResumeStatus = 0;
    }
    /* Clear the EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}
