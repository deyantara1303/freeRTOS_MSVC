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
 * NOTE: Windows will not be running the FreeRTOS demo threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Windows port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Windows
 * port for further information:
 * http://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html
 * 
 ******************************************************************************
 *
 * NOTE:  Console input and output relies on Windows system calls, which can
 * interfere with the execution of the FreeRTOS Windows port.  This demo only
 * uses Windows system call occasionally.  Heavier use of Windows system calls
 * can crash the port.
 */
/*

/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Priorities at which the tasks are created.
 */
#define workerTASK_FBS_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define schedulerTASK_FBS_PRIORITY		( tskIDLE_PRIORITY + 2 )

 /* Scheduler task frequency between frames
  */
#define schedulerTASK_FBS_FREQUENCY_MS   pdMS_TO_TICKS( 120UL )
  /*-----------------------------------------------------------*/

/*
* Data structure to hold number of tasks and frames; used by Scheduler task
*/
struct xTaskParam {
	uint32_t task_num;
	uint32_t frame_num;
};

/*
* Task handles for scheduling control
*/
TaskHandle_t xTask[6] = { 0,1,2,3,4,5 };

/*
 * C function (prototype) for Worker/Scheduler tasks
 */
static void workerTaskFBS(int32_t* pvParameter);
static void schedulerTaskFBS(struct xTaskParam* pvParameters);

void main_exercise( void )
{
	static struct xTaskParam xTaskParam = { 6, 5 };
	static int32_t xParam[6] = { 0,1,2,3,4,5 };

	/*
	 * Create the task instances.
	 */
	xTaskCreate(workerTaskFBS,			/* The function that implements the worker tasks. */
		"WorkerTask0", 							/* The text name assigned to the tasks - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the tasks. */
		&xParam[0], 							/* The parameter passed to the worker tasks are task numbers*/
		workerTASK_FBS_PRIORITY,/* The priorities assigned to the tasks, in this case same for all */
		&xTask[0]);							/* The task handles */

	xTaskCreate(workerTaskFBS,
		"WorkerTask1", 						
		configMINIMAL_STACK_SIZE, 		
		&xParam[1],
		workerTASK_FBS_PRIORITY,
		&xTask[1]);

	xTaskCreate(workerTaskFBS,
		"WorkerTask2",
		configMINIMAL_STACK_SIZE,
		&xParam[2],
		workerTASK_FBS_PRIORITY,
		&xTask[2]);

	xTaskCreate(workerTaskFBS,
		"WorkerTask3", 							
		configMINIMAL_STACK_SIZE, 	
		&xParam[3],
		workerTASK_FBS_PRIORITY,
		&xTask[3]);

	xTaskCreate(workerTaskFBS,
		"WorkerTask4",
		configMINIMAL_STACK_SIZE,
		&xParam[4],
		workerTASK_FBS_PRIORITY,
		&xTask[4]);

	xTaskCreate(workerTaskFBS,
		"WorkerTask5",
		configMINIMAL_STACK_SIZE,
		&xParam[5],
		workerTASK_FBS_PRIORITY,
		&xTask[5]);

	xTaskCreate(schedulerTaskFBS,					/* The function that implements the scheduler task. */
		"SchedulerTask",										/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE,				/* The size of the stack to allocate to the task. */
		&xTaskParam,											/* The parameter passed to the task; in this case task and frame numbers*/
		schedulerTASK_FBS_PRIORITY,			/* The priority assigned to the task, in this case highest */
		NULL);														/* The task handle; not required */

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

static void workerTaskFBS(int32_t* pvParameter)
{
	TickType_t xNextWakeTime = xTaskGetTickCount();
	int xCount;

	/* If task parameter is 5 i.e if task 5 is running, then enter infinte loop,
	* else count till 1E6, print no.of cycles counted and relative clock ticks for all other worker tasks and suspend self
	*/
	for (;;)
	{
		if (*pvParameter != 5)
		{
			for (xCount=0; xCount < 1e6; xCount++);
			printf("Task%d:\tI counted %d cycles, took %d ticks\n", *pvParameter, xCount, xNextWakeTime);
			vTaskSuspend(NULL);
		}
		else
		{
			printf("Task%d:\n", *pvParameter);
			for (; ; );
		}
	}
}

static void schedulerTaskFBS(struct xTaskParam* pvParameters)
{
	const TickType_t xBlockTime = schedulerTASK_FBS_FREQUENCY_MS;
	TickType_t xNextWakeTime = xTaskGetTickCount();
	int i = 0;

	/*
	* Schedule only 5 frames. Iterate over the frames. 
	* Check if any task has overrun, then print identifier and suspend that task.
	* Schedule eligible tasks per frame and go into "Blocked" state.
	*/
	for (;;)
	{
		for (; i < pvParameters->frame_num; i++)
		{
			if (i > 0)
			{
				printf("Checking frame%d\n\n", i-1);
				for (int j = 0; j < pvParameters->task_num; j++)
				{
					if (eTaskGetState(xTask[j]) != eSuspended)
					{
						printf("Task%d in frame %d was not suspended :( \n\n", j, i-1);
						vTaskSuspend(xTask[j]);
					}
				}
			}
			else
			{
				printf("It's the very first frame\n");
			}

			printf("Scheduling for frame %d...\n", i);
			switch (i)
			{
			case 0:
				vTaskResume(xTask[0]);
				vTaskResume(xTask[1]);
				vTaskResume(xTask[2]);
				vTaskResume(xTask[3]);
				vTaskResume(xTask[4]);
				break;

			case 1:
				break;

			case 2:
				vTaskResume(xTask[0]);
				vTaskResume(xTask[1]);
				vTaskResume(xTask[4]);
				break;

			case 3:
				vTaskResume(xTask[2]);
				vTaskResume(xTask[3]);
				break;

			case 4:
				vTaskResume(xTask[0]);
				vTaskResume(xTask[1]);
				vTaskResume(xTask[4]);
				break;

			}

			/* Place this task in the blocked state until it is time to run again.
			The block time is specified in ticks, pdMS_TO_TICKS() was used to
			convert a time specified in milliseconds into a time specified in ticks.
			While in the Blocked state this task will not consume any CPU time. */
			vTaskDelayUntil(&xNextWakeTime, xBlockTime);
		}

		/*
		* Delete tasks after job completion
		*/
		printf("\n----------------------------------------------------------------------------------\
			\nDeleting all worker tasks and suspending self\n");
		for (int i = 0; i <= 5; i++)
		{
			vTaskDelete(xTask[i]);
		}
		vTaskSuspend(NULL);
	}
}

/*-----------------------------------------------------------*/
