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


extern void shell_default_output_telnet(const char *str1, int len1, const char *str2, int len2);
extern void shell_prompt_telnet(char *str);
extern void shell_exit_telnet(void);

extern void shell_default_output_serial(const char *text1, int len1, const char *text2, int len2);
extern void shell_prompt_serial(char *str);
extern void shell_exit_serial(void);


void
shell_default_output(const char *str1, int len1, const char *str2, int len2)
{
#if TELNET_ENABLE
	shell_default_output_telnet(str1, len1, str2, len2);
#endif
	shell_default_output_serial(str1, len1, str2, len2);
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


PROCESS_NAME(testcoffee_process);

/*---------------------------------------------------------------------------*/


PROCESS(shell_debug_process, "flash");
SHELL_COMMAND(pure_command,
	      "flash",
	      "flash dump: dump flash content)",
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
	}
	else if(strncmp(argv[0], "info", 4) == 0){
		show_system_info(3);
	}
	else if(strncmp(argv[0], "default", 7) == 0){
		int fd;
		int r;

		fd = cfs_open("default.json", CFS_WRITE | CFS_READ);
		if(fd < 0) {
			printf("open failed fd=[%d] \r\n",fd);	
			PROCESS_EXIT(); 
		}

		/* Write buffer. */
		r = cfs_write(fd, "hello this is cfs test!", 24);
		if(r < 0) {
			printf("write failed fd=[%d] \r\n",fd);	
			PROCESS_EXIT(); 
		}

	    cfs_seek(fd, 0, CFS_SEEK_SET);
		memset(buf,0,64);
		/* Read buffer. */
		r = cfs_read(fd, buf, 64);
		if(r < 0) {
			printf("read failed fd=[%d] \r\n",fd);	
			PROCESS_EXIT(); 
		}
		
		printf("read  fd=[%d] size: %d \r\n",fd, r);	
		buffer_dump((uint8_t*)buf, r);
		
		cfs_close(fd);

	}
	else if(strncmp(argv[0], "flash", 5) == 0){
		/* Erase 3 sectors */
		//xmem_erase(6 * 512, 0);
		/* Erase external flash pages */
		xmem_erase(6 * 1024, 0);

		for(i = 1 ; i < 6 * 512; i++){
			buf[0] = 0x55;
			xmem_pwrite(buf, 1, i - 1);
		}

		memset(buf,0,64);
		for(i = 0 ; i < 6 * 512; ){
			if(6 * 512 - i < 64){
				xmem_pread(buf, size - i, i );
				buffer_dump((uint8_t*)buf, size - i);
				i += (6 * 512 -i);
				if(6 * 512 - i <= 0){
					break;
				}
			}
			else{
				xmem_pread(buf, 64, i );
				buffer_dump((uint8_t*)buf,64);
				i += 64;
			}
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


	}



	
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS(shell_dbg_switch_process, "debug switch");
SHELL_COMMAND(dbg_sw_command,
		"debug",
		"debug [enable|disable] [module]: turn on/off the debug info of module",
		&shell_dbg_switch_process);

PROCESS_THREAD(shell_dbg_switch_process, ev, data)
{
	static char* argv[5];
	static int argc;
	static int enable = 0;
	static int mod_start = 0;
	static int mod_end = 0;
	static int line_start = 0;
	static int line_end = 0;
	
	
	PROCESS_BEGIN();
	
	if(data != NULL) {
		argc = str_split((char*)data,(char*)" ",argv,5);
		/*printf("\r\ncoap client cli [%d] \r\n",argc);	*/

		if(strncmp(argv[0], "line", 2) == 0) {
			if(argc == 2){
				sscanf(argv[1],"%d-%d",&line_start,&line_end);
				enable = -1;
				mod_start = -1;
				mod_end = -1;
				/*line_start = -1;*/
				/*line_end = -1;*/
			}
			else{
				goto ERROR;
			}
			
			
		} 
		else if(strncmp(argv[0], "mod", 3) == 0) {
			if(argc == 2){
				sscanf(argv[1],"%d-%d",&mod_start,&mod_end);
				enable = -1;
				/*mod_start = -1;*/
				/*mod_end = -1;*/
				line_start = -1;
				line_end = -1;
			}
			else{
				goto ERROR;
			}
		}
		else if(strncmp(argv[0], "enable", 3) == 0) {
				enable = 1;
				mod_start = -1;
				mod_end = -1;
				line_start = -1;
				line_end = -1;
		}
		else if(strncmp(argv[0], "disable", 3) == 0) {
				enable = 0;
				mod_start = -1;
				mod_end = -1;
				line_start = -1;
				line_end = -1;
		}
		else{
			goto ERROR;
		}


	}

	trace_print_filter_set(enable,mod_start,mod_end,line_start,line_end);
	goto DONE;
	
ERROR:
	printf("\r\nParameter Error !!\r\n");	
    PROCESS_EXIT();
DONE:	
	printf("\r\ntoggle module debug switch\r\n"
			"{enable:%d,mod_start:%d,mod_end:%d,line_start:%d,line_end:%d}\r\n",
			enable,mod_start,mod_end,line_start,line_end);	
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
	char buf[] = "hello world";

	PROCESS_BEGIN();

	PROCESS_PAUSE();
	
	uip_ds6_nbr_dump();
	rpl_print_neighbor_list();	
	shell_default_output(buf,strlen(buf),NULL,0);
	
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





