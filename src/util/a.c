#include "myfastdfs.h"
#include <time.h>

int main(void)
{
	mf_info info = {0};
	int res = my_upload("a.txt", "/etc/fdfs/client.conf", &info);
	struct tm *pTmp = localtime(&info.create_timestamp); 
	
	if(res == 0)
	{
		printf("%s|%s|%s|%d-%d-%d %d:%d:%d|NULL|NULL", info.file_id, info.source_ip_addr, "a.txt",	\
										 pTmp->tm_year + 1900, pTmp->tm_mon + 1, pTmp->tm_mday, pTmp->tm_hour, pTmp->tm_min, pTmp->tm_sec	\
										 );
	}
	else if(strlen(info.source_ip_addr)>0)
	{
		info.create_timestamp = time(NULL);
		printf("%s|NULL|%s|%d-%d-%d %d:%d:%d|NULL|NULL", info.file_id, "a.txt",	\
										 pTmp->tm_year + 1900, pTmp->tm_mon + 1, pTmp->tm_mday, pTmp->tm_hour, pTmp->tm_min, pTmp->tm_sec	\
										 );
	}
	return 0;
}