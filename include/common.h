#ifndef COMMON_H
#define COMMON_H

#define SIGHUP     1ULL
#define SIGINT     2ULL
#define SIGQUIT    3ULL
#define SIGTERM    15ULL

#define PATH_LEN      256
#define HTTP_BODY_LEN 2048

#define SUCCESS              0
#define ERR_NULL             100
#define ERR_NOT_READY        101
#define ERR_DIR              102
#define ERR_PARAM            103
#define ERR_NETWORK          104
#define ERR_TRACKER          105
#define ERR_EV_NEW           106
#define ERR_DO_NOT_EXIST     107

#define HTTP_OK     200
#define HTTP_200    HTTP_OK
#define HTTP_400    400
#define HTTP_503    503

#endif