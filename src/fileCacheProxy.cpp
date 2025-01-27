#include "fileCacheProxy.h"

fileCacheProxy::Garbo fileCacheProxy::m_garbo;
fileCacheProxy *fileCacheProxy::_Instance = new fileCacheProxy();

fileCacheProxy::fileCacheProxy()
{
  isReady = false;

  mPath2Handle["/"]         = INVOEKED_FUNC(httpd_handler);
  mPath2Handle["/fileinfo"] = INVOEKED_FUNC(fileinfo_handler);
  mPath2Handle["/upload"]   = INVOEKED_FUNC(upload_handler);
  mPath2Handle["/delete"]   = INVOEKED_FUNC(delete_handler);
  
}

fileCacheProxy* fileCacheProxy::getInstance()
{
    return fileCacheProxy::_Instance;
}

int fileCacheProxy::signal_handle(unsigned int signum)
{
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Receive signal {}", signum );
  switch (signum)
  {
  case SIGHUP:
  case SIGINT:
  case SIGQUIT:
  case SIGTERM:
    isReady = false;
    SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Ready to exit..." );
    break;
  
  default:
    break;
  }

  return 0;
}

int fileCacheProxy::init()
{
  if(isReady)
  {
    return 0;
  }

  /*=======================================

    Step1. Get work directory and make log directories

  =========================================*/
  pid = getpid();

  char proc_path[PATH_LEN];
  sprintf((char *)proc_path, "/proc/%d/exe", pid);
  readlink((const char *)proc_path, (char *)mWorkDir, sizeof(mWorkDir));
  char *endPos = strrchr(mWorkDir, '/');
  endPos[1] = 0;
  
  mlogDir       = string_format("%s/log", mWorkDir);
  mFclogFile    = string_format("%s/fileCacheProxy.log", mlogDir.c_str());
  
  mRecordDir    = string_format("%s/records/", mWorkDir);
  mProclogFile  = string_format("%s/process.log", mRecordDir.c_str());

  if(mkdir(mlogDir.c_str(), PRIVILEAGE_644))
  {
    perror("mkdir for log directory");
  }
  if(mkdir(mRecordDir.c_str(), PRIVILEAGE_644))
  {
    perror("mkdir for record diretory");
  }

  /*=======================================

    Step2. Initialize logger with sdflog

  =========================================*/

  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%l] %v");

  m_fc_rotating_logger = spdlog::rotating_logger_mt(FC_LOGGER_NAME, mFclogFile.c_str(), 1024 * 1024 * logFileSize, logFileBkupNum); 
  m_fc_rotating_logger->flush_on(spdlog::level::info);
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger, "LOGGER INITIALIZED.");

  fFileID = std::ofstream(mProclogFile);

  /*=======================================

    Step3. Load configurations.

  =========================================*/

  Config myConfig(configFileName);
  if(myConfig.Load())
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"----Config file Read Error! service exit!");
    return ERR_DO_NOT_EXIST;
  }

  std::string strVal = myConfig.getValue("localhost");
  if ( strVal.empty() )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"----Config->LocalHost Read Error! service exit!");
    return ERR_PARAM;
  }
  localhost = strVal;
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"----Config->LocalHost {}", localhost );

  strVal = myConfig.getValue("port");
  if ( strVal.empty() )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"----Config->Port Read Error! service exit!");
    return ERR_PARAM;
  }
  port = str2int(strVal);
  if( port < 0 || port > 65535 )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"----Config->Port Read Error! service exit!");
    return ERR_PARAM;
  }
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"----Config->port {}", port );

  strVal = myConfig.getValue("ThreadCount");
  if ( strVal.empty() )
  {
    ThreadCount = 1;
  }
  else
  {
    ThreadCount = str2int(strVal);
    if(ThreadCount < 1)
      ThreadCount = 1;
  }
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"----Config->ThreadCount {}", ThreadCount );

  strVal = myConfig.getValue("expiredays");
  if ( strVal.empty() )
  {
    expiredays = 7;
  }
  else
  {
    expiredays = str2int(strVal);
    if(expiredays < 0 )
    {
      expiredays = 7;
    }
  }
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"----Config->expiredays {}", expiredays );

  strVal = myConfig.getValue("clientConfPath");
  if ( !doFileExists(strVal) )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"----Config->clientConfPath Read Error! service exit!");
    return ERR_PARAM;
  }
  strncpy(mClientConfPath, strVal.c_str(), sizeof(mClientConfPath));
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"----Config->clientConfPath {}", mClientConfPath );
  
  /*=======================================

    Step 4. Bind socket

  =========================================*/

  listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_IP);
  if ( listenfd < 0 )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"socket Create failed! service exit!");
    return ERR_NETWORK;
  }

  int optval = 1;
  if ( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) // SO_REUSEADDR是让端口释放后立即就可以被再次使用
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"socket configure failed! service exit!");
    return ERR_NETWORK;
  }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family       = AF_INET;
  servaddr.sin_addr.s_addr  = inet_addr(localhost.c_str());
  servaddr.sin_port         = htons(port);
  if ( bind(listenfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"socket bind failed! service exit!");
    return ERR_NETWORK;
  }

  if ( listen(listenfd, SOMAXCONN) < 0 )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"socket listen failed! service exit!");
    return ERR_NETWORK;
  }

  /*=======================================

    Step 5. thread pool

  =========================================*/

  mThreadPool = new std::threadpool(ThreadCount);

  /*=======================================

    Step 6. fastdfs

  =========================================*/
  
	log_init();
  if ( fdfs_client_init(mClientConfPath) )
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"tracker group init failed! service exit!");
    return ERR_TRACKER;
  }

  /*=======================================

    Step end. 

  =========================================*/
  isReady = true;
  return SUCCESS;
}

