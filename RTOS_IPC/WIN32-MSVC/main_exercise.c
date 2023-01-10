/******************************************************************************
 * NOTE: Windows will not be running the FreeRTOS demo threads continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Windows port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Windows
 * port for further information:
 * http://www.freertos.org/FreeRTOS-Windows-Simulator-Emulator-for-Visual-Studio-and-Eclipse-MingW.html

/* Standard includes. */
#include <stdio.h>
#include <assert.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"

/* Enum for controller names */
enum Controllers { Primary = 1, Reserve };

/* Structure to hold data of the sensor tasks*/
typedef struct info {
	int32_t begin;
	int32_t end;
} Info;

/* Priorities at which the tasks are created.
 */
#define prodTASK_IPC_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define consTASK_IPC_PRIORITY		( tskIDLE_PRIORITY + 2 )

 /* Sensor Task frequencies
  */
#define sensor1TASK_IPC_FREQUENCY_MS   pdMS_TO_TICKS( 200UL )
#define sensor2aTASK_IPC_FREQUENCY_MS   pdMS_TO_TICKS( 500UL )
#define sensor2bTASK_IPC_FREQUENCY_MS   pdMS_TO_TICKS( 1400UL )

  /*Bit definitions for events of sensor1, sensor2a and sensor 2b*/
 #define BIT_1					( 0x01 )
 #define BIT_2A					( 0x02 )
 #define BIT_2B					( 0x03 )

/*Bit definition for Controller running*/
#define BIT_XX					(0x09)

 /*
 * Task handles for controllers
 */
TaskHandle_t xTask1, xTask2;

/* The event groups handles used by the sensor and controller tasks. */
EventGroupHandle_t xSensorEventGroup, xControllerEventGroup;

/* Queue Handles and Queue Set */
QueueHandle_t xQueue1, xQueue2a, xQueue2b;
static QueueSetHandle_t xQueueSet;

/*
 * C function (prototype) for Sensors/Controller tasks
 */
static void sensor1TaskIPC(Info* pvParameter);
static void sensor2aTaskIPC(Info* pvParameter);
static void sensor2bTaskIPC(Info* pvParameter);
static void consTaskIPC(int32_t* pvParameters);

/*
* Helper function for incrementing counter
*/
int32_t incCounter(int32_t data, int32_t begin, int32_t end);

/*Entry point*/
void main_exercise(void)
{
	/*Controller Tasks parameters*/
	static int32_t xParam[2] = { Primary, Reserve };

	/* Sensor Tasks parameters*/
	static Info info1 = { 100, 199 };
	static Info info2a = { 200, 249 };
	static Info info2b = { 250, 299 };

	/*
	 * Create the task instances.
	 */
	xTaskCreate(sensor1TaskIPC,			/* The function that implements the Sensor task 1. */
		"Sensor 1", 							/* The text name assigned to the tasks - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE, 		/* The size of the stack to allocate to the tasks. */
		&info1, 						                     	/* The parameter passed to the task are counter initial and end values*/
		prodTASK_IPC_PRIORITY,/* The priorities assigned to the tasks, in this case same for all sensor tasks*/
		NULL);							/* The task handle, not required */

	xTaskCreate(sensor2aTaskIPC,				/* The function that implements the Sensor task 2A. */
		"Sensor 2A",
		configMINIMAL_STACK_SIZE,
		&info2a,
		prodTASK_IPC_PRIORITY,
		NULL);

	xTaskCreate(sensor2bTaskIPC,				/* The function that implements the Sensor task 2B. */
		"Sensor 2B",
		configMINIMAL_STACK_SIZE,
		&info2b,
		prodTASK_IPC_PRIORITY,
		NULL);
 
     	xTaskCreate(consTaskIPC,							/* The function that implements the Controller task 1. */
		"Controller 1",										/* The text name assigned to the task - for debug only as it is not used by the kernel. */
		configMINIMAL_STACK_SIZE,				/* The size of the stack to allocate to the task. */
		&xParam[0],											/* The parameter passed to the task; in this case task and frame numbers*/
		consTASK_IPC_PRIORITY,			/* The priority assigned to the task, in this case highest */
		&xTask1);														/* The task handle for controller 1*/

	   xTaskCreate(consTaskIPC,							/* The function that implements the Controller task 2. */
		"Controller 2",										/* The text name assigned to the task - for debug only as it is not used by the kernel. */
	   configMINIMAL_STACK_SIZE,				/* The size of the stack to allocate to the task. */
	   &xParam[1],											/* The parameter passed to the task; in this case task and frame numbers*/
	   consTASK_IPC_PRIORITY,			/* The priority assigned to the task, in this case highest */
	  &xTask2);														/* The task handle for controller 2 */

	/*
	* Create the Queues
	*  xQueue1 -> Hold sensor1 data
	*  xQueue2a -> Hold sensor2a data
	*  xQueue2b -> Hold sensor2b data
	*/
	xQueue1 = xQueueCreate(1, sizeof(int));
	xQueue2a = xQueueCreate(1, sizeof(int));
	xQueue2b = xQueueCreate(1, sizeof(int));

	/* Create a queue set containing Queue1, Queue2a and Queue 2b
	*/
	xQueueSet = xQueueCreateSet(3);
	xQueueAddToSet(xQueue1, xQueueSet);
	xQueueAddToSet(xQueue2a, xQueueSet);
	xQueueAddToSet(xQueue2b, xQueueSet);

	/* Attempt to create the event groups. */
	xSensorEventGroup = xEventGroupCreate();
	xControllerEventGroup = xEventGroupCreate();
	if (xSensorEventGroup == NULL && xControllerEventGroup == NULL)
	{
		printf("Event groups could not be created, exiting system\n");
	}
	else
	/*
	 *  Start the task instances.
	 */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
	for (;; );
}

