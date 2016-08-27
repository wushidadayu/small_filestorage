#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "make_log.h"
#include "redis_op.h"
#include "fcgi_stdio.h"
#include "cJSON.h"

#define DB_INFONAME "cloudfile"
#define ERROR_RET "{\"games\":[]}"

char* rstrchr(char *name, char ch)
{
	if(name ==NULL)
		return NULL;
	int len = strlen(name);
	if(len <= 0)
		return NULL;
	while((--len) >= 0)
	{
		if(name[len] == ch)
			return name+len;
	}
	return NULL;
}

char* memstr(char* full_data, int full_data_len, char* substr) 
{ 
	if (full_data == NULL || full_data_len <= 0 || substr == NULL) { 
		return NULL; 
	} 

	if (*substr == '\0') { 
		return NULL; 
	} 

	int sublen = strlen(substr); 

	int i; 
	char* cur = full_data; 
	int last_possible = full_data_len - sublen + 1; 
	for (i = 0; i < last_possible; i++) { 
		if (*cur == *substr) { 
			//assert(full_data_len - i >= sublen);  
			if (memcmp(cur, substr, sublen) == 0) { 
				//found  
				return cur; 
			} 
		} 
		cur++; 
	} 

	return NULL; 
} 

//cmd=newFile&fromId=0&count=8
void create_objects(int act, int count)
{
	if(count <= 0 || count > 10 || act < 0)
	{
		LOG("fileuplaod", "listdata", "input param error, [count:%d,act:%d], line:%d", count, act, __LINE__);
		printf("%s",ERROR_RET);
		return;
	}
	cJSON *root,*thm,*fld;char *out;int i;
	root=cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "games", thm=cJSON_CreateArray());
	
	redisContext *top =  rop_connectdb_nopwd("127.0.0.1", "6379");
	if(top==NULL)
	{
		LOG("fileuplaod", "listdata", "rop_connectdb_nopwd() error, line:%d", __LINE__);
		printf("%s",ERROR_RET);
		return;
	}
	char values[10][VALUES_ID_SIZE];
	char picurl[100];
	char url[300];
	char *tmp;
	int val_len;
	int res = rop_range_list(top, DB_INFONAME, act, act+count, values, &val_len);
	if(res!=0)
	{
		LOG("fileuplaod", "listdata", "rop_range_list() error, line:%d", __LINE__);
		rop_disconnect(top);
		return;
	}
	rop_disconnect(top);
	//LOG("fileuplaod", "listdata", "datalen:%d, line:%d", val_len, __LINE__);
	for (i=0;i<val_len;i++)
	{
		cJSON_AddItemToArray(thm,fld=cJSON_CreateObject());	
		//类型
		tmp = rstrchr(values[i], '|');
		if(tmp==NULL)
			continue;
		*tmp++ = '\0';
		sprintf(picurl, "static/file_png/%s.png", tmp);
		cJSON_AddStringToObject(fld,"picurl_m",picurl);
		//用户
		tmp = rstrchr(values[i], '|');
		if(tmp==NULL)
			continue;
		*tmp++ = '\0';
		//时间
		tmp = rstrchr(values[i], '|');
		if(tmp==NULL)
			continue;
		*tmp++ = '\0';
		cJSON_AddStringToObject(fld,"descrip",tmp);
		//文件名
		tmp = rstrchr(values[i], '|');
		if(tmp==NULL)
			continue;
		*tmp++ = '\0';
		cJSON_AddStringToObject(fld,"title_m",tmp);
		//ip地址
		tmp = rstrchr(values[i], '|');
		if(tmp==NULL)
			continue;
		*tmp++ = '\0';
		sprintf(url, "http://%s/", tmp);
		//group
		tmp = rstrchr(values[i], '|');
		if(tmp==NULL)
			continue;
		*tmp++ = '\0';
		cJSON_AddStringToObject(fld,"id",tmp);
		strcat(url, tmp);
		cJSON_AddStringToObject(fld,"url",url);
		
		cJSON_AddNumberToObject(fld,"kind",2);
		cJSON_AddStringToObject(fld,"title_s","rect");
		cJSON_AddNumberToObject(fld,"pv",0);
		cJSON_AddNumberToObject(fld,"hot",1);
	}
	
	out=cJSON_Print(root);	
	cJSON_Delete(root);	
	printf("%s\n",out);	
	free(out);
}

int main(void)
{
	char data[200];
	int act, count, i, tmplen;
	char *cmd, *tmp, *tmp2;
	while (FCGI_Accept() >= 0) {
		int isread = 0;
		char *buf = getenv("QUERY_STRING");
		//printf("%s",buf);

		printf("Content-type: text/javascript\r\n"
	    "\r\n");
		if (buf != NULL) {
			sprintf(data, "&%s", buf);
		}
		else {
			LOG("fileuplaod", "listdata", "not request data error, line:%d", __LINE__);
			printf("%s",ERROR_RET);
			continue;
		}
		
		act = count = 0;
		*cmd = '\0';
		tmp = data;
		
		LOG("fileuplaod", "listdata", "request: %s, line:%d", data, __LINE__);
		//解析请求头
		while(tmp != '\0')
		{
			tmp = strchr(tmp, '&');
			if(tmp == NULL)
				break;
			*tmp++ = '\0';
			tmp2 = strchr(tmp, '&');
			if(tmp2!=NULL)
				tmplen = tmp2 - tmp;
			else
				tmplen = strlen(tmp);
			if(memstr(tmp, tmplen, "cmd="))
				cmd = tmp+strlen("cmd=");
			else if(memstr(tmp, tmplen, "fromId="))
				act = atoi(tmp+strlen("fromId="));
			else if(memstr(tmp, tmplen, "count="))
				count = atoi(tmp+strlen("count="));
		}
		
		create_objects(act, count);
	
	}
	return 0;
}
