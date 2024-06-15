/*************************************************************************
**    IXXAT Automation GmbH
**************************************************************************
**
**  $Workfile: BCI.H $
**    Summary: headerfile of the BCI (basic CAN interface)
**   $Archive: /database/produkte/treiber/BCI V4/common/src/BCI.H $
**  $Revision: 1.27 $
**      $Date: 2003/07/02 10:39:30 $
**     Author: A.Rothenhaeusler, J.Stolberg, M.Marb
**
**************************************************************************
**************************************************************************
**
**  Functions: BCI_Init
**             BCI_OpenBoard
**             BCI_CloseBoard
**             BCI_ResetCan
**             BCI_StartCan
**             BCI_InitCan
**             BCI_StopCan
**             BCI_SetAccMask
**             BCI_RegisterRxId
**             BCI_UnregisterRxId
**             BCI_ConfigRxQueue
**             BCI_TransmitCanMsg
**             BCI_ReceiveCanMsg
**             BCI_GetCanStatus
**             BCI_GetBoardStatus
**             BCI_GetBoardInfo
**             BCI_GetErrorString
**
**   Compiler: GNU-C x.y for Linux
**    Remarks:
**
**   $History: BCI.H $
 * 
**
**************************************************************************
**    all rights reserved
*************************************************************************/




#ifndef BCI_H
#define BCI_H


/*************************************************************************
**    constants and macros
*************************************************************************/

/*
** data types
*/
#ifndef UINT8
#define UINT8  unsigned char
#endif

#ifndef UINT16
#define UINT16 unsigned short
#endif

#ifndef UINT32
#define UINT32 unsigned int
#endif

#ifndef BYTE 
#define BYTE   unsigned char
#endif

#ifndef WORD
#define WORD   unsigned short
#endif

#ifndef DWORD
#define DWORD  unsigned int
#endif




/*
** Length of the error strings
*/
#define ERROR_STRING_LEN 64

/*
** maximum number of supported boards
*/
#define BCI_MAX_BOARDS      4

/*
** BCI revision
*/
#define BCI_MAJOR_REV 4
#define BCI_MINOR_REV 4
#define BCI_SUB_REV   4

/*
**  some baudrates
*/
#define BCI_10KB            0x67,0x2f
#define BCI_20KB            0x53,0x2f
#define BCI_50KB            0x47,0x2F
#define BCI_100KB           0x43,0x2F
#define BCI_125KB           0x03,0x1C
#define BCI_250KB           0x01,0x1C
#define BCI_500KB           0x00,0x1C
#define BCI_1000KB          0x00,0x14

/*
**  parameter definitions
*/

/* message frame format */
#define BCI_MFF_11_DAT           0          /* standard mode, data frame */
#define BCI_MFF_11_RMT           1          /* standard mode, remote frame */
#define BCI_MFF_29_DAT           2          /* extended mode, data frame */
#define BCI_MFF_29_RMT           3          /* extended mode, remote frame */
#define BCI_MFF_STS_MSG          8          /* CAN status message */
#define BCI_MFF_RMT_MASK         1          /* remote frame mff mask */
#define BCI_MFF_EXT_MASK         2          /* extended mode mff mask */

/* timeout conditions */
#define BCI_NO_WAIT         (0)
#define BCI_WAIT_FOREVER    (-1)

/* acceptance mask type */
#define BCI_11B_MASK            0
#define BCI_29B_MASK            1


/*
** return values:
*/
#define BCI_OK                (1)           /* successfully finished */
#define BCI_BUSY              (0)           /* no access at the moment */
#define BCI_NO                (0)           /* no message available/timeout */
#define BCI_HDL_ERR          (-1)           /* wrong or unknown board handle */
#define BCI_INST_ERR         (-2)           /* install error */
#define BCI_FW_ERR           (-3)           /* download firmware error */
#define BCI_ISR_ERR          (-4)           /* install ISR error */
#define BCI_PARA_ERR         (-5)           /* parameter error */
#define BCI_STATE_ERR        (-6)           /* BCI is in wrong state */
#define BCI_CMND_ERR         (-7)           /* no command request possible */
#define BCI_RESP_ERR         (-8)           /* no board response */
#define BCI_SERV_ERR         (-9)           /* error in service */
#define BCI_USER_ERR         (-10)          /* wrong handling by user */
#define BCI_TIMER_ERR        (-11)          /* wrong timer state */

