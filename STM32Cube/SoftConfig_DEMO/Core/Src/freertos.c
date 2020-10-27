/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 * @Author            : Fabian Castano
 * @Institution       : University of Antioquia
 * @Project           : DUNE-Daphne
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// declaration of external variables
extern volatile STR_FLAGS _Events; // flags to handle different events
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
// Task handle variable
xTaskHandle taskLed2Handle;   // variable to handle the task Led 2
xTaskHandle taskLed3Handle;
xTaskHandle taskISRAttnHandle;

// Semaphore handle variable
SemaphoreHandle_t semaphButton1;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void taskLed2(void *arg);  // prototype of task Led 2.
void taskLed3(void *arg);
void taskISRAttn(void *arg);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	semaphButton1 = xSemaphoreCreateBinary(); // creation of binary semaphore to button 1
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */
	osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
	defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	xTaskCreate(taskLed2, "Task_LED2", 512, NULL, 1, &taskLed2Handle); // task to blink LED 2
	xTaskCreate(taskLed3, "Task_LED3", 512, NULL, 1, &taskLed3Handle); // task to blink LED 2
	xTaskCreate(taskISRAttn, "Task_ISR_Attn", 512, NULL, 1, &taskISRAttnHandle); // task to handle interruption attention
	/* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread to blink a LED.-
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument) {
	/* USER CODE BEGIN StartDefaultTask */

	printf("Software DEMO_FC, DUNE-Daphne STM32Cube with FreeRTOS\n");
	/* Infinite loop */
	for (;;) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		// osDelay(1000);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
	/* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/**
 * @brief  Function implementing the defaultTask thread to blink a LED 2.-
 * @param  argument: Not used
 * @retval None
 */
void taskLed2(void *arg) {

	uint8_t led2State = 0;

	/* Infinite loop */
	while (TRUE) {
		// read the state of LED 2 pin.
		led2State = !HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
		// blink the LED 2 every 1 sec.
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, led2State);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(taskLed2Handle);
}

/**
 * @brief  Function implementing the defaultTask thread to blink a LED 3.-
 * @param  argument: Not used
 * @retval None
 */
void taskLed3(void *arg) {

	/* Infinite loop */
	while (TRUE) {

		if (xSemaphoreTake(semaphButton1, portMAX_DELAY) == pdTRUE) {
			// turn on the LED 3 for 1 sec when Button 1 was pressed.
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, HIGH);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, LOW);

			printf("Light turned off\n");
		}

	}
	vTaskDelete(taskLed3Handle);
}

/**
 * @brief  Function implementing the interruption handle.-
 * @param  argument: Not used
 * @retval None
 */
void taskISRAttn(void *arg) {

	/* Infinite loop */
	while (TRUE) {
		if (fl_ext_it_btn) {
			fl_ext_it_btn = FALSE; // interruption attended
			xSemaphoreGive(semaphButton1);
			printf("Semaphore delivered\n");
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
	}
	vTaskDelete(taskISRAttnHandle);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
