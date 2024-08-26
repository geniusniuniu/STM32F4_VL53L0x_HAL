/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "vl53l0x.h"

#define FILTER_N	16

void SystemClock_Config(void);
void vl53l0x_test(void);


uint16_t Filter_Window(uint16_t Dis) 
{
    static uint16_t filter_buf[FILTER_N + 1];
    int i;    
    uint16_t filter_sum = 0;
    filter_buf[FILTER_N] = Dis;
    for(i = 0; i < FILTER_N; i++) 
    {
        filter_buf[i] = filter_buf[i + 1]; // 所有数据左移，低位仍掉
        filter_sum += filter_buf[i];
    }
    return (uint16_t)(filter_sum / FILTER_N);
}

static char buf[VL53L0X_MAX_STRING_LENGTH];//测试模式字符串字符缓冲区


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();

/******************这是TOF初始化的必要代码，不可删除************************************/
	while(vl53l0x_init(&vl53l0x_dev))//vl53l0x初始化
	{
		printf("VL53L0X Error!!!\n\r");
		delay_ms(100);
		LED0=!LED0;//DS0闪烁
	}
	printf("VL53L0X OK\r\n");

	//修改TOF读取模式，现在是默认模式，可以在宏定义中找到别的模式
	if(VL53L0X_ERROR_NONE == vl53l0x_set_mode(&vl53l0x_dev,Default_Mode)) 
		printf("VL53L0X MODE SET OK\r\n");
/******************上述TOF初始化的必要代码，不可删除************************************/

	vl53l0x_test();//vl53l0x测试


}


//VL53L0X测试程序
void vl53l0x_test(void)
{   
	 u8 i=0;
	 VL53L0X_Error status = 0; 
	 while(1)
	 {
		status = vl53l0x_start_single_test(&vl53l0x_dev,&vl53l0x_data,buf);
		Filter_Window(Distance_data); 
		 
		if(status == VL53L0X_ERROR_NONE)
		{
			printf("%d\r\n",Distance_data);
		}
		
		delay_ms(30);
		
		i++;
		if(i==20)	//0.6s闪一次灯
		{
			 i = 0;
			 LED1=!LED1;
		} 
	 }
}











/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
