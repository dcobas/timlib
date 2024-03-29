/* ==================================================================== */
/* Low level timing library interface. This API will be implemented on  */
/* all timing receiver cards, on all the CTR species, and TG8 cards.    */
/* As long as only this interface is used, higher level implementations */
/* will remain hardware independent, so one FESA class can pilot all.   */
/* Some functionality will not be available on the TG8s.                */
/* The existance of the Telegram library is assumed.                    */
/* These library routines will be available on Linux platforms where    */
/* the CTR PMC/PCI timing receiver cards are installed.                 */

/* Julian Lewis AB/CO/HT April 2004.                                    */

#ifndef TIM_LIB
#define TIM_LIB

#include <tgm/tgm.h>

#if 0
#ifdef __cplusplus
extern "C"
{
#endif
#endif

/* ==================================================================== */
/* On some cards like the Tg8, delays are 16 bits, on the CTR they are  */
/* up to 32 bits.On the Tg8 the PulseWidth is always 1us and can not be */
/* modified etc etc etc.                                                */

typedef enum {
   TimLibErrorSUCCESS,  /* All went OK, No error                        */

   TimLibErrorHARDWARE, /* Invalid TimLibHardware value for this device */
   TimLibErrorSTART,    /* Invalid TimLibStart    value for this device */
   TimLibErrorMODE,     /* Invalid TimLibMode     value for this device */
   TimLibErrorCLOCK,    /* Invalid TimLibClock    value for this device */
   TimLibErrorPWIDTH,   /* Invalid PulseWidth     value for this device */
   TimLibErrorDELAY,    /* Invalid Delay          value for this device */
   TimLibErrorNO_REMOTE,/* No remote control available on this module   */
   TimLibErrorMODULE,   /* Invalid module number, not installed         */
   TimLibErrorCOUNTER,  /* Invalid counter number for this module       */
   TimLibErrorPTIM,     /* Invalid PTIM equipment number                */
   TimLibErrorCTIM,     /* Invalid CTIM equipment number                */
   TimLibErrorBLOCKED,  /* That operation is blocked, its not permitted */
   TimLibErrorPPM,      /* PPM is not supported on this device type     */

   TimLibErrorMACHINE,  /* Invalid machine for this PTIM object         */
   TimLibErrorGROUP,    /* Invalid group number/value for PTIM object   */

   TimLibErrorINIT,     /* Library has not been initialized             */
   TimLibErrorOPEN,     /* Can't open a driver file handle, fatal       */
   TimLibErrorCONNECT,  /* Can't connect to that object                 */
   TimLibErrorWAIT,     /* No connections to wait for                   */
   TimLibErrorTIMEOUT,  /* Timeout in wait                              */
   TimLibErrorQFLAG,    /* Queue flag must be set 0, queueing is needed */

   TimLibErrorIO,       /* Unspecified IO error                         */
   TimLibErrorNOT_IMP,  /* Not implemented                              */

   TimLibErrorEXISTS,   /* Equipment already exists                     */
   TimLibErrorNOMEM,    /* Out of resource space or memory              */

   TimLibErrorNOT_ENAB, /* Module is not enabled                        */

   TimLibErrorNOT_FOUND,/* Couldn't find a cycle for that time stamp    */

   TimLibErrorSTRING,   /* Bad cycle string                             */

   TimLibERRORS         /* Total errors */
 } TimLibError;

#define TimLibErrorSTRING_SIZE 64

/* ==================================================================== */
/* There are currently many different devices that can receive events   */
/* from the timing cable, or the controls netwrok. Each device offers   */
/* device specific features. The NETWORK device is implemented accross  */
/* DTM and uses UDP packets, it offers telegram access and restricted   */
/* event subscriptions for synchronization of application programs.     */

typedef enum {
   TimLibDevice_ANY,        /* Use whatever is available */
   TimLibDevice_CTR,        /* Timing events are comming from a CTR */
   TimLibDevice_TG8_CPS,    /* Timing events are comming from a CPS TG8 */
   TimLibDevice_TG8_SPS,    /* Timing events are comming from a SPS TG8 */
   TimLibDevice_NETWORK,    /* Timing events arrive on the netwrok UDP  */

   TimLibDEVICES            /* Number of different timing event sources */
 } TimLibDevice;

/* ==================================================================== */
/* This enumeration describes hardware interrupt sources. This class of */
/* connection is useful for a FESA aquisition task which could log the  */
/* times of any output or incomming CTIM. In addition the 1Hz and 1KHz  */
/* interrupt connections can be useful.                                 */

typedef enum {
   TimLibHardwareCTIM          = 0x01,      /* Any connected CTIM interrupt */
   TimLibHardwareCOUNTER_1     = 0x02,      /* Any counter 1 output */
   TimLibHardwareCOUNTER_2     = 0x04,      /* Any counter 2 output */
   TimLibHardwareCOUNTER_3     = 0x08,      /* Any counter 3 output */
   TimLibHardwareCOUNTER_4     = 0x10,      /* Any counter 4 output */
   TimLibHardwareCOUNTER_5     = 0x20,      /* Any counter 5 output */
   TimLibHardwareCOUNTER_6     = 0x40,      /* Any counter 6 output */
   TimLibHardwareCOUNTER_7     = 0x80,      /* Any counter 7 output */
   TimLibHardwareCOUNTER_8     = 0x100,     /* Any counter 8 output */
   TimLibHardwarePLL_ITERATION = 0x200,     /* Phase locked loop iteration */
   TimLibHardwareGMT_EVENT_IN  = 0x400,     /* Any incomming timing frame */
   TimLibHardware1HZ           = 0x800,     /* Tne one second UTC time pulse */
   TimLibHardware1KHZ          = 0x1000,    /* One Kilo Hertz */
   TimLibHardwareMATCHED       = 0x2000     /* Incomming event trigger matched */
 } TimLibHardware;

#define TimLibHardwareBITS 0x3FFF

/* ==================================================================== */
/* There are 3 classes of interrupt that can be connected to, hardware, */
/* see above, PTIM equipment, and CTIM equipment.                       */

typedef enum {
   TimLibClassHARDWARE,     /* Class is direct hardware connection */
   TimLibClassCTIM,         /* A Ctim timing object carried by an event on the cable */
   TimLibClassPTIM          /* A PTIM timing object implemented on a counter */
 } TimLibClass;

/* ==================================================================== */
/* The time comes in the UTC second, the nanosecond in the second, and  */
/* the millisecond modulo which is called the CTrain value. As each     */
/* timing cable has a different CTrain value, for example the CPS and   */
/* the PSB have different CTrains, it is important to know on which GMT */
/* cable the relavent device is connected; this explains the Machine    */
/* parameter in the TimLibTime structure.                               */

typedef struct {
   unsigned long Second;    /* UTC Second */
   unsigned long Nano;      /* Nano second in the second */
   unsigned long CTrain;    /* Machine millisecond in cycle */
   TgmMachine    Machine;   /* Machine */
 } TimLibTime;

/* ==================================================================== */
/* Get all interesting version information for the TimLib library, the  */
/* version of the driver, and the version of each modules firmware/VHDL */
/* This structure is returned from TimLibGetModuleVersion.              */

#define TimLibMODULES 16

typedef enum {
   TimLibModuleTypeNONE,
   TimLibModuleTypeCTRI,
   TimLibModuleTypeCTRP,
   TimLibModuleTypeCTRV,
   TimLibModuleTypeCPS_TG8,
   TimLibModuleTypeSPS_TG8,

   TimLibModuleTYPES
 } TimLibModuleType;

typedef struct {
   unsigned long    DrvVer;                /* Driver version            */
   TimLibModuleType ModTyp;                /* Type of module CTR/TG8    */
   unsigned long    CorVer;                /* Correct VHDL/Firmware ver */
   unsigned long    ModVer[TimLibMODULES]; /* Module  VHDL/Firmware ver */
 } TimLibModuleVersion;

/* ==================================================================== */
/* The sources of start in a counter. The chained_stop starts from the  */
/* output of the previous counter and stops from the output of the next */
/* counter. This is sometimes needed when using burst mode.             */

typedef enum {
   TimLibStartNORMAL,       /* The next millisecond tick starts the counter */
   TimLibStartEXT1,         /* The counter uses external start number one */
   TimLibStartEXT2,         /* The counter uses external start number two */
   TimLibStartCHAINED,      /* The output of the previous counter is the start */
   TimLibStartSELF,         /* The output of this counter is the start (Divide) */
   TimLibStartREMOTE,       /* The counter waits for a remote start, see later */
   TimLibStartPPS,          /* The counter waits for the PPS */
   TimLibStartCHAINED_STOP, /* The counter is started by previous and stopped by next */

   TimLibSTARTS             /* The number of possible counter starts */
 } TimLibStart;

/* ==================================================================== */
/* In burst mode, the stop can be EXT2 start, or the output of the next */
/* counter if the start is chained_stop. Multiple start permits many    */
/* starts for only one load. In Multiple Burst many bursts can be made  */
/* for a single load.                                                   */

typedef enum {
   TimLibModeNORMAL,        /* One load, one start and one output mode */
   TimLibModeMULTIPLE,      /* One load, multiple starts, multiple outputs */
   TimLibModeBURST,         /* Like MULTIPLE, but terminated by external start two */
   TimLibModeMULT_BURST,    /* Both mode 1 & 2 = Multiple bursts */

   TimLibMODES              /* There are 4 possible counter configuration modes */
 } TimLibMode;

/* ==================================================================== */
/* These clocks on the CTR cards are phase locked on to UTC time with a */
/* short term RMS jitter of 100ps. The peak to peak is better than one  */
/* nano second. On the TG8 the jitter is horrible 500ns, and the many   */
/* clocks are not available, namley 10, 40MHz, and CHAINED.             */

typedef enum {
   TimLibClock1KHZ,         /* The 1KHZ CTrain clock from the 01 events */
   TimLibClock10MHZ,        /* Divided down 40MHZ phase synchronous with 1KHZ */
   TimLibClock40MHZ,        /* Recovered 40MHZ by the PLL from the data edges */
   TimLibClockEXT1,         /* External clock one */
   TimLibClockEXT2,         /* External clock two */
   TimLibClockCHAINED,      /* The output of the previous counter is the clock */

   TimLibCLOCKS             /* The number of possible clock sources */
 } TimLibClock;

/* ==================================================================== */
/* The output routing mask specifies which signals are sent to which of */
/* the output connectors. N.B. On some cards (CTRP/I) there are more    */
/* counters on the card (8) than there are front pannel connectors (4). */
/* Each counter 1..8, owns a physical output channel leading to a front */
/* pannel connector. The output mask for a given counter selects what   */
/* signals are assigned to the counters output connector.               */

typedef enum {
   TimLibOutputCTIM       = 0x01,  /* Any connected CTIM interrupt */
   TimLibOutputCNTR_1     = 0x02,  /* Counter 1 output */
   TimLibOutputCNTR_2     = 0x04,  /* Counter 2 output */
   TimLibOutputCNTR_3     = 0x08,  /* Counter 3 output */
   TimLibOutputCNTR_4     = 0x10,  /* Counter 4 output */
   TimLibOutputCNTR_5     = 0x20,  /* Counter 5 output */
   TimLibOutputCNTR_6     = 0x40,  /* Counter 6 output */
   TimLibOutputCNTR_7     = 0x80,  /* Counter 7 output */
   TimLibOutputCNTR_8     = 0x100, /* Counter 8 output */

   /* These next signals can be routed directly to a counter output lemo */

   TimLibOutput40MHZ      = 0x200, /* 40MHz clock bit */
   TimLibOutputEXT1       = 0x400, /* Ext-1 clock bit */
   TimLibOutputEXT2       = 0x800  /* Ext-2 clock bit */
 } TimLibOutput;

#define TimLibOutputBITS 0xFFF

/* ==================================================================== */
/* The polarity of a counter output.                                    */

typedef enum {
   TimLibPolarityTTL_BAR = 0x01, /* Negative TTL Bar */
   TimLibPolarityTTL     = 0x02  /* Posative TTL */
 } TimLibPolarity;

/* ==================================================================== */
/* Counters can be directly controlled from the RT task. These are the  */
/* possible remote control commands that can be used in direct control. */

typedef enum {
   TimLibRemoteLOAD  = 0x01,   /* Load the counter with a new set of control values */
   TimLibRemoteSTOP  = 0x02,   /* Kill counter no matter what its state */
   TimLibRemoteSTART = 0x04,   /* Start the counter, the startREMOTE must be set */
   TimLibRemoteOUT   = 0x08,   /* Make output on output pin, counter not disturbed */
   TimLibRemoteBUS   = 0x10    /* Make a bus interrupt now,  counter not disturbed */
 } TimLibRemote;

#define TimLibRemoteBITS 0x1F

/* ==================================================================== */
/* Status of timing receiver, for Tg8 modules the PLL bit is set OK     */

typedef enum {
   TimLibStatusGMT_OK  = 0x01,  /* General machine timing being recieved */
   TimLibStatusPLL_OK  = 0x02,  /* PLL locked */
   TimLibStatusSELF_OK = 0x04,  /* Self test OK */
   TimLibStatusENABLED = 0x08,  /* Timing Reception enabled */
   TimLibStatusBUS_OK  = 0x10   /* No Bus Errors detected */
 } TimLibStatus;

#define TimLibStatusBITS 0x1F

/* ==================================================================== */
/* Enable states for counters. When directly controling a counter, its  */
/* possible to also control the Bus Interrupt behaviour.                */

typedef enum {
   TimLibEnableNOOUT   = 0, /* No bits means do nothing */
   TimLibEnableOUT     = 1, /* Bit 0 means out */
   TimLibEnableBUS     = 2, /* Bit 1 means bus */
   TimLibEnableOUTBUS  = 3, /* Both bits means bus and out */

   TimLibENABLES
 } TimLibEnable;

/* ==================================================================== */
/* Each PTIM equipment has a set of current control values.  The values */
/* to be read or written should have their bit set in the CcvMask.      */

typedef enum {
   TimLibCcvMaskENABLE   = 0x0001,
   TimLibCcvMaskSTART    = 0x0002,
   TimLibCcvMaskMODE     = 0x0004,
   TimLibCcvMaskCLOCK    = 0x0008,
   TimLibCcvMaskPWIDTH   = 0x0010,
   TimLibCcvMaskDELAY    = 0x0020,
   TimLibCcvMaskOMASK    = 0x0040,
   TimLibCcvMaskPOLARITY = 0x0080,
   TimLibCcvMaskCTIM     = 0x0100,
   TimLibCcvMaskPAYLOAD  = 0x0200,
   TimLibCcvMaskMACHINE  = 0x0400,
   TimLibCcvMaskGRNUM    = 0x0800,
   TimLibCcvMaskGRVAL    = 0x1000
 } TimLibCcvMask;

#define TimLibCcvMaskBITS 0x1FFF

typedef struct {
   TimLibEnable     Enable;     /* Enable = 1, Disable = 0 */
   TimLibStart      Start;      /* The counters start. */
   TimLibMode       Mode;       /* The counters operating mode. */
   TimLibClock      Clock;      /* Clock specification. */
   unsigned long    PulsWidth;  /* Number of 40MHz ticks, 0 = as fast as possible. */
   unsigned long    Delay;      /* 32 bit delay to load into counter. */
   TimLibOutput     OutputMask; /* Output lemo connectors mask */
   TimLibPolarity   Polarity;   /* Polarity of output */
   unsigned long    Ctim;       /* CTIM triggering event of this action */
   unsigned long    Payload;    /* Payload part of the trigger */
   TgmMachine       Machine;    /* Tgm machine of telegram */
   unsigned long    GrNum;      /* Telegram's group number */
   unsigned long    GrVal;      /* Telegram's group value */
 } TimLibCcv;

/* When creating a new PTIM object, the default settings will be as follows */

#define TimLibCcvDEFAULT_ENABLE      0
#define TimLibCcvDEFAULT_START       TimLibStartNORMAL
#define TimLibCcvDEFAULT_MODE        TimLibModeNORMAL
#define TimLibCcvDEFAULT_CLOCK       TimLibClock1KHZ
#define TimLibCcvDEFAULT_PULSE_WIDTH 400
#define TimLibCcvDEFAULT_DELAY       1
#define TimLibCcvDEFAULT_OUTPUT_MASK 0
#define TimLibCcvDEFAULT_POLARITY    TimLibPolarityTTL_BAR
#define TimLibCcvDEFAULT_CTIM        100
#define TimLibCcvDEFAULT_PAYLOAD     0
#define TimLibCcvDEFAULT_MACHINE     TgmCPS
#define TimLibCcvDEFAULT_GRNUM       1
#define TimLibCcvDEFAULT_GRVAL       24

/* ==================================================================== */
/* For the CTR module there is a basic IO capability                    */

typedef enum {
   TimLibLemoOUT_1 = 0x001, /* Output:  Counter 1        Lemo state */
   TimLibLemoOUT_2 = 0x002, /* Output:  Counter 2        Lemo state */
   TimLibLemoOUT_3 = 0x004, /* Output:  Counter 3        Lemo state */
   TimLibLemoOUT_4 = 0x008, /* Output:  Counter 4        Lemo state */
   TimLibLemoOUT_5 = 0x010, /* Output:  Counter 5        Lemo state */
   TimLibLemoOUT_6 = 0x020, /* Output:  Counter 6        Lemo state */
   TimLibLemoOUT_7 = 0x040, /* Output:  Counter 7        Lemo state */
   TimLibLemoOUT_8 = 0x080, /* Output:  Counter 8        Lemo state */
   TimLibLemoXST_1 = 0x100, /* Input:   External Start 1 Lemo state */
   TimLibLemoXST_2 = 0x200, /* Input:   External Start 2 Lemo state */
   TimLibLemoXCL_1 = 0x400, /* Input:   External Clock 1 Lemo state */
   TimLibLemoXCL_2 = 0x800  /* Input:   External Clock 2 Lemo state */
 } TimLibLemo;

/* ==================================================================== */
/* Full CTR card status bit definitions                                 */

typedef enum {
   TimLibCstStatCTRXE   = 0x0001, /* Beam Energy extension */
   TimLibCstStatCTRXI   = 0x0002, /* IO extension */
   TimLibCstStatV1_PCB  = 0x0004, /* PCB version 1 */
   TimLibCstStatV2_PCB  = 0x0008, /* PCB version 2 */

   TimLibCstStatS1      = 0x0010, /* External start 1 input value */
   TimLibCstStatS2      = 0x0020, /* External start 2 input value */
   TimLibCstStatX1      = 0x0040, /* External clock 1 input value */
   TimLibCstStatX2      = 0x0080, /* External clock 2 input value */

   TimLibCstStatO1      = 0x0100, /* Lemo output 1 state */
   TimLibCstStatO2      = 0x0200, /* Lemo output 2 state */
   TimLibCstStatO3      = 0x0400, /* Lemo output 3 state */
   TimLibCstStatO4      = 0x0800, /* Lemo output 4 state */

   TimLibCstStatO5      = 0x1000, /* Lemo output 5 state */
   TimLibCstStatO6      = 0x2000, /* Lemo output 6 state */
   TimLibCstStatO7      = 0x4000, /* Lemo output 7 state */
   TimLibCstStatO8      = 0x8000, /* Lemo output 8 state */
   
   TimLibCstStatIDOkP            = 0x10000, /* PCB Id read Ok */
   TimLibCstStatDebugHistory     = 0x20000, /* Debug History Mode On */
   TimLibCstStatUtcPllEnabled    = 0x40000, /* Utc Pll Enabled 0=Brutal 1=Soft lock */
   TimLibCstStatExtendedMemory   = 0x80000, /* Extended memory for VHDL revision March 2008 */
   TimLibCstStatTemperatureOk    = 0x100000 /* Sensor present */

 } TimLibCstStat;

#define TimLibCstSTATAE 21

/* ==================================================================== */
/* Extended status. Valid for CTR cards only                            */

typedef struct {
   unsigned long Valid;     /* if not zero Pll information is valid */
   unsigned long Error;     /* Phase error */
   unsigned long Integrator;/* Integrator value */
   unsigned long Dac;       /* Value on DAC */
   unsigned long LastItLen; /* Last iteration length */
   unsigned long KI;        /* Constant of integration */
   unsigned long KP;        /* Constant of proportionality */
   unsigned long NumAverage;/* Numeric average */
   unsigned long Phase;     /* Pll phase */
   float    AsPrdNs;        /* Asynchronous period */
 } TimLibPllBlock;          /* Pll status block */

typedef struct {
   unsigned long Valid;     /* if not zero receiver information is valid */
   TimLibTime    LastRset;  /* Time when last reset */
   unsigned long PrtyErrs;  /* Number of parity errors */
   unsigned long SyncErrs;  /* Number of synchronization errors */
   unsigned long CodeErrs;  /* Number of code violations */
   unsigned long QueuErrs;  /* Number of queue overflow errors */
   unsigned long TotlErrs;  /* Total error count */
 } TimLibRecBlock;          /* Receiver errors block */

typedef struct {
   unsigned long Valid;     /* if not zero Card State is valid */
   TimLibCstStat Stat;      /* Full IO status */
   unsigned long IdMSL;     /* ID chip Most significant long */
   unsigned long IdLSL;     /* ID chip Lest significant long */
 } TimLibCstBlock;          /* Card State block */

typedef struct {
   unsigned long Valid;            /* if not zero Board extended memory present */
   unsigned long PllErrThresh;  /* Pll error threshold to generate a pll error RW */
   unsigned long PllDacLowPass; /* Low Passed DAC value used if the GMT is missed (CIC filter) RO */
   unsigned long PllDacCIConst; /* Log2 of the number of avarages of the DAC Value CIC low pass filter RW */
   unsigned long PllMonCIConst; /* Log2 of the interrupt reduction factor (averages over this count)  RW */
   unsigned long PllPhaseDCM;   /* (Not used yet) Phase of the spartan DLL. Not used in the standard VHDL RW */
   unsigned long PllUtcPhasErr; /* Phase error Utc */
   unsigned long Temperature;   /* (Not used yet) Temperature in units of 0.5 degrees C */
   unsigned long MsMissed;      /* Number of millisencods missed since last power up. RO */
   TimLibTime    MsLastErr;     /* Time last MS was missed */
   unsigned long PllErrCount;   /* Number of pll errors since last power up (past threshold ) RO */
   TimLibTime    PllLastErr;    /* Time of last PLL error */
   unsigned long FrmMissed;     /* Number of missed timing frames */
   TimLibTime    FrmMissedLast; /* Time when last frame was missed */
   unsigned long RecBadCycles;  /* Number of bad reception cycles since last power up */
   unsigned long RecRcvdFrms;   /* Number of received frames since last power up */
   unsigned long RecSentFrms;   /* Value of last sent frames event */
   unsigned long PllUtcErrs;    /* Number of UTC PLL errors */
   TimLibTime    StartOne;      /* Time of the last External-1 start */
 } TimLibExtBlock;                 /* Extended memory block */

typedef struct {
   unsigned long  Module; /* Module number 1..n if 0 module stats are invalid */
   TimLibPllBlock Pll;    /* Pll status block */
   TimLibRecBlock Rec;    /* Receiver errors block */
   TimLibCstBlock Cst;    /* Card State block */
   TimLibExtBlock Ext;    /* Extended memory block */
 } TimLibModuleStats;     /* Module statistics */

/* ==================================================================== */
/* This routine could have been hidden from the user of the Timing lib, */
/* however, in some circumstances, the initialization can take several  */
/* minutes to complete. Hence I have decided to make an initialization  */
/* routine publicly available, and force users to call it.              */
/* This routine performs the following initialization functions...      */
/*    1) Opens a connection to the driver                               */
/*    2) Checks the Firmware/VHDL version against the latest revision   */
/*       Some EProms/FPGAs may need updating, this takes a while.       */
/*    3) Load all relavent CTIM and PTIM definitions if needed.         */
/* The device parameter specifies which type of device the library must */
/* use and initialize. If its set to ANY, the library will try to find  */
/* a device by attempting to open each device driver in turn and in the */
/* order of the DEVICE enumeration: CTR_PCI,CTR_VME,TG8_CPS,TG8_SPS and */
/* finaly NETWORK.                                                      */

TimLibError TimLibInitialize(TimLibDevice device); /* Initialize hardware/software */

/* ==================================================================== */
/* Get the version, returns a string containing date time of compiled.  */

char *TimLibGetVersion();

/* ==================================================================== */
/* Get VHDL/Firmware version of all modules, and the correct version.   */

TimLibError TimLibGetModuleVersion(TimLibModuleVersion *tver);

/* ==================================================================== */
/* Get the status of a module and its device type.                      */

TimLibStatus TimLibGetStatus(unsigned long module, TimLibDevice *dev);

/* ==================================================================== */
/* Connect to an interrupt. If you are connecting to either a CTIM      */
/* interrupt or to a hardware interrupt, you may need to specify on     */
/* which device the interrupt should be connected. This is achieved by  */
/* the module parameter. If the module is zero, the system will decide  */
/* which device to use, otherwise module contains a value between 1 and */
/* the number of installed timing receiver cards. For PTIM objects the  */
/* module parameter must be set to zero or the real module on which the */
/* PTIM object is implemented. On PTIM objects the module is implicit.  */

TimLibError TimLibConnect(TimLibClass   iclss,   /* Class of interrupt */
			  unsigned long equip,   /* Equipment or hardware mask */
			  unsigned long module); /* For HARD or CTIM classes */

/* ==================================================================== */
/* Connect to a CTIM with specified payload                             */

TimLibError TimLibConnectPayload(unsigned long ctim,    /* The CTIM ID you want to connect to */
				 unsigned long payload, /* The 16 bit payload in a long */
				 unsigned long module); /* The module, or zero means don't care */

/* ==================================================================== */
/* Disconnect from an interrupt                                         */

TimLibError TimLibDisConnect(TimLibClass   iclss,   /* Class of interrupt */
			     unsigned long equip,   /* Equipment or hardware mask */
			     unsigned long module); /* For HARD or CTIM classes */

/* ==================================================================== */
/* Set queueing On or Off, and the time out value in micro seconds.     */
/* A timeout value of zero means no time out, you wait for ever.        */

TimLibError TimLibQueue(unsigned long qflag,   /* 0=>Queue, 1=>NoQueue  */
			unsigned long tmout);  /* 0=>No time outs       */

/* ==================================================================== */
/* To know if a call to wait will block, this call returns the Queue    */
/* size. If the size iz greater than zero a call to wait will not block */
/* and return without waiting. If the qflag is set to NoQueue, zero is  */
/* allways returned and all calls to wait will block.                   */

unsigned long TimLibGetQueueSize();

/* ==================================================================== */
/* Wait for an interrupt. The parameters are all returned from the call */
/* so you can know which interrupt it was that came back. Note, when    */
/* waiting for a hardware interrupt from either CTIM or from a counter, */
/* it is the CTIM or PTIM object that caused the interrupt returned.    */
/* The telegram will have been read already by the high prioity task    */
/* get_tgm_ctr/tg8, be aware of the race condition here, hence payload. */
/* This routine is a blocking call, it waits for interrupt or timeout.  */
/* Any NULL argument  is permitted, and no value will be returned.      */

/* Arguments:                                                           */
/*    iclss:   The class of the interrupt CTIM, PTIM, or hardware       */
/*    equip:   The PTIM, CTIM equipment, or hardware mask               */
/*    plnum:   If class is PTIM this is the PLS line number             */
/*    source:  The hardware source of the interrupt                     */
/*    onzero:  The time of the interrupt                                */
/*    trigger: The arrival time of the event that triggered the action  */
/*    start:   The time the start of the counter occured                */
/*    ctim:    The CTIM equipment number of the triggering event        */
/*    payload: The payload of the triggering event                      */
/*    module:  The module number 1..n of the timing receiver card       */
/*    missed:  The number of missed events since the last wait          */
/*    qsize:   The number of remaining interrupts on the queue          */
/*    mch:     The Tgm machine of the trigger event                     */

TimLibError TimLibWait(TimLibClass    *iclss,   /* Class of interrupt */
		       unsigned long  *equip,   /* PTIM CTIM or hardware mask */
		       unsigned long  *plnum,   /* Ptim line number 1..n or 0 */
		       TimLibHardware *source,  /* Hardware source of interrupt */
		       TimLibTime     *onzero,  /* Time of interrupt/output */
		       TimLibTime     *trigger, /* Time of counters load */
		       TimLibTime     *start,   /* Time of counters start */
		       unsigned long  *ctim,    /* CTIM trigger equipment ID */
		       unsigned long  *payload, /* Payload of trigger event */
		       unsigned long  *module,  /* Module that interrupted */
		       unsigned long  *missed,  /* Number of missed interrupts */
		       unsigned long  *qsize,   /* Remaining interrupts on queue */
		       TgmMachine     *mch);    /* Corresponding TgmMachine */

/* ==================================================================== */
/* Set the Ccv of a PTIM equipment. Note neither the counter number nor */
/* the trigger condition can be changed.                                */

TimLibError TimLibSet(unsigned long ptim,      /* PTIM to write to */
		      unsigned long plnum,     /* Ptim line number 1..n or 0 */
		      unsigned long grnum,     /* Group number or Zero */
		      unsigned long grval,     /* Group value for number */
		      TimLibCcvMask ccvm,      /* Which values to write */
		      TimLibCcv     *ccv);     /* Current control value */

/* ==================================================================== */
/* Get the Ccv of a PTIM equipment.                                     */
/* When reading, the valid fields are determined by the module type.    */
/* Some modules types, eg Tg8 have missing fields in the ccv structure. */

TimLibError TimLibGet(unsigned long ptim,
		      unsigned long plnum,     /* Ptim line number 1..n or 0 */
		      unsigned long grnum,
		      unsigned long grval,
		      TimLibCcvMask *ccvm,     /* Valid fields in ccv */
		      TimLibCcv     *ccv);

/* ==================================================================== */
/* By writing to the driver this call simulates an interrupt for the    */
/* connected clients. Also it can be used as a way of synchronizing     */
/* processes, this is especially important in Linux systems where the   */
/* schedular is not preemptive.                                         */

/* Arguments:                                                                     */
/*    iclss:   Class of interrupt to simulate, PTIM, CTIM or Hardware             */
/*    equip:   Equipment number for PTIM or CTIM, hardware mask for Hardware      */
/*    module:  When class is CTIM or Hardware, the module number is used          */
/*    machine: Telegram ID is used for PTIM interrupts if grnum is not zero       */
/*    grnum:   If zero, no telegram checking, else the PTIM triggers group number */
/*    grval:   The telegram group value for the PTIM trigger                      */

TimLibError TimLibSimulate(TimLibClass   iclss,
			   unsigned long equip,
			   unsigned long module,
			   TgmMachine    machine,
			   unsigned long grnum,
			   unsigned long grval);

/* ==================================================================== */
/* Set a counter under full remote control (IE under DSC tasks control) */
/* This feature permits you to do what you like with counters even if   */
/* there is no timing cable attached. With this you can drive stepper   */
/* motors, wire scanners or whatever. No PTIM or CTIM is involved, the  */
/* configuration is loaded directly by the application. Note that when  */
/* the argument remflg is set to 1, the counter can not be written to   */
/* by incomming triggers so all PTIM objects using the counter stop     */
/* overwriting the counter configuration and are effectivley disabled.  */
/* Setting the remflg 0 permits PTIM triggers to write to the counter   */
/* configuration, the write block is removed. Also note that in some    */
/* cases it is useful to perform remote actions, such as remoteSTOP,    */
/* even if the remflg is set to zero. The remflg simply blocks PTIM     */
/* overwrites, the counter configuration can still be accessed !        */

TimLibError TimLibRemoteControl(unsigned long remflg, /* 0 = Normal, 1 = Remote */
				unsigned long module, /* Module or zero */
				unsigned long cntr,   /* 1..8 counter number */
				TimLibRemote  rcmd,   /* Command */
				TimLibCcvMask ccvm,   /* Fields to be set */
				TimLibCcv     *ccv);  /* Value to load in counter */

/* ==================================================================== */
/* Get a counters remote configuration                                  */

TimLibError TimLibGetRemote(unsigned long module,
			    unsigned long cntr,
			    unsigned long *remflg,
			    TimLibCcvMask *ccvm,
			    TimLibCcv     *ccv);

/* ==================================================================== */
/* Read the instantaneous value of the time in UTC. The module parameter*/
/* can be set to zero in which case the system decideds which module to */
/* read the time from, otherwise it can be set to a value between 1 and */
/* the number of installed modules.                                     */

TimLibError TimLibGetTime(unsigned long module, /* Module number to read from */
			  TimLibTime    *utc);  /* Returned time value */

/* ==================================================================== */
/* Read a machines telegram from a timing receiver. The module can be   */
/* either zero, in which case the system decides which device to use,   */
/* or it can be explicitly set between 1 and the number of installed    */
/* modules. The telegram object returned has an opaque structure and    */
/* can only be decoded through the Tgm library routine .....            */

/* long grval = TgmGetGroupValueFromTelegram(unsigned long grnum,       */
/*                                           TgmTelegram   *telegram)   */

/* WARNING: The only task that should call this routine will be, get_tgm_lib,  */
/* all other, LOWER PRIORITY tasks must NEVER call this routine, instead they  */
/* should call the telegram library directly like this ...                     */

/* TgmTelegram telegram;                                                       */
/*                                                                             */
/* if (TgmGetTelegram(machine, index, offset, &telegram) == TgmSUCCESS) { ...  */
/*                                                                             */
/* For more information on this function see the Tgm library man pages.        */

extern int TimLibClient;

TimLibError TimLibGetTelegram(unsigned long module,
			      TgmMachine    machine,
			      TgmTelegram   *telegram);

/* ==================================================================== */
/* Convert a TimLibError into a string. The returned char pointer is    */
/* either NULL if the supplied error is out of range, or it points to a */
/* static string contained on the library routines heap. You must copy  */
/* the string if you need to be sure it is not overwritten. Obviously   */
/* this routine is therfore not thread safe, but you don't need to free */
/* allocated memory thus avoiding potential memory leaks.               */

char *TimLibErrorToString(TimLibError error);

/* ==================================================================== */
/* Lets you know how many installed modules there are on this host.     */

unsigned long TimLibGetInstalledModuleCount();

/* ==================================================================== */
/* Get the description of a given PTIM equipment. The dimension returns */
/* the PPM dimension, counter and module are obvious.                   */

TimLibError TimLibGetPtimObject(unsigned long ptim, /* PTIM equipment number */
				unsigned long *module,
				unsigned long *counter,
				unsigned long *dimension);

/* ==================================================================== */
/* Get the list of all defined PTIM objects                             */

TimLibError TimLibGetAllPtimObjects(unsigned long *ptimlist,  /* List of ptim equipments */
				    unsigned long *psize,     /* Number of ptims in list */
				    unsigned long size);      /* Max size of list */

/* ==================================================================== */
/* Get the list of all defined CTIM objects                             */

TimLibError TimLibGetAllCtimObjects(unsigned long *ctimlist,  /* List of ctim equipments */
				    unsigned long *csize,     /* Number of ctims in list */
				    unsigned long size);      /* Max size of list */

/* ==================================================================== */
/* Get the event code corresponding to a given CTIM equipment number.   */

TimLibError TimLibGetCtimObject(unsigned long ctim, /* CTIM equipment number */
				unsigned long *eventcode);

/* ==================================================================== */
/* In some cases when running a GUI under Linux, say, a file handle to  */
/* put in a "select" is needed so that one can wait on multiple file    */
/* handles simultaneously. This routine returns such a handle suitable  */
/* to check for waiting interrupts. Don not read directly from it, call */
/* the wait routine. The queue flag must be on for this to work !!      */

TimLibError TimLibGetHandle(int *fd);

/* ==================================================================== */
/* Create a new PTIM object, the CCV settings will be defaulted.        */


TimLibError TimLibCreatePtimObject(unsigned long ptim, /* PTIM equipment number */
				   unsigned long module,
				   unsigned long counter,
				   unsigned long dimension);

/* ==================================================================== */
/* Create a new CTIM object. If a payload is to be used for this event  */
/* be sure to set the low 16-Bits to 0xFFFF                             */

TimLibError TimLibCreateCtimObject(unsigned long ctim, /* CTIM equipment number */
				   unsigned long eventcode);

/* ==================================================================== */
/* Set the debug level                                                  */

void TimLibSetDebug(unsigned long level); /* Zero means no debug */

/* ==================================================================== */
/* Get the cable identifier to which a given module is attached so that */
/* the correct module can be used to read telegrams. This function will */
/* be used by the program get_tgm_tim only; it is of no interest to the */
/* majority of clients because calls to ReadTelegram are diverted.      */

TimLibError TimLibGetCableId(unsigned long module,  /* The given module */
			     unsigned long *cable); /* The cable ID */

/* ==================================================================== */
/* Get telegram information from a time stamp.                          */
/* This routine searches the telegram history for a cycle in which your */
/* time stamp occured. It then returns the cycles time stamp in which   */
/* the time you gave occurd and its present and next tags.              */

TimLibError TimLibGetTgmInfo(TimLibTime    stamp,       /* Time you want telegram information about */
			     TimLibTime    *cyclestamp, /* Time of cycle for given stamp */
			     unsigned long *cytag,      /* Cycle tag */
			     unsigned long *ncytag);    /* Next cycle tag */


/* ==================================================================== */
/* Get a group value from the telegram for given cycle stamp.           */

TimLibError TimLibGetGroupValueFromStamp(TimLibTime    stamp, /* Cycle stamp  */
					 unsigned long gn,    /* Group number */
					 unsigned long next,  /* Next flag    */
					 unsigned long *gv);  /* Group value  */

/* ==================================================================== */
/* Convert a cycle Id string like CPS.USER.SFTPRO into a slot index.    */
/* No abbreviations are supported, seperators are dots. Strict Syntax.  */
/* The result is slix, the slot index ranging from [0..(GroupSize-1)].  */
/* No change is made to slix on error always check return before using. */

TimLibError TimLibStringToSlot(char          *cyid,     /* Cycle ID string */
			       unsigned long *slix);    /* Resulting slot index */

/* ==================================================================== */
/* Special IO routine for CTR only, Get CTR LEMO values                 */

TimLibError TimLibGetIoStatus(unsigned long module,
			      TimLibLemo *input);   /* Input values */

/* ==================================================================== */
/* Special IO routine for CTR only, Set Counter Output LEMO values      */

TimLibError TimLibSetOutputs(unsigned long module,
			     TimLibLemo output,     /* Output values */
			     TimLibLemo mask);      /* Which ones you eant to set */

/* ==================================================================== */
/* Get specific status information string                               */

char *TimLibGetSpecificInfo(unsigned long module);  /* The given module */

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */
/* This initialize will return a new file descriptor each time it is    */
/* called. Today the drivers support a maximum of 16 file descriptors   */
/* so the resource is limited and should be used economically; always   */
/* call "close(fd)" when the descriptor is no longer needed. In case of */
/* errors, a zero is returned.                                          */

int TimLibFdInitialize(TimLibDevice device);            /* Given device */

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */
/* This is just like the usual connect except you must supply a fd that */
/* is open and was obtained from FdInitialize. Should you supply a fd   */
/* with a bad value the driver will check if it belongs to your process */
/* and if it dose it will use it. This may cause unpredictable results. */

TimLibError TimLibFdConnect(int         fd,       /* File descriptor */
			  TimLibClass   iclss,    /* Class of interrupt */
			  unsigned long equip,    /* Equipment or hardware mask */
			  unsigned long module);  /* For HARD or CTIM classes */

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */

TimLibError TimLibFdQueue(int         fd,        /* File descriptor */
			unsigned long qflag,     /* 0=>Queue, 1=>NoQueue  */
			unsigned long tmout);    /* 0=>No time outs       */

/* ==================================================================== */
/* For multi-threaded applications, each thread needs its own filedesc. */

TimLibError TimLibFdWait(int          fd,        /* File descriptor */
		       TimLibClass    *iclss,    /* Class of interrupt */
		       unsigned long  *equip,    /* PTIM CTIM or hardware mask */
		       unsigned long  *plnum,    /* Ptim line number 1..n or 0 */
		       TimLibHardware *source,   /* Hardware source of interrupt */
		       TimLibTime     *onzero,   /* Time of interrupt/output */
		       TimLibTime     *trigger,  /* Time of counters load */
		       TimLibTime     *start,    /* Time of counters start */
		       unsigned long  *ctim,     /* CTIM trigger equipment ID */
		       unsigned long  *payload,  /* Payload of trigger event */
		       unsigned long  *module,   /* Module that interrupted */
		       unsigned long  *missed,   /* Number of missed interrupts */
		       unsigned long  *qsize,    /* Remaining interrupts on queue */
		       TgmMachine     *mch);     /* Corresponding TgmMachine */

/* ==================================================================== */
/* Get the module statistics for the given module                       */

TimLibError TimLibGetModuleStats(unsigned long module,
				 TimLibModuleStats *stats);


/* ==================================================================== */
/* Control how the PLL locks after synchronization loss                 */

TimLibError TimLibSetPllLocking(unsigned long module,
				unsigned long lockflag); /* 1=> Brutal, else Slow */

/* ==================================================================== */
/* Get the module that best suits a given CTIM based on the cable ID    */

unsigned long TimLibGetModuleForCtim(unsigned long ctim);

#if 0
#ifdef __cplusplus
}
#endif
#endif

#endif
