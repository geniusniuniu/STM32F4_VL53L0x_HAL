#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "vl53l0x.h"
#include "vl53l0x_platform.h"

#define FILTER_N	16

void SystemClock_Config(void);
void vl53l0x_test(void);
uint8_t sta = 6;

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
	MX_USART1_UART_Init();
	SystemClock_Config();
	
	MX_GPIO_Init();
	LED_Init();
	XShut_PinInit();
	VL53L0X_i2c_init();//初始化IIC总线
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET  );	//使用片选信号启动第一个tof
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);	
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);		
	HAL_Delay(200);	//等待，确保tof启动

	//使用默认地址初始化第一个tof
	while(vl53l0x_init(&Xaxis_vl53l0x_dev,Xaxis_VL53L0X_Xshut))//vl53l0x初始化
	{
		printf("Xaxis_VL53L0x Error!!!\n\r");
		HAL_Delay(100);
		LED0=!LED0;//DS0闪烁
	}
	printf("Xaxis_VL53L0XInit OK\r\n");
	
	//修改第一个tof的iic操作地址
	sta = vl53l0x_Addr_set(&Xaxis_vl53l0x_dev,TOF_X_ADDR);
	printf("Addr:%#x\r\n",Xaxis_vl53l0x_dev.I2cDevAddr);

	
	//启动第二个TOF
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
	HAL_Delay(200);	//等待，确保tof启动
	
	//使用默认地址初始化第二个tof
	while(vl53l0x_init(&Yaxis_vl53l0x_dev,Yaxis_VL53L0X_Xshut))//vl53l0x初始化
	{
		printf("Yaxis_VL53L0x Error!!!\n\r");
		HAL_Delay(100);
		LED0=!LED0;//DS0闪烁
	}
	printf("Yaxis_VL53L0XInit OK\r\n");
	
	//修改第二个tof的iic操作地址
	vl53l0x_Addr_set(&Yaxis_vl53l0x_dev,TOF_Y_ADDR);
	printf("Addr:%#x\r\n",Yaxis_vl53l0x_dev.I2cDevAddr);
//	
//	

	//启动第三个TOF
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
	HAL_Delay(200);	//等待，确保tof启动	
	
	//使用默认地址初始化第三个tof
	while(vl53l0x_init(&Zaxis_vl53l0x_dev,Zaxis_VL53L0X_Xshut))//vl53l0x初始化
	{
		printf("Zaxis_VL53L0x Error!!!\n\r");
		HAL_Delay(100);
		LED0=!LED0;//DS0闪烁
	}
	printf("Zaxis_VL53L0XInit OK\r\n");
	
	//修改第三个tof的iic操作地址
	//vl53l0x_Addr_set(&Zaxis_vl53l0x_dev,TOF_Z_ADDR);
	Zaxis_vl53l0x_dev.I2cDevAddr = TOF_Z_ADDR;
	printf("Addr:%#x\r\n",Zaxis_vl53l0x_dev.I2cDevAddr);
	
	//修改TOF读取模式，现在是默认模式，可以在宏定义中找到别的模式
	if(VL53L0X_ERROR_NONE == vl53l0x_set_mode(&Xaxis_vl53l0x_dev,Default_Mode)) 
		printf("Xaxis_VL53L0X MODE SET OK\r\n");
	
	vl53l0x_test();//vl53l0x测试（死循环）


}


//VL53L0X测试程序
void vl53l0x_test(void)
{   
	 u8 i=0;
	 VL53L0X_Error status = 0; 
	 while(1)
	 {
		status = vl53l0x_start_single_test(&Xaxis_vl53l0x_dev,&vl53l0x_data,buf);
		Filter_Window(Distance_data); 
		 
		if(status == VL53L0X_ERROR_NONE)
		{
			printf("%d\r\n",Distance_data);
		}
		else
			printf("%d\r\n",status);
		
		delay_ms(10);
		
		i++;
		if(i==20)	//闪灯
		{			 
			LED1 =! LED1;
			i = 0;
		} 
	 }
}


//printf重定向
int fputc(int ch, FILE *f) 
{
	uint8_t temp[1] = {ch};
	HAL_UART_Transmit(&huart1, temp, 1, 2);//
	return ch;
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
