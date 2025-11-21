/**
  ******************************************************************************
  * @file    usbd_winusb.c
  * @author  MCD Application Team
  * @brief   This file provides the WINUSB core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                WINUSB Class  Description
  *          ===================================================================
  *           This module manages the WINUSB-compatible class V2.0
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
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

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_winusb.h"
#include "usbd_ctlreq.h"

/* --- WebUSB minimal support (Vendor control + URL descriptor) --- */
#define WEBUSB_VENDOR_CODE       0x22u
#define WEBUSB_REQ_GET_URL       0x0002u
/* Landing page URL (index 1). Scheme 0x01 = https. Change as needed. */
static const char* const WEBUSB_URL_STR = "elmot.xyz";
/* URL descriptor: bLength, bDescriptorType(0x03), bScheme, URL... */
static const uint8_t WEBUSB_URL_DESCRIPTOR[] = {
  (uint8_t)(3 + sizeof("elmot.xyz") - 1), /* bLength */
  0x03, /* bDescriptorType = WebUSB URL */
  0x01, /* bScheme = https */
  /* URL bytes */
  'e','l','m','o','t','.','x','y','z'
};


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_WINUSB
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_WINUSB_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_WINUSB_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_WINUSB_Private_Macros
  * @{
  */
/**
  * @}
  */
/** @defgroup USBD_WINUSB_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_WINUSB_Init(USBD_HandleTypeDef *pdev,
                                     uint8_t cfgidx);

static uint8_t  USBD_WINUSB_DeInit(USBD_HandleTypeDef *pdev,
                                       uint8_t cfgidx);

static uint8_t  USBD_WINUSB_Setup(USBD_HandleTypeDef *pdev,
                                      USBD_SetupReqTypedef *req);

static uint8_t  *USBD_WINUSB_GetFSCfgDesc(uint16_t *length);

static uint8_t  *USBD_WINUSB_GetHSCfgDesc(uint16_t *length);

static uint8_t  *USBD_WINUSB_GetOtherSpeedCfgDesc(uint16_t *length);

static uint8_t  *USBD_WINUSB_GetDeviceQualifierDesc(uint16_t *length);

static uint8_t  USBD_WINUSB_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_WINUSB_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_WINUSB_EP0_RxReady(USBD_HandleTypeDef  *pdev);
/**
  * @}
  */

/** @defgroup USBD_WINUSB_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_WINUSB =
{
  USBD_WINUSB_Init,
  USBD_WINUSB_DeInit,
  USBD_WINUSB_Setup,
  NULL, /*EP0_TxSent*/
  USBD_WINUSB_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
  USBD_WINUSB_DataIn, /*DataIn*/
  USBD_WINUSB_DataOut,
  NULL, /*SOF */
  NULL,
  NULL,
  USBD_WINUSB_GetHSCfgDesc,
  USBD_WINUSB_GetFSCfgDesc,
  USBD_WINUSB_GetOtherSpeedCfgDesc,
  USBD_WINUSB_GetDeviceQualifierDesc,
};

/* USB WINUSB device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WINUSB_CfgFSDesc[USB_WINUSB_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_WINUSB_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of Vendor Specific interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0xFF,         /*bInterfaceClass: Vendor Specific */
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of WINUSB *************************/
  /* 18 */
  0x09,         /*bLength: WINUSB Descriptor size*/
  WINUSB_DESCRIPTOR_TYPE, /*bDescriptorType: WINUSB*/
  0x11,         /*bWINUSBUSTOM_HID: WINUSB Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of WINUSB class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_WINUSB_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of WinUSB endpoints ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  WINUSB_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  WINUSB_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  WINUSB_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  WINUSB_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  WINUSB_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  WINUSB_FS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 41 */
};

/* USB WINUSB device HS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WINUSB_CfgHSDesc[USB_WINUSB_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_WINUSB_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of Vendor-Specific interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0xFF,         /*bInterfaceClass: Vendor Specific */
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of WINUSB *************************/
  /* 18 */
  0x09,         /*bLength: WINUSB Descriptor size*/
  WINUSB_DESCRIPTOR_TYPE, /*bDescriptorType: WINUSB*/
  0x11,         /*bWINUSBUSTOM_HID: WINUSB Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of WINUSB class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_WINUSB_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of WinUSB endpoints ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  WINUSB_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  WINUSB_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  WINUSB_HS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  WINUSB_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  WINUSB_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  WINUSB_HS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 41 */
};

