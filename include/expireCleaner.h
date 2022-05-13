#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fastdfs/client_func.h"
#include "fastdfs/fdfs_global.h"
#include "fastdfs/fdfs_client.h"

#define PROCESS_COUNT	10

typedef struct {
	int file_type;  //index
	char *file_id;
} FileEntry;

typedef struct {
	int bytes;  //file size
	char *filename;
	int count;   //total file count
	int delete_count;
	int success_count;  //success upload count
	int64_t time_used;  //unit: ms
} TestFileInfo;
