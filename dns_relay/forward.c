#include"forward.h"

void forward(void* arg)
{
	int is_over_rec = 0;
	thread_num++;//新开线程，线程数自增
	int No_x_thread;
	for (int i = 0; i < Max_Thread; i++)
	{
		if (rec_reading[i] == -1)
		{
			No_x_thread = i;//给该线程分配一个当前没有使用的线程号
			rec_reading[No_x_thread] = 0;
			break;
		}
	}
	//("当前线程数为：%d\n", thread_num);
	//新建一个本地用于和服务器连接的socket
	int local_forward_fd;
	local_forward_fd = socket(AF_INET, SOCK_DGRAM, 0);

	//利用刚才主线程里面的修改过的local地址，将这个UDP socket绑定一个为0的端口号，即系统随机选择，保证不会重复
	bind(local_forward_fd, (SOCKADDR*)&local_socket_addr, sa_size);

	//从主函数传进来的那个等会需要返回到win系统的去的地址（主要是port）
	SOCKADDR_IN local_back_addr = ((buf_and_size*)arg)->local;

	//准备select，多路同步I/O
	struct timeval timout = { 2, 0 }; //设置select超时时间，分别为秒，毫秒；
	struct fd_set fds;				//创建一个即将被select的fd的集合
	int maxfd = 0;					//这个值必须为集合中最大的fd的值加一，在这里集合中只有一个fd，即local_forward_fd

	//从主函数中传进来的包信息，先提取到该函数的局部变量data_to_send中
	REPLY_DATA data_to_send, reply;
	int len = ((buf_and_size*)arg)->len;
	for (int i = 0; i < len; i++)
	{
		data_to_send.packet[i] = ((buf_and_size*)arg)->buf[i];
	}
	data_to_send.len = len;
	get_domain_and_type(&data_to_send);
	char readable_domain[256];
	int temp_int = 12;
	int num = 0;
	getNAME(data_to_send.packet, data_to_send.len, &temp_int, readable_domain, 256, &num);
	//printf("\n\n readable_domain:%s \n\n\n", readable_domain);
	//data_to_send.packet是请求报

	/*
	char requset_temp_domain[256];//如果有www.去掉，方便一次性比较

	if (readable_domain[0] == readable_domain[1] && readable_domain[1] == readable_domain[2] \
		&& readable_domain[2] == 'w' && readable_domain[3] == '.')
	{
		int i, j;
		for (i = 0, j = 4; readable_domain[j] != '\0'; i++, j++)//去掉www.
		{
			requset_temp_domain[i] = readable_domain[j];
		}
		requset_temp_domain[i] = readable_domain[j];
	}
	else strcpy(requset_temp_domain, readable_domain);
	//*/

	//新加拦截功能，获取到查询包中的domian之后，在ip-domian对照表中查询是否属于需要拦截的对像
	for (int i = 0; i < ip_domain_num; i++)
	{
		if (strcmp(readable_domain, ip_domain[i].domain_name) == 0)  //||data_to_send.QTYPE==1)
		{
			if (strcmp("0.0.0.0", ip_domain[i].ip) == 0)
			{
				data_to_send.packet[3] = 0x83;

			}
			else if(data_to_send.QTYPE==0x1c)
			{
				break;
			}
			else
			{
				flags[No_x_thread] = i;
			}
			Answer answer;
			int pos = 12;

			//构造响应包
			data_to_send.packet[2] = 0x81;

			data_to_send.packet[6] = 0x00;
			data_to_send.packet[7] = 0x01;

			questionExtract(data_to_send.packet, data_to_send.len, &pos, &answer);
			data_to_send.packet[pos++] = 0xc0;
			data_to_send.packet[pos++] = 0x0c;

			data_to_send.packet[pos++] = 0x00;
			data_to_send.packet[pos++] = 0x01;
			
			data_to_send.packet[pos++] = 0x00;
			data_to_send.packet[pos++] = 0x01;
			
			data_to_send.packet[pos++] = 0x00;
			data_to_send.packet[pos++] = 0x00;
			data_to_send.packet[pos++] = 0xc0;
			data_to_send.packet[pos++] = 0x02;

			data_to_send.packet[pos++] = 0x00;
			data_to_send.packet[pos++] = 0x04;

			data_to_send.len = pos +4;

			char temp_ip[20];
			strcpy(temp_ip, ip_domain[flags[No_x_thread]].ip);
			char* ch;
			ch = strtok(temp_ip, ".");
			while (ch != NULL)
			{
				data_to_send.packet[pos++] = atoi(ch);
				//printf("\n++++++++ %c +++++++++++\n", atoi(ch));
				ch = strtok(NULL, ".");
			}
			debugOutput(data_to_send.packet, data_to_send.len);

			sendto(local_socket_fd, data_to_send.packet, data_to_send.len, 0, (SOCKADDR*)&local_back_addr, sa_size); //将数据返还给刚才从主函数来的地址
			output_in_use = 0;
			rec_reading[No_x_thread] = -1;//归还rec读线程锁
			thread_num--;
			_endthread(); //结束线程
			return;
		}
	}


	//向dns服务器转发刚才收到的查询包
	//这里试一下如果不采用读写同步会怎样
	//因为前提是，这个rec记录数组是一个线性数组
	//多个线程就算同时操作该数组，理论上不会同时操作同一个元素，
	//并且元素只在尾后添加，不会导致前面已确定的数组下标发生改变
	//测试使用

	while (rec_writing)Sleep(1);//接下来准备读取rec,只要rec没在写，都可以读

	rec_reading[No_x_thread] = 1;//将该线程的读标志置为一
	int i = check_in_cache(data_to_send);
	reply = rec[i];
	rec_reading[No_x_thread] = 0;//读rec完成，将读标志置为0；
	int rec_timeout = -1;
	if (i != -1)//缓存中找到了记录
	{
		rec_timeout = if_timeout(reply);
		if (rec_timeout == 0)//没超时，可以用
		{
			rec_reading[No_x_thread] = -1;//归还rec读线程锁
			flags[No_x_thread] = -1;
			reply.packet[0] = data_to_send.packet[0];
			reply.packet[1] = data_to_send.packet[1];

			//在此返回DNS服务
			sendto(local_socket_fd, reply.packet, reply.len, 0, (SOCKADDR*)&local_back_addr, sa_size); //将数据返还给刚才从主函数来的地址
			while (output_in_use)Sleep(1);//保证同时只有一个线程在输出，防止命令行输出混乱
			output_in_use = 1;//锁住命令行输出
			printf("来自IP：");
			set_color(6);
			printf("%s", inet_ntoa(local_back_addr.sin_addr));
			restore_color();
			printf(" 的DNS解析请求\n");
			set_color(10);//绿色输出不用重新请求的提示语
			if (option != 0)printf("缓存中找到记录->记录未超时，不再请求->记录解析如下\n");
			restore_color();
			debugOutput(reply.packet, reply.len);
			output_in_use = 0;//解锁输出
			thread_num--;
			_endthread(); //结束线程
			return;
		}
	}

	//在此继续DNS服务
	//缓存中没找到 或者 找到了但是超时了，继续向服务器请求
	sendto(local_forward_fd, data_to_send.packet, len, 0, (SOCKADDR*)&dns_server_addr, sa_size);

	//select操作，由于这里的select相当于只是给recvfrom定时，所以只用调用一次，
	//若要检测多个socket，得放在while里面
	FD_ZERO(&fds);													 //先清空要检测的是否能读的socket集合的宏
	FD_SET(local_forward_fd, &fds);									 //向集合中添加local_forward_fd的宏
	maxfd = maxfd > local_forward_fd ? maxfd : local_forward_fd + 1; //设置最大fd值为集合中所有fd的最大值加一
	switch (select(maxfd, &fds, NULL, NULL, &timout)) //轮询，直到超时
	/*
	解释一下select函数，五个参数，第一个参数是int类型，是所有集合中所有fd的最大值加一

	第二三四个参数类型为struct fd_set，在头函数winsock2中，
	分别是要检测的是否能读fd的集合，是否能写的fd的集合，是否出错的fd的集合

	第五个函数参数类型为struct timeval* ，在头函数winsock2中，储存秒和毫秒之后，select超时
	这个参数设置为0，select会立刻返回，设置为null，select会阻塞，知道检测到可写可读或出错为止

	select函数返回值为：正值，表示检测到有socket可读或可写或出错
	负值，表示select函数本身出错，
	0，超时，想再次检测需重新设置FD_ZERO和FD_SET
	*/
	{
	case -1:
		break; //不用管的情况
	case 0:
		break;								  //不用管
	default:								  //有socket可读或可写或出错
		if (FD_ISSET(local_forward_fd, &fds)) //FD_ISSET，该宏判断某个socket是否被select函数标记为可读或可写或出错
		{
			len = recvfrom(local_forward_fd, reply.packet, 512, 0, NULL, 0); //这里不需要知道dns服务器的地址啥的，因为都知道了，就直接写null和0
			//if (flags[No_x_thread] != -1)
			//{
			//	int f = flags[No_x_thread];
			//	Answer answer;
			//	int pos = 12;
			//	if (f == -2)
			//	{
			//		printf("\n STOP!!!!!! f:%d \n",f);
			//		reply.packet[3] = 0x83;
			//	}
			//	else
			//	{
			//		printf("\n CHAGE!!!!!!!!! f:%d \n", f);
			//		questionExtract(reply.packet, len, &pos, &answer);
			//		if (reply.packet[3] == 0x83)
			//		{
			//			reply.packet[3] = 0x80;
			//			int tt_pos = pos;
			//			reply.packet[]
			//		}
			//		printf("pos:%d\tlen:%d\n", pos, len);
			//		while (pos < len)
			//		{
			//			printf("pos:%d\tlen:%d\n", pos, len);
			//			resourceExtract(reply.packet, len, &pos, &answer, f);/**/
			//		}
			//	}
			//	printf("forward.c \n");
			//	debugOutput(reply.packet, len);
			//}
			if (len > 0)
			{
				//这里就获得了来自服务器的数据，buf为数组，len为有效长度
				reply.len = len;
				sendto(local_socket_fd, reply.packet, reply.len, 0, (SOCKADDR*)&local_back_addr, sa_size); //将数据返还给刚才从主函数来的地址
				//reply.packet是响应报

				extract_info_from_reply(&reply);
				if (reply.ttl != -1)//存在ttl的项才进入缓存，不存在的不缓存
				{
					//同上，测试取消线程同步
					///*
					while (1)//接下来准备修改rec，先检查rec是否正在使用中
					{
						int reading = 0;
						for (int i = 0; i < Max_Thread; i++)//检查有没线程在读
						{
							if (rec_reading[i] == 1)//只要有一个线程在读，就不能写
							{
								reading = 1;
								break;
							}
						}
						if (!reading && !rec_writing && !file_in_use)break;//没有读也没有写也没有在更新文件，跳出，可写rec
						Sleep(1);
					}
					//*/
					//写文件时保证不写rec，可读rec
					//while (file_in_use)Sleep(1);//文件正在写，则不能写
					rec_writing = 1;//锁住写rec，表示有线程正在修改rec
					if (rec_timeout == 1)
					{
						rec[i] = reply;//记录i超时了，用新获得的记录reply替换记0录i
						rec_changed++;//改变的记录数量自增
					}
					else//根本没有这个记录，向后加入,
					{
						if (rec_num < MAX_REC)//没有超出记录上限
						{
							rec[rec_num++] = reply;//新增记录并且记录总数自增
							rec_changed++;//改变的记录数量自增
						}
						else is_over_rec = 1;
					}
					rec_writing = 0;//解锁写rec
					if (rec_changed >= 10)//超过十条以上的记录改变了
					{
						while (file_in_use)Sleep(1);//文件正在写，则不能写
						file_in_use = 1;//锁住文件，防止同时修改
						rec_reading[No_x_thread] = 1;//锁住rec，防止写入时出错
						rec_changed = 0;//重新计算修改的记录数
						FILE* fp = fopen(cache_file, "wb");
						for (int i = 0; i < rec_num; i++)
						{
							store_info_from_reply(rec[i], fp);//重写文件
						}
						fclose(fp);
						file_in_use = 0;//解锁文件
						rec_reading[No_x_thread] = 0;//解锁rec
					}
				}
				while (output_in_use)Sleep(1);
				output_in_use = 1;
				printf("来自IP：");
				set_color(6);
				printf("%s", inet_ntoa(local_back_addr.sin_addr));
				restore_color();
				printf(" 的DNS解析请求\n");
				set_color(12);//粉红色输出重新请求的记录提示
				if (rec_timeout != -1 && option != 0 && reply.ttl != -1)printf("缓存中找到记录->记录已超时->超时记录更新成功->返回包解析如下\n");
				else if (!is_over_rec && option != 0 && reply.ttl != -1)printf("缓存中未找到记录->新建记录成功，当前记录总条数:%d->返回包解析如下\n", rec_num);
				else if (!is_over_rec && option != 0 && reply.ttl == -1)printf("缓存中未找到记录->返回答案类型与请求不符->返回包解析如下\n");
				else if (is_over_rec && option != 0) printf("缓存中未找到记录->缓存数量超出上限，不再新增记录->记录解析如下\n");
				restore_color();
				debugOutput(reply.packet, reply.len);
				output_in_use = 0;
			}
		}
	}
	//printf("time: %d", times);
	rec_reading[No_x_thread] = -1;//归还线程rec读锁；
	flags[No_x_thread] = -1;
	thread_num--;
	_endthread(); //结束线程
}