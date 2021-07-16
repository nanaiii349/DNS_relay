#include"analyse.h"

long getTime()
{
	time_t seconds;
	seconds = time(NULL);
	return seconds;
}

UINT16 getUINT16(char* packet, int len, int* a)
{
	UINT16 n = packet[*a] & (0x00ff);//packet【0】
	UINT16 m = packet[*a + 1] & (0x00ff);
	//printf("getUINT16:%x %x\t", n, m);
	*a += 2;
	return (n << 8) | m;
}

UINT32 getUINT32(char* packet, int len, int* a)
{
	UINT32 n1 = packet[*a] & (0x000000ff);
	UINT32 n2 = packet[*a + 1] & (0x000000ff);
	UINT32 n3 = packet[*a + 2] & (0x000000ff);
	UINT32 n4 = packet[*a + 3] & (0x000000ff);
	*a += 4;
	return (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;
}

int getTTL(REPLY_DATA* reply, int len, int pos)
{
	int ttl = -1;
	//printf("查询type：%d\n", reply->QTYPE);
	while (pos < reply->len)
	{
		ttl = -1;
		int temp = -1;
		pos += 2;										   //NAME
		UINT16 type = getUINT16(reply->packet, len, &pos); //type
		pos += 2;										   //CLASS
		temp = getUINT32(reply->packet, 0, &pos);
		//printf("回应type：%d,ttl：%d\n", type, temp);
		if (type == reply->QTYPE)
		{
			ttl = temp;
		}
		int l = getUINT16(reply->packet, 0, &pos);
		//域名或者ip的长度在这
		pos += l;
	}
	return ttl;
}

int get_domain_and_type(REPLY_DATA* reply)
{
	reply->domain_len = 0;
	for (int i = 12; reply->packet[i] != '\x00'; i++, reply->domain_len++)
	{
		reply->domain_name[reply->domain_len] = reply->packet[i];
	}
	reply->domain_name[reply->domain_len] = '\x00';
	reply->domain_len++; //包括最后一个\0
	int pos = 12 + reply->domain_len;
	//获取type
	int temp = pos;
	reply->QTYPE = getUINT16(reply->packet, 0, &temp);//明文type
	reply->query_type[0] = reply->packet[pos++];
	reply->query_type[1] = reply->packet[pos++];//原文type
	return pos;
}

void extract_info_from_reply(REPLY_DATA* reply)
{
	reply->time_stamp = time(NULL);
	int pos = get_domain_and_type(reply);
	//获取ttl
	pos += 2;
	reply->ttl = getTTL(reply, reply->len, pos); //也可以直接用下面的代码
}

UINT8 getQR(char packet[], int len)
{
	UINT8 tmp = 0x80;
	return (tmp & packet[2]) >> 7;
}

UINT8 getRCODE(char packet[], int len)
{
	UINT8 tmp = 0x0f;
	return  tmp & packet[3];
}

void getNAME(char packet[], int len, int* n, char name[], int board, int* num)
{
	int pos = *n;
	int cnt = 0;
	int p_pos = 0;
	int flag = 0;//避免读域名外面的\x00
	int j = *num;
	UINT8 tmp;

	/*printf("\npacket\n");
	for (int i = 0; i < 60; i++)
	{
		printf("%x ", packet[pos + i] & 0xff);
		if (i % 10 == 9)
			printf("\n");
	}
	printf("\n");*/


	while (cnt < board && packet[pos] != '\x00')
	{
		if (packet[pos] == '\xc0')
		{
			p_pos = packet[++pos];//该指针指向的新地址
			pos++;
			cnt += 2;
			flag = 1;
			//printf("\nuse getNAME!!!new pos:%d \n",p_pos);
			getNAME(packet, len, &p_pos, name, 256, &j);
			break;
		}
		else
		{
			unsigned char uchar = packet[pos++];
			unsigned int l = uchar;
			cnt++;
			for (int k = 0; k < l && l != 0 && cnt < board; k++)
			{
				tmp = packet[pos++] & 0xff;
				name[j++] = tmp;
				cnt++;
				//printf("pos:%d %c ", pos-1, name[j - 1]);
			}
			//printf("\n");
			if (cnt < board && packet[pos] != '\x00')
				name[j++] = '.';
		}
	}
	if (packet[pos] == '\x00' && flag == 0)
	{
		pos++;
		name[j] = '\0';
	}
	//printf("name:%s \n", name);
	*num = j;
	*n = pos; 
	return;
}