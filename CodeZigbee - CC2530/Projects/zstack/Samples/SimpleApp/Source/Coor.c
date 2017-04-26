
/******************************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "OSAL.h"
#include "sapi.h"
#include "hal_key.h"
#include "hal_led.h"
#include "DebugTrace.h"
#include "SimpleApp.h"
#include "hal_uart.h"

/*********************************************************************
 * CONSTANTS
 */

// Application States
#define APP_INIT                           0
#define APP_START                          1

// Application osal event identifiers
#define MY_START_EVT                0x0001

// Same definitions as in SimpleSensor.c
#define TEMP_REPORT     0x01
#define BATTERY_REPORT 0x02
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 myAppState = APP_INIT;
static uint8 myStartRetryDelay = 10;
static void Uart0_Cb(uint8 port, uint8 event);
/*********************************************************************
 * GLOBAL VARIABLES
 */

// Inputs and Outputs for Collector device
#define NUM_OUT_CMD_COLLECTOR                0
#define NUM_IN_CMD_COLLECTOR                 1

// List of output and input commands for Collector device
const cId_t zb_InCmdList[NUM_IN_CMD_COLLECTOR] =
{
  SENSOR_REPORT_CMD_ID
};

// Define SimpleDescriptor for Collector device
const SimpleDescriptionFormat_t zb_SimpleDesc =
{
  MY_ENDPOINT_ID,             //  Endpoint
  MY_PROFILE_ID,              //  Profile ID
  DEV_ID_COLLECTOR,          //  Device ID
  DEVICE_VERSION_COLLECTOR,  //  Device Version
  0,                          //  Reserved
  NUM_IN_CMD_COLLECTOR,      //  Number of Input Commands
  (cId_t *) zb_InCmdList,     //  Input Command List
  NUM_OUT_CMD_COLLECTOR,     //  Number of Output Commands
  (cId_t *) NULL              //  Output Command List
};

/******************************************************************************
 * @fn          zb_HandleOsalEvent
 *
 * @brief       The zb_HandleOsalEvent function is called by the operating
 *              system when a task event is set
 *
 * @param       event - Bitmask containing the events that have been set
 *
 * @return      none
 */
void zb_HandleOsalEvent( uint16 event )
{
  if( event & ZB_ENTRY_EVENT ){
    halUARTCfg_t uConfig;
    uConfig.configured = TRUE; 
    uConfig.baudRate = HAL_UART_BR_9600;
    uConfig.flowControl = FALSE;
    uConfig.flowControlThreshold = 48;
    uConfig.idleTimeout = 6; 
    uConfig.rx.maxBufSize = 128;
    uConfig.tx.maxBufSize = 128;      
    uConfig.intEnable = TRUE;//enable interrupts
    uConfig.callBackFunc = &Uart0_Cb;
    //uConfig.callBackFunc = 0;    
    HalUARTOpen(HAL_UART_PORT_0,&uConfig);
    HalUARTWrite(HAL_UART_PORT_0,"\nZB_ENTRY_EVENT\n", (byte)osal_strlen("\nZB_ENTRY_EVENT\n"));      
  }
}
/*********************************************************************
 * @fn      zb_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 EVAL_SW4
 *                 EVAL_SW3
 *                 EVAL_SW2
 *                 EVAL_SW1
 *
 * @return  none
 */