#ifdef LINUX
#define BCI_TEST_ERR          (-51)         /* BCI firmware test failed */
#define BCI_LOADER_TEST_ERR   (-52)         /* loader test failed */
#define BCI_INT_TEST_ERR      (-53)         /* interrupt to PC test failed */
#define BCI_SEMA_TEST_ERR     (-54)         /* semaphore test failed */
#define BCI_LIST_ERR          (-55)         /* interface list error */
#define BCI_CREATE_ERR        (-56)         /* interface create error */
#endif /* ifdef LINUX */

/*
**  additional usb return values
*/
#define BCI_BM_ERR               (-96)      /* Error in Boormanager communication */
#define BCI_NUMDEVICES_ERR       (-97)      /* Too many devices with bootmanager v1 */
#define BCI_TIMEOUT_ERR          (-98)      /* Timeout occured */
#define BCI_CLAIM_ERR            (-99)      /* Error Claiming Interface */
#define BCI_FIFO_ERR             (-100)     /* Fifo Error */
#define BCI_NOT_IMPLEMENTED_ERR  (-101)     /* Function not implemented */
#define BCI_SET_AI_ERR           (-102)     /* Error setting alternate interface */


/*
** defines for the CAN Status
*/
#define BCI_CAN_INIT_MODE     0x0001
#define BCI_CAN_WARN_LEVEL    0x0002
#define BCI_CAN_BUS_OFF       0x0004
#define BCI_CAN_DATA_OVR      0x0008
#define BCI_CAN_RX_STAT       0x0010
#define BCI_CAN_TX_PEND       0x0020
#define BCI_CAN_LOWSPEED_ERR  0x0040
#define BCI_CAN_STT_STAT      0x0080
#define BCI_CAN_QUE_OVR       0x0100
#define BCI_CAN_QUE_EMPTY     0x0200



/*
** Defines for the parameter mode in the function BCI_RegFilterListEntry
*/
#define BCI_ACC_ALL         (~0x01)         /* Accept all CAN identifiers */
#define BCI_REJECT_ALL      (~0x02)         /* Reject all CAN identifiers */
#define BCI_MASK            0x04            /* Accept only specific CAN identifier */

/*
** Defines for parameter mode in the function BCI_ConfigRxQueue
*/
#define BCI_POLL_MODE       0x00            /* No interrupt from MC to PC */
#define BCI_LATENCY_MODE    0x01            /* Interrupt after every received
                                               message */
#define BCI_THROUGHPUT_MODE 0x02            /* Interrupt when queue is full */

/*
  BCI states and transitions:

  - UNCONFIGURED:       The board is unconfigured. No board or CAN
                        functionality is available.
                        Board firmware is not running.

  - BOARD-INITIALIZED:  The board is running and ready for CAN
                        initialization.
                        The CAN controller is unconfigured.

  - CAN-INITIALIZED:    CAN controller is configured/initialized
                        but not running. No CAN communication is
                        possible.

  - CAN-RUNNING:        CAN controller is running and able to receive
                        and transmit CAN messages.

                                BCI_Init |
                                         v
                                  +--------------+
                                  | UNCONFIGURED |
                                  +--------------+
                                      |     ^
                        BCI_OpenBoard |     | BCI_CloseBoard
                                      v     |
                                +-------------------+
                                | BOARD-INITIALIZED |
                                +-------------------+
                                      |     ^
                          BCI_InitCan |     | BCI_ResetCan
                                      v     |
                                 +-----------------+
                                 | CAN-INITIALIZED |
                                 +-----------------+
                                      |     ^
                         BCI_StartCan |     | BCI_StopCan
                                      v     |
                                  +-------------+
                                  | CAN-RUNNING |
                                  +-------------+
*/



/*************************************************************************
**    data types
*************************************************************************/

typedef enum
{ BCI_IPCI320_ISA,              /* iPC-I 320 - ISA */
  BCI_IPCI320_PCI,              /* iPC-I 320 - PCI */
  BCI_IPCI165_ISA,              /* iPC-I 165 - ISA */
  BCI_IPCI165_PCI,              /* iPC-I 165 - PCI */
  BCI_USBCOMPACT                /* USB-to-CAN compact */

}
BCI_t_BrdType;

typedef struct
{
  UINT32 id;                    /* message identifier */
  UINT8 dlc;                    /* Data Length Code (0 - 8) */
  UINT8 mff;                    /* message frame format (see defines) */
  UINT8 a_data[8];              /* CAN data */
  UINT32 time_stamp;            /* receive time stamp */
}
BCI_ts_CanMsg;

