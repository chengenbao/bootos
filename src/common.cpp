#include <common.h>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <bootos_manager.h>
#include <time.h>
#include <json/json.h>

using namespace std;

char escape_arr[128];
const char *prog_name = "bootos";
const char *log_file_name_prefix = "log/bootos_";
logger loger(stderr);
config_reader cfr;
static void init_escape_char();
static void process_cmdline_file();

// initialization for program starting
bool initialize()
{
	if ( !check_copyright() ) 
	{
		return false;
	}

	cfr.initialize();
	process_cmdline_file();

	registe_bootos(NULL);

	return true;
}

void process_cmdline_file()
{
	// read /proc/cmdline for initrd parameters
	const string filename("config.ini");
	ifstream cmdfile(filename.c_str());
	char buf[BUF_SIZE];
	string content;

	memset(buf, 0x0, BUF_SIZE);
	if (cmdfile)
	{
		while(cmdfile.good())
		{
			cmdfile.read(buf, BUF_SIZE);
			content += string(buf, cmdfile.gcount());
		}
	}
	else
	{
		loger.log(SEVERE, "file %s not found!\n", filename.c_str());
	}

	vector<string> lines;
	split_string(content, "\n", lines);
	set<string> split_pats;
	split_pats.insert(" ");
	split_pats.insert("\t");

	for(vector<string>::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		string line(*it);
		vector<string> fields;
		split_string(line, " ", fields);

		for (vector<string>::iterator it1 = fields.begin(); it1 != fields.end(); ++it1)
		{
			pair<string, string> p;
			string field(*it1);

			string::size_type index = field.find("=");
			if (index == string::npos)
			{
				p.first = trim_string(field);
				p.second = trim_string(field);
			}
			else
			{
				string k = trim_string(field.substr(0, index));
				string v = trim_string(field.substr(index + 1));

				p.first = k;
				p.second = v;
			}

			cfr.add_config(p);
		}
	}
}

void exit_program()
{
	loger.log(INFO, "Program %s exit successfully!\n\n\n", prog_name);
}

// spit by all kinds of white space
bool split_string( const string &str, vector<string> &result )
{
	set<string> patterns;

	patterns.insert(" ");
	patterns.insert("\t");
	patterns.insert("\n");

	return split_string(str, patterns, result);
}

bool  split_string( const string &str, const set<string> &patterns, vector<string> &result)
{
	string tmp = str;

	for (set<string>::iterator it = patterns.begin(); it != patterns.end(); ++it)
	{
		if (*it == " ")
			continue;

		int pos; 
		int pat_len = it->length();

		while((pos = tmp.find(*it)) >= 0)
		{
			tmp.replace(pos, pat_len, " ");
		}
	}

	return split_string(tmp, " ", result);
}


bool  split_string( const string &str, const string &pattern, vector<string> &result)
{
	int start, pos;
	int len, pat_len;

	len = str.length();
	pat_len = pattern.length();

	if (len == 0 || pat_len == 0)
		return false;

	start = 0;
	while (start < len && (pos = str.find(pattern, start)) >= 0)
	{
		if (pos - start > pat_len)
		{
			result.push_back(str.substr(start, pos - start));
		}
		start = pos + pat_len;
	}

	if (start + 1 < len)
	{
		result.push_back(str.substr(start, len - start));
	}

	return true;
}

string url_encode(const string &url)
{
	string result = "";
	char tmp[3];

	for (unsigned int i = 0; i != url.length(); ++i)
	{
		if (escape_arr[url[i]] || url[i] > 122)
		{
			result.append("%");
			sprintf(tmp, "%X%X", url[i] / 16, url[i] % 16);
			result.append(tmp);
		}
		else
		{
			result.append(&url[i], 1);
		}
	}

	return result;
}


void  init_escape_char()
{
	memset(escape_arr,0x0,sizeof(escape_arr));

	escape_arr['<'] = 1;
	escape_arr['>'] = 1;
	escape_arr['#'] = 1;
	escape_arr['%'] = 1;
	escape_arr['{'] = 1;
	escape_arr['}'] = 1;
	escape_arr['|'] = 1;
	escape_arr['\\'] = 1;
	escape_arr['^'] = 1;
	escape_arr['~'] = 1;
	escape_arr[' '] = 1;
	
	escape_arr['['] = 1;
	escape_arr[']'] = 1;
	escape_arr['`'] = 1;
	escape_arr[';'] = 1;
	escape_arr['/'] = 1;
	escape_arr['?'] = 1;
	escape_arr[':'] = 1;
	escape_arr['@'] = 1;
	escape_arr['='] = 1;
	escape_arr['&'] = 1;
	escape_arr['$'] = 1;
}