void zb_HandleKeys( uint8 shift, uint8 keys )
{
  uint8 startOptions;
  uint8 logicalType;

  // Shift is used to make each button/switch dual purpose.
  if ( shift )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
      if ( myAppState == APP_INIT  )
      {
        // In the init state, keys are used to indicate the logical mode.
        // Key 1 starts device as a coordinator

        zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType );
        if ( logicalType != ZG_DEVICETYPE_ENDDEVICE )
        {
          logicalType = ZG_DEVICETYPE_COORDINATOR;
          zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
        }

        // Do more configuration if necessary and then restart device with auto-start bit set
        // write endpoint to simple desc...dont pass it in start req..then reset


        zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        startOptions = ZCD_STARTOPT_AUTO_START;
        zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        zb_SystemReset();

      }
      else
      {
        // Turn ON Allow Bind mode indefinitely
        zb_AllowBind( 0xFF );
        HalLedSet( HAL_LED_1, HAL_LED_MODE_ON );
      }
    }
    if ( keys & HAL_KEY_SW_2 )
    {
      if ( myAppState == APP_INIT )
      {
        // In the init state, keys are used to indicate the logical mode.
        // Key 2 starts device as a router

        zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType );
        if ( logicalType != ZG_DEVICETYPE_ENDDEVICE )
        {
          logicalType = ZG_DEVICETYPE_ROUTER;
          zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
        }

        zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        startOptions = ZCD_STARTOPT_AUTO_START;
        zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        zb_SystemReset();
      }
      else
      {
        // Turn OFF Allow Bind mode indefinitely
        zb_AllowBind( 0x00 );
        HalLedSet( HAL_LED_1, HAL_LED_MODE_OFF );
      }
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
}
/******************************************************************************
 * @fn          zb_StartConfirm
 *
 * @brief       The zb_StartConfirm callback is called by the ZigBee stack
 *              after a start request operation completes
 *
 * @param       status - The status of the start operation.  Status of
 *                       ZB_SUCCESS indicates the start operation completed
 *                       successfully.  Else the status is an error code.
 *
 * @return      none
 */
void zb_StartConfirm( uint8 status )
{

 // If the device sucessfully started, change state to running
  if ( status == ZB_SUCCESS )//Bay h da xac dinh dc logicMode
  {
    myAppState = APP_START;
  }
  else
  {
    // Try again later with a delay
    osal_start_timerEx( sapi_TaskID, MY_START_EVT, myStartRetryDelay );
  }
}
/******************************************************************************
 * @fn          zb_SendDataConfirm
 *
 * @brief       The zb_SendDataConfirm callback function is called by the
 *              ZigBee after a send data operation completes
 *
 * @param       handle - The handle identifying the data transmission.
 *              status - The status of the operation.
 *
 * @return      none
 */
void zb_SendDataConfirm( uint8 handle, uint8 status )
{
}
/******************************************************************************
 * @fn          zb_BindConfirm
 *
 * @brief       The zb_BindConfirm callback is called by the ZigBee stack
 *              after a bind operation completes.
 *
 * @param       commandId - The command ID of the binding being confirmed.
 *              status - The status of the bind operation.
 *
 * @return      none
 */
void zb_BindConfirm( uint16 commandId, uint8 status )
{
}
/******************************************************************************
 * @fn          zb_AllowBindConfirm
 *
 * @brief       Indicates when another device attempted to bind to this device
 *
 * @param
 *
 * @return      none
 */
void zb_AllowBindConfirm( uint16 source )
{
}
/******************************************************************************
 * @fn          zb_FindDeviceConfirm
 *
 * @brief       The zb_FindDeviceConfirm callback function is called by the
 *              ZigBee stack when a find device operation completes.
 *
 * @param       searchType - The type of search that was performed.
 *              searchKey - Value that the search was executed on.
 *              result - The result of the search.
 *
 * @return      none
 */
void zb_FindDeviceConfirm( uint8 searchType, uint8 *searchKey, uint8 *result )
{
}
/******************************************************************************
 * @fn          zb_ReceiveDataIndication
 *
 * @brief       The zb_ReceiveDataIndication callback function is called
 *              asynchronously by the ZigBee stack to notify the application
 *              when data is received from a peer device.
 *
 * @param       source - The short address of the peer device that sent the data
 *              command - The commandId associated with the data
 *              len - The number of bytes in the pData parameter
 *              pData - The data sent by the peer device
 *
 * @return      none
 */