typedef struct
{
  UINT16 can0_status;           /* CAN 0 status */
  UINT16 can0_busload;          /* CAN 0 busload */
  UINT16 can1_status;           /* CAN 1 status */
  UINT16 can1_busload;          /* CAN 1 busload */
  UINT16 cpu_load;              /* CPU load */
  UINT16 counter;               /* firmeware live sign counter */
}
BCI_ts_BrdSts;

typedef struct
{
  UINT16 fw_version;            /* firmware version (BCD coded, e.g. 100H = V1.00) */
  UINT16 num_can;               /* number of CAN controllers (1,2) */
  char can_type[2][10];         /* CAN type, e. g. "SJA1000", "82527" */
}
BCI_ts_BrdInfo;

typedef char BCI_t_ErrString[40];

typedef UINT32 BCI_t_BoardType;

typedef unsigned long BCI_BRD_HDL;

/*************************************************************************
**    global variables
*************************************************************************/

/*************************************************************************
**    function prototypes
*************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
**
** Function    : BCI_Init
**
** Description : Initializes the BCI structures.
**               function must be called first. After that the
**               BCI is in the state UNCONFIGURED.
**               Call of this function reenumerates the usb bus and
**               gets all device descriptors.
**
** Parameters  : -
** Returnvalue : BCI_OK            - OK
**               BCI_NUMDEVICES_ERR - Too many devices with bootmanager v1 
**
*************************************************************************/
int BCI_Init();


/*************************************************************************
**
** Function    : BCI_OpenBoard
**
** Description : The function:
**               - opens the device
**               - loads and starts the firmware
**               - (if bootmanager v1)executes port reset 
**               - (if bootmanager v1)reopenes device
**               - sets the alternate interface
**               - delivers a handle for further board usage
**               After that the CAN controller can be initialized.
**               BCI state transition:  UNCONFIGURED -> BOARD-INITIALIZED
**
** Parameters  : p_hdl    (OUT) - pointer to the distributed board handle
**               type     (IN)  - unused
**               location (IN)  - number of the device (0-3)
**               irq_num  (IN)  - unused
**
** Returnvalue : BCI_OK         - OK
**               BCI_HDL_ERR    - wrong board handle
**               BCI_STATE_ERR  - wrong BCI state
**               BCI_CLAIM_ERR  - Error claiming interface
**               BCI_BM_ERR     - Error in bootmanager communication
**               BCI_SET_AI_ERR - Error setting alternate interface
**               BCI_FW_ERR     - Error downloading firmware
**
*************************************************************************/
int BCI_OpenBoard(BCI_BRD_HDL * p_hdl,
                   BCI_t_BoardType type, UINT16 location, UINT8 irq);


