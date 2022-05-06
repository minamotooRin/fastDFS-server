#include <iostream>
#include "fileCacheProxy.h"

void signal_handler(int signum)
{
  fileCacheProxy::getInstance()->signal_handle(signum);
}

int main(int argc, const char **argv, const char **envp)
{
  signal(SIGHUP, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);

  int iRet = fileCacheProxy::getInstance()->init();
  if( iRet )
  {
    std::cerr << "fileCacheProxy init failed. [Errno:" << iRet << "]" << std::endl;
    return 0;
  }

  std::cout << "start service ..." << std::endl;
  fileCacheProxy::getInstance()->startService();
  std::cout << "stop service ..." << std::endl;

  return 0;
}