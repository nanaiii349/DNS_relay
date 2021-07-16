#pragma once
#include"globals.h"

void getRec(FILE* fp);//从文件读取所有记录

int check_in_cache(REPLY_DATA data_to_search);//搜索记录是否存在 存在返回下标 不存在去服务器寻找并写进文件并返回-1 

int if_timeout(REPLY_DATA reply);//检查是否超时 超时返回1 未超时返回0，t为在rec中的下标

void store_info_from_reply(REPLY_DATA reply, FILE* fp);
