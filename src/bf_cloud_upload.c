#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "make_log.h"
#include "redis_op.h"
#include "myfastdfs.h"
#include "fcgi_stdio.h"

#define DB_INFONAME "cloudfile"
#define FDFS_CLIENT_CONF "/etc/fdfs/client.conf"

char* getl(char *buf, int *len, char *src)
{
	if(src ==NULL || buf == NULL)
	{
		*len = 0;
		return src;
	}
	int i, max = (*len) - 1;
	char *newtmp = buf;
	for(i=0; i<max; i++)
	{
		if(*src =='\n' || *src == '\r')
		{
			src++;
			break;
		}
		*newtmp++ = *src++;
	}
	if(*src =='\n' || *src == '\r')
		src++;
	*len = i;
	buf[i] = '\0';
	return src;
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

int operat(char *filename, char *buf, int buf_len)
{
	if(filename == NULL || buf == NULL ||buf_len<=0)
		return -1;
		
	FILE *f = fopen(filename, "wb");
	if(f==NULL)
	{
		LOG("fileuplaod", "upload", "[-][upload]fopen error, line:%d", __LINE__);
		return -1;
	}
	int res = fwrite(buf, 1, buf_len, f);
	if(res != buf_len)
	{
		LOG("fileuplaod", "upload", "[-][upload]fwrite error res:%d input:%d, line:%d", res, buf_len, __LINE__);
		fclose(f);
		return -1;
	}
	fclose(f);
	
	//开子进程将文件上传到fastdfds
	int pip[2] = {0};
	res = pipe(pip);
	if(res != 0)
	{
		LOG("fileuplaod", "upload", "pipe() error: %d", res);
		return -1;
	}
	pid_t pid = fork();
	if(-1 == pid)
	{
		LOG("fileuplaod", "upload", "fork() error: %d", pid);
		return -1;
	}
	else if(pid == 0)
	{
		close(pip[0]);
		dup2(pip[1], STDOUT_FILENO);
		res = 0;
		res = execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", filename, NULL);
			//res = execlp("fdfs_download_file", "fdfs_download_file", "/etc/fdfs/client.conf", argv[2], NULL);
		if(res != 0)
			LOG("fileuplaod", "upload", "execl() error: %d", res);

		close(pip[1]);
		exit(-1);
	}
	char resinfo[1024];
	close(pip[1]);
	res = read(pip[0], resinfo, sizeof(resinfo));
	int child_s;
	wait(&child_s);
	close(pip[0]);
	if(res < 0 || child_s != 0)
	{
		LOG("fileuplaod", "upload", "read() error: %d", child_s);
		return -1;
	}
	if(resinfo[res-1] == '\n')
		resinfo[res-1] = '\0';
	LOG("fileuplaod", "upload", "info: %s", resinfo);
	
	//将信息存入数据库
	redisContext *top =  rop_connectdb_nopwd("127.0.0.1", "6379");
	if(top==NULL)
	{
		LOG("fileuplaod", "upload", "rop_connectdb_nopwd() error");
		return -1;
	}
	char db_value[1024];
	sprintf(db_value, "%s||%s", filename, resinfo);
	res = rop_list_push( top, DB_INFONAME, db_value);
	if(res != 0)
	{
		LOG("fileuplaod", "upload", "rop_list_push() error:%d", res);
		rop_disconnect(top);
		return -1;
	}
	rop_disconnect(top);
	return 0;
}

void buffcp(char *buf, int len)
{
	int res = fread(buf, 1, len, stdin);
	
	return;
	
	int i;
	char *ch = buf;
	int chi;
	for(i=0; i<len; i++)
	{
		chi = getchar();
		*ch++ = chi;
		putchar(chi);
		
	}
}

int main(void)
{
	char line[200];
	char fname[100];
	int llen, content_len;
	char *end;
	while (FCGI_Accept() >= 0) {
		int isread = 0;
		char *buf = getenv("CONTENT_LENGTH");
		int len;

		printf("Content-type: text/html\r\n"
	    "\r\n"
	    "<title>FastCGI echo</title>"
	    "<h1>FastCGI echo</h1>\n");
		if (buf != NULL) {
			len = strtol(buf, NULL, 10);
		}
		else {
			len = 0;
		}
		if (len <= 0) {
			printf("<h2>upload file error!</h2>\n");
			continue;
		}
		
		char *info = malloc(len);
		if(info == NULL)
		{
			LOG("fileuplaod", "upload", "[-][upload]malloc line:%d", __LINE__);
			return -1;
		}
		char *tmp;
		if(info != NULL)
		{
			buffcp(info, len);
			
			llen = sizeof(line);
			tmp = getl(line, &llen, info);
			if(llen > 0)
			{
				end = memstr(tmp, len-(tmp-info), line);
				if(NULL != end)
				{
					if(*(end-2) == '\r')
						end -= 2;
					else
						end -= 1;
					
					llen = sizeof(line);
					tmp = getl(line, &llen, tmp);
					if(llen>0)
					{
						char *tmp_name = strstr(line, "filename=");
						if(tmp_name != NULL)
						{
							strcpy(fname, tmp_name+strlen("filename=\""));
							char *fnameend = strchr(fname, '\"');
							if(fnameend!=NULL)
								*fnameend = '\0';
							printf("fname=%s", fname);
							llen = sizeof(line);
							tmp = getl(line, &llen, tmp);
							llen = sizeof(line);
							tmp = getl(line, &llen, tmp);
							if(tmp != NULL)
							{
								content_len = end - tmp;
								int res = operat(fname, tmp, content_len);
								if(res==0)
								{
										printf("<h2>upload file success!!!!</h2>\n");
										unlink(fname);
										goto FREEINFO;
								}else
									LOG("fileuplaod", "upload", "[-][upload]operat line:%d", __LINE__);
							}else
								LOG("fileuplaod", "upload", "[-][upload]getl line:%d", __LINE__);
						}else
							LOG("fileuplaod", "upload", "[-][upload]strstr line:%d", __LINE__);
					}else
						LOG("fileuplaod", "upload", "[-][upload]getl line:%d", __LINE__);
					
				}else
					LOG("fileuplaod", "upload", "[-][upload]memstr line:%d", __LINE__);
			}else
				LOG("fileuplaod", "upload", "[-][upload]getl line:%d", __LINE__);
			printf("<h2>upload file error!</h2>\n");
FREEINFO:
			if(info != NULL)
				free(info);
		}

		//PrintEnv("Request environment", environ);
		//PrintEnv("Initial environment", initialEnv);
	} /* while */

	return 0;
}
