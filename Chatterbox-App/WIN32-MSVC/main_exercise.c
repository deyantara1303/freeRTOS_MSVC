/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * NOTE: Windows will not be running the FreeRTOS project threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Windows port, or
 * this project application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Windows
 * port for further information:
 * http://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
 *
 * NOTE 2:  This file only contains the source code that is specific to exercise 2
 * Generic functions, such FreeRTOS hook functions, are defined
 * in main.c.
 ******************************************************************************
 *
 * NOTE:  Console input and output relies on Windows system calls, which can
 * interfere with the execution of the FreeRTOS Windows port.  This demo only
 * uses Windows system call occasionally.  Heavier use of Windows system calls
 * can crash the port.
 */

/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Priorities at which the tasks are created.
 */
#define mainTASK3_CHATTERBOX_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainTASK2_CHATTERBOX_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainTASK1_CHATTERBOX_PRIORITY		( tskIDLE_PRIORITY + 1 )


/* Output frequencey
 */
#define mainTASK_CHATTERBOX_OUTPUT_FREQUENCY_MS   pdMS_TO_TICKS( 1000UL )
/*-----------------------------------------------------------*/

/*
  * Data structure
 */
static struct xTaskParam
{
	char xOutStr[10];
	int32_t xInstNum;
};

/*
 * C function (prototype) for task
 */
static void mainTaskChatterbox(struct xTaskParam* pvParameters);

/*
* Task handles for deletion control
*/
//	TaskHandle_t xTask1_t;
//	TaskHandle_t xTask2_t;
TaskHandle_t xTask3_t;

/*-----------------------------------------------------------*/

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_exercise( void )
{
/*
 * Initialize data structures
 */
	static struct xTaskParam xTask1 = { "Task1\n", 1 };
	static struct xTaskParam xTask2 = { "Task2\n", 2 };
	static struct xTaskParam xTask3 = { "Task3\n", 3 };

	/* 
	 * Create the task instances.
     */
	xTaskCreate(mainTaskChatterbox,			/* The function that implements the task. */
		"Task1", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task. */
		&xTask1, 							/* The parameter passed to the task - not used in this simple case. */
		mainTASK1_CHATTERBOX_PRIORITY,/* The priority assigned to the task. */
		NULL);							/* The task handle is not required, so NULL is passed. */

	xTaskCreate(mainTaskChatterbox,			/* The function that implements the task. */
		"Task2", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task. */
		&xTask2, 							/* The parameter passed to the task - not used in this simple case. */
		mainTASK2_CHATTERBOX_PRIORITY,/* The priority assigned to the task. */
		NULL);							/* The task handle is not required, so NULL is passed. */

	xTaskCreate(mainTaskChatterbox,			/* The function that implements the task. */
		"Task3", 							/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the task. */
		&xTask3, 							/* The parameter passed to the task - not used in this simple case. */
		mainTASK3_CHATTERBOX_PRIORITY,/* The priority assigned to the task. */
		&xTask3_t);							/* The task handle is not required, so NULL is passed. */

	 /*
	  *  Start the task instances.
	  */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* 
 * C function for tasks
 */
static void mainTaskChatterbox(struct xTaskParam* pvParameters)
{
TickType_t xNextWakeTime;
const TickType_t xBlockTime = mainTASK_CHATTERBOX_OUTPUT_FREQUENCY_MS;

/*
* Counter to terminate task 3 after 5 iterations
*/ 
int32_t xExecCount = 5;

	xNextWakeTime = xTaskGetTickCount();

	for (;;)
	{
		switch (pvParameters->xInstNum)
		{
		case 1: printf(pvParameters->xOutStr);
			break;

		case 2: printf(pvParameters->xOutStr);
			break;

		case 3: if (xExecCount)
					{
						printf(pvParameters->xOutStr);
						xExecCount--;
					}
					else
					{
						printf("Terminating Task 3 and deleting\n");
						vTaskDelete(xTask3_t);
					}
			  break;

		default:
			printf("Invalid data\n");
			break;
		}
		/* Place this task in the blocked state until it is time to run again.
			The block time is specified in ticks, pdMS_TO_TICKS() was used to
			convert a time specified in milliseconds into a time specified in ticks.
			While in the Blocked state this task will not consume any CPU time. */
		vTaskDelayUntil(&xNextWakeTime, xBlockTime);
	}
}

