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
#include "logger.h"

#include "event2/event.h"
#include "event2/http.h"
#include "event2/buffer.h"
#include "thread_pool.h"

#include "fastdfs/client_func.h"
#include "fastdfs/fdfs_global.h"
#include "fastdfs/fdfs_client.h"

#define SIGHUP     1ULL
#define SIGINT     2ULL
#define SIGQUIT    3ULL
#define SIGTERM    15ULL

#define PATH_LEN      256
#define HTTP_BODY_LEN 2048

#define ERR_NULL             100
#define ERR_DO_NOT_EXIST     102
#define ERR_PARAM            103
#define ERR_NETWORK          104
#define ERR_TRACKER          105
#define ERR_EV_NEW           106

#define HTTP_OK     200
#define HTTP_200    HTTP_OK
#define HTTP_400    400
#define HTTP_503    503

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
        event_base *ev;
        evhttp* ev_listen;
        TrackerServerInfo info;
        ~threadParam(){
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

    int initLog();
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

    std::map<std::string, cb> mPath2Handle;

    int isReady;
    pid_t pid;

    char mWorkDir[PATH_LEN];
    char mlogDir[PATH_LEN];
    char mRecordDir[PATH_LEN];

    char mFclogFile[PATH_LEN];
    char mProclogFile[PATH_LEN];

    fixed_thread_pool *mThreadPool;

    std::shared_ptr<spdlog::logger> m_fc_rotating_logger;
    std::shared_ptr<spdlog::logger> m_process_rotating_logger;

    std::vector<threadParam *> threadParams;

    int err_no;

    int listenfd;

    // Load from config
    std::string localhost;
    uint16_t port;
    int ThreadCount;
    int expiredays;
    char mClientConfPath[PATH_LEN];
};


#endif