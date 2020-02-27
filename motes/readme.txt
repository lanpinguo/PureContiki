1\
coap server 0 fe80::0212:4b00:1005:fdf1
coap server 0 fe80::0212:4b00:1940:c17a
coap server 1 fe80::0212:4b00:18f1:d9d2
coap server 2 fe80::0212:4b00:1940:c0e3

2\
./tunslip6 -v5 -s /dev/ttyACM0 fd00::1/64
./tunslip6 -s /dev/ttyACM0 fd00::1/64     
3\
wget -6 "http://[fd00::212:4b00:1005:fdf3]/"

4\
ping6 fd00::212:4b00:1005:fdf1
ping6 fd00::212:4b00:18f1:d9d2
ping6 fd00::212:4b00:1005:fdf3
ping6 fd00::1  

5、
Compile version with using tsch as mac layer
/work/contiki/motes (tsch-debug *)$ make USING_TSCH=1 all

Compile version with using csma as mac layer
/work/contiki/motes (tsch-debug *)$ make USING_TSCH=1 all

