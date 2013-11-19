/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_audio_codec.h"
#include "stm32f4xx_it.h"
#include "waveplayer.h"

/* Select the media where the Wave file is stored */
#ifndef MEDIA_IntFLASH
#define MEDIA_IntFLASH /* Wave file stored in internal flash */
#endif

#endif /* __MAIN_H */