/*************************************************************************
**
** Function    : BCI_CloseBoard
**
** Description : Resets the device and closes the handle
**               After that, the firmware and CAN controller is stopped.
**               This function can be called from every BCI state.
**               BCI state transition:  any state -> UNCONFIGURED
**
** Parameters  : brd_hdl (IN)  - board handle
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_RESP_ERR  - no board response
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_CloseBoard(BCI_BRD_HDL brd_hdl);


/*************************************************************************
**
** Function    : BCI_InitCan
**
** Description : Initialization of the CAN controller with the specified
**               bit timing parameters.
**               After that, the CAN controller is initialized and
**               further configuration of message filter and receive
**               queue is possible.
**               BCI state transition:  BOARD-INITIALIZED -> CAN-INITIALIZED
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (IN) - number of the CAN controller (0,1)
**               bt0     (IN) - value for the Bus-Timing-0
**               bt1     (IN) - value for the Bus-Timing-1
**               mode    (IN) - unused
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_RESP_ERR  - no board response
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_InitCan (BCI_BRD_HDL Interface, UINT8 CANNum, UINT8 bt0, UINT8 bt1,
                 UINT8 mode);


/*************************************************************************
**
** Function    : BCI_ResetCan
**
** Description : Set CAN controller to init mode by means of a hardware
**               reset (hard termination of a possible transmission).
**               The CAN controller must be initialized again.
**               BCI state transition:
**                 CAN-RUNNING/CAN-INITIALIZED -> BOARD-INITIALIZED
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (IN) - number of the CAN controller (0,1)
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_RESP_ERR  - no board response
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_ResetCan (BCI_BRD_HDL brd_hdl, UINT8 can_num);


/**********************************************************************
**
** Function    : BCI_StartCan
**
** Description : Starts the CAN controller (set to running mode).
**               After that the CAN controller is able to receive and
**               transmit messages.
**               BCI state transition:  CAN-INITIALIZED -> CAN-RUNNING
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (in) - number of the CAN controller (0,1)
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_RESP_ERR  - no board response
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_StartCan (BCI_BRD_HDL hdl, UINT8 can_num);


/*************************************************************************
**
** Function    : BCI_StopCan
**
** Description : Stops the CAN controller softly (a possible transmission
**               is finished first). The function returns after the CAN
**               has reached the init mode.
**               After that, the CAN controller configuration, received
**               messages as well as further configurations
**               (filter, queue) are still valid.
**               BCI state transition:  CAN-RUNNING -> CAN-INITIALIZED
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (IN) - number of the CAN controller (0,1)
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_RESP_ERR  - no board response
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_StopCan (BCI_BRD_HDL brd_hdl, UINT8 can_num);


/*************************************************************************
**
** Function    : BCI_ReceiveCanMsg
**
** Description : Reads a received CAN message from the receive queue.
**               The function blocks if no message is available and
**               the parameter timeout is not equal to BCI_NO_WAIT.
**               The BCI must be in the state CAN-RUNNING.
**
** Parameters  : brd_hdl (IN)  - board handle
**               can_num (IN)  - number of the CAN controller (0,1)
**               can_msg (OUT) - structure, to store the received CAN
**                               message
**               timeout (IN)  - >0: timeout in msec if no message is
**                                   available
**                                0: BCI_NO_WAIT for polling mode
**                               -1: BCI_WAIT_FOREVER
**
** Returnvalue : BCI_OK          - CAN message received
**               BCI_HDL_ERR     - wrong or unknown board handle
**               BCI_NO          - no CAN message received (BCI_NO_WAIT)
**               BCI_PARA_ERR    - invalid pointer to rx msg buffer
**               BCI_STATE_ERR   - BCI is in wrong state
**               BCI_TIMEOUT_ERR - timeout occured
**
*************************************************************************/
int BCI_ReceiveCanMsg (BCI_BRD_HDL brd_hdl,
                       UINT8 can_num, BCI_ts_CanMsg * can_msg, int timeout);


/*************************************************************************
**
** Function    : BCI_TransmitCanMsg
**
** Description : Transmits a CAN message
**               The CAN message is transmitted by means of the structure
**               BCI_ts_CanMsg.
**               The BCI must be in the state CAN-RUNNING.
**
** Parameters  : brd_hdl (IN) - board handle
**               can_num (IN) - number of the CAN controller (0,1)
**               can_msg (IN) - pointer to the CAN message
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong or unknown board handle
**               BCI_BUSY      - queue full, repeat later
**               BCI_PARA_ERR  - Invalid TX Msg
**               BCI_STATE_ERR - BCI is in wrong state
**
*************************************************************************/
int BCI_TransmitCanMsg (BCI_BRD_HDL brd_hdl, UINT8 can_num, BCI_ts_CanMsg * can_msg);


/*************************************************************************
**
** Function    : BCI_SetAccMask
**
** Description : Function not implemented
**
** Parameters  : brd_hdl  (IN) - unused
**               can_num  (IN) - unused
**               type     (IN) - unused
**               acc_code (IN) - unused
**               acc_mask (IN) - unused
**
** Returnvalue : BCI_NOT_IMPLEMENTED_ERR - Function not implemented
**
*************************************************************************/
int BCI_SetAccMask (BCI_BRD_HDL brd_hdl,
                    UINT8 can_num, UINT8 type, UINT32 acc_code, UINT32 acc_mask);


/*************************************************************************
**
** Function    : BCI_RegisterRxId
**
** Description : Function not implemented
**
** Parameters  : brd_hdl (IN) - unused
**               can_num (IN) - unused
**               mff     (IN) - unused
**               id      (IN) - unused
**
** Returnvalue : BCI_NOT_IMPLEMENTED_ERR - Function not implemented
**
*************************************************************************/
int BCI_RegisterRxId (BCI_BRD_HDL brd_hdl, UINT8 can_num, UINT8 mff, UINT32 id);


