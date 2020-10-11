#ifndef __HPMA115_H
#define __HPMA115_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Byte sequence for the autosend data format from the compact series,
 * HPMA115C0-003 and HPMA115C0-004. (Table 7 in the Honeywell data sheet.)
 *
 * Index of bytes, omitting the two header bytes.
 */
enum {
	HEAD_H = 0,
	HEAD_L,
    LEN_H,
    LEN_L,
    DATA0_H,
    DATA0_L,
    DATA1_H,
    DATA1_L,
    DATA2_H,
    DATA2_L,
    DATA3_H,
    DATA3_L,
    CS_H = 30,
    CS_L,
    AUTO_SEND_DATA_BYTES,
};

enum {
	AUTO_HEAD_H = 0x42,  // First byte of auto-send data header.
	AUTO_HEAD_L = 0x4D,  // Second byte of auto-send data header.
	CMD_HEAD    = 0x68,  // Header byte of a command.
};

typedef enum {
	AWAIT_HEAD_H,
	AWAIT_HEAD_L,
	RECEIVE_DATA,
	AWAIT_CS_H,
	AWAIT_CS_L
} state_t;

/*
 * The compact sensor.
 */
typedef struct {
	uint32_t lastRead;
	uint16_t pm1;
	uint16_t pm25;
	uint16_t pm4;
	uint16_t pm10;
	uint16_t aqi;
	uint16_t checksum;
	uint8_t bufIndex;
	uint8_t buf[AUTO_SEND_DATA_BYTES];
	state_t state;
} hpma_004_t;

void receiveBytes(uint8_t *byteBuffer, uint8_t bufferSize, hpma_004_t *hpma);

#endif  // __HPMA115_H
