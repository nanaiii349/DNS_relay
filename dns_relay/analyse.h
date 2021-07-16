#pragma once
#include"globals.h"

int get_domain_and_type(REPLY_DATA* reply);

void extract_info_from_reply(REPLY_DATA* reply);

long getTime();

UINT16 getUINT16(char* packet, int len, int* a);

UINT32 getUINT32(char* packet, int len, int* a);

UINT8 getQR(char packet[], int len);

UINT8 getRCODE(char packet[], int len);

void getNAME(char packet[], int len, int* n, char name[], int board, int FLAG_CNAME);

int getTTL(REPLY_DATA* reply, int len, int pos);