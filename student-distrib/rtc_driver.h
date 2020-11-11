/* rtc_driver.h - Real-time clock driver.
 * vim:ts=4 noexpandtab
 */
#include "interrupts.h"
#include "lib.h"
#include "filesystem_driver.h"

#define MAX_RTC_RATE 32768

extern int32_t rtc_open(const uint8_t * filename);
extern int32_t rtc_close(int32_t fd);
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t rtc_write(int32_t passed, const void* buf, int32_t nbytes);

extern void rtc_optable();
