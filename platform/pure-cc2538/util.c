#include "contiki.h"

#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "cfs/cfs.h"
#include "net/netstack.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "crc32.h"

#include "xmem.h"
#include "dbg.h"
#include "util.h"
#include "shell.h"
#include "serial-shell.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "i2c.h"

#define DEBUG DEBUG_PRINT
#define MODULE_ID CONTIKI_MOD_NONE
#include "net/ip/uip-debug.h"


extern int show_system_info(uint32_t mode);
extern int log_system_reset();


extern void shell_default_output_telnet(const char *str1, int len1, const char *str2, int len2);
extern void shell_prompt_telnet(char *str);
extern void shell_exit_telnet(void);

extern void shell_raw_output_serial(const char *text1, int len1, const char *text2, int len2);
extern void shell_default_output_serial(const char *text1, int len1, const char *text2, int len2);
extern void shell_prompt_serial(char *str);
extern void shell_exit_serial(void);

extern uint32_t xmemExist(void);


static int log_fd = -1;


void
shell_default_output(const char *str1, int len1, const char *str2, int len2)
{
#if TELNET_ENABLE
	shell_default_output_telnet(str1, len1, str2, len2);
#endif
	shell_default_output_serial(str1, len1, str2, len2);
}

void
shell_raw_output(const char *str1, int len1, const char *str2, int len2)
{
#if TELNET_ENABLE
	shell_default_output_telnet(str1, len1, str2, len2);
#endif
	shell_raw_output_serial(str1, len1, str2, len2);
}

/*---------------------------------------------------------------------------*/
void
shell_exit(void)
{
#if TELNET_ENABLE
	shell_exit_telnet();
#endif
	shell_exit_serial();
}

void
shell_prompt(char *str)
{
#if TELNET_ENABLE
	shell_prompt_telnet(str);
#endif
	shell_prompt_serial(str);
}

static unsigned int 
log_output_file(void *user_data, const char *data, unsigned int len)
{
	if (len > 0 && log_fd >= 0){
		cfs_write(log_fd, data, len);
	} 
	return 0;
}

static unsigned int 
log_output_terminal(void *user_data, const char *data, unsigned int len)
{
	if (len > 0) dbg_send_bytes((unsigned char*)data, len);
	return 0;
}

int log_system_init()
{
	if(xmemExist()){
		log_fd = cfs_open("running.log", CFS_WRITE);
	}
	
	if(log_fd >= 0){
		log_system_std_out_set(UTIL_LOG_OUT_FILE);
	}
	else{
		printf("cfs open running.log failed, switch to terminal \r\n");
		log_system_std_out_set(UTIL_LOG_OUT_TERMIANL);
		return -1;
	}

	return 0;
}


int log_system_reset()
{
	trace_output_terminal_set(NULL);
	cfs_close(log_fd);
	log_fd = -1;
	cfs_remove("running.log");

	return 0;
}


int log_system_std_out_set(int mode)
{

	if(mode == UTIL_LOG_OUT_FILE){
		if(log_fd >= 0){
			trace_output_terminal_set(log_output_file);
		}
	}
	else{
		trace_output_terminal_set(log_output_terminal);
	}

	return 0;
}

PROCESS_NAME(testcoffee_process);

/*---------------------------------------------------------------------------*/


PROCESS(shell_debug_process, "util");
SHELL_COMMAND(pure_command,
	      "util",
	      "general debug tools",
	      &shell_debug_process);


/*---------------------------------------------------------------------------*/