/* Sensor1 Task*/
static void sensor1TaskIPC(Info* pvParameter)
{
	const TickType_t xBlockTime = sensor1TASK_IPC_FREQUENCY_MS;
	TickType_t xNextWakeTime = xTaskGetTickCount();
	int32_t data = pvParameter->begin;
	EventBits_t uxBits;

	for (;;)
	{
		/*Synchronize the start time of sensor tasks*/
		vTaskDelayUntil(&xNextWakeTime, xBlockTime);

	//printf("Sensor 1 at %d\n", xTaskGetTickCount());

		/*Increment counter by 1, store value.
		Reset at overflow*/
		data = incCounter(data, pvParameter->begin, pvParameter->end);

		if (xQueue1 != NULL)
		{
			xQueueSend(xQueue1, &data, (TickType_t)0U);
			uxBits = xEventGroupSetBits(xSensorEventGroup, BIT_1);						/*Set bit_1 to indicate controller for data in Queue1*/
		}
		else
		{
			printf("Queue1 not created\n");
		}
	}
}

/*Sensor2A Task*/
static void sensor2aTaskIPC(Info* pvParameter)
{
	const TickType_t xBlockTime = sensor2aTASK_IPC_FREQUENCY_MS;
	TickType_t xNextWakeTime = xTaskGetTickCount();
	int32_t data = pvParameter->begin;
	EventBits_t uxBits;

	for (;;)
	{
		/*Synchronize the start time of sensor tasks*/
		vTaskDelayUntil(&xNextWakeTime, xBlockTime);

	//printf("Sensor 2a at %d\n", xTaskGetTickCount());

		/*Increment counter by 1, store value.
		Reset at overflow*/
		data = incCounter(data, pvParameter->begin, pvParameter->end);

		if (xQueue2a != NULL)
		{
			xQueueSend(xQueue2a, &data, (TickType_t)0U);
			uxBits = xEventGroupSetBits(xSensorEventGroup,   BIT_2A);					 /*Set bit2A to indicate controller for data in Queue 2A*/
		}
		else
		{
			printf("Queue2a not created\n");
		}
	}
}

/* Sensor2B Task*/
static void sensor2bTaskIPC(Info* pvParameter)
{
	const TickType_t xBlockTime = sensor2bTASK_IPC_FREQUENCY_MS;
	TickType_t xNextWakeTime = xTaskGetTickCount();
	int32_t data = pvParameter->begin;
	EventBits_t uxBits;

	for (;;)
	{
		/*Synchronize the start time of sensor tasks*/
		vTaskDelayUntil(&xNextWakeTime, xBlockTime);

	//printf("Sensor 2b at %d\n", xTaskGetTickCount());

		/*Increment counter by 1, store value. 
		Reset at overflow*/
		data = incCounter(data, pvParameter->begin, pvParameter->end);

		if (xQueue2b != NULL)
		{
			xQueueSend(xQueue2b, &data, (TickType_t)0U);
			uxBits = xEventGroupSetBits(xSensorEventGroup, BIT_2B);					/*Set bit2B to indicate controller for data in Queue 2B*/
		}
		else
		{
			printf("Queue2b not created\n");
		}
	}
}

/*Helper function for incrementing counter and reset at overflow*/
int32_t incCounter(int32_t data, int32_t begin, int32_t end)
{
	if (data < end)
	{
		data = data + 1;
	}
	else
		data = begin;

	return data;
}

