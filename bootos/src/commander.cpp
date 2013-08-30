#include <commander.h>
#include <pthread.h>

void commander::start()
{
	pthread_t thread;

	pthread_create(&thread, NULL, _thread_wrapper<commander, &commander::run>, this);

	//pthread_join(thread, NULL);
}

void commander::run()
{
	SOCKET sock;
	struct sockaddr_in srv_addr;

#ifdef _WIN32
	WSADATA wsa_data;
	if( WSAStartup( MAKEWORD( 1, 1 ), &wsa_data ) )//初始化
	{
		loger.log(SEVERE, "Error, can not initialize socket!\n");
		abort();
	}
#endif

	//  创建套接字
	if( (sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		loger.log(SEVERE, "创建套接字失败！\n");
		abort();
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(PORT);
	srv_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	memset( & ( srv_addr.sin_zero ), 0, sizeof( srv_addr.sin_zero ) );

	if (bind(sock, (struct sockaddr *)&srv_addr, sizeof(sockaddr)) == -1)
	{
		loger.log(SEVERE, "调用bind失败！\n");
#ifdef _WIN32
		WSACleanup( );
#endif
		abort();
	}

	// 监听
	if (listen(sock, BACK_LOG) == -1)
	{
		loger.log(SEVERE, "调用监听失败！\n");
#ifdef _WIN32
		WSACleanup( );
#endif
		abort();
	}

	while(true)
	{
		struct sockaddr_in cli_addr;
		int size = sizeof(sockaddr);
		SOCKET cli_sock = accept(sock, (struct sockaddr *) &cli_addr, &size);
		pthread_t handle_thread;
		pthread_create(&handle_thread, NULL, handle_socket, &cli_sock);
	}

	closesocket(sock);
#ifdef _WIN32
		WSACleanup( );
#endif
}
