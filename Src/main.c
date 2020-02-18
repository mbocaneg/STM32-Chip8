/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
#include "keypad/Keypad.h"
#include "keypad/config.h"
#include "SSD1306/SSD1306.h"
#include "chip8/chip8.h"
#include "chip8/chip8_config.h"
#include "chip8/roms.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
osThreadId chip8TaskHandle;
osThreadId drawTaskHandle;
osThreadId keyscanTaskHandle;
osThreadId delayTaskHandle;
osThreadId soundTaskHandle;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */
void chip8_task(void const *argument);
void draw_task(void const *argument);
void keyscan_task(void const *argument);
void delay_counter_task(void const *argument);
void sound_counter_task(void const *argument);
void _print_give_sem();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char str_buf[64];
Keypad keypad;
ssd1306 oled;
SemaphoreHandle_t redraw_signal;
Chip8 c8;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	//load game rom onto main memory
	chip8_loadmem(&c8, invaders, sizeof(invaders));

	chip8_get_tick = _get_tick;

	//initialize game window
	_window_init();

	vSemaphoreCreateBinary(redraw_signal);

	//initialize Chip8 core
	chip8_init(&c8);

	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
//  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
//  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */

	osThreadDef(chip8_task, chip8_task, osPriorityNormal, 0, 128 * 3);
	chip8TaskHandle = osThreadCreate(osThread(chip8_task), NULL);

	osThreadDef(draw_task, draw_task, osPriorityNormal, 0, 128);
	drawTaskHandle = osThreadCreate(osThread(draw_task), NULL);

	osThreadDef(delay_task, delay_counter_task, osPriorityNormal, 0, 128);
	delayTaskHandle = osThreadCreate(osThread(delay_task), NULL);

	osThreadDef(keyscan_task, keyscan_task, osPriorityNormal, 0, 128);
	keyscanTaskHandle = osThreadCreate(osThread(keyscan_task), NULL);

	/* USER CODE END RTOS_THREADS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE()
	;
	__HAL_RCC_GPIOA_CLK_ENABLE()
	;
	__HAL_RCC_GPIOB_CLK_ENABLE()
	;

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : PA0 PA1 PA2 PA3 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PA4 PA5 PA6 PA7 */
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/*
Main Chip8 task. Responsible for cycling the
chip8.
*/
void chip8_task(void const *argument) {

	for (;;) {
		while (!c8.halt) {
			chip8_clockcycle(&c8);
		}

		_window_kill();
	}

}

