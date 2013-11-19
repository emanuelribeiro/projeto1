/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Extern variables ----------------------------------------------------.-----*/

 /* This is an audio file stored in the Flash memory as a constant table of 16-bit data.
    The audio format should be WAV (raw / PCM) 16-bits, Stereo (sampling rate may be modified) */
extern uint16_t AUDIO_SAMPLE[];

/* Defines -------------------------------------------------------------------*/

/* Audio file size and start address are defined here since the audio file is 
    stored in Flash memory as a constant table of 16-bit data */
#define AUDIO_FILE_SZE          990000
#define AUDIO_START_ADDRESS     58 /* Offset relative to audio file header size */

/* Private variables ---------------------------------------------------------*/

__IO uint8_t volume = 70, AudioPlayStart = 0;
extern __IO uint8_t PauseResumeStatus ;
static __IO uint32_t TimingDelay;

/* Private functions ---------------------------------------------------------*/
static void EXTILine_Config(void);

/**
  * @brief  Play wave from a mass storge
  * @param  AudioFreq: Audio Sampling Frequency
  * @retval None
*/

void WavePlayBack(uint32_t AudioFreq)
{ 
  /* 
  Normal mode description:
  Start playing the audio file (using DMA stream) .
  Using this mode, the application can run other tasks in parallel since 
  the DMA is handling the Audio Transfer instead of the CPU.
  The only task remaining for the CPU will be the management of the DMA 
  Transfer Complete interrupt or the Half Transfer Complete interrupt in 
  order to load again the buffer and to calculate the remaining data.  
  Circular mode description:
  Start playing the file from a circular buffer, once the DMA is enabled it 
  always run. User has to fill periodically the buffer with the audio data 
  using Transfer complete and/or half transfer complete interrupts callbacks 
  (EVAL_AUDIO_TransferComplete_CallBack() or EVAL_AUDIO_HalfTransfer_CallBack()...
  In this case the audio data file is smaller than the DMA max buffer 
  size 65535 so there is no need to load buffer continuously or manage the 
  transfer complete or Half transfer interrupts callbacks. */  
  
  /* Start playing */
  AudioPlayStart = 1;
  
  /* Initialize wave player (Codec, DMA, I2C) */
  WavePlayerInit(AudioFreq);
  
  /* Play on */
  AudioFlashPlay((uint16_t*)(AUDIO_SAMPLE + AUDIO_START_ADDRESS), AUDIO_FILE_SZE, AUDIO_START_ADDRESS);
  
  /* Infinite loop */
  while(1)
  {
		if (PauseResumeStatus != 2)
		{
			WavePlayerPauseResume(PauseResumeStatus);
			PauseResumeStatus = 2;
		}
	}
}

/**
  * @brief  Pause or Resume a played wave
  * @param  state: if it is equal to 0 pause Playing else resume playing
  * @retval None
  */
void WavePlayerPauseResume(uint8_t state)
{ 
  EVAL_AUDIO_PauseResume(state);   
}

/**
  * @brief  Configure the volune
  * @param  vol: volume value
  * @retval None
  */
uint8_t WaveplayerCtrlVolume(uint8_t vol)
{ 
  EVAL_AUDIO_VolumeCtl(vol);
  return 0;
}


/**
  * @brief  Stop playing wave
  * @param  None
  * @retval None
  */
void WavePlayerStop(void)
{ 
  EVAL_AUDIO_Stop(CODEC_PDWN_SW);
}
 
/**
* @brief  Initializes the wave player
* @param  AudioFreq: Audio sampling frequency
* @retval None
*/
int WavePlayerInit(uint32_t AudioFreq)
{
	EXTILine_Config();
	
  /* Initialize I2S interface */  
  EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);
  
  /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */  
  EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, volume, AudioFreq );  
  
  return 0;
}

/**
* @brief  Play wave file from internal Flash
* @param  None
* @retval None
*/
uint32_t AudioFlashPlay(uint16_t* pBuffer, uint32_t FullSize, uint32_t StartAdd)
{ 
  EVAL_AUDIO_Play((uint16_t*)pBuffer, (FullSize - StartAdd));
  return 0;
}

/*--------------------------------
Callbacks implementation:
the callbacks prototypes are defined in the stm324xg_eval_audio_codec.h file
and their implementation should be done in the user code if they are needed.
Below some examples of callback implementations.
--------------------------------------------------------*/
/**
* @brief  Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
  /* Calculate the remaining audio data in the file and the new size 
  for the DMA transfer. If the Audio files size is less than the DMA max 
  data transfer size, so there is no calculation to be done, just restart 
  from the beginning of the file ... */
  /* Check if the end of file has been reached */
  
#ifdef AUDIO_MAL_MODE_NORMAL  

/* Replay from the beginning */
AudioFlashPlay((uint16_t*)(AUDIO_SAMPLE + AUDIO_START_ADDRESS),AUDIO_FILE_SZE, AUDIO_START_ADDRESS);
    
#else /* #ifdef AUDIO_MAL_MODE_CIRCULAR */
  
  
#endif /* AUDIO_MAL_MODE_CIRCULAR */
}

/**
* @brief  Manages the DMA Half Transfer complete interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{  
#ifdef AUDIO_MAL_MODE_CIRCULAR
    
#endif /* AUDIO_MAL_MODE_CIRCULAR */
  
  /* Generally this interrupt routine is used to load the buffer when 
  a streaming scheme is used: When first Half buffer is already transferred load 
  the new data to the first half of buffer while DMA is transferring data from 
  the second half. And when Transfer complete occurs, load the second half of 
  the buffer while the DMA is transferring from the first half ... */
  /* 
  ...........
  */
}

/**
* @brief  Manages the DMA FIFO error interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_Error_CallBack(void* pData)
{
  /* Stop the program with an infinite loop */
  while (1)
  {}
  
  /* could also generate a system reset to recover from the error */
  /* .... */
}

/**
* @brief  Get next data sample callback
* @param  None
* @retval Next data sample to be sent
*/
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
  return 0;
}

/**
  * @brief  Configures EXTI Line0 (connected to PA0 pin) in interrupt mode
  * @param  None
  * @retval None
  */
static void EXTILine_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;
	
  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
  /* Configure PE0 and PE1 pins as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect EXTI Line to PE1 pins */
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

  /* Configure EXTI Line1 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
