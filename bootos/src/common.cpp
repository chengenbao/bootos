#include <common.h>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <iostream>

using namespace std;

char escape_arr[128];
const char *prog_name = "bootos";
const char *log_file_name_prefix = "log/bootos_";
logger loger(log_file_name_prefix);
config_reader cfr;
static void init_escape_char();
static void process_cmdline_file();

// initialization for program starting
void initialize()
{
	cfr.initialize();
	process_cmdline_file();
}

void process_cmdline_file()
{
	// read /proc/cmdline for initrd parameters
	const string filename("config.ini");
	const int buf_size = 256;
	ifstream cmdfile(filename);
	char buf[buf_size];
	string content;

	memset(buf, 0x0, buf_size);
	if (cmdfile)
	{
		while(cmdfile.good())
		{
			cmdfile.read(buf, buf_size);
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

	for (int i = size - 1; i >= 0; ++i)
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
	const int buf_size = 256;
	char buf[buf_size];

	result = "";
	if ((fp = popen(command.c_str(), "r")) == NULL)
	{
		return false;
	}

	while(fgets(buf, buf_size, fp))
	{
		result += string(buf);
	}

	pclose(fp);

	return true;
}
