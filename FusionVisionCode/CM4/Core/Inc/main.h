/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define SCREEN_LED_Pin GPIO_PIN_9
#define SCREEN_LED_GPIO_Port GPIOF
#define IR_LED_PWR_Pin GPIO_PIN_0
#define IR_LED_PWR_GPIO_Port GPIOC
#define ADC_LIGHT_SENSOR_Pin GPIO_PIN_0
#define ADC_LIGHT_SENSOR_GPIO_Port GPIOA
#define ADC_CURRENT_Pin GPIO_PIN_3
#define ADC_CURRENT_GPIO_Port GPIOA
#define OV_PWDN_Pin GPIO_PIN_5
#define OV_PWDN_GPIO_Port GPIOA
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define BTN_3_Pin GPIO_PIN_15
#define BTN_3_GPIO_Port GPIOF
#define LEPTON_VSYNC_Pin GPIO_PIN_13
#define LEPTON_VSYNC_GPIO_Port GPIOE
#define LEPTON_RST_Pin GPIO_PIN_14
#define LEPTON_RST_GPIO_Port GPIOE
#define LEPTON_PWR_Pin GPIO_PIN_15
#define LEPTON_PWR_GPIO_Port GPIOE
#define BTN_2_Pin GPIO_PIN_10
#define BTN_2_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define OV_RST_Pin GPIO_PIN_15
#define OV_RST_GPIO_Port GPIOA
#define COMPASS_INTB_Pin GPIO_PIN_3
#define COMPASS_INTB_GPIO_Port GPIOD
#define COMPASS_INTA_Pin GPIO_PIN_6
#define COMPASS_INTA_GPIO_Port GPIOD
#define BTN_1_Pin GPIO_PIN_0
#define BTN_1_GPIO_Port GPIOE
#define LD2_Pin GPIO_PIN_1
#define LD2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
