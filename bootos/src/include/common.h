// common utilities
// Author: chengenbao
// Email��genbao.chen@gmail.com


#ifndef UTILITY_H
#define UTILITY_H

#define WINDOWS 1

#ifdef WINDOWS
#include <windows.h>
#define sleep(x) Sleep(x) 
#endif

typedef void * (*task_func)(void *);

#include <logger.h>
#include <config_reader.h>
#include <string>
#include <set>

using namespace std;

// ȫ�ֱ�������
extern logger loger;
extern config_reader cfr;

// ��������
const int MSEC_PER_SEC = 1000;

// ���ú�������
void initialize();
void exit_program();
bool split_string( const string &str, const string &pattern, vector<string> &result);
bool  split_string(const string &str, const set<string> &patterns, vector<string> &result);
string trim_string( const string &str);

#endif