/*
Keyscan task. Responsible for polling the keyboard
and registering keypresses. Whenever a keypress is
detected, the corresponding key flag is set in the
chip8's keypad register. It is cleared when the key
is released.
*/
void keyscan_task(void const *argument) {

	Keypad keypad;
	keypad._write_row = _write_row;
	keypad._read_col = _read_col;
	keypad._get_ms_tick = _get_ms_tick;
	keypad_init(&keypad, keymap);
	for (;;) {
		if (keypad_getchar(&keypad)) {
			char key = keypad.keypress;

			(key == '0') ? chip8_keyset(&c8, 0x00) : chip8_keyreset(&c8, 0x00);
			(key == '1') ? chip8_keyset(&c8, 0x01) : chip8_keyreset(&c8, 0x01);
			(key == '2') ? chip8_keyset(&c8, 0x02) : chip8_keyreset(&c8, 0x02);
			(key == '3') ? chip8_keyset(&c8, 0x03) : chip8_keyreset(&c8, 0x03);
			(key == '4') ? chip8_keyset(&c8, 0x04) : chip8_keyreset(&c8, 0x04);
			(key == '5') ? chip8_keyset(&c8, 0x05) : chip8_keyreset(&c8, 0x05);
			(key == '6') ? chip8_keyset(&c8, 0x06) : chip8_keyreset(&c8, 0x06);
			(key == '7') ? chip8_keyset(&c8, 0x07) : chip8_keyreset(&c8, 0x07);
			(key == '8') ? chip8_keyset(&c8, 0x08) : chip8_keyreset(&c8, 0x08);
			(key == '9') ? chip8_keyset(&c8, 0x09) : chip8_keyreset(&c8, 0x09);
			(key == 'A') ? chip8_keyset(&c8, 0x0A) : chip8_keyreset(&c8, 0x0A);
			(key == 'B') ? chip8_keyset(&c8, 0x0B) : chip8_keyreset(&c8, 0x0B);
			(key == 'C') ? chip8_keyset(&c8, 0x0C) : chip8_keyreset(&c8, 0x0C);
			(key == 'D') ? chip8_keyset(&c8, 0x0D) : chip8_keyreset(&c8, 0x0D);
			(key == '*') ? chip8_keyset(&c8, 0x0E) : chip8_keyreset(&c8, 0x0E);
			(key == '#') ? chip8_keyset(&c8, 0x0F) : chip8_keyreset(&c8, 0x0F);

		} else {
			chip8_keyreset(&c8, 0x00);
			chip8_keyreset(&c8, 0x01);
			chip8_keyreset(&c8, 0x02);
			chip8_keyreset(&c8, 0x03);
			chip8_keyreset(&c8, 0x04);
			chip8_keyreset(&c8, 0x05);
			chip8_keyreset(&c8, 0x06);
			chip8_keyreset(&c8, 0x07);
			chip8_keyreset(&c8, 0x08);
			chip8_keyreset(&c8, 0x09);
			chip8_keyreset(&c8, 0x0A);
			chip8_keyreset(&c8, 0x0B);
			chip8_keyreset(&c8, 0x0C);
			chip8_keyreset(&c8, 0x0D);
			chip8_keyreset(&c8, 0x0E);
			chip8_keyreset(&c8, 0x0F);
		}

		vTaskDelay(2 / portTICK_RATE_MS);
	}
}

/*
Task that is responsible for drawing onto the OLED.
It is set of whenever the chip8's redraw flag is set
to true. At which point, the chip8's graphics buffer
is loaded onto the OLED's graphics memory.
*/
void draw_task(void const *argument) {

	while (1) {
		if (xSemaphoreTake(redraw_signal, 9999999)) {
			for (uint16_t y = 0; y < 32; y++) {
				for (uint16_t x = 0; x < 64; x++) {
					bool state = (chip8_pixel_test(&c8, x, y)) ? true : false;
					if (state == true) {
						ssd1306_fill_rectangle(&oled, x * 2, y * 2, 2, 2, true);
					} else {
						ssd1306_fill_rectangle(&oled, x * 2, y * 2, 2, 2,
								false);
					}

				}
			}
			ssd1306_refresh(&oled);
		}

	}
}

/*
 * Chip8 sound and delay counter tasks. Both count down at a rate of 60 times/second. Used
 * by the Chip8 for system delays and generating sound
 * TODO: implement sound generation
 * */
void sound_counter_task(void const *argument) {

	for (;;) {
		if (c8.sound > 0) {
			c8.sound -= 1;
			vTaskDelay(16 / portTICK_RATE_MS);
		}
	}

}
void delay_counter_task(void const *argument) {

	for (;;) {
		if (c8.delay > 0) {
			c8.delay -= 1;
			vTaskDelay(16 / portTICK_RATE_MS);
		}
	}

}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */

/*
 * Debug task used to test for keypresses.
 * */
void StartDefaultTask(void const * argument) {

	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	for (;;) {
//    osDelay(1);
		if (keypad_getchar(&keypad)) {
			memset(str_buf, 0, 64);
			sprintf(str_buf, "%c\r\n", keypad.keypress);
			HAL_UART_Transmit(&huart1, (uint8_t *) str_buf, strlen(str_buf),
					50);
		}
	}
	/* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM1) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
