/**
  ******************************************************************************
  * @file    usbd_winusb.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_winusb.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_WINUSB_H
#define __USB_WINUSB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_WINUSB
  * @brief This file is the Header file for USBD_winusb.c
  * @{
  */


/** @defgroup USBD_WINUSB_Exported_Defines
  * @{
  */
#define WINUSB_EPIN_ADDR                 0x81U
#define WINUSB_EPIN_SIZE                 0x02U

#define WINUSB_EPOUT_ADDR                0x01U
#define WINUSB_EPOUT_SIZE                0x02U

#define USB_WINUSB_CONFIG_DESC_SIZ       32U
#define USB_WINUSB_DESC_SIZ              9U

#ifndef WINUSB_HS_BINTERVAL
#define WINUSB_HS_BINTERVAL            0x05U
#endif /* WINUSB_HS_BINTERVAL */

#ifndef WINUSB_FS_BINTERVAL
#define WINUSB_FS_BINTERVAL            0x05U
#endif /* WINUSB_FS_BINTERVAL */

#ifndef USBD_WINUSB_OUTREPORT_BUF_SIZE
#define USBD_WINUSB_OUTREPORT_BUF_SIZE  0x02U
#endif /* USBD_WINUSB_OUTREPORT_BUF_SIZE */
#ifndef USBD_WINUSB_REPORT_DESC_SIZE
#define USBD_WINUSB_REPORT_DESC_SIZE   163U
#endif /* USBD_WINUSB_REPORT_DESC_SIZE */

#define WINUSB_DESCRIPTOR_TYPE           0x21U
#define WINUSB_REPORT_DESC               0x22U

#define WINUSB_REQ_SET_PROTOCOL          0x0BU
#define WINUSB_REQ_GET_PROTOCOL          0x03U

#define WINUSB_REQ_SET_IDLE              0x0AU
#define WINUSB_REQ_GET_IDLE              0x02U

#define WINUSB_REQ_SET_REPORT            0x09U
#define WINUSB_REQ_GET_REPORT            0x01U

#define WEBUSB_REQ_GET_URL_INDEX   0x02u    /* wIndex value for WebUSB GET_URL request */
  /**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
typedef enum
{
  WINUSB_IDLE = 0U,
  WINUSB_BUSY,
}
WINUSB_StateTypeDef;

typedef struct _USBD_WINUSB_Itf
{
  uint8_t                  *pReport;
  int8_t (* Init)(void);
  int8_t (* DeInit)(void);
  int8_t (* OutEvent)(uint8_t event_idx, uint8_t state);

} USBD_WINUSB_ItfTypeDef;

typedef struct
{
  uint8_t              Report_buf[USBD_WINUSB_OUTREPORT_BUF_SIZE];
  uint32_t             Protocol;
  uint32_t             IdleState;
  uint32_t             AltSetting;
  uint32_t             IsReportAvailable;
  WINUSB_StateTypeDef     state;
}
USBD_WINUSB_HandleTypeDef;
/**
  * @}
  */



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef  USBD_WINUSB;
#define USBD_WINUSB_CLASS    &USBD_WINUSB
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_WINUSB_SendReport(USBD_HandleTypeDef *pdev,
                                   uint8_t *report,
                                   uint16_t len);



uint8_t  USBD_WINUSB_RegisterInterface(USBD_HandleTypeDef   *pdev,
                                           USBD_WINUSB_ItfTypeDef *fops);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_WINUSB_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