int fileCacheProxy::startService(void)
{
  if ( !isReady)
  {
    return ERR_NOT_READY;
  }

  for ( int i = 0; ThreadCount > i; ++i )
  {
    TrackerServerInfo * info = new TrackerServerInfo;
    memset(info, 0, sizeof(TrackerServerInfo));
    ConnectionInfo * connectResult;
    int err_no;
    int testNum = 0;
    while ( testNum < MAX_CONNECTION_TEST )
    {
      if ( info->count > 0 ) tracker_disconnect_server(info);
      connectResult = tracker_get_connection_r( info, &err_no);
      if ( info->count > 0 ) break;
      sleep(1u);
      ++testNum;
      SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Thread {} trying {} time ...", i, testNum);
    }
    if(testNum >= MAX_CONNECTION_TEST)
    {
      continue;
    }

    event_base *ev = event_base_new();
    if ( nullptr == ev ) 
    {
      continue; 
    }

    evhttp* ev_listen = evhttp_new(ev);
    if ( nullptr == ev_listen ) 
    {
      event_base_free(ev);
      continue; 
    }

    // It's also OK to use evhttp_bind_socket to get listenfd directly
    if ( evhttp_accept_socket(ev_listen, listenfd) ) 
    {
      evhttp_free(ev_listen);
      event_base_free(ev);
      return ERR_EV_NEW;
    }

    threadParam * cbParam  = new threadParam(i, ev, ev_listen, info, connectResult);
    
    threadParams.push_back(cbParam);

    evhttp_set_gencb(ev_listen, static_cast<cb>(mPath2Handle["/"]), cbParam);
    for(auto &it : mPath2Handle)
    {
      evhttp_set_cb(ev_listen, it.first.c_str(), it.second, cbParam);
    }

    cbParam -> retval = mThreadPool->commit(event_base_dispatch, ev);
  }

  while(isReady)
  {
    sleep(1u); // s
  }

  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Service exited.");

  return 0;
}

void fileCacheProxy::httpd_handler(struct evhttp_request * req, void * arg)
{
  evhttp_send_reply(req, HTTP_400, "Bad Request", nullptr);
  return ;
}

void fileCacheProxy::upload_handler(struct evhttp_request * req, void * arg)
{
  // Process header 
  struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
  const char * file_ext_name = evhttp_find_header(headers, "FileExt");
	FDFSMetaData meta_list[MAX_FDFSMetaData_CNT] = {0};
	int meta_count = 0;

  // Process body
  evbuffer * buf = evhttp_request_get_input_buffer(req);
  if ( buf == nullptr )
  {
    evhttp_send_reply(req, HTTP_400, "Bad Request", nullptr);
    return ;
  }
  size_t file_size = evbuffer_get_length(buf);
  char * file_content = new char[file_size];
  evbuffer_copyout(buf, file_content, file_size);

  // Setup connection between storageServer
  threadParam * cbParam = (threadParam * )arg;
  ConnectionInfo * pTrackerServer = cbParam->connectResult;

	ConnectionInfo storageServer = {0};
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1] = {0};
	int store_path_index = 0;
  int result = tracker_query_storage_store( pTrackerServer, &storageServer, group_name, &store_path_index);
  if ( result != 0)
  {
    evhttp_send_reply(req, HTTP_503, "Service Temporarily Unavailable", nullptr);
    return ;
  }

  ConnectionInfo * pStorageServer = tracker_make_connection(&storageServer, &result);
  if(pStorageServer == nullptr)
  {
    evhttp_send_reply(req, HTTP_503, "Service Temporarily Unavailable", nullptr);
    return ;
  }

  // Uploading file;
  char remote_filename[PATH_LEN] = {0};
  result = storage_upload_by_filebuff(pTrackerServer, \
    pStorageServer, store_path_index, \
    file_content, file_size, file_ext_name, \
    meta_list, meta_count, \
    group_name, remote_filename);
  if(result != 0)
  {
    evhttp_send_reply(req, HTTP_503, "Service Temporarily Unavailable", nullptr);
    return ;
  }

  nlohmann::json j = nlohmann::json::object();
  j["FileID"] = string_format("%s/%s", group_name, remote_filename);
  std::string j_str = j.dump();

  evbuffer * resp_body = evbuffer_new();
  evbuffer_add_printf(resp_body, "%s", j_str.c_str());

  struct evkeyvalq *response_headers = evhttp_request_get_output_headers(req);
  evhttp_add_header(response_headers, "Content-Length", std::to_string(j_str.size()).c_str());
  evhttp_add_header(response_headers, "Content-Type", "application/json");

  evhttp_send_reply(req, HTTP_200, "OK", resp_body);
  evbuffer_free(resp_body);

  // Recording
  fFileID << j_str << std::endl;

  return ;
}