CONST uint8 strDevice[] = "Device:0x";
CONST uint8 strTemp[] = "Temp: ";
CONST uint8 strBattery[] = "Battery: ";
void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData,int8 r_power  )
{
  uint8 buf[32];
  uint8 *pBuf;
  uint8 tmpLen;
  uint8 sensorReading;

  if (command == SENSOR_REPORT_CMD_ID)
  {
    HalUARTWrite(HAL_UART_PORT_0,"RECEI_REPORT:", (byte)osal_strlen("RECEI_REPORT")); 
    // Received report from a sensor
    sensorReading = pData[1];

    // If tool available, write to serial port

    tmpLen = (uint8)osal_strlen( (char*)strDevice );
    pBuf = osal_memcpy( buf, strDevice, tmpLen );
    _ltoa( source, pBuf, 16 );
    pBuf += 4;
    *pBuf++ = ' ';

    if ( pData[0] == BATTERY_REPORT )
    {
      tmpLen = (uint8)osal_strlen( (char*)strBattery );
      pBuf = osal_memcpy( pBuf, strBattery, tmpLen );

      *pBuf++ = (sensorReading / 10 ) + '0';    // convent msb to ascii
      *pBuf++ = '.';                            // decimal point ( battery reading is in units of 0.1 V
      *pBuf++ = (sensorReading % 10 ) + '0';    // convert lsb to ascii
      *pBuf++ = ' ';
      *pBuf++ = 'V';
    }
    else
    {
      tmpLen = (uint8)osal_strlen( (char*)strTemp );
      pBuf = osal_memcpy( pBuf, strTemp, tmpLen );

      *pBuf++ = (sensorReading / 10 ) + '0';    // convent msb to ascii
      *pBuf++ = (sensorReading % 10 ) + '0';    // convert lsb to ascii
      *pBuf++ = ' ';
      *pBuf++ = 'C';
    }

    *pBuf++ = '\r';
    *pBuf++ = '\n';
    *pBuf = '\0';

#if defined( MT_TASK )
    debug_str( (uint8 *)buf );
#endif

    // can also write directly to uart
    HalUARTWrite(HAL_UART_PORT_0,buf, (byte)osal_strlen((char *)buf)); 

  }
}

static void Uart0_Cb(uint8 port, uint8 event){
  uint8 startOptions;
  uint8 logicalType;
  uint8  ch;
  while (Hal_UART_RxBufLen(port))
  {
    HalUARTRead ( port, &ch, 1);
    if( ch == '1' ){
      if ( myAppState == APP_INIT )
      {
        HalUARTWrite(HAL_UART_PORT_0,"\nStartCoord\n", (byte)osal_strlen("\nStartCoord\n")); 

        // In the init state, keys are used to indicate the logical mode.
        // Key 1 starts device as a coordinator

        zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType );
        if ( logicalType != ZG_DEVICETYPE_ENDDEVICE )
        {
          logicalType = ZG_DEVICETYPE_COORDINATOR;
          zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
        }

        // Do more configuration if necessary and then restart device with auto-start bit set
        // write endpoint to simple desc...dont pass it in start req..then reset


        zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        startOptions = ZCD_STARTOPT_AUTO_START;
        zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        zb_SystemReset();
      }
    }else if( ch == '2' ){
      if ( myAppState == APP_INIT )
      {
        HalUARTWrite(HAL_UART_PORT_0,"\nStartRout\n", (byte)osal_strlen("\nStartRout\n")); 
        
        // In the init state, keys are used to indicate the logical mode.
        // Key 2 starts device as a router

        zb_ReadConfiguration( ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType );
        if ( logicalType != ZG_DEVICETYPE_ENDDEVICE )
        {
          logicalType = ZG_DEVICETYPE_ROUTER;
          zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
        }

        zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        startOptions = ZCD_STARTOPT_AUTO_START;
        zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
        zb_SystemReset();
      }
    }else if( ch == '3' ){
      
    }else if( ch == '4' ){
      
      HalUARTWrite(HAL_UART_PORT_0,"\nClearSTARTUP_OPTION\n", (byte)osal_strlen("\nClearSTARTUP_OPTION\n")); 
      
      // If SW5 is pressed and held while powerup, force auto-start and nv-restore off and reset
      uint8 startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
      zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
      zb_SystemReset();
      
    }else if( ch == '5' ){
      HalUARTWrite(HAL_UART_PORT_0,"AllowBind\n", (byte)osal_strlen("AllowBind\n")); 
      zb_AllowBind( 0xFF );
    }else if( ch == '6' ){
      HalUARTWrite(HAL_UART_PORT_0,"NoAllowBind\n", (byte)osal_strlen("NoAllowBind\n")); 
      zb_AllowBind( 0 );
    }
  }
}