/*Controller tasks*/
static void consTaskIPC(int32_t* pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 500 / portTICK_PERIOD_MS;

	int32_t msg_1 = 0, msg_2 = 0;
	QueueSetMemberHandle_t xActivatedMember;

	for (;;)
	{
		switch (*pvParameters)
		{
			/*
			* Controller 1 operations
			*/
			case Primary:
			{
				xEventGroupSetBits(xControllerEventGroup, BIT_XX);					/*Set bitXX to indicate Controller 1 still running*/

				/* Wait a maximum of 500ms for either BIT_2A or BIT_2B to be set within the event group.  Clear the bits before exiting. */
				uxBits = xEventGroupWaitBits(xSensorEventGroup,   /* The event group */
																		BIT_1 | BIT_2A | BIT_2B,	 /* The bits within the sensor event group to wait for. */
																		pdTRUE,        /* BIT_1, BIT_2A & BIT_2B should be cleared before returning. */
																		pdFALSE,       /* Don't wait for all bits, either bit will do. */
																		xTicksToWait);	  /* Wait a maximum of 500ms for either bit to be set. */

				/* xEventGroupWaitBits() returned because either BIT_1, BIT_2A or BIT_2B was set. */
				if ( (uxBits & BIT_1) != 0 || ( (uxBits & BIT_2A) != 0 || (uxBits & BIT_2B) != 0) )
				{
					xActivatedMember = xQueueSelectFromSet(xQueueSet, 200 / portTICK_PERIOD_MS);

					if (xActivatedMember == xQueue1)
					{
						xQueueReceive(xQueue1, &msg_1, (TickType_t)0U);				/* Receive Queue1 sensor data*/
					}
					else if (xActivatedMember == xQueue2a)
					{
						xQueueReceive(xQueue2a, &msg_2, (TickType_t)0U);			/* Receive Queue2a sensor data*/
					}
					else if (xActivatedMember == xQueue2b)
					{
						xQueueReceive(xQueue2b, &msg_2, (TickType_t)0U);			/* Receive Queue2b sensor data*/
					}

					printf("Controller 1 has received data at %d; Sensor1: %d, Sensor2: %d\n", xTaskGetTickCount(), msg_1, msg_2);
				}

				/*Fail Controller 1 after 2000ms*/
				if (xTaskGetTickCount() >= 2000)
				{
					printf("Controller 1 had an error at %d\n", xTaskGetTickCount());
					vTaskDelete(xTask1);
				}
			break;
			}

			/*
			* Controller 2 operations
			*/
			case Reserve:
			{
				/* Wait a maximum of 500ms for BIT_XX to be set within the event group.  Clear the bits before exiting. */
				uxBits = xEventGroupWaitBits(xControllerEventGroup,   /* The event group */
																					BIT_XX,	 /* The bits within the sensor event group to wait for. */
																					pdTRUE,        /* BIT_XX should be cleared before returning. */
																					pdFALSE,       /* Don't wait for both bits, either bit will do. */
																					xTicksToWait);	  /* Wait a maximum of 500ms for bit to be set. */

				if ((uxBits & BIT_XX) == 0)
				{
					/* Wait a maximum of 500ms for either BIT_2A or BIT_2B to be set within the event group.  Clear the bits before exiting. */
					uxBits = xEventGroupWaitBits(xSensorEventGroup,   /* The event group */
						BIT_1 | BIT_2A | BIT_2B,	 /* The bits within the sensor event group to wait for. */
						pdTRUE,        /* BIT_1, BIT_2A & BIT_2B should be cleared before returning. */
						pdFALSE,       /* Don't wait for all bits, either bit will do. */
						xTicksToWait);	  /* Wait a maximum of 500ms for either bit to be set. */

					/* xEventGroupWaitBits() returned because either BIT_1, BIT_2A or BIT_2B was set. */
					if ((uxBits & BIT_1) != 0 || ((uxBits & BIT_2A) != 0 || (uxBits & BIT_2B) != 0))
					{
						xActivatedMember = xQueueSelectFromSet(xQueueSet, 200 / portTICK_PERIOD_MS);

						if (xActivatedMember == xQueue1)
						{
							xQueueReceive(xQueue1, &msg_1, (TickType_t)0U);				/* Receive Queue1 sensor data*/
						}
						else if (xActivatedMember == xQueue2a)
						{
							xQueueReceive(xQueue2a, &msg_2, (TickType_t)0U);			/* Receive Queue2a sensor data*/
						}
						else if (xActivatedMember == xQueue2b)
						{
							xQueueReceive(xQueue2b, &msg_2, (TickType_t)0U);			/* Receive Queue2b sensor data*/
						}

						printf("Controller 2 has received data at %d; Sensor1: %d, Sensor2: %d\n", xTaskGetTickCount(), msg_1, msg_2);
					}
				}
			}
			break;
		}
	}
}

/*-----------------------------------------------------------*/