/*************************************************************************
**
** Function    : BCI_UnregisterRxId
**
** Description : Function not implemented
**
** Parameters  : brd_hdl (IN) - unused
**               can_num (IN) - unused
**               mff     (IN) - unused
**               id      (IN) - unused
**
** Returnvalue : BCI_NOT_IMPLEMENTED_ERR - Function not implemented
**
*************************************************************************/
int BCI_UnregisterRxId (BCI_BRD_HDL brd_hdl, UINT8 can_num, UINT8 mff, UINT32 id);


/*************************************************************************
**
** Function    : BCI_GetCanStatus
**
** Description : This function reads the CAN status either from cyclic buffer
**               messages or from a request to the device
**
** Parameters  : brd_hdl (IN)  - board handle
**               can_num (IN)  - number of the CAN controller (0,1)
**               can_sts (OUT) - pointer to store the status:
**                               - BCI_CAN_INIT_MODE:
**                                 CAN controller must be in "init mode"
**                                 for configuration.
**                               - BCI_CAN_WARN_LEVEL:
**                                 The CAN controller switches to "warning
**                                 level" if communication problems occur.
**                                 (internal error counter >= 96)
**                                 State is automatically cleared by the
**                                 CAN controller after communication
**                                 recovery.
**                               - BCI_CAN_BUS_OFF:
**                                 The CAN controller switches to "bus off"
**                                 and additionally to "init mode"
**                                 if serious communication problems occur.
**                                 (internal error counter == 255) To restart
**                                 the CAN controller, call BCI_StartCan.
**                                 "bus off" state is then automatically
**                                 cleared by the CAN controller after
**                                 128 bus-idle sequences.
**                               - BCI_CAN_DATA_OVR:
**                                 CAN controller switches to "data overrun",
**                                 if one or more receive messages are lost.
**                                 Communication is still possible there.
**                                 State is only cleared by means of the
**                                 function BCI_StartCan.
**                               - BCI_CAN_TX_PEND:
**                                 CAN controller is sending a message at the
**                                 moment.


**
** Returnvalue : BCI_OK        - OK
**               BCI_RESP_ERR  - No response from device
**               BCI_STATE_ERR - Wrong bci state
**
*************************************************************************/
int BCI_GetCanStatus (BCI_BRD_HDL brd_hdl, UINT8 can_num, UINT16 * can_sts);


/*************************************************************************
**
** Function    : BCI_GetErrorString
**
** Description : Function delievers a zero-terminated error string
**               based on the specified error define.
**               The string has a maximum length of ERROR_STRING_LEN characters.
**
** Parameters  : error (IN)  - BCI error define
**
** Returnvalue : pointer to constant error string
**
*************************************************************************/
const char *BCI_GetErrorString (int error);


/*************************************************************************
**
** Function    : BCI_GetBoardStatus
**
** Description : This function reads the board information.
**               The following information is available:
**               CAN status 0, CAN status 1,
**               CAN Busload 0, CAN Busload 1,
**               microcontroller CPU load
**
** Parameters  : brd_hdl (IN)  - board handle
**               brd_sts (OUT) - structure to store information
**                               about the board
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong board handle
**               BCI_STATE_ERR - wrong bci state
**
*************************************************************************/
int BCI_GetBoardStatus (BCI_BRD_HDL brd_hdl, BCI_ts_BrdSts * brd_sts);


/*************************************************************************
**
** Function    : BCI_GetBoardInfo
**
** Description : This function reads board information from the board.
**               The following information is available:
**               - firmware version
**               - number of available CAN controllers
**               - type of the CAN controllers
**
** Parameters  : brd_hdl  (IN)  - board handle
**               brd_info (OUT) - structure to store information
**                                about the board
**
** Returnvalue : BCI_OK        - OK
**               BCI_HDL_ERR   - wrong board handle
**               BCI_STATE_ERR - wrong bci state
**
*************************************************************************/
int BCI_GetBoardInfo (BCI_BRD_HDL brd_hdl, BCI_ts_BrdInfo * brd_info);


/*************************************************************************
**
** Function    : BCI_ConfigRxQueue
**
** Description : function not implemented
**
** Parameters  : brd_hdl (IN) - unused
**               can_num (IN) - unused
**               mode    (IN) - unused
**
** Returnvalue : BCI_NOT_IMPLEMENTED_ERR        - function not implemented
**
*************************************************************************/
int BCI_ConfigRxQueue (BCI_BRD_HDL brd_hdl, UINT8 can_num, UINT8 mode);



#ifdef __cplusplus
}
#endif

#endif