PROCESS_THREAD(shell_debug_process, ev, data)
{
	char* argv[5];
	int argc;
	static int i = 0;
	static	uint32_t addr,size;
	int rc;
	static uint8_t buf[64]; 
	
	PROCESS_BEGIN();
	
	if(data == NULL) {
		PROCESS_EXIT(); 
	}
	argc = str_split((char*)data,(char*)" ",argv,5);
	/*printf("\r\ncoap client cli [%d] \r\n",argc);	*/
	(void)argc;
	if(strncmp(argv[0], "test", 4) == 0) {
		process_start(&testcoffee_process,NULL);
		PROCESS_EXIT(); 
	}
	else if(strncmp(argv[0], "info", 4) == 0){
		show_system_info(3);
		PROCESS_EXIT(); 
	}
	else if(strncmp(argv[0], "log", 3) == 0){

		if(argc >= 2){

			if(strncmp(argv[1], "reset", 5) == 0){
				log_system_reset();
				printf("reset log system done \r\n");	
			}
			else if(strncmp(argv[1], "start", 5) == 0){
				log_system_init();
				printf("start log system done \r\n");	
			}
			else if(strncmp(argv[1], "terminal", 3) == 0){
				log_system_std_out_set(UTIL_LOG_OUT_TERMIANL);
				printf("output to terminal \r\n");	
			}
			else if(strncmp(argv[1], "file", 4) == 0){
				log_system_std_out_set(UTIL_LOG_OUT_FILE);
				printf("output to log file \r\n");	
			}
			PROCESS_EXIT(); 
		}
		else{
			goto UNKNOWN_CMD;
		}
	
	}
	else if(strncmp(argv[0], "erase", 5) == 0){
		if(argc == 3){
			sscanf(argv[1],"%lu", &addr);
			sscanf(argv[2],"%lu", &size);
		}
		/* Erase external flash pages */
		rc = xmem_erase(size * XMEM_ERASE_UNIT_SIZE, addr * XMEM_ERASE_UNIT_SIZE);
		if(rc < 0){
			printf("\r\nxmem erase failed");	
		}
		else{
			printf("\r\nxmem erase %u bytes from address 0x%lx done \r\n",
					rc, addr*1024);	
		}
		PROCESS_EXIT(); 

	}
	else if(strncmp(argv[0], "dump", 4) == 0){
		
		if(argc == 3){
			sscanf(argv[1],"%lu", &addr);
			sscanf(argv[2],"%lu", &size);
		}
		printf("\r\ndump at addr 0x%lx, size %lu bytes:  \r\n",addr,size);
		for(i = 0 ; i < size;  ){
			if(size - i < 64){
				xmem_pread(buf, size - i, addr + i );
				buffer_dump((uint8_t*)buf, size - i);
				i += (size -i);
			}
			else{
				xmem_pread(buf, 64, addr + i );
				buffer_dump((uint8_t*)buf,64);
				i += 64;
			}
		}

		PROCESS_EXIT(); 

	}

UNKNOWN_CMD:
	
	printf("unknown command\r\n");

	
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
int mod_map_parse(char * in, uint32_t * map)
{
	char * argv[32] = {NULL};
	int argc;
	int i,j,st=0,se=0,rc;

	
	
	argc = str_split(in, ",", argv, 31);
	/*printf("count:%d\r\n",argc);*/
	for(i = 0 ; i < argc; i++)
	{
		/*printf("%s\r\n",argv[i]);*/
		rc = sscanf(argv[i], "%u-%u", &st,&se);
		if(rc < 2){
			rc = sscanf(argv[i], "%u", &st);
			if(rc < 1){
				return -1;
			}
			if(st < 32){
				map[0] |= (1 << st);
			}
			else{
				map[1] |= (1 << (st % 32));
			}
		}
		else{
			if(se < st){
				return -1;
			}

			for(j = st; j <= se ; j++){
				if(j < 32){
					map[0] |= (1 << j);
				}
				else{
					map[1] |= (1 << (j % 32));
				}
			}
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------*/

PROCESS(shell_dbg_switch_process, "debug switch");
SHELL_COMMAND(dbg_sw_command,
		"debug",
		"debug [enable|disable] [module]: turn on/off the debug info of module",
		&shell_dbg_switch_process);

PROCESS_THREAD(shell_dbg_switch_process, ev, data)
{
	char* argv[5];
	int argc;
	int line_start = -1;
	int line_end = -1;
	static uint32_t mod_map[2] = {0};

	
	
	PROCESS_BEGIN();
	
	if(data != NULL) {
		argc = str_split((char*)data,(char*)" ",argv,5);
		/*printf("\r\ncoap client cli [%d] \r\n",argc);	*/

		if(strncmp(argv[0], "line", 2) == 0) {
			if(argc == 2){
				sscanf(argv[1],"%d-%d",&line_start,&line_end);
				trace_print_filter_set(-1,NULL,line_start,line_end);
			}
			else{
				goto ERROR;
			}
			
			
		} 
		else if(strncmp(argv[0], "mod", 3) == 0) {
			if(argc == 2){
				memset(mod_map,0,sizeof(mod_map));
				if(mod_map_parse(argv[1],mod_map) < 0){
					goto ERROR;
				}	
				trace_print_filter_set(-1,mod_map,-1,-1);
			}
			else{
				goto ERROR;
			}
		}
		else if(strncmp(argv[0], "enable", 3) == 0) {
			trace_print_filter_set(1,NULL,-1,-1);
		}
		else if(strncmp(argv[0], "disable", 3) == 0) {
			trace_print_filter_set(0,NULL,-1,-1);
		}
		else{
			goto ERROR;
		}


	}

	goto DONE;
	
ERROR:
	printf("\r\nParameter Error !!\r\n");	
    PROCESS_EXIT();
DONE:	
	printf("\r\ntoggle module debug switch\r\n"
			"module map: %08lx%08lx, line range : %d ~ %d\r\n",
			mod_map[1],mod_map[0],line_start,line_end);	
	PROCESS_END();
}


PROCESS(shell_list_neighbor_process, "list rpl neighbor");
SHELL_COMMAND(list_neighbor_command,
	      "lsnb",
	      "lsnb: list rpl neighbors",
	      &shell_list_neighbor_process);


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(shell_list_neighbor_process, ev, data)
{

	PROCESS_BEGIN();

	PROCESS_PAUSE();
	
	uip_ds6_nbr_dump();
	rpl_print_neighbor_list();	
	
	PROCESS_END();
}


/*---------------------------------------------------------------------------*/


void
shell_pure_init(void)
{
  shell_register_command(&pure_command);
  shell_register_command(&list_neighbor_command);
  shell_register_command(&dbg_sw_command);
}





