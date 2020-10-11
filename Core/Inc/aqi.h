#ifndef __AQI_H
#define __AQI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Return the AQI according the the PM2.5 count. */
uint32_t aqi_pm25(uint16_t c_pm25);

/* Return the AQI according to the PM10 count. */
uint32_t aqi_pm10(uint16_t c_pm10);

#endif  // __AQI_H
