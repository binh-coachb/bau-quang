
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
#define NUM_OUT_CMD_COLLECTOR                1
#define NUM_IN_CMD_COLLECTOR                 1

// List of output and input commands for Collector device
const cId_t zb_InCmdList[NUM_IN_CMD_COLLECTOR] =
{
  SENSOR_REPORT_CMD_ID
};

const cId_t zb_OutCmdList[NUM_OUT_CMD_COLLECTOR] =
{
  CTRL_PUMP_CMD_ID
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
  (cId_t *) zb_OutCmdList     //  Output Command List
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
  uint8 startOptions;
  uint8 logicalType;
  
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
    HalUARTOpen(HAL_UART_PORT_0,&uConfig);
    HalUARTWrite(HAL_UART_PORT_0,"\nENTRY\n", (byte)osal_strlen("\nENTRY\n"));      
    
    startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;    
    zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
    
    logicalType = ZG_DEVICETYPE_COORDINATOR;
    zb_WriteConfiguration(ZCD_NV_LOGICAL_TYPE, sizeof(uint8), &logicalType);
    
    zb_ReadConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
    startOptions = ZCD_STARTOPT_AUTO_START;
    zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
    
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
  return;
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
    zb_AllowBind( 0xFF );
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
  //HalUARTWrite(HAL_UART_PORT_0,"SendDataConfirm ", (byte)osal_strlen("SendDataConfirm ")); 
  /*if ( status != ZSuccess )
  { 
    HalUARTWrite(HAL_UART_PORT_0,"Fail\n", (byte)osal_strlen("Fail\n"));
  }
  else {
    HalUARTWrite(HAL_UART_PORT_0,"Success\n", (byte)osal_strlen("Success\n"));
  }*/
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
  //HalUARTWrite(HAL_UART_PORT_0,"BindConfirm\n", (byte)osal_strlen("BindConfirm\n")); 
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
  HalUARTWrite(HAL_UART_PORT_0,"Allow\n", (byte)osal_strlen("Allow\n")); 
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
  //HalUARTWrite(HAL_UART_PORT_0,"FindDeviceConfirm\n", (byte)osal_strlen("FindDeviceConfirm\n")); 
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

void zb_ReceiveDataIndication( uint16 source, uint16 command, uint16 len, uint8 *pData,int8 r_power  )
{
  static uint8 sendData[3];
  if (command == SENSOR_REPORT_CMD_ID && len >=2 )
  {     
    sendData[0] = '#';
    sendData[1] = *(pData+1);
    sendData[2] = r_power;
    HalUARTWrite(HAL_UART_PORT_0,sendData, (byte)3);  
    
  }
}

static void Uart0_Cb(uint8 port, uint8 event){
  uint8  ch;
  uint8 startOptions;
  if ((event&HAL_UART_RX_TIMEOUT) || (event&HAL_UART_RX_ABOUT_FULL)){
    while (Hal_UART_RxBufLen(port))
    {
      HalUARTRead ( port, &ch, 1);      
      //0xFFFF Gui toi tat ca thiet bi khac
      //0xFFFD Gui toi thiet bi dang turned ON
      //0xFFFD Gui toi thiet bi la Coor va Rout
      if( ch == '?' ){
        HalUARTWrite(HAL_UART_PORT_0,"\nCoordinator\n", (byte)osal_strlen("\nCoordinator\n"));  
      }
      else if( ch == 'r' )
      {
          startOptions = ZCD_STARTOPT_CLEAR_STATE | ZCD_STARTOPT_CLEAR_CONFIG;
          zb_WriteConfiguration( ZCD_NV_STARTUP_OPTION, sizeof(uint8), &startOptions );
          zb_SystemReset();
      }
      else if( ch == 's' )
      {        
        static uint8 pData = 1;        
        zb_SendDataRequest( 0xFFFF, CTRL_PUMP_CMD_ID, 1, &pData, 0, FALSE, 0 );
        pData = ~pData;
        HalUARTWrite(HAL_UART_PORT_0,"\nSent\n", (byte)osal_strlen("\nSent\n"));  
      }
      
    }
  }
}

