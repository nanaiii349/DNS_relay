#include"globals.h"
#include"forward.h"
#include"parma.h"

int main(int carg,char **varg)//varg[0]固定为是程序执行的位置，varg[1]之后才是输入的参数
{
	color_out = GetStdHandle(STD_OUTPUT_HANDLE);//设置颜色的handel
	memset(rec_reading, -1, sizeof(rec_reading));//首先将所有的线程读置为负一
	memset(flags, -1, sizeof(flags));
	int is_set_parma_success = 1;//命令行参数输入是否正确
	for (int i = 1; i < carg; i++)//输入几个参数就循环几次
	{
		is_set_parma_success &= judge_and_set_parma(varg, i);//每次用varg数组中的下一个字符串设置参数
		//这里采用&=运算符，即每次设定都必须正确，若有一次设定不正确，则表示输入的命令有误，则报错
	}
	if (!is_set_parma_success)
	{
		printf("请输入正确的参数格式！如：\ndnsrelay [-d | -dd] [dns-server-ipaddr] [filename]");
		return 0;
	}
	split_line();
	set_color(4);//红色输出解析模式
	if (option == 1)
		printf("简易输出调试模式\n");
	else if (option == 2)
		printf("全输出调试模式\n");
	else
		printf("无输出调试模式\n");
	printf("缓存文件路径：%s\n", cache_file);
	printf("DNS服务器IP地址：%s\n", DNS_SERVER_IP);
	read_ip_domain();//拦截功能
	restore_color();
	split_line();
	//win平台下，socket编程的必须步骤，在linux下可省略
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata); //加载socket编程的动态dll

	//新建一udp socket，用于接收从win来的查询包
	local_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	//配置本地监听socket的信息
	local_socket_addr.sin_family = AF_INET;									   //af_inet表示走网络传送
	local_socket_addr.sin_port = htons(53);									   //固定监听本地dns查询端口53
	inet_pton(AF_INET, LOCAL_IP, &(local_socket_addr.sin_addr.S_un.S_addr));   //配置本地ip
	memset(local_socket_addr.sin_zero, 0, sizeof(local_socket_addr.sin_zero)); //置零
	if (bind(local_socket_fd, (SOCKADDR*)&local_socket_addr, sa_size) != 0)		//绑定socket与端口，实现recvfrom监听
	{
		printf("服务器无法绑定端口，请查看端口是否被占用,程序退出\n");
		exit(-1);
	}
	//配置连接dns服务器的socket的信息
	dns_server_addr.sin_family = AF_INET;										//af_inet表示走网络传送
	dns_server_addr.sin_port = htons(53);										//固定的dns服务器方的端口
	inet_pton(AF_INET, DNS_SERVER_IP, &(dns_server_addr.sin_addr.S_un.S_addr)); //配置dns服务器的ip
	memset(dns_server_addr.sin_zero, 0, sizeof(dns_server_addr.sin_zero));		//置零
	//注意，直接向dns服务器发信息不需要绑定端口

	local_socket_addr.sin_port = htons(0); //bind之后将本地ip和端口不再影响recvfrom，将端口改为0，即随机选取可用端口，方便下文转发使用
	unsigned char buf[512];
	SOCKADDR_IN localTemp;
	FILE* fp = fopen(cache_file, "rb");
	if (fp == NULL)
	{
		printf("无法打开缓存，将重写缓存\n");
	}
	else
	{
		getRec(fp);//启动程序时读取缓存
		fclose(fp);
	}
	while (1)
	{
		int flag = recvfrom(local_socket_fd, buf, 512, 0, (SOCKADDR*)&localTemp, &sa_size); //accpet和recvfrom的最后一个大小都是int*类型
		if (flag > 0)
		{
			buf_and_size temp;
			temp.buf = buf;
			temp.len = flag;
			temp.local = localTemp;
			while (thread_num > Max_Thread)Sleep(1);//最多允许同时运行max_thread个线程
			_beginthread(forward, 0, (void*)&temp);
			//printf("thread_num: %d\n", thread_num);
		}
	}
	WSACleanup();
}

//记录一下bind函数和udp与tcp的关系
//在tcp连接中，服务器端的socket需要bind指定的port和指定的ip，bind之后才能listen监听这个端口
//客户端一方不必要bind，可直接使用connect函数连接服务器的IP和port，这样做系统将会随机选择一个可用端口分配给客户端
//当然客户端也可以bind并指定一个port，这样会强制客户端使用该port通信
//在udp连接中，某socket不需要bind即可调用
