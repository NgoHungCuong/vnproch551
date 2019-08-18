#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include "KT_BinIO.h"
#include "KT_ProgressBar.h"

KT_BinIO ktFlash;

uint8_t u8Buff[64];
uint8_t u8Mask[8];

/* Detect MCU */
uint8_t u8DetectCmd[64] = {
	0xA1, 0x12, 0x00, 0x51, 0x11, 0x4D, 0x43, 0x55,
	0x20, 0x49, 0x53, 0x50, 0x20, 0x26, 0x20, 0x57,
	0x43, 0x48, 0x2e, 0x43, 0x4e
};
uint8_t u8DetectRespond = 6;

/* Get Bootloader Version, Chip ID */
uint8_t u8IdCmd[64] = {
	0xA7, 0x02, 0x00, 0x1F, 0x00
};
uint8_t u8IdRespond = 30;

/* Enable ISP */
uint8_t u8InitCmd[64] = {
	0xA8, 0x0E, 0x00, 0x07, 0x00, 0xFF, 0xFF, 0xFF,
	0xFF, 0x03, 0x00, 0x00, 0x00, 0xFF, 0x52, 0x00,
	0x00
};
uint8_t u8InitRespond = 6;

/* Set Flash Address */
uint8_t u8AddessCmd[64] = {
	0xA3, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00
};
uint8_t u8AddessRespond = 6;

/* Erase ??? */
uint8_t u8EraseCmd[64] = {
	0xA4, 0x01, 0x00, 0x08
};
uint8_t u8EraseRespond = 6;

/* Reset */
uint8_t u8ResetCmd[64] = {
	0xA2, 0x01, 0x00, 0x01 /* if 0x00 not run, 0x01 run*/
};
uint8_t u8ResetRespond = 6;

/* Write */
uint8_t u8WriteCmd[64] = {
	0xA5, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	/* byte 4 Low Address (first = 1) */
	/* byte 5 High Address */
};
uint8_t u8WriteRespond = 6;

uint8_t u8ReadCmd[64] = {
	0x00
};
uint8_t u8ReadRespond = 6;

libusb_device_handle *h;

uint32_t Write(uint8_t *p8Buff, uint8_t u8Length);
uint32_t Read(uint8_t *p8Buff, uint8_t u8Length);

