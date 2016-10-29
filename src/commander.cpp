#include <commander.h>
#include <pthread.h>

void commander::start() {
  pthread_t thread;

  pthread_create(&thread,
                 NULL,
                 _thread_wrapper < commander,
                 &commander::run > ,
                 this);

  //pthread_join(thread, NULL);
}

void commander::run() {
  SOCKET sock;
  struct sockaddr_in srv_addr;

#ifdef _WIN32
  WSADATA wsa_data;
  if( WSAStartup( MAKEWORD( 1, 1 ), &wsa_data ) )//初始化
  {
      loger.log(ERROR, "Can not initialize socket!\n");
      abort();
  }
#endif

  //  创建套接字
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    loger.log(ERROR, "创建套接字失败！\n");
    abort();
  }

  int port = atoi(cfr.get_config_value("config.bootos_port").c_str());
  if (!port) {
    port = PORT;
  }

  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(port);
#ifdef _WIN32
  srv_addr.sin_addr.S_un.S_addr = INADDR_ANY;
#else
  srv_addr.sin_addr.s_addr = INADDR_ANY;
#endif
  memset(&(srv_addr.sin_zero), 0, sizeof(srv_addr.sin_zero));

  if (bind(sock, (struct sockaddr *) &srv_addr, sizeof(sockaddr)) == -1) {
    loger.log(ERROR, "调用bind失败！\n");
#ifdef _WIN32
    WSACleanup( );
#endif
    abort();
  }

  // 监听
  if (listen(sock, BACK_LOG) == -1) {
    loger.log(ERROR, "调用监听失败！\n");
#ifdef _WIN32
    WSACleanup( );
#endif
    abort();
  }

  loger.log(INFO, "Server started\n");
  while (true) {
    struct sockaddr_in cli_addr;
    int size = sizeof(sockaddr);
    SOCKET cli_sock =
        accept(sock, (struct sockaddr *) &cli_addr, (socklen_t * ) & size);
    loger.log(INFO, "receive a connection.\n");
    if (cli_sock != INVALID_SOCKET) {
      pthread_t handle_thread;
      pthread_create(&handle_thread, NULL, handle_socket, &cli_sock);
    }
  }

  closesocket(sock);
#ifdef _WIN32
  WSACleanup( );
#endif
}
