#include <rtthread.h>
#include <lwip/netdb.h> /* 为了解析主机名，需要包含netdb.h头文件 */
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */
static const char send_data[] = "This is TCP Server from RT-Thread.";
void tcp_senddata(const char* url, int port, const char * data)
{
	struct hostent *host;
	int sock, err, result, timeout, index;
	struct sockaddr_in server_addr;
	rt_uint8_t *buffer_ptr;

	/* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
	host = gethostbyname(url);
	/* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		/* 创建socket失败 */
		rt_kprintf("Socket error\n");
		return;
	}

	/* 神奇内存 */
	buffer_ptr = rt_malloc(sizeof(data));
	/* 构造发生数据 */
	for (index = 0; index < sizeof(data); index ++)
		buffer_ptr[index] = index & 0xff;

	timeout = 10;
	/* 设置发送超时时间10ms */
	lwip_setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	/* 初始化预连接的服务端地址 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	/* 连接到服务端 */
	err = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	rt_kprintf("TCP thread connect error code: %d\n", err);
result=-1;
	while(result == -1)
	{
		/* 发送数据到sock连接 */
		//result = send(sock, buffer_ptr, length, MSG_DONTWAIT);
		result=send(sock, data, strlen(data), 0);
		if(result == -1) //数据发送错误处理
		{
			rt_kprintf("TCP thread send error: %d\n", result);
			lwip_close(sock);	//关闭连接，重新创建连接
			rt_thread_delay(10);
			if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
				rt_kprintf("TCP Socket error:%d\n",sock);
			err = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
			rt_kprintf("TCP thread connect error code: %d\n", err);
		}
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* 输出tcpclient函数到finsh shell中 */
FINSH_FUNCTION_EXPORT(tcp_senddata, send a packet through tcp connection);
#endif