void fileCacheProxy::delete_handler(struct evhttp_request * req, void * arg)
{
  struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
  const char *fileID = evhttp_find_header(headers, "FileID");
  if ( fileID == nullptr )
  {
    evhttp_send_reply(req, HTTP_400, "FileID is NULL", nullptr);
    return ;
  }

  threadParam * cbParam = (threadParam * )arg;
  ConnectionInfo * pTrackerServer = cbParam->connectResult;

	int result = storage_delete_file1(pTrackerServer, NULL, fileID) ;
	if (result != 0)
	{
    evbuffer * resp_body = evbuffer_new();
    evbuffer_add_printf(resp_body, "%s", STRERROR(result));
    evhttp_send_reply(req, HTTP_400, "Bad Request", resp_body);
    evbuffer_free(resp_body);
    return ;
	}

  nlohmann::json j = nlohmann::json::object();
  std::string j_str = j.dump();

  evbuffer * resp_body = evbuffer_new();
  evbuffer_add_printf(resp_body, "%s", j_str.c_str());

  struct evkeyvalq *response_headers = evhttp_request_get_output_headers(req);
  evhttp_add_header(response_headers, "Content-Length", std::to_string(j_str.size()).c_str());
  evhttp_add_header(response_headers, "Content-Type", "application/json");

  evhttp_send_reply(req, HTTP_200, "OK", nullptr);

  return ;
}
void fileCacheProxy::fileinfo_handler(struct evhttp_request * req, void * arg)
{
  struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
  const char *fileID = evhttp_find_header(headers, "FileID");
  if ( fileID == nullptr )
  {
    evhttp_send_reply(req, HTTP_400, "FileID is NULL", nullptr);
    return ;
  }

  FDFSFileInfo FileInfo;
  if ( fdfs_get_file_info1(fileID, &FileInfo) )
  {
    evhttp_send_reply(req, HTTP_400, "Bad Request", nullptr);
    return ;
  }

  nlohmann::json j = nlohmann::json::object();
  j["FileInfo"]["CreateTime"] = get_time_now();;
  j["FileInfo"]["FileSize"]   = FileInfo.file_size;
  j["FileInfo"]["FileCrc32"]  = FileInfo.crc32;
  std::string j_str = j.dump();
    
  struct evbuffer *buff = evbuffer_new();
  evbuffer_add_printf(buff, "%s", j_str.c_str());

  struct evkeyvalq *response_headers = evhttp_request_get_output_headers(req);
  evhttp_add_header(response_headers, "Content-Length", std::to_string(j_str.size()).c_str());
  evhttp_add_header(response_headers, "Content-Type", "application/json");

  evhttp_send_reply(req, HTTP_OK, "OK", buff);
  evbuffer_free(buff);

  return ;
}

fileCacheProxy::~fileCacheProxy()
{
  for(auto it : threadParams) 
  {
    delete it;
  }
  fdfs_client_destroy();
  close(listenfd);
  delete(mThreadPool);
}

fileCacheProxy::threadParam::threadParam
(int _threadID, event_base * _ev, evhttp * _ev_listen, TrackerServerInfo * _info, ConnectionInfo * _connectResult ):
threadID(_threadID), ev(_ev), ev_listen(_ev_listen), info(_info), connectResult(_connectResult)
{
  
}

fileCacheProxy::threadParam::~threadParam()
{
  tracker_close_connection_ex(connectResult, true);
  
  if(info)
  {
    delete info;
  }
  
  event_base_loopbreak(ev); // why can't exit ???
  // retval.get(); 

  evhttp_free(ev_listen);
  event_base_free(ev);
}