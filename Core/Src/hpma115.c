#include "hpma115.h"
#include "stm32f3xx_hal.h"
#include "aqi.h"
#include "string.h"

void processReceivedBytes(hpma_004_t *hpma);

void receiveBytes(uint8_t *byteBuffer, uint8_t bufferSize, hpma_004_t *hpma) {

	uint8_t byte;
	for (uint8_t i = 0; i < bufferSize; ++i) {
		byte = byteBuffer[i];

		switch (hpma->state) {
			case AWAIT_HEAD_H:
				if (byte == AUTO_HEAD_H) {
					hpma->buf[HEAD_H] = byte;
					hpma->state = AWAIT_HEAD_L;
				}
				break;
			case AWAIT_HEAD_L:
				if (byte == AUTO_HEAD_H) {
					// Got a second start byte in a row. This could happen
					// if the previous byte was the end of a checksum.
					// Remain in this state to see what happens.
				} else if (byte != AUTO_HEAD_L) {
					hpma->state = AWAIT_HEAD_H;
					// Failed to find the header signature. Start again.
					return;
				} else {
					hpma->buf[HEAD_L] = byte;
					hpma->bufIndex = HEAD_L + 1;
					hpma->state = RECEIVE_DATA;
				}
				break;
			case RECEIVE_DATA:
				// Fill the buffer up to the first checksum byte.
				hpma->buf[hpma->bufIndex++] = byte;
				if (hpma->bufIndex == CS_H) {
					hpma->state = AWAIT_CS_H;
				}
				break;
			case AWAIT_CS_H:
				hpma->buf[CS_H] = byte;
				hpma->state = AWAIT_CS_L;
				break;
			case AWAIT_CS_L:
				hpma->buf[CS_L] = byte;

				// We have accumulated a complete message.
				// Process it and reset our state to await the next message.
				processReceivedBytes(hpma);
				hpma->state = AWAIT_HEAD_H;
				break;
			default:
				// TODO: Signal error
				break;
		}
	}
}

void processReceivedBytes(hpma_004_t *hpma) {
	// Compute the checksum.
	uint32_t expected_sum = (hpma->buf[CS_H] << 8) + hpma->buf[CS_L];
	uint32_t actual_sum = 0;
	for (uint8_t i = 0; i < CS_H; ++i) {
		actual_sum += hpma->buf[i];
	}

	if (actual_sum != expected_sum) {
		return;
	}

	// Everything's great. We can report the readings.
	// Add the high and low bytes for each reading.
	hpma->pm1  = (hpma->buf[DATA0_H] << 8) + hpma->buf[DATA0_L];
	hpma->pm25 = (hpma->buf[DATA1_H] << 8) + hpma->buf[DATA1_L];
	hpma->pm4  = (hpma->buf[DATA2_H] << 8) + hpma->buf[DATA2_L];
	hpma->pm10 = (hpma->buf[DATA3_H] << 8) + hpma->buf[DATA3_L];

	// As a bonus, we an do the crazy AQI conversion and report that, too.
	uint32_t aqi25 = aqi_pm25(hpma->pm25);
	uint32_t aqi10 = aqi_pm10(hpma->pm10);

	// The worse pollutant determines the AQI. Because the EPA said so.
	hpma->aqi = (aqi25 > aqi10) ? aqi25 : aqi10;

	// Useful for debugging; updated only if data was successfully captured and parsed.
	hpma->lastRead = HAL_GetTick();
}