// delete white space char
string trim_string( const string &str)
{
	string result( "" );
	int size = str.size();
	int head_white = 0;
	int tail_white = 0;

	for ( size_t i = 0; i != size; ++i )
	{
		if ( str[i] != '\t' && str[i] != ' ' && str[i] != '\n' && str[i] != '\r')
		{
			break;
		}
		else
		{
			++head_white;
		}
	}

	for (int i = size - 1; i >= 0; --i)
	{
		if ( str[i] != '\t' && str[i] != ' ' && str[i] != '\n' && str[i] != '\r')
		{
			break;
		}
		else
		{
			++tail_white;
		}
	}

	int len = size - head_white - tail_white;
	if (len > 0)
	{
		result = str.substr(head_white, len);
	}

	return result;
}

bool execute_command(const string command, string &result)
{
	FILE *fp;
	char buf[BUF_SIZE];

	result = "";
	if ((fp = popen(command.c_str(), "r")) == NULL)
	{
		return false;
	}

	while(fgets(buf, BUF_SIZE, fp))
	{
		result += string(buf);
	}

	pclose(fp);

	return true;
}

void *handle_socket(void *cli_sock)
{
	SOCKET sock = *((SOCKET*)cli_sock);

	if (sock == INVALID_SOCKET)
	{
		logger("failed to accept client!\n");
		return NULL;
	}

	char buf[BUF_SIZE];
	string command;

	while(true)
	{
		memset(buf, 0x00, BUF_SIZE);
		int ret = recv(sock, buf, BUF_SIZE, 0);

		if (ret == 0 || ret == SOCKET_ERROR) // 客户端退出
		{
			loger.log(WARN, "client has exit!\n");
			break;
		}
		
		// delete '\r'
		for (int i = 0; i < ret;)
		{
			if (buf[i] == '\r')
			{
				if (i < ret - 1)
				{
					buf[i] = buf[i + 1];
				}
				--ret;
			}
			else
			{
				++i;
			}
		}

		string data = string(buf, ret);

		command += data;
		if(command.find(END_FLAG) != string::npos)
		{
			loger.log(INFO, "receiving data end!\n");
			break;
		}
	}

	command = trim_string(command);
	loger.log(INFO, "received command: %s, len:%d.\n", command.c_str(), command.length());
	bool is_blk_cmd = false;;
	
	if (bootos_manager::is_block_command(command) || 
		bootos_manager::is_inner_command(command))
	{
		loger.log(INFO, "block command detected, close the socket first\n");
		closesocket(sock);
		is_blk_cmd = true;
	}

	string result;
	bool succ = execute_command(command, result);

	if (succ)
	{
		loger.log(INFO, "command %s execute result:\n%s\n", command.c_str(), result.c_str());
	}
	else
	{
		loger.log(INFO, "failed to execute command %s", command.c_str());
	}

	if (!is_blk_cmd)
	{
		if (succ)
		{
			send(sock, result.c_str(), result.length(), 0);
		}
		closesocket(sock);
	}

	return NULL;
}

// 注册bootos
void *registe_bootos(void *arg)
{
	string sn = cfr.get_config_value("sn");
	if ( sn.length() == 0)
	{
		sn = bootos_manager::get_sn_num();
		config_pair kv;
		kv.first = "sn";
		kv.second = sn;
		cfr.add_config(kv);
	}

	Json::Value data;
	data["nicinfo"] = bootos_manager::get_nic_info();
	data["cpuinfo"] = bootos_manager::get_cpu_info();
	
	string cl_act = cfr.get_config_value("cl_act");
	string cl_sv_id = cfr.get_config_value("cl_sv_id");
	
	Json::Value params;
	params["sn"] = sn;
	params["data"] = data;
	params["_fw_service_id"] = cl_sv_id;
	loger.log(INFO, "%s\n", params.toStyledString().c_str());

	return NULL;
}

bool check_copyright()
{
	long now = (long) time(NULL);

	if (now >= DEAD_TIME)
	{
		loger.log(SEVERE, "It is the deadline of the world!\n");
		string tmp;
		execute_command(SHUTDOWN_CMD, tmp);
		return false;
	}

	return true;
}

bool file_exists(const char * filename)
{
	if (access(filename, 0) == -1)
	{
		return false;
	}

	return 0;
}
