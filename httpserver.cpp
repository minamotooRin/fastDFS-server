int httpserver_bindsocket(const char *localhost, uint16_t port)
{
  int iRet;
  int listenfd;

  if ( nullptr == localhost  )
  {
    return -1;
  }

  listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if ( listenfd < 0 )
  {
    return -1;
  }

  int optval = 1;
  iRet = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if ( iRet < 0 )
  {
    return -1;
  }

  struct sockaddr servaddr; // [rsp-28h] [rbp-28h]
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(localhost);
  servaddr.sin_port = htons(port);

  iRet = bind(listenfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  if ( iRet < 0 )
  {
    return -1;
  }

  iRet = listen(listenfd, SOMAXCONN);
  if ( iRet < 0 )
  {
    return -1;
  }

  // 可该改变flag后重新设置，此处意味不明
  int flags = fcntl(listenfd, F_GETFL, 0);
  if ( flags < 0 || fcntl(listenfd, F_SETFL, flags) < 0 )
    return -1;

  return listenfd;

  // close(listenfd);
}

int  connect_tracker(int a1)
{
  int result; // rax
  int v2; // [rsp-14h] [rbp-14h]
  _DWORD *v3; // [rsp-10h] [rbp-10h]

  v3 = (char *)g_pTrackerServer + 24 * a1;
  while ( 1 )
  {
    if ( *v3 > 0 )
      tracker_disconnect_server_ex(v3, 1LL);
    tracker_get_connection_r_ex(&g_tracker_group, v3, &v2);
    if ( v3 )
    {
      result = (unsigned int)*v3;
      if ( (signed int)result > 0 )
        break;
    }
    sleep(1u);
  }
  return result;
}

int httpserver_start(const char *localhost, uint16_t port, int iThreadCount)
{
  int listenfd;
  event_base *event_base;
  struct event* ev_listen; 

  listenfd = httpserver_bindsocket(localhost, port);
  if ( listenfd & 0x80000000 ) // 即 < 0
  {
    return -1;
  }

  g_pWorkThreads = malloc(sizeof(pthread_t) * iThreadCount);
  if ( nullptr == g_pWorkThreads )
  {
    return -1;
  }
  memset(g_pWorkThreads, 0, sizeof(pthread_t) * iThreadCount);

  g_pTrackerServer = malloc(24LL * iThreadCount);
  if ( nullptr == g_pTrackerServer )
  {
    return -1;
  }
  memset(g_pTrackerServer, 0, 24LL * iThreadCount);

  for ( int i = 0; iThreadCount > i; ++i )
  {
    event_base = event_base_new();
    if ( nullptr == event_base )
      return -1;
    ev_listen = evhttp_new(event_base);
    if ( nullptr == ev_listen )
      return -1; 
    if ( (unsigned int)evhttp_accept_socket(ev_listen, listenfd) )
      return -1;
    evhttp_set_gencb(ev_listen, httpd_handler, i);
    evhttp_set_cb(ev_listen, "/fileinfo", fileinfo_request_cb, i);
    evhttp_set_cb(ev_listen, "/upload", upload_request_cb, i);
    evhttp_set_cb(ev_listen, "/delete", delete_request_cb, i);
    if ( pthread_create(g_pWorkThreads + i, nullptr, event_base_dispatch, event_base))
      return -1;
    connect_tracker(i);
  }

  for ( int i = 0; iThreadCount > i; ++i )
    pthread_join(*(g_pWorkThreads + i), nullptr);

  return 0;
}