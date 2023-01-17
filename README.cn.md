# Pure Contiki
基于Contiki开源仓库，扩展添加了pure-cc2538硬件平台。该硬件平台支持智能开关、传感器、6lowpan网关等子类型。

#### 描述
pure-cc2538平台的产品均在contiki/motes目录下；

完全开发了一套基于6lowan的OTA升级算法，支持节点的无线升级；
使用中可以根据需求，选择不同的编译类型以裁减无线升级（OTA）、跳频（TSCH）和数据加密等功能；

#### 编译方法：
进入产品目录下：
cd contiki/motes 
1、通用编译，不带无线升级（OTA）、跳频（TSCH）和数据加密等功能:
$ make all

2、增强编译，支持跳频:
$ make USING_TSCH=1 all

3、增强编译，支持跳频和数据加密:
$ make USING_TSCH=1 MAKE_WITH_SECURITY=1 all

4、增强编译，支持跳频、数据加密和无线升级：
$ make USING_TSCH=1 MAKE_WITH_SECURITY=1 OTA_ENABLE=1 all


#### 烧写方法：
1、普通版本，不支持无线升级：
使用TI的flash烧写工具 Flash Programmer 2；
选择串口连接的芯片类型为CC2538F53;
选择好目的镜像文件（*.hex）
然后执行 擦除，编程和校验；

2、增强版本，支持无线升级：

首先，芯片整片擦除；
然后，选择bootloader.hex文件，执行编程和校验；
其次，选择目的镜像文件，执行编程和校验；



####部署方法：

1、首先，需要在网关节点上配置6lowpan隧道，模拟以太端口：
	./tunslip6 -s /dev/ttyACM0 fd00::1/64 &    
tunslip6工具由 Contiki/tools下的源码编译得到；

2、启动border server：



