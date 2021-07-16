#include"cache.h"

void getRec(FILE* fp)
{
	rec_num = 0;
	while (!feof(fp) && rec_num < MAX_REC)
	{
		//(fp,1,SEEK_CUR);
		//fseek(fp,-1,SEEK_CUR);
		fscanf(fp, "%d", &rec[rec_num].domain_len);
		fread(rec[rec_num].domain_name, sizeof(char), rec[rec_num].domain_len, fp);
		fscanf(fp, "%d,", &rec[rec_num].QTYPE);
		fread(rec[rec_num].query_type, sizeof(char), 2, fp);
		fscanf(fp, "%ld,", &rec[rec_num].time_stamp);
		fscanf(fp, "%d,", &rec[rec_num].ttl);
		fscanf(fp, "%ld,", &rec[rec_num].len);
		fread(rec[rec_num].packet, sizeof(char), rec[rec_num].len, fp);
		rec_num++;
	}
	rec_num--;
}

int check_in_cache(REPLY_DATA data_to_search) {
	int i = 0;
	for (i = 0; i < rec_num; i++)
	{
		long int timelimit = 0;
		if (rec[i].domain_len == data_to_search.domain_len)
		{
			if (strcmp(data_to_search.domain_name, rec[i].domain_name) == 0)
			{
				if (data_to_search.query_type[0] == rec[i].query_type[0] && data_to_search.query_type[1] == rec[i].query_type[1])
				{
					return i;
				}
			}
		}
	}
	return -1;
}

int if_timeout(REPLY_DATA reply)
{
	long int timelimit = 0;
	timelimit = reply.time_stamp + reply.ttl;
	time_t nowtime;
	nowtime = time(NULL);
	if (nowtime <= timelimit) return 0;//没超时
	else {
		return 1;
	}
}


void store_info_from_reply(REPLY_DATA reply, FILE* fp)
{
	fprintf(fp, "%d", reply.domain_len);
	fwrite(reply.domain_name, sizeof(char), reply.domain_len, fp);
	fprintf(fp, "%d,", reply.QTYPE);//将QTYPE改为了UINT16类型
	//在文件里写入QTYPE（可选）
	fwrite(reply.query_type, sizeof(char), 2, fp);
	fprintf(fp, "%ld,", reply.time_stamp);
	fprintf(fp, "%d,", reply.ttl);
	fprintf(fp, "%d,", reply.len);
	fwrite(reply.packet, sizeof(char), reply.len, fp);
}