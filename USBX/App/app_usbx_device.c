/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_usbx_device.c
 * @author  MCD Application Team
 * @brief   USBX Device applicative file
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
#include "app_usbx_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
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
static ULONG cdc_acm_interface_number;
static ULONG cdc_acm_configuration_number;
static UX_SLAVE_CLASS_CDC_ACM_PARAMETER cdc_acm_parameter;

/* USER CODE BEGIN PV */
static TX_THREAD ux_device_app_thread;
static TX_THREAD ux_cdc_write_thread;
static TX_THREAD ux_cdc2_write_thread;
static UX_SLAVE_CLASS_CDC_ACM_PARAMETER cdc2_acm_parameter;

extern PCD_HandleTypeDef           hpcd_USB_DRD_FS;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID app_ux_device_thread_entry(ULONG thread_input);
/* USER CODE END PFP */

/**
 * @brief  Application USBX Device Initialization.
 * @param  memory_ptr: memory pointer
 * @retval status
 */
UINT MX_USBX_Device_Init(VOID *memory_ptr)
{
	UINT ret = UX_SUCCESS;
	UCHAR *device_framework_high_speed;
	UCHAR *device_framework_full_speed;
	ULONG device_framework_hs_length;
	ULONG device_framework_fs_length;
	ULONG string_framework_length;
	ULONG language_id_framework_length;
	UCHAR *string_framework;
	UCHAR *language_id_framework;
	UCHAR *pointer;
	TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

	/* USER CODE BEGIN MX_USBX_Device_Init0 */

	/* USER CODE END MX_USBX_Device_Init0 */
	/* Allocate the stack for USBX Memory */
	if (tx_byte_allocate(byte_pool, (VOID **) &pointer,
			USBX_DEVICE_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
	{
		/* USER CODE BEGIN USBX_ALLOCATE_STACK_ERROR */
		return TX_POOL_ERROR;
		/* USER CODE END USBX_ALLOCATE_STACK_ERROR */
	}

	/* Initialize USBX Memory */
	if (ux_system_initialize(pointer, USBX_DEVICE_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
	{
		/* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERROR */
		return UX_ERROR;
		/* USER CODE END USBX_SYSTEM_INITIALIZE_ERROR */
	}

	/* Get Device Framework High Speed and get the length */
	device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED,
			&device_framework_hs_length);

	/* Get Device Framework Full Speed and get the length */
	device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED,
			&device_framework_fs_length);

	/* Get String Framework and get the length */
	string_framework = USBD_Get_String_Framework(&string_framework_length);

	/* Get Language Id Framework and get the length */
	language_id_framework = USBD_Get_Language_Id_Framework(&language_id_framework_length);

	/* Install the device portion of USBX */
	if (ux_device_stack_initialize(device_framework_high_speed,
			device_framework_hs_length,
			device_framework_full_speed,
			device_framework_fs_length,
			string_framework,
			string_framework_length,
			language_id_framework,
			language_id_framework_length,
			UX_NULL) != UX_SUCCESS)
	{
		/* USER CODE BEGIN USBX_DEVICE_INITIALIZE_ERROR */
		return UX_ERROR;
		/* USER CODE END USBX_DEVICE_INITIALIZE_ERROR */
	}

	/* Initialize the cdc acm class parameters for the device */
	cdc_acm_parameter.ux_slave_class_cdc_acm_instance_activate   = USBD_CDC_ACM_Activate;
	cdc_acm_parameter.ux_slave_class_cdc_acm_instance_deactivate = USBD_CDC_ACM_Deactivate;
	cdc_acm_parameter.ux_slave_class_cdc_acm_parameter_change    = USBD_CDC_ACM_ParameterChange;

	/* USER CODE BEGIN CDC_ACM_PARAMETER */

	/* USER CODE END CDC_ACM_PARAMETER */

	/* Get cdc acm configuration number */
	cdc_acm_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_CDC_ACM, 0);

	/* Find cdc acm interface number */
	cdc_acm_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_CDC_ACM, 0);

	/* Initialize the device cdc acm class */
	if (ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name,
			ux_device_class_cdc_acm_entry,
			cdc_acm_configuration_number,
			cdc_acm_interface_number,
			&cdc_acm_parameter) != UX_SUCCESS)
	{
		/* USER CODE BEGIN USBX_DEVICE_CDC_ACM_REGISTER_ERROR */
		return UX_ERROR;
		/* USER CODE END USBX_DEVICE_CDC_ACM_REGISTER_ERROR */
	}

	/* USER CODE BEGIN MX_USBX_Device_Init1 */
	/* Initialize the cdc acm class parameters for the device */
	cdc2_acm_parameter.ux_slave_class_cdc_acm_instance_activate   = USBD_CDC2_ACM_Activate;
	cdc2_acm_parameter.ux_slave_class_cdc_acm_instance_deactivate = USBD_CDC2_ACM_Deactivate;
	cdc2_acm_parameter.ux_slave_class_cdc_acm_parameter_change    = USBD_CDC2_ACM_ParameterChange;

	/* Get cdc acm configuration number */
	cdc_acm_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_CDC2_ACM, 0);

	/* Find cdc acm interface number */
	cdc_acm_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_CDC2_ACM, 0);

	/* Initialize the device cdc acm class */
	if (ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name,
			ux_device_class_cdc_acm_entry,
			cdc_acm_configuration_number,
			cdc_acm_interface_number,
			&cdc2_acm_parameter) != UX_SUCCESS)
	{
		return UX_ERROR;
	}

	/* Allocate the stack for device application main thread */
	if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 1024,
			TX_NO_WAIT) != TX_SUCCESS)
	{
		return TX_POOL_ERROR;
	}

	/* Create the device application main thread */
	if (tx_thread_create(&ux_device_app_thread, "USBX Device App Main Thread", app_ux_device_thread_entry,
			0, pointer, 1024, 10,
			10, UX_NO_TIME_SLICE,
			TX_AUTO_START) != TX_SUCCESS)
	{
		return TX_THREAD_ERROR;
	}

	/* Allocate the stack for usbx cdc 1 acm read thread */
	if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 1024, TX_NO_WAIT) != TX_SUCCESS)
	{
		return TX_POOL_ERROR;
	}

	/* Create the usbx cdc 1 acm write thread */
	if (tx_thread_create(&ux_cdc_write_thread, "cdc_acm_write_usbx_app_thread_entry",
			usbx_cdc_acm_write_thread_entry, 1, pointer,
			1024, 20, 20, TX_NO_TIME_SLICE,
			TX_AUTO_START) != TX_SUCCESS)
	{
		return TX_THREAD_ERROR;
	}

	/* Allocate the stack for usbx cdc 2 acm write thread */
	if (tx_byte_allocate(byte_pool, (VOID **) &pointer, 1024, TX_NO_WAIT) != TX_SUCCESS)
	{
		return TX_POOL_ERROR;
	}

	/* Create the usbx cdc 2 acm write thread */
	if (tx_thread_create(&ux_cdc2_write_thread, "cdc_acm2_write_usbx_app_thread_entry",
			usbx_cdc2_acm_write_thread_entry, 1, pointer,
			1024, 20, 20, TX_NO_TIME_SLICE,
			TX_AUTO_START) != TX_SUCCESS)
	{
		return TX_THREAD_ERROR;
	}
	/* USER CODE END MX_USBX_Device_Init1 */

	return ret;
}

/* USER CODE BEGIN 1 */
VOID app_ux_device_thread_entry(ULONG thread_input)
{
	/* initialize the device controller HAL driver */
	MX_USB_PCD_Init();

	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x00, PCD_SNG_BUF, 0x40);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x80, PCD_SNG_BUF, 0x80);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x81, PCD_SNG_BUF, 0xC0);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x01, PCD_SNG_BUF, 0x100);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x82, PCD_SNG_BUF, 0x140);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x83, PCD_SNG_BUF, 0x180);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x03, PCD_SNG_BUF, 0x1C0);
	HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x84, PCD_SNG_BUF, 0x200);

	/* Initialize and link controller HAL driver */
	ux_dcd_stm32_initialize((ULONG)USB_DRD_FS, (ULONG)&hpcd_USB_DRD_FS);

	/* Start the USB device */
	HAL_PCD_Start(&hpcd_USB_DRD_FS);
}
/* USER CODE END 1 */
