#include "expireCleaner.h"

int main(int argc, char *argv[])
{
	ConnectionInfo *pTrackerServer;
	int result;
	char file_id[ID_LEN];
	
	if (argc < 3)
	{
		printf("Usage: %s <config_file> <file_of_fileID_list>\n", argv[0]);
		return 1;
	}

	log_init();
	g_log_context.log_level = LOG_ERR;
	ignore_signal_pipe();

	Config conf(argv[1]);
	if(conf.Load() != 0 )
	{
		printf("Cannot open %s.\n", argv[1]);
		return 0;
	}

	std::string conf_filename = conf.getValue("clientConfPath");
	if ((result=fdfs_client_init(conf_filename.c_str())) != 0)
	{
		return result;
	}

	pTrackerServer = tracker_get_connection();
	if (pTrackerServer == NULL)
	{
		fdfs_client_destroy();
		return errno != 0 ? errno : ECONNREFUSED;
	}

	FILE * fp = fopen(argv[2], "r");
	if( fp == NULL )
	{
		printf("Cannot open %s.\n", argv[2]);
		return 0;
	}

	while(fgets(file_id, sizeof(file_id), fp)) {
		if ((result=storage_delete_file1(pTrackerServer, NULL, file_id)) != 0)
		{
			printf("delete file: %s fail, " \
				"error no: %d, error info: %s\n", \
				file_id, result, STRERROR(result));
		}
		else
		{
			printf("delete file: %s success.\n", file_id);
		}
	}

	tracker_close_connection_ex(pTrackerServer, true);
	fdfs_client_destroy();

	return result;
}

