#include"globals.h"

char LOCAL_IP[20] = "";//根据本机IP配置
char DNS_SERVER_IP[20] = "";
int local_socket_fd;
int sa_size = sizeof(SOCKADDR);

int file_in_use = 0, thread_num = 0;

int option = 0;//输出级别，0为不输出，1为精简输出，2为全输出

int option_is_set = 0, cache_file_is_set = 0, dns_server_ip_is_set = 0;

REPLY_DATA rec[MAX_REC];

int rec_num = 0, rec_changed = 0;//changed是文件中记录修改数，超过5则重读文件  

int rec_writing = 0, rec_reading[Max_Thread];//线程同步rec是否在读或写
//读和写的区别在于，写只能同时有一个线程在写，读可以同时很多线程一起读
//有线程读时不能写，有线程写时不能读

int output_in_use = 0, sys_color = 15;//调试输出的线程锁,系统输出颜色，默认15为白

HANDLE color_out;//控制系统输出颜色的handle

char cache_file[512] = "dnsrelay.txt";

SOCKADDR_IN local_socket_addr, dns_server_addr; //全局变量，在每个线程都会用到

int ip_domain_num=0;//ip域名对照表的个数
IP_DOMAIN ip_domain[100];//IP域名对照表

int color_in_use;//命令行颜色修改的线程同步

int flags[Max_Thread];

int times = 0;

void set_color(int i)//为修改颜色设计线程同步
{
	while (color_in_use)Sleep(1);
	color_in_use = 1;
	SetConsoleTextAttribute(color_out, i);
}
void restore_color()
{
	SetConsoleTextAttribute(color_out, sys_color);
	color_in_use = 0;
}

