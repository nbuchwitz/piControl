#ifndef _REVPI_CORE_H
#define _REVPI_CORE_H

#include <linux/netdevice.h>
#include <linux/types.h>

#include "ModGateRS485.h"
#include "piControlMain.h"
#include "PiBridgeMaster.h"
#include "RevPiDevice.h"

typedef enum {
	piBridgeStop = 0,
	piBridgeInit = 1,	// MGate Protocol
	piBridgeRun = 2,	// IO Protocol
	piBridgeDummy = 99	// dummy value to force update of led state
} enPiBridgeState;

typedef struct _SRevPiProcessImage {
	struct {
		u8 i8uStatus;
		u8 i8uIOCycle;
		u16 i16uRS485ErrorCnt;
		u8 i8uCPUTemperature;
		u8 i8uCPUFrequency;
	} __attribute__ ((__packed__)) drv;	// 6 bytes
	struct {
		union {
			u8 outputs;
			u8 leds;
		};
		u16 i16uRS485ErrorLimit1;
		u16 i16uRS485ErrorLimit2;
		u16 rgb_leds;
	} __attribute__ ((__packed__)) usr;	// 7 bytes (if device has no rgb_leds, only 5 bytes are used in process image)
} __attribute__ ((__packed__)) SRevPiProcessImage;

typedef struct _SRevPiCore {
	SRevPiProcessImage image;
	struct rpi_firmware *fw;

	// piGate stuff
	INT8U i8uLeftMGateIdx;	// index of left GateModule in RevPiDevice_asDevice_m
	INT8U i8uRightMGateIdx;	// index of right GateModule in RevPiDevice_asDevice_m
	INT8U ai8uInput[KB_PD_LEN * MODGATECOM_MAX_MODULES];
	INT8U ai8uOutput[KB_PD_LEN * MODGATECOM_MAX_MODULES];

	// piBridge stuff
	struct rt_mutex lockBridgeState;
	enPiBridgeState eBridgeState;	// 0=stopped, 1=init, 2=running
	struct gpio_desc *gpio_sniff1a;
	struct gpio_desc *gpio_sniff1b;
	struct gpio_desc *gpio_sniff2a;
	struct gpio_desc *gpio_sniff2b;

	// watchdog stuff, Connect only
	struct gpio_desc *gpio_x2di;
	struct gpio_desc *gpio_x2do;
	struct gpio_desc *gpio_wdtrigger;

	// handle user telegrams
	struct rt_mutex lockUserTel;
	struct semaphore semUserTel;
	bool pendingUserTel;
	SIOGeneric requestUserTel;
	SIOGeneric responseUserTel;
	int statusUserTel;

	// handle mGate telegrams
	struct rt_mutex lockGateTel;
	struct semaphore semGateTel;
	bool pendingGateTel;
	INT16U i16uCmdGateTel;
	INT8U i8uAddressGateTel;
	INT8U ai8uSendDataGateTel[MAX_TELEGRAM_DATA_SIZE];
	INT8U i8uSendDataLenGateTel;
	INT8U ai8uRecvDataGateTel[MAX_TELEGRAM_DATA_SIZE];
	INT16U i16uRecvDataLenGateTel;
	int statusGateTel;

	// piIO thread
	struct task_struct *pIoThread;
	struct hrtimer ioTimer;
	struct semaphore ioSem;
} SRevPiCore;

extern SRevPiCore piCore_g;

u8 revpi_core_find_gate(struct net_device *netdev, u16 module_type);
void revpi_core_gate_connected(SDevice *revpi_dev, bool connected);
int revpi_core_init(void);
void revpi_core_fini(void);
#endif /* _REVPI_CORE_H */
