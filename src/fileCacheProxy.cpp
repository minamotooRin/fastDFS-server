#include "fileCacheProxy.h"

using namespace std;

fileCacheProxy::Garbo fileCacheProxy::m_garbo;
fileCacheProxy *fileCacheProxy::_Instance = new fileCacheProxy();

fileCacheProxy::fileCacheProxy()
{
  isReady = false;

  mPath2Handle["/fileinfo"] = fileinfo_handler;
  mPath2Handle["/upload"]   = upload_handler;
  mPath2Handle["/delete"]   = delete_handler;

}

fileCacheProxy* fileCacheProxy::getInstance()
{
    return fileCacheProxy::_Instance;
}

bool fileCacheProxy::doFileExists(string& name) {
    ifstream f(name.c_str());
    return f.good();
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
  
  mlogDir       = string_format("%s/log/", mWorkDir);
  mFclogFile    = string_format("%s/log/fileCacheProxy.log", mWorkDir);
  mProclogFile  = string_format("%s/log/process.log", mWorkDir);
  mRecordDir    = string_format("%s/records/", mWorkDir);

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

  m_process_rotating_logger = spdlog::rotating_logger_mt(PROCESS_LOGGER_NAME, mProclogFile.c_str(), 1024 * 1024 * logFileSize, logFileBkupNum); \
  m_process_rotating_logger->flush_on(spdlog::level::info);
  SPDLOG_LOGGER_INFO(m_process_rotating_logger, "LOGGER INITIALIZED.");

  /*=======================================

    Step3. Load configurations.

  =========================================*/

  Config myConfig(configFileName);
  if(myConfig.Load())
  {
    SPDLOG_LOGGER_ERROR(m_fc_rotating_logger,"----Config file Read Error! service exit!");
    return ERR_DO_NOT_EXIST;
  }

  string strVal = myConfig.getValue("localhost");
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

  mThreadPool = new threadpool(ThreadCount);

  /*=======================================

    Step 6. fastdfs

  =========================================*/
  
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
    TrackerServerInfo info = {0};
    int testNum = 0;
    while ( testNum < MAX_CONNECTION_TEST )
    {
      if ( info.count > 0 ) tracker_disconnect_server(&info);
      ConnectionInfo *connectResult = tracker_get_connection_r( &info, &err_no);
      if ( info.count > 0 ) break;
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

    threadParam *cbParam  = new threadParam;
    cbParam->threadID     = i;
    cbParam->ev           = ev;
    cbParam->ev_listen    = ev_listen;
    cbParam->info         = info;
    threadParams.push_back(cbParam);

    // It's also OK to use evhttp_bind_socket to get listenfd directly
    if ( evhttp_accept_socket(ev_listen, listenfd) ) 
    {
      evhttp_free(ev_listen);
      event_base_free(ev);
      return ERR_EV_NEW;
    }

    evhttp_set_gencb(ev_listen, httpd_handler, cbParam);
    for(auto &it : mPath2Handle)
    {
      evhttp_set_cb(ev_listen, it.first.c_str(), it.second, cbParam);
    }

    mThreadPool->commit(event_base_dispatch, ev);

  }

  while(isReady)
  {
    sleep(1u); // s
  }
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Service exited.");

  return 0;
}

int fileCacheProxy::signal_handle(unsigned int signum)
{
  SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Receive signal {}", signum );
  if ( signum == SIGHUP ||
       signum == SIGINT ||
       signum == SIGQUIT ||
       signum == SIGTERM )
  {
    isReady = false;
    SPDLOG_LOGGER_INFO(m_fc_rotating_logger,"Ready to exit..." );
  }
  return 0;
}


void fileCacheProxy::httpd_handler(struct evhttp_request * req, void * arg)
{
  evhttp_send_reply(req, HTTP_400, "Bad Request", nullptr);
  return ;
}
void fileCacheProxy::upload_handler(struct evhttp_request * req, void * arg)
{
  
  evhttp_send_reply(req, HTTP_200, "OK", nullptr);
  return ;
}
void fileCacheProxy::delete_handler(struct evhttp_request * req, void * arg)
{
  evhttp_send_reply(req, HTTP_503, "Service Temporarily Unavailable", nullptr);
  return ;
}
void fileCacheProxy::fileinfo_handler(struct evhttp_request * req, void * arg)
{
  struct evkeyvalq *headers = evhttp_request_get_input_headers(req);
  const char *fileID = evhttp_find_header(headers, "FileID");
  if ( fileID == nullptr )
  {
    return evhttp_send_reply(req, HTTP_400, "Bad Request", nullptr);
  }

  FDFSFileInfo FileInfo;
  if ( fdfs_get_file_info1(fileID, &FileInfo) )
  {
    return evhttp_send_reply(req, HTTP_400, "Bad Request", nullptr);
  }

  time_t timep;
  time (&timep);
  char sTime[64];
  strftime(sTime, sizeof(sTime), "%Y-%m-%d %H:%M:%S",localtime(&timep) );

  char replyContext[HTTP_BODY_LEN] = {0};
  snprintf(
    replyContext,
    HTTP_BODY_LEN,
    "<FileInfo><CreateTime>%s</CreateTime><FileSize>%ld</FileSize><FileCrc32>%d</FileCrc32></FileInfo>",
    sTime,
    FileInfo.file_size,
    FileInfo.crc32);
    
  struct evbuffer *buff = evbuffer_new();
  evbuffer_add_printf(buff, "%s", replyContext);

  evhttp_add_header(headers, "Content-Length", std::to_string(strlen(replyContext)).c_str());
  evhttp_add_header(headers, "Content-Type", "fileinfo");

  evhttp_send_reply(req, HTTP_OK, "OK", buff);

  evbuffer_free(buff);

  return ;
}