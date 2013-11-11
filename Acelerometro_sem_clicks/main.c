#include <stdio.h>
#include "stm32f4xx.h"

#include "stm32f4_discovery_lis302dl.h"


/* Private function prototypes -----------------------------------------------*/
static void Mems_Config(void);
static void EXTILine_Config(void);
static void prvSetupHardware( void );
static void prvSetupPeripherals(void);
uint32_t LIS302DL_TIMEOUT_UserCallback(void);

/**
* @brief  configure the MEMS accelometer
* @param  None
* @retval None
*/
static void Mems_Config(void)
{ 
  uint8_t ctrl = 0;
  
  LIS302DL_InitTypeDef  LIS302DL_InitStruct;
	
  /* Set configuration of LIS302DL*/
  LIS302DL_InitStruct.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
  LIS302DL_InitStruct.Output_DataRate = LIS302DL_DATARATE_100;
  LIS302DL_InitStruct.Axes_Enable = LIS302DL_X_ENABLE | LIS302DL_Y_ENABLE;
  LIS302DL_InitStruct.Full_Scale = LIS302DL_FULLSCALE_2_3;
  LIS302DL_InitStruct.Self_Test = LIS302DL_SELFTEST_NORMAL;
  LIS302DL_Init(&LIS302DL_InitStruct);
    
  /* Configure Interrupt control register: enable free-fall 
	on INT1 and INT 2. */
  ctrl = 0x11;
  LIS302DL_Write(&ctrl, LIS302DL_CTRL_REG3_ADDR, 1);
  
  /* Enable interrupt on X axis high event interrupt on INT1. */
  ctrl = 0x42;
  LIS302DL_Write(&ctrl, LIS302DL_FF_WU_CFG1_REG_ADDR, 1);
	
	/* Enable interrupt on Y axis high event interrupt on INT2. */
  ctrl = 0x48;
	LIS302DL_Write(&ctrl, LIS302DL_FF_WU_CFG2_REG_ADDR, 1);

  /* Sets threshold on both of the reading interrupts. */
  ctrl = 0x0A;
  LIS302DL_Write(&ctrl, LIS302DL_FF_WU_THS1_REG_ADDR, 1);
	LIS302DL_Write(&ctrl, LIS302DL_FF_WU_THS2_REG_ADDR, 1);
	
	/* Sets duration on both of the reading interrupts. */
  ctrl = 0x2F;
  LIS302DL_Write(&ctrl, LIS302DL_FF_WU_DURATION1_REG_ADDR, 1);
	LIS302DL_Write(&ctrl, LIS302DL_FF_WU_DURATION2_REG_ADDR, 1);  
}

static void EXTILine_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;
	
  /* Enable GPIOE clock (for accelerometer interrupts) */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  /* Enable the GPIOD Clock (for LEDS) */
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
	
	/* Connect EXTI Line to accelerometer pins (PE0 and PE1) */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	
  /* Configure EXTI Line0 for the X-axis. */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	/* Configure EXTI Line1 for the Y-axis 
	changing only what is different (the line) */
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_Init(&EXTI_InitStructure);
  
  /* Enable and set EXTI Line0 Interrupt to the lowest priority (for the X-axis) */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable and set EXTI Line1 Interrupt to the lowest priority (for the Y-axis),
	changing only what is different (the line) */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
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
  while (1);
}

int main()
{
	uint8_t src;
	
	prvSetupHardware();
	prvSetupPeripherals();
	
	LIS302DL_Read(&src, LIS302DL_FF_WU_SRC1_REG_ADDR, 1);
	LIS302DL_Read(&src, LIS302DL_FF_WU_SRC2_REG_ADDR, 1);
	
	while(1);
	
	return 0;
}


/**
  * @brief  Previous setup of the hardware for the program.
  * @param  None.
  * @retval None.
  */
static void prvSetupHardware( void )
{
	/* MEMS Accelerometre configure to manage PAUSE, RESUME and Controle Volume operation */
  Mems_Config();
  
  /* EXTI configue to detect interrupts on Z axis click and on Y axis high event */
  EXTILine_Config();
}

/**
  * @brief  Previous setup of the peripherals for the program.
  * @param  None.
  * @retval None.
  */
static void prvSetupPeripherals(void)
{	
	
}

/**
  * @brief  Redirect the printf function with the characters to the debug channel.
  * @param  ch - int - printable character.
						f - FILE *f - where to print (not used, but necessary to make the overload of the function).
  * @retval None.
  */
int fputc (int ch, FILE *f)
{
	return ITM_SendChar(ch);
}

/**
  * @brief  X axis high level interrupt.
  * @param  None.
  * @retval None.
  */
void EXTI0_IRQHandler(void)
{
	uint8_t read, src;
	
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
		LIS302DL_Read(&src, LIS302DL_FF_WU_SRC1_REG_ADDR, 1);			/* Reads the latch address, to clean it. */
		LIS302DL_Read(&read, LIS302DL_OUT_X_ADDR, 1);							/* Reads the value of the x-axis. */
		if (read & 0x80) {																				/* If the high level interrupt was negative: */
			GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
		} else {																									/* If the high level interrupt was positive: */
			GPIO_ToggleBits(GPIOD, GPIO_Pin_13);
		}

    EXTI_ClearITPendingBit(EXTI_Line0);												/* Clear the EXTI line 0 pending bit */
  }
}

/**
  * @brief  Y axis high level interrupt.
  * @param  None.
  * @retval None.
  */
void EXTI1_IRQHandler(void)
{
	uint8_t read, src;
	
	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
		LIS302DL_Read(&src, LIS302DL_FF_WU_SRC2_REG_ADDR, 1);			/* Reads the latch address, to clean it. */
		LIS302DL_Read(&read, LIS302DL_OUT_Y_ADDR, 1);							/* Reads the value of the y-axis. */
		if (read & 0x80) {																				/* If the high level interrupt was negative: */
			GPIO_ToggleBits(GPIOD, GPIO_Pin_14);
		} else {																									/* If the high level interrupt was positive: */
			GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
		}
		
    EXTI_ClearITPendingBit(EXTI_Line1);												/* Clear the EXTI line 1 pending bit */
  }
}
