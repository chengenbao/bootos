#ifndef BOOTOS_MANAGER_H
#include <common.h>
#include <string>
#include <set>
#include <map>
#include <json/json.h>

using namespace std;

class bootos_manager
{
public:
	static set<string> BLOCK_COMMAND;
	static map<string, task_func> INNER_COMMAND;
	static map<string, string> REGISTE_STATUS;
	static map<string, string> IMM_CONFIG_STATUS;
	static const string IMM_PID_FILE;
	static const string IMM_SUCCESS_FLAG_FILE;
	static const string IMM_FAILED_FLAG_FILE;
	static string get_imm_status();
	static string get_sn_num();
	static Json::Value get_nic_info();
	static Json::Value get_cpu_info();

	static bool is_block_command(const string &command);
	static bool is_inner_command(const string &commnad);

	static set<string> init_block_command();
	static map<string, task_func> init_inner_command();
	static map<string, string> init_registe_status();
	static map<string, string> init_imm_config_status();
};

#endif