uint32_t Write(uint8_t *p8Buff, uint8_t u8Length)
{
	int len;
	if (libusb_bulk_transfer(h, 0x02, (unsigned char*)p8Buff, u8Length, &len, 5000) != 0) {
		return 0;
	} else {
		return 1;
	}
	return 0;
}
uint32_t Read(uint8_t *p8Buff, uint8_t u8Length)
{
	int len;
	if (libusb_bulk_transfer(h, 0x82, (unsigned char*)p8Buff, u8Length, &len, 5000) != 0) {
		return 0;
	} else {
		return 1;
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	uint32_t i;
	KT_BinIO ktBin;
	KT_ProgressBar ktProg;

	printf("\033[2J\033[1;1H");
	printf("------------------------------------------------------------------\n");
	printf("CH551 Programmer by VNPro\n");
	printf("------------------------------------------------------------------\n");
	if (argc != 2) {
		printf("usage: vnproch551 flash_file.bin\n");
		printf("------------------------------------------------------------------\n");
        return 1;
	}
    /* load flash file */
	ktBin.u32Size = 10 * 1024;
	ktBin.InitBuffer();
	if (!ktBin.Read((char*)argv[1])) {
		printf("Read file: ERROR\n");
		return 0;
	}

	libusb_init(NULL);
	
	h = libusb_open_device_with_vid_pid(NULL, 0x4348, 0x55e0);

	if (h == NULL) {
		printf("Found no CH551 USB\n");
		return 1;
	}
	
	libusb_claim_interface(h, 0);
	
	/* Detect MCU */
	if (!Write(u8DetectCmd, u8DetectCmd[1] + 3)) {
		printf("Send Detect: Fail\n");
		return 1;
	}

	if (!Read(u8Buff, u8DetectRespond)) {
		printf("Read Detect: Fail\n");
		return 1;
	}

	/* Check MCU ID */
	if ((u8Buff[4] != 0x51) && (u8Buff[5] != 0x11)) {
		printf("Not support\n");
		return 1;
	}
	
	/* Bootloader and Chip ID */
	if (!Write(u8IdCmd, u8IdCmd[1] + 3)) {
		printf("Send ID: Fail\n");
		return 1;
	}
	
	if (!Read(u8Buff, u8IdRespond)) {
		printf("Read ID: Fail\n");
		return 1;
	}
	
	printf("Bootloader: %d.%d.%d\n", u8Buff[19], u8Buff[20], u8Buff[21]);
	printf("ID: %02X %02X %02X %02X\n", u8Buff[22], u8Buff[23], u8Buff[24], u8Buff[25]);
	/* check bootloader version */
	if ((u8Buff[19] != 0x02) || (u8Buff[20] != 0x03) || (u8Buff[21] != 0x01)) {
		printf("Not support\n");
		return 1;
	}
	/* Calc XOR Mask */

	uint8_t u8Sum;

	u8Sum = u8Buff[22] + u8Buff[23] + u8Buff[24] + u8Buff[25];
	for (i = 0; i < 8; ++i) {
		u8Mask[i] = u8Sum;
	}
	u8Mask[7] += 0x51;
	printf("XOR Mask: ");
	for (i = 0; i < 8; ++i) {
		printf("%02X ", u8Mask[i]);
	}
	printf("\n");

	/* init or erase ??? */
	if (!Write(u8InitCmd, u8InitCmd[1] + 3)) {
		printf("Send Init: Fail\n");
		return 1;
	}
	
	if (!Read(u8Buff, u8InitRespond)) {
		printf("Read Init: Fail\n");
		return 1;
	}

	/* Bootloader and Chip ID */
	if (!Write(u8IdCmd, u8IdCmd[1] + 3)) {
		printf("Send ID: Fail\n");
		return 1;
	}
	
	if (!Read(u8Buff, u8IdRespond)) {
		printf("Read ID: Fail\n");
		return 1;
	}

	/* Set Flash Address to 0 */
	if (!Write(u8AddessCmd, u8AddessCmd[1] + 3)) {
		printf("Send Address: Fail\n");
		return 1;
	}
	
	if (!Read(u8Buff, u8AddessRespond)) {
		printf("Read Address: Fail\n");
		return 1;
	}

	/* Erase or unknow */
	if (!Write(u8EraseCmd, u8EraseCmd[1] + 3)) {
		printf("Send Erase: Fail\n");
		return 1;
	}
	
	if (!Read(u8Buff, u8EraseRespond)) {
		printf("Read Erase: Fail\n");
		return 1;
	}
	
	/* Write */
	printf("Write\n");
	/* Progress */
	uint32_t n;
	n = 10 * 1024 / 56;
	ktProg.SetMax(n);
	ktProg.SetNum(50);
	ktProg.SetPos(0);
	ktProg.Display();

	for (i = 0; i < n; ++i) {
		uint16_t u16Tmp;
		uint32_t j;
		/* Write flash */
		memmove(&u8WriteCmd[8], &ktBin.pReadBuff[i * 0x38], 0x38);
		for (j = 0; j < 7; ++j) {
			uint32_t ii;
			for (ii = 0; ii < 8; ++ii) {
				u8WriteCmd[8 + j * 8 + ii] ^= u8Mask[ii];
			}
		}
		u16Tmp = i * 0x38;
		u8WriteCmd[3] = (uint8_t)u16Tmp;
		u8WriteCmd[4] = (uint8_t)(u16Tmp >> 8);
		if (!Write(u8WriteCmd, u8WriteCmd[1] + 3)) {
			printf("Send Write: Fail\n");
			return 1;
		}
		
		if (!Read(u8Buff, u8WriteRespond)) {
			printf("Read Write: Fail\n");
			return 1;
		}
		ktProg.SetPos(i + 1);
		ktProg.Display();
	}

	/* Reset and Run */
	Write(u8ResetCmd, u8ResetCmd[1] + 3);
		//printf("Send Reset: Fail\n");
		//return 1;
	//}
	printf("\n");
	printf("Write complete!!!\n");
	printf("------------------------------------------------------------------\n");

	return 0;
}