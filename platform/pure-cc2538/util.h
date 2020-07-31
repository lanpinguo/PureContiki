
#ifndef _UTIL_H_
#define _UTIL_H_


typedef enum {
	UTIL_LOG_OUT_TERMIANL = 1,
	UTIL_LOG_OUT_FILE,
	
}UTIL_LOG_OUT_MODE_e;

void
shell_pure_init(void);
int log_system_init();
int log_system_reset();

#endif /*_UTIL_H_*/


