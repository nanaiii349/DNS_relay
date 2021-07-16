#pragma once
#include"globals.h"
#include"analyse.h"

void debugOutput(char* packet, int len);

void resourceOutput(Answer* answer, int* num);

void resourceExtract(char* packet, int len, int* pos, Answer* answer,int f);

void questionOutput(Answer* answer);

void questionExtract(char* packet, int len, int* pos, Answer* answer);

void headerOutput(Answer* answer);

void headerExtract(char* packet, int len, Answer* answer);

void typeOutput(UINT16 TYPE);

void split_line();