/* USB WINUSB device Other Speed Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WINUSB_OtherSpeedCfgDesc[USB_WINUSB_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_WINUSB_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x00,         /*iConfiguration: Index of string descriptor describing
  the configuration*/
  0xC0,         /*bmAttributes: bus powered */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

  /************** Descriptor of Vendor-Specific  interface ****************/
  /* 09 */
  0x09,         /*bLength: Interface Descriptor size*/
  USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
  0x00,         /*bInterfaceNumber: Number of Interface*/
  0x00,         /*bAlternateSetting: Alternate setting*/
  0x02,         /*bNumEndpoints*/
  0xFF,         /*bInterfaceClass: Vendor Specific */
  0x00,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
  0x00,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of WINUSB *************************/
  /* 18 */
  0x09,         /*bLength: WINUSB Descriptor size*/
  WINUSB_DESCRIPTOR_TYPE, /*bDescriptorType: WINUSB*/
  0x11,         /*bWINUSBUSTOM_HID: WINUSB Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of WINUSB class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_WINUSB_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
  /******************** Descriptor of WinUSB endpoints ********************/
  /* 27 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  WINUSB_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  WINUSB_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  WINUSB_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 34 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  WINUSB_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  WINUSB_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  WINUSB_FS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 41 */
};

/* USB WINUSB device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WINUSB_Desc[USB_WINUSB_DESC_SIZ] __ALIGN_END =
{
  /* 18 */
  0x09,         /*bLength: WINUSB Descriptor size*/
  WINUSB_DESCRIPTOR_TYPE, /*bDescriptorType: WINUSB*/
  0x11,         /*bWINUSBUSTOM_HID: WINUSB Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of WINUSB class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  USBD_WINUSB_REPORT_DESC_SIZE,/*wItemLength: Total length of Report descriptor*/
  0x00,
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WINUSB_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_WINUSB_Private_Functions
  * @{
  */

/**
  * @brief  USBD_WINUSB_Init
  *         Initialize the WINUSB interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_WINUSB_Init(USBD_HandleTypeDef *pdev,
                                     uint8_t cfgidx)
{
  uint8_t ret = 0U;
  USBD_WINUSB_HandleTypeDef     *hhid;

  /* Open EP IN */
  USBD_LL_OpenEP(pdev, WINUSB_EPIN_ADDR, USBD_EP_TYPE_INTR,
                 WINUSB_EPIN_SIZE);

  pdev->ep_in[WINUSB_EPIN_ADDR & 0xFU].is_used = 1U;

  /* Open EP OUT */
  USBD_LL_OpenEP(pdev, WINUSB_EPOUT_ADDR, USBD_EP_TYPE_INTR,
                 WINUSB_EPOUT_SIZE);

  pdev->ep_out[WINUSB_EPOUT_ADDR & 0xFU].is_used = 1U;

  pdev->pClassData = USBD_malloc(sizeof(USBD_WINUSB_HandleTypeDef));

  if (pdev->pClassData == NULL)
  {
    ret = 1U;
  }
  else
  {
    hhid = (USBD_WINUSB_HandleTypeDef *) pdev->pClassData;

    hhid->state = WINUSB_IDLE;
    ((USBD_WINUSB_ItfTypeDef *)pdev->pUserData)->Init();

    /* Prepare Out endpoint to receive 1st packet */
    USBD_LL_PrepareReceive(pdev, WINUSB_EPOUT_ADDR, hhid->Report_buf,
                           USBD_WINUSB_OUTREPORT_BUF_SIZE);
  }

  return ret;
}

/**
  * @brief  USBD_WINUSB_Init
  *         DeInitialize the WINUSB layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_WINUSB_DeInit(USBD_HandleTypeDef *pdev,
                                       uint8_t cfgidx)
{
  /* Close WINUSB EP IN */
  USBD_LL_CloseEP(pdev, WINUSB_EPIN_ADDR);
  pdev->ep_in[WINUSB_EPIN_ADDR & 0xFU].is_used = 0U;

  /* Close WINUSB EP OUT */
  USBD_LL_CloseEP(pdev, WINUSB_EPOUT_ADDR);
  pdev->ep_out[WINUSB_EPOUT_ADDR & 0xFU].is_used = 0U;

  /* FRee allocated memory */
  if (pdev->pClassData != NULL)
  {
    ((USBD_WINUSB_ItfTypeDef *)pdev->pUserData)->DeInit();
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  return USBD_OK;
}

/**
  * @brief  USBD_WINUSB_Setup
  *         Handle the WINUSB specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_WINUSB_Setup(USBD_HandleTypeDef *pdev,
                                      USBD_SetupReqTypedef *req)
{
  USBD_WINUSB_HandleTypeDef *hhid = (USBD_WINUSB_HandleTypeDef *)pdev->pClassData;
  uint16_t len = 0U;
  uint8_t  *pbuf = NULL;
  uint16_t status_info = 0U;
  uint8_t ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_VENDOR:
      /* Handle WebUSB control requests at device level */
      if (req->bRequest == WEBUSB_VENDOR_CODE)
      {
        if (req->wIndex == WEBUSB_REQ_GET_URL)
        {
          uint16_t send_len = MIN((uint16_t)sizeof(WEBUSB_URL_DESCRIPTOR), req->wLength);
          USBD_CtlSendData(pdev, (uint8_t*)WEBUSB_URL_DESCRIPTOR, send_len);
          return USBD_OK;
        }
      }
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case WINUSB_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case WINUSB_REQ_GET_PROTOCOL:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->Protocol, 1U);
          break;

        case WINUSB_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case WINUSB_REQ_GET_IDLE:
          USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->IdleState, 1U);
          break;

        case WINUSB_REQ_SET_REPORT:
          hhid->IsReportAvailable = 1U;
          USBD_CtlPrepareRx(pdev, hhid->Report_buf, req->wLength);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          if (req->wValue >> 8 == WINUSB_REPORT_DESC)
          {
            len = MIN(USBD_WINUSB_REPORT_DESC_SIZE, req->wLength);
            pbuf = ((USBD_WINUSB_ItfTypeDef *)pdev->pUserData)->pReport;
          }
          else
          {
            if (req->wValue >> 8 == WINUSB_DESCRIPTOR_TYPE)
            {
              pbuf = USBD_WINUSB_Desc;
              len = MIN(USB_WINUSB_DESC_SIZ, req->wLength);
            }
          }

          USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&hhid->AltSetting, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hhid->AltSetting = (uint8_t)(req->wValue);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
  return ret;
}

/**
  * @brief  USBD_WINUSB_SendReport
  *         Send WINUSB Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_WINUSB_SendReport(USBD_HandleTypeDef  *pdev,
                                   uint8_t *report,
                                   uint16_t len)
{
  USBD_WINUSB_HandleTypeDef     *hhid = (USBD_WINUSB_HandleTypeDef *)pdev->pClassData;

  if (pdev->dev_state == USBD_STATE_CONFIGURED)
  {
    if (hhid->state == WINUSB_IDLE)
    {
      hhid->state = WINUSB_BUSY;
      USBD_LL_Transmit(pdev, WINUSB_EPIN_ADDR, report, len);
    }
    else
    {
      return USBD_BUSY;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_WINUSB_GetFSCfgDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_WINUSB_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_WINUSB_CfgFSDesc);
  return USBD_WINUSB_CfgFSDesc;
}

/**
  * @brief  USBD_WINUSB_GetHSCfgDesc
  *         return HS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_WINUSB_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_WINUSB_CfgHSDesc);
  return USBD_WINUSB_CfgHSDesc;
}

/**
  * @brief  USBD_WINUSB_GetOtherSpeedCfgDesc
  *         return other speed configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_WINUSB_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_WINUSB_OtherSpeedCfgDesc);
  return USBD_WINUSB_OtherSpeedCfgDesc;
}

/**
  * @brief  USBD_WINUSB_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_WINUSB_DataIn(USBD_HandleTypeDef *pdev,
                                       uint8_t epnum)
{
  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  ((USBD_WINUSB_HandleTypeDef *)pdev->pClassData)->state = WINUSB_IDLE;

  return USBD_OK;
}

/**
  * @brief  USBD_WINUSB_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_WINUSB_DataOut(USBD_HandleTypeDef *pdev,
                                        uint8_t epnum)
{

  USBD_WINUSB_HandleTypeDef     *hhid = (USBD_WINUSB_HandleTypeDef *)pdev->pClassData;

  ((USBD_WINUSB_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf[0],
                                                            hhid->Report_buf[1]);

  USBD_LL_PrepareReceive(pdev, WINUSB_EPOUT_ADDR, hhid->Report_buf,
                         USBD_WINUSB_OUTREPORT_BUF_SIZE);

  return USBD_OK;
}

/**
  * @brief  USBD_WINUSB_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_WINUSB_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_WINUSB_HandleTypeDef     *hhid = (USBD_WINUSB_HandleTypeDef *)pdev->pClassData;

  if (hhid->IsReportAvailable == 1U)
  {
    ((USBD_WINUSB_ItfTypeDef *)pdev->pUserData)->OutEvent(hhid->Report_buf[0],
                                                              hhid->Report_buf[1]);
    hhid->IsReportAvailable = 0U;
  }

  return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
static uint8_t  *USBD_WINUSB_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_WINUSB_DeviceQualifierDesc);
  return USBD_WINUSB_DeviceQualifierDesc;
}

/**
* @brief  USBD_WINUSB_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: WINUSB Interface callback
  * @retval status
  */
uint8_t  USBD_WINUSB_RegisterInterface(USBD_HandleTypeDef   *pdev,
                                           USBD_WINUSB_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;

  if (fops != NULL)
  {
    pdev->pUserData = fops;
    ret = USBD_OK;
  }

  return ret;
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
