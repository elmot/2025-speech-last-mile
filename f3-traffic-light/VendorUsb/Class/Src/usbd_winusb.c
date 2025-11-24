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
#include "usbd_desc.h"

/* --- Microsoft OS 2.0 descriptor support (WINUSB auto-driver) --- */
#define MS_OS_20_VENDOR_CODE       0x20u    /* Must match BOS capability bVendorCode */
#define MS_OS_20_DESCRIPTOR_INDEX  0x07u    /* wIndex for MS OS 2.0 descriptor set */

/* --- WebUSB support --- */

/* WebUSB URL descriptor for index 1: https://localhost:8080/usb-traffic-light.html */
/* Format: bLength, bDescriptorType(=3), bScheme(1=https), URL bytes (no scheme) */
static const uint8_t WEBUSB_URL_DESC_IDX1[] = {
  /* bLength = 3 + strlen("localhost:8080/usb-traffic-light.html") = 3 + 9 = 12 (0x0C) */
  0x0C, 0x03, 0x01,
  /* "localhost" (9 bytes) */
  'e','l','m','o','t','.','x','y','z'
};

/* Full Microsoft OS 2.0 Descriptor Set */
/* Total length = 0x00A2 (162) */
static const uint8_t MS_OS_20_DESCRIPTOR_SET[0x00A2] =
{
  /* Microsoft OS 2.0 descriptor set header (Table 10) */
  0x0A, 0x00,                          /* wLength */
  0x00, 0x00,                          /* wDescriptorType = MS_OS_20_SET_HEADER (0x00) */
  0x00, 0x00, 0x03, 0x06,              /* dwWindowsVersion = 0x06030000 (WINBLUE) */
  0xA2, 0x00,                          /* wTotalLength = 162 */

  /* Compatible ID feature descriptor (Table 13) */
  0x14, 0x00,                          /* wLength */
  0x03, 0x00,                          /* wDescriptorType = FEATURE_COMPATIBLE_ID */
  'W','I','N','U','S','B', 0x00, 0x00, /* compatibleID (WINUSB) */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* subCompatibleID */

  /* Registry property (DeviceInterfaceGUIDs, REG_MULTI_SZ) (Table 14) */
  0x84, 0x00,                          /* wLength */
  0x04, 0x00,                          /* wDescriptorType = FEATURE_REG_PROPERTY */
  0x07, 0x00,                          /* wPropertyDataType = REG_MULTI_SZ */
  0x2A, 0x00,                          /* wPropertyNameLength */
  /* L"DeviceInterfaceGUIDs" */
  'D',0,'e',0,'v',0,'i',0,'c',0,'e',0,'I',0,'n',0,'t',0,'e',0,'r',0,
  'f',0,'a',0,'c',0,'e',0,'G',0,'U',0,'I',0,'D',0,'s',0,0,0,
  0x50, 0x00,                          /* wPropertyDataLength */
  /* L"1f0c50e7-da29-4179-8a69-fb66b337402b\0\0" GUID */
  '{',0,'1',0,'f',0,'0',0,'c',0,'5',0,'0',0,'e',0,'7',0,'-',0,
  'd',0,'a',0,'2',0,'9',0,'-',0,'4',0,'1',0,'7',0,'9',0,'-',0,
  '8',0,'a',0,'6',0,'9',0,'-',0,'f',0,'b',0,'6',0,'6',0,'b',0,
  '3',0,'3',0,'7',0,'4',0,'0',0,'2',0,'b',0,'}',0,0,0,0
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
  0xFF,         /*bInterfaceSubClass */
  0x00,         /*nInterfaceProtocol */
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of WinUSB endpoints ********************/
  /* 18 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  WINUSB_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  WINUSB_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  WINUSB_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 25 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  WINUSB_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  WINUSB_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  WINUSB_FS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 32 */
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
  0xFF,         /*bInterfaceSubClass */
  0x00,         /*nInterfaceProtocol */
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of WinUSB endpoints ********************/
/* 18 */
    0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  WINUSB_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  WINUSB_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  WINUSB_HS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 25 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  WINUSB_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  WINUSB_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  WINUSB_HS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 32 */
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
  0xFF,         /*bInterfaceSubClass */
  0x00,         /*nInterfaceProtocol */
  0,            /*iInterface: Index of string descriptor*/
  /******************** Descriptor of WinUSB endpoints ********************/
  /* 18 */
  0x07,          /*bLength: Endpoint Descriptor size*/
  USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/

  WINUSB_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
  0x03,          /*bmAttributes: Interrupt endpoint*/
  WINUSB_EPIN_SIZE, /*wMaxPacketSize: 2 Byte max */
  0x00,
  WINUSB_FS_BINTERVAL,          /*bInterval: Polling Interval */
  /* 25 */

  0x07,          /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: */
  WINUSB_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
  0x03, /* bmAttributes: Interrupt endpoint */
  WINUSB_EPOUT_SIZE,  /* wMaxPacketSize: 2 Bytes max  */
  0x00,
  WINUSB_FS_BINTERVAL,  /* bInterval: Polling Interval */
  /* 32 */
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
      /* Serve WebUSB GET_URL requests */
      if (req->bRequest == WEBUSB_VENDOR_CODE &&
          (req->bmRequest == 0xC0)) /* Device-to-host, vendor, device */
      {
        /* Per WebUSB spec: wIndex == 0x0002 indicates GET_URL; wValue = URL index */
        uint8_t url_index = (uint8_t)(req->wValue & 0xFF);
        if (req->wIndex == WEBUSB_REQ_GET_URL_INDEX && url_index == 0x01)
        {
          uint16_t send_len = MIN((uint16_t)sizeof(WEBUSB_URL_DESC_IDX1), req->wLength);
          USBD_CtlSendData(pdev, (uint8_t*)WEBUSB_URL_DESC_IDX1, send_len);
          return USBD_OK;
        }
        /* Some hosts mistakenly pass index in wIndex; accept that too for robustness */
        if (req->wIndex == 0x0001)
        {
          uint16_t send_len = MIN((uint16_t)sizeof(WEBUSB_URL_DESC_IDX1), req->wLength);
          USBD_CtlSendData(pdev, (uint8_t*)WEBUSB_URL_DESC_IDX1, send_len);
          return USBD_OK;
        }
      }
      /* Serve Microsoft OS 2.0 Descriptor Set request */
      if (req->bRequest == MS_OS_20_VENDOR_CODE &&
          req->wIndex == MS_OS_20_DESCRIPTOR_INDEX &&
          (req->bmRequest == 0xC0)) /* Device-to-host, vendor, device */
      {
        uint16_t send_len = MIN((uint16_t)sizeof(MS_OS_20_DESCRIPTOR_SET), req->wLength);
        USBD_CtlSendData(pdev, (uint8_t*)MS_OS_20_DESCRIPTOR_SET, send_len);
        return USBD_OK;
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
