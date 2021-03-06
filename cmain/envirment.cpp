#include "envirment.h"

#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

using std::string;
using std::map;
using std::pair;

static map<string, string> envirment_map;

void fl_setenv(const char * key, const char * value)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		envirment_map[key] = value;
	}
	else
	{
		envirment_map.insert(pair<string,string>(key,value));
	}
}

const char * fl_getenv(const char * key)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return envirment_map[key].c_str();
	}
	else
	{
		return NULL;
	}
}

const char * fl_getenv(const char * key, const char * def)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return envirment_map[key].c_str();
	}
	else
	{
		return def;
	}
}

int fl_getenv(const char * key, const int def)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return atoi(envirment_map[key].c_str());
	}
	else
	{
		return def;
	}
}

double fl_getenv(const char * key, const double def)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return atof(envirment_map[key].c_str());
	}
	else
	{
		return def;
	}
}

bool fl_reload_config(const char * configName)
{
	envirment_map.clear();
	bool isReloadOk = fl_load_config(configName);
	if (false == isReloadOk)
	{
		fprintf(stderr,"reload config file %s failed\n",configName);
	}
	return isReloadOk;
}

bool fl_load_config(const char * configName)
{
	//check is the file exist;
	if (0 != access(configName,F_OK))
	{
		fprintf(stderr,"config file %s is not found\n",configName);
		return false;
	}

	//check is the file a text file
	struct stat s;
	stat(configName,&s);
	if (false == S_ISREG(s.st_mode))
	{
		fprintf(stderr,"%s is not a config file\n",configName);
		return false;
	}

	//open config file,then handle
	FILE * cf = fopen(configName,"r");
	if (NULL == cf)
	{
		fprintf(stderr,"open config file %s failed\n",configName);
		return false;
	}
	char str[512] , key[256] , value[256];
	int i = 0 , maxlen = 0;
	bool isKeyOk;
	char * ch;
	while (fgets(str,sizeof(str),cf) != NULL)
	{

		//this is a comment
		if (str[0] == '#' || str[0] == '\n')
		{
			continue;
		}

		//get key
		ch = str , i=0 , isKeyOk = true;
		maxlen = sizeof(key) - 1;
		while (*ch && i < maxlen)
		{
			if (*ch == ' ')
			{
				++ch;
				continue;
			}
			if (*ch == '=')
			{
				++ch;
				break;
			}
			if (*ch == '\n')
			{
				isKeyOk = false;
				++ch;
				break;
			}
			key[i++] = *ch++;
		}
		key[i++] = 0;

		//check the config pair is or not only have the key,but no value
		if (false == isKeyOk)
		{
			fprintf(stderr,"error config pair found, key=[%s] found, but has no value\n",key);
			fclose(cf);
			return false;
		}

		//get value
		i=0;
		maxlen = sizeof(value) - 1;
		while (*ch != '\0' && i < maxlen)
		{
			if (*ch == ' ' || *ch == '\n')
			{
				++ch;
				continue;
			}
			value[i++] = *ch++;
		}
		value[i++] = 0;

		//check the value is or not empty
		if (0 == strlen(value))
		{
			fprintf(stderr,"error config pair found, key=[%s] found, but has no value\n",key);
			fclose(cf);
			return false;
		}

		fl_setenv(key, value);
	}
	fclose(cf);
	return true;
}
