#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
	time_t create_timestamp;
	int crc32;
	int source_id;   //source storage id
	int64_t file_size;
	char source_ip_addr[20];  //source storage ip address
	char file_id[128];
} mf_info;

int my_upload(char *filename,const char *confname, mf_info *info);
