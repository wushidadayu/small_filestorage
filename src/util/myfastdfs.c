include "myfastdfs.h"

int my_upload(char *filename,const char *confname, mf_info *info)
{
	char *local_filename = filename;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	ConnectionInfo *pTrackerServer;
	int result;
	int store_path_index;
	ConnectionInfo storageServer;
	char file_id[128];
	FDFSFileInfo file_info;
	

	//ignore_signal_pipe();

	if ((result=fdfs_client_init(confname)) != 0)
	{
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		return errno != 0 ? errno : ECONNREFUSED;
	}

	*group_name = '\0';
	if ((result=tracker_query_storage_store(pTrackerServer, \
	                &storageServer, group_name, &store_path_index)) != 0)
	{
		fdfs_client_destroy();
		return result;
	}

	result = storage_upload_by_filename1(pTrackerServer, \
			&storageServer, store_path_index, \
			local_filename, NULL, \
			NULL, 0, group_name, file_id);
	if (result == 0)
	{
		memset(&file_info, 0, sizeof(file_info));
		result = fdfs_get_file_info_ex1(file_id, true, &file_info);
		if (result == 0)
		{
			memcpy(info, &file_info, sizeof(file_info));
		}
		strcpy(info->file_id, file_id);
	}

	tracker_disconnect_server_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}

/*
int main(void)
{
	mf_info info = {0};
	int res = my_upload("a.txt", "/etc/fdfs/client.conf", &info);
	if(res == 0)
	{
		char szDatetime[32];
		printf("file_id: %s\n", info.file_id);
		printf("source storage id: %d\n", info.source_id);
		printf("source ip address: %s\n", info.source_ip_addr);
		printf("file create timestamp: %s\n", formatDatetime(
			info.create_timestamp, "%Y-%m-%d %H:%M:%S", \
			szDatetime, sizeof(szDatetime)));
		printf("file size: %"PRId64"\n", \
			info.file_size);
		printf("file crc32: %u (0x%08X)\n", \
			info.crc32, info.crc32);
	}
	else
		printf("file_id: %s\n", info.file_id);
	return 0;
}
*/