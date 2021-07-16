#pragma once
#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <Windows.h> //这个头不能放在winsock2那个头前面，不然会报错
#include <WS2tcpip.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#pragma comment(lib, "ws2_32.lib") //加载这个库才能使用socket

#define MAX_REC 10000
#define Max_Thread 5

typedef struct buf_and_size //用于多线程传多个参数时的结构体
{
	unsigned char* buf;
	int len;
	SOCKADDR_IN local;
} buf_and_size;

typedef struct
{
	UINT16 ID;	 //由客户程序设置并由服务器返回结果。客户程序通过它来确定响应与查询是否匹配
	UINT8 QR;	 //0 表示查询报，1 1 表示响应报
	UINT8 RCODE; //响应码 (Response coded) ，仅用于响应报. 值为 0( 没有差错),值为3 表示名字差错。从权威名字服务器返回，表示在查询中指定域名不存在
	//UINT16 NAME; //资源记录部分名字的指针
	char NAME[256];
	char QNAME[256]; //问题部分名字
	char RDATA[256];
	UINT16 QTYPE;
	UINT16 TYPE;
	UINT16 QCLASS;
	UINT16 CLASS;
	UINT32 TTL;
	int RDLENGTH;
	long TIME;
	/* data */
} Answer;
//所以需要从里面提四个参数domainname，type，包信息，ttl
typedef struct
{
	UINT16 QTYPE;
	char query_type[2];
	int ttl;
	long time_stamp;
	int len;
	int domain_len;
	char domain_name[255];
	char packet[512];

} REPLY_DATA;

typedef struct
{
	char ip[20];//从ip-域名对照表中读取的ip
	char domain_name[256];//从ip-域名对照表中读取的域名
}IP_DOMAIN;

extern char LOCAL_IP[20];//本地IP地址
extern char DNS_SERVER_IP[20];//远程dns服务器地址

extern char cache_file[512];//缓存文件路径

extern int local_socket_fd;//用于监听本地win向中继器发出dns请求包的socket

extern int sa_size ;//结构体大小

extern int thread_num;//全局总线程数

extern int option;//输出级别，0为不输出，1为精简输出，2为全输出

extern int option_is_set;//命令行输入的参数是否让option设置成功
extern int cache_file_is_set;//命令行输入的参数是否让cache的路径设置成功
extern int dns_server_ip_is_set;//命令行设置的参数是否让dns服务器的ip设置成功

extern REPLY_DATA rec[MAX_REC];//常驻内存中的储存所有记录的数组
extern int rec_num;//总记录数

extern int rec_changed;//changed是文件中记录修改数，超过5则将常驻缓存的记录写入文件中长时间保存文件  

extern int file_in_use;//线程同步锁，表示当前缓存文件是否在正在被使用（写）
extern int rec_writing;//线程同步锁
extern int rec_reading[Max_Thread];//线程同步锁，表示rec是否在读或写
//读和写的区别在于，写只能同时有一个线程在写，读可以同时很多线程一起读
//有线程读时不能写，有线程写时不能读
extern int output_in_use;//线程同步锁，表示当前命令行是否有线程正在输出

extern int sys_color;//调试输出的线程锁,系统输出颜色，默认15为白
extern HANDLE color_out;//控制系统输出颜色的handle
extern SOCKADDR_IN local_socket_addr, dns_server_addr; //每个线程都会向

extern int ip_domain_num;//ip域名对照表的个数
extern IP_DOMAIN ip_domain[100];//IP域名对照表

extern int color_in_use;//命令行颜色修改的线程同步

extern flags[Max_Thread];

extern times;

void set_color(int i);//为修改颜色设计线程同步
void restore_color();