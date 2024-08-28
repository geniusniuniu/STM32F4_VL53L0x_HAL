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
        filter_buf[i] = filter_buf[i + 1]; // �����������ƣ���λ�Ե�
        filter_sum += filter_buf[i];
    }
    return (uint16_t)(filter_sum / FILTER_N);
}

static char buf[VL53L0X_MAX_STRING_LENGTH];//����ģʽ�ַ����ַ�������


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
	VL53L0X_i2c_init();//��ʼ��IIC����
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET  );	//ʹ��Ƭѡ�ź�������һ��tof
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);	
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);		
	HAL_Delay(200);	//�ȴ���ȷ��tof����

	//ʹ��Ĭ�ϵ�ַ��ʼ����һ��tof
	while(vl53l0x_init(&Xaxis_vl53l0x_dev,Xaxis_VL53L0X_Xshut))//vl53l0x��ʼ��
	{
		printf("Xaxis_VL53L0x Error!!!\n\r");
		HAL_Delay(100);
		LED0=!LED0;//DS0��˸
	}
	printf("Xaxis_VL53L0XInit OK\r\n");
	
	//�޸ĵ�һ��tof��iic������ַ
	sta = vl53l0x_Addr_set(&Xaxis_vl53l0x_dev,TOF_X_ADDR);
	printf("Addr:%#x\r\n",Xaxis_vl53l0x_dev.I2cDevAddr);

	
	//�����ڶ���TOF
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
	HAL_Delay(200);	//�ȴ���ȷ��tof����
	
	//ʹ��Ĭ�ϵ�ַ��ʼ���ڶ���tof
	while(vl53l0x_init(&Yaxis_vl53l0x_dev,Yaxis_VL53L0X_Xshut))//vl53l0x��ʼ��
	{
		printf("Yaxis_VL53L0x Error!!!\n\r");
		HAL_Delay(100);
		LED0=!LED0;//DS0��˸
	}
	printf("Yaxis_VL53L0XInit OK\r\n");
	
	//�޸ĵڶ���tof��iic������ַ
	vl53l0x_Addr_set(&Yaxis_vl53l0x_dev,TOF_Y_ADDR);
	printf("Addr:%#x\r\n",Yaxis_vl53l0x_dev.I2cDevAddr);
//	
//	

	//����������TOF
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
	HAL_Delay(200);	//�ȴ���ȷ��tof����	
	
	//ʹ��Ĭ�ϵ�ַ��ʼ��������tof
	while(vl53l0x_init(&Zaxis_vl53l0x_dev,Zaxis_VL53L0X_Xshut))//vl53l0x��ʼ��
	{
		printf("Zaxis_VL53L0x Error!!!\n\r");
		HAL_Delay(100);
		LED0=!LED0;//DS0��˸
	}
	printf("Zaxis_VL53L0XInit OK\r\n");
	
	//�޸ĵ�����tof��iic������ַ
	//vl53l0x_Addr_set(&Zaxis_vl53l0x_dev,TOF_Z_ADDR);
	Zaxis_vl53l0x_dev.I2cDevAddr = TOF_Z_ADDR;
	printf("Addr:%#x\r\n",Zaxis_vl53l0x_dev.I2cDevAddr);
	
	//�޸�TOF��ȡģʽ��������Ĭ��ģʽ�������ں궨�����ҵ����ģʽ
	if(VL53L0X_ERROR_NONE == vl53l0x_set_mode(&Xaxis_vl53l0x_dev,Default_Mode)) 
		printf("Xaxis_VL53L0X MODE SET OK\r\n");
	
	vl53l0x_test();//vl53l0x���ԣ���ѭ����


}


//VL53L0X���Գ���
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
		if(i==20)	//����
		{			 
			LED1 =! LED1;
			i = 0;
		} 
	 }
}


//printf�ض���
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
