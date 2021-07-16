#include"parma.h"

int judge_and_set_parma(char** varg, int pos)//设置命令行参数的函数，pos为要从varg数组中第几个来设置变量
{
	int is_success = 1;//命令行参数输入是否正确
	if (varg[pos][0] == '-' && !option_is_set)//如果这个字符串的第一个为字符'-'，并且调试级别这个参数还没被设定过
	{
		if (strcmp(varg[pos], "-d") == 0)option = 1;
		else if (strcmp(varg[pos], "-dd") == 0)option = 2;
		else option = 0;
		option_is_set = 1;
	}
	else if (isdigit(varg[pos][0]) && !dns_server_ip_is_set)//如果第一个字符为数字，并且dns ip还没被设定过
	{
		strcpy(DNS_SERVER_IP, varg[pos]);
		dns_server_ip_is_set = 1;
	}
	else if (isalpha(varg[pos][0]) && varg[pos][1] == ':' && varg[pos][2] == '\\' && !cache_file_is_set)//同上如果路径没被设定过
	{
		strcpy(cache_file, varg[pos]);
		cache_file_is_set = 1;
	}
	else is_success = 0;//如果以上皆不是或者尝试设定一个已经设定过的参数，即输入重复参数，返回0，表示设置失败
	return is_success;
}

int read_ip_domain()
{
	FILE* fp=fopen("ip_domain.txt","r");
	if (fp == NULL)
	{
		printf("无法打开ip-域名对照表，网站拦截功能无法开启！\n");
		return;
	}
	else
	{
		printf("读取ip-域名对照表成功，网站拦截功能开启成功！\n");
		while (!feof(fp))
		{
			fscanf(fp, "%s %s", ip_domain[ip_domain_num].ip, ip_domain[ip_domain_num].domain_name);
			ip_domain_num++;
		}
		ip_domain_num--;
	}
}
