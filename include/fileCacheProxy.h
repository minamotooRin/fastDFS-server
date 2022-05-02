#ifndef _FILE_CACHE_PROXY_H
#define _FILE_CACHE_PROXY_H

#include <vector>

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/types.h>

#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#include "configReader.h"
#include "utilities.h"
#include "logger.h"
#include "common.h"

#include "event.h"
#include "http.h"
#include "buffer.h"

#include "thread_pool.h"

#include "fastdfs/client_func.h"
#include "fastdfs/fdfs_global.h"
#include "fastdfs/fdfs_client.h"

typedef void (*cb)(struct evhttp_request *, void *);

class fileCacheProxy
{
public:

    static fileCacheProxy *getInstance();

    class Garbo{
    public:
        ~Garbo(){
            delete fileCacheProxy::_Instance;
            fileCacheProxy::_Instance = nullptr;
        }
    };
    static Garbo m_garbo;

    ~fileCacheProxy()
    {
        close(listenfd);
        for(auto it : threadParams) 
        {
          delete it;
        }
        delete(mThreadPool);
    }

    int init();
    int startService(void);
    int signal_handle(unsigned int signum);

    static bool doFileExists(std::string& name);

private:

    struct threadParam
    {
        int threadID;
        event_base * ev;
        evhttp * ev_listen;

        TrackerServerInfo * info;
        ConnectionInfo * connectResult;

        threadParam(int _threadID, event_base * _ev, evhttp * _ev_listen, TrackerServerInfo * _info, ConnectionInfo * _connectResult );

        ~threadParam(){
            if(info)
            {
                delete info;
            }
            if(ev_listen)
            {
                evhttp_free(ev_listen);
            }
            if(ev)
            {
                event_base_loopbreak(ev);
                event_base_free(ev);
            }
        }
    };

    fileCacheProxy();
    static fileCacheProxy *_Instance;
    
    static void httpd_handler(struct evhttp_request * req, void * arg);
    static void upload_handler(struct evhttp_request * req, void * arg);
    static void delete_handler(struct evhttp_request * req, void * arg);
    static void fileinfo_handler(struct evhttp_request * req, void * arg);

    static constexpr int PRIVILEAGE_644 = (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    static constexpr int logFileSize                 = 4;
    static constexpr int logFileBkupNum              = 4;
    static constexpr const char* configFileName      = "fileCacheProxy.conf" ;
    static constexpr const char* FC_LOGGER_NAME      = "fileCacheProxy";
    static constexpr const char* PROCESS_LOGGER_NAME = "process";

    static constexpr int MAX_CONNECTION_TEST = 16;
    static constexpr int MAX_FDFSMetaData_CNT = 32;

    int isReady;
    pid_t pid;
    char mWorkDir[PATH_LEN];

    int listenfd;
    std::map<std::string, cb> mPath2Handle;

    std::threadpool *mThreadPool;
    std::vector<threadParam *> threadParams;

    // Logger
    std::string mlogDir;
    std::string mFclogFile;
    std::shared_ptr<spdlog::logger> m_fc_rotating_logger;

    // FDFS logger
    std::string mRecordDir;
    std::string mProclogFile;
    std::shared_ptr<spdlog::logger> m_process_rotating_logger;

    // Load from config
    std::string localhost;
    uint16_t port;
    int ThreadCount;
    int expiredays;
    char mClientConfPath[PATH_LEN];

};


#endif