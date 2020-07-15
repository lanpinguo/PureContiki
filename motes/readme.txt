1\
coap server 0 fe80::0212:4b00:1005:fdf1
coap server 0 fe80::0212:4b00:1940:c17a
coap server 1 fe80::0212:4b00:18f1:d9d2
coap server 2 fe80::0212:4b00:1940:c0e3

2\
./tunslip6 -v5 -s /dev/ttyACM0 fd00::1/64 &
./tunslip6 -s /dev/ttyACM0 fd00::1/64 &    
3\
wget -6 "http://[fd00::212:4b00:1005:fdf3]/"

4\
ping6 fd00::212:4b00:1005:fdf1
ping6 fd00::212:4b00:18f1:d9d2
ping6 fd00::212:4b00:1005:fdf3
ping6 fd00::212:4b00:1940:c08f
ping6 fd00::1  

5、
Compile version with using tsch as mac layer
/work/contiki/motes (tsch-debug *)$ make USING_TSCH=1 all

Compile version with using csma as mac layer
/work/contiki/motes (tsch-debug *)$ make USING_TSCH=1 all


/work/contiki/motes (tsch-debug *)$ make USING_TSCH=1 MAKE_WITH_SECURITY=1  all

/work/contiki/motes (tsch-debug *)$ make USING_TSCH=1 MAKE_WITH_SECURITY=1 OTA_ENABLE=1 all

6\
border-router-6lowpan.service
pure-bridge-agent.service
pure-web.service
systemctl status pure-bridge-agent

7\
	CONTIKI_MOD_NONE ,			/* 00 */
	CONTIKI_MOD_SLIP_BRG,		/* 01 */
	CONTIKI_MOD_RF ,			/* 02 */
	CONTIKI_MOD_RF_RX,			/* 03 */
	CONTIKI_MOD_RF_TX,			/* 04 */
	CONTIKI_MOD_MAC ,			/* 05 */	
	CONTIKI_MOD_LINK ,			/* 06 */	
	CONTIKI_MOD_ND6,			/* 07 */
	CONTIKI_MOD_DS6_ROUTE,		/* 08 */		
	CONTIKI_MOD_RPL,			/* 09 */
	CONTIKI_MOD_RPL_OF0,		/* 10 */
	CONTIKI_MOD_RPL_ICMP6,		/* 11 */
	CONTIKI_MOD_RPL_DAG,		/* 12 */
	CONTIKI_MOD_RPL_DAG_ROOT,	/* 13 */
	CONTIKI_MOD_DS6_NBR,		/* 14 */
	CONTIKI_MOD_SICS,			/* 15 */
	CONTIKI_MOD_NET ,			/* 16 */
	CONTIKI_MOD_IP6,			/* 17 */
	CONTIKI_MOD_TCP,			/* 18 */
	CONTIKI_MOD_TSCH,			/* 19 */
	CONTIKI_MOD_TSCH_SLOT,		/* 20 */
	CONTIKI_MOD_TSCH_LOG,		/* 21 */
	CONTIKI_MOD_CCM_STAR,		/* 22 */

