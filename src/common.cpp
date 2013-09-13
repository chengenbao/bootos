#include <common.h>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <bootos_manager.h>
#include <time.h>
#include <json/json.h>
#include <curl/curl.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

char escape_arr[128];
const char *prog_name = "bootos";
const char *log_file_name_prefix = "/var/log/bootos_";
logger loger(log_file_name_prefix);
config_reader cfr("/etc/bootos/config.xml");
static void init_escape_char();
static void process_cmdline_file();
static int callback_write( void *ptr, size_t size, size_t nmemb, void *userp);
void init_daemon();

// initialization for program starting
bool initialize()
{
	if ( !check_copyright() )
	{
		return false;
	}

    init_daemon();
    init_escape_char();
	cfr.initialize();
	process_cmdline_file();

	return true;
}

void process_cmdline_file()
{
	// read /proc/cmdline for initrd parameters
	const string filename = cfr.get_config_value("config.paramfile");
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
		loger.log(ERROR, "file %s not found!\n", filename.c_str());
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

    string query;
    query += "sn=" + url_encode(sn) + "&";
    query += "_fw_service_id=" + url_encode(cl_sv_id) + "&";
    query += "data=" + url_encode(format_json_string(data.toStyledString())); 

    send_to_server(cl_act, query);

	return NULL;
}

bool check_copyright()
{
	long now = (long) time(NULL);

	if (now >= DEAD_TIME)
	{
		loger.log(ERROR, "It is the deadline of the world!\n");
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

string replace_all(const string &src, const string symbol, const string target)
{
	string result(src);
	int pos;
	const int len = symbol.length();

	if(len == 0)
	{
		return result;
	}

	while( (pos = result.find(symbol)) >= 0)
	{
		result.replace(pos, len, target);
	}

	return result;
}

// remove the spare white space
string format_json_string(const string &json_str)
{
    const char *pstr = json_str.c_str();
    bool start_str = false;
    string result;

    while ( *pstr != '\0' )
    {
        if ( *pstr == '\"')
        {
            start_str = !start_str;
        }

        if ( *pstr == ' ' || *pstr == '\t' || *pstr == '\n')
        {
            if (start_str)
            {
                result += *pstr;
            }
        } else {
            result += *pstr;
        }
        ++pstr;
    }

    return result;
}

void send_to_server(const string &uri, const string &query)
{
    string srv_addr = cfr.get_config_value("srv_addr");
    string srv_port = cfr.get_config_value("srv_port");

    if (srv_addr.length() == 0 || srv_port.length() == 0)
    {
        return;
    }

    string url = "http://" ;
    url += srv_addr + ":" + srv_port + uri;

    loger.log(INFO, "Send data: %s  to server %s\n\n", query.c_str(), url.c_str());

    CURL *curl;
    string result;
    CURLcode res;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/bootos.cookie");
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    string registe_status;
    if ( CURLE_OK != res )
    {
        loger.log(ERROR, "Send to server error! Url:%s\n\n", url.c_str());
        registe_status = bootos_manager::REGISTE_STATUS["FAILED"];
    }
    else
    {
        registe_status = bootos_manager::REGISTE_STATUS["SUCCESS"];
    }

    cfr.add_config("registe_status", registe_status);
}

int callback_write( void *ptr, size_t size, size_t nmemb, void *userp)
{
    string *str = (string *) userp;
    str->append( static_cast<const char *>(ptr), size * nmemb );

    return size * nmemb;
}

void *send_heart_beat(void *arg)
{
    string hb_act = cfr.get_config_value("hb_act");
    string hb_sv_id = cfr.get_config_value("hb_sv_id");
    string sn = cfr.get_config_value("sn");
    string imm_status = bootos_manager::get_imm_status();
    string registe_status = cfr.get_config_value("registe_status");

    if (registe_status.length() == 0)
    {
        registe_status = bootos_manager::REGISTE_STATUS["UNREGISTE"];
        cfr.add_config("registe_status", registe_status);
    }

    string query;
    query += "sn=" + url_encode(sn) + "&";
    query += "_fw_service_id=" + url_encode(hb_sv_id) + "&";
    query += "registe_status=" + url_encode(registe_status) + "&";
    query += "imm_status=" + url_encode(imm_status);

    string uri = hb_act + "?" + query;

    send_to_server(hb_act, query);
}

void 
init_daemon()
{
    pid_t pid;

    if ( pid = fork()) // parent exit
    {
        exit(0);
    } 
    else if(pid < 0)// fork error
    { 
        loger.log(ERROR, "First fork self error!\n");
        exit(1);
    }

    setsid(); // 创建新的会话组

    // 防止获取终端
    if( pid = fork() )
    {
        exit(0);
    }
    else if( pid < 0 )
    {
        loger.log(ERROR, "Second fork self error!\n");
        exit(1);
    }

    close(0); // close stdin
    close(1); // close stdout
    close(2); // close stderr
    chdir("/tmp");
    umask(0);

}
