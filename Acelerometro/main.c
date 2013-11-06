#include <stdio.h>
#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f4_discovery_lis302dl.h"


/* Private function prototypes -----------------------------------------------*/
static void Mems_Config(void);
static void EXTILine_Config(void);
static void prvSetupHardware( void );
static void prvSetupPeripherals(void);
uint32_t LIS302DL_TIMEOUT_UserCallback(void);

uint8_t ClickReg, XOffset, YOffset;
uint8_t read[3];
uint32_t TimingDelay;

/**
* @brief  configure the mems accelometer
* @param  None
* @retval None
*/
static void Mems_Config(void)
{ 
  uint8_t ctrl = 0;
  
  LIS302DL_InitTypeDef  LIS302DL_InitStruct;
  LIS302DL_InterruptConfigTypeDef LIS302DL_InterruptStruct;  
	
  /* Set configuration of LIS302DL*/
  LIS302DL_InitStruct.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
  LIS302DL_InitStruct.Output_DataRate = LIS302DL_DATARATE_100;
  LIS302DL_InitStruct.Axes_Enable = LIS302DL_X_ENABLE | LIS302DL_Y_ENABLE;
  LIS302DL_InitStruct.Full_Scale = LIS302DL_FULLSCALE_2_3;
  LIS302DL_InitStruct.Self_Test = LIS302DL_SELFTEST_NORMAL;
  LIS302DL_Init(&LIS302DL_InitStruct);
    
  /* Set configuration of Internal High Pass Filter of LIS302DL*/
  LIS302DL_InterruptStruct.Latch_Request = LIS302DL_INTERRUPTREQUEST_LATCHED;
  LIS302DL_InterruptStruct.SingleClick_Axes = LIS302DL_CLICKINTERRUPT_X_ENABLE |  LIS302DL_CLICKINTERRUPT_Y_ENABLE;
  LIS302DL_InterruptConfig(&LIS302DL_InterruptStruct);
  
  /* Configure Interrupt control register: enable Click interrupt on INT1 and
     INT2 on Z axis high event */
  ctrl = 0x3F;
  LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG3_ADDR, 1);
  
  /* Enable Interrupt generation on click on Z axis */
/*  ctrl = 0x45;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_CFG_REG_ADDR, 1);
 */ 
  /* Configure Click Threshold on X/Y axis (10 x 0.5g) */
  ctrl = 0x77;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_THSY_X_REG_ADDR, 1);

  /* Configure Click Threshold on Z axis (10 x 0.5g) */
/*  ctrl = 0x0A;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_THSZ_REG_ADDR, 1);
*/  
  /* Enable interrupt on Y axis high event */
/*  ctrl = 0x4C;
  LIS302DL_Write(&ctrl, LIS302DL_FF_WU_CFG1_REG_ADDR, 1);
  */
  /* Configure Time Limit */
  ctrl = 0x03;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_TIMELIMIT_REG_ADDR, 1);
    
  /* Configure Latency */
  ctrl = 0x7F;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_LATENCY_REG_ADDR, 1);
  
  /* Configure Click Window */
  ctrl = 0x7F;
  LIS302DL_Write(&ctrl, LIS302DL_CLICK_WINDOW_REG_ADDR, 1);   
}

static void EXTILine_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;
	
  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  /* Enable the GPIO_LED Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	/* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
  /* Configure PE0 and PE1 pins as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* Connect EXTI Line to PE1 pins */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
	
  /* Configure EXTI Line1 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  
  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  MEMS accelerometre management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t LIS302DL_TIMEOUT_UserCallback(void)
{
  /* MEMS Accelerometer Timeout error occured */
  while (1) {
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
	}
}

void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

int main()
{
	uint8_t Buffer[3];
	
	prvSetupHardware();
	prvSetupPeripherals();
	
	LIS302DL_Read(&ClickReg, LIS302DL_CLICK_SRC_REG_ADDR, 1);
	LIS302DL_Read(read, LIS302DL_OUT_X_ADDR, 3);
                  
  XOffset = read[0];
	YOffset = read[2];
	
	while(1) {
		/* Read click status register */
		Buffer[0] = 10;
	}
	
	return 0;
}



static void prvSetupHardware( void )
{
	/* MEMS Accelerometre configure to manage PAUSE, RESUME and Controle Volume operation */
  Mems_Config();
  
  /* EXTI configue to detect interrupts on Z axis click and on Y axis high event */
  EXTILine_Config();
}

static void prvSetupPeripherals(void)
{	
	
}

int fputc (int ch, FILE *f)
{
	return ITM_SendChar(ch);
}

void EXTI0_IRQHandler(void)
{	
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {		
		LIS302DL_Read(&ClickReg, LIS302DL_CLICK_SRC_REG_ADDR, 1);
		LIS302DL_Read(read, LIS302DL_OUT_X_ADDR, 3);
		if(ClickReg & 0x01) {
			if(((read[0] - XOffset) & 0x80) != 0) {
				GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
			} else {
				GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
			}
		} else if(ClickReg & 0x04) {
			if(((read[2] - YOffset) & 0x80) != 0) {
				GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
			} else {
				GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
			}
		}
//		LIS302DL_Read(&ClickReg, LIS302DL_CLICK_SRC_REG_ADDR, 1);
    /* Clear the EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}
