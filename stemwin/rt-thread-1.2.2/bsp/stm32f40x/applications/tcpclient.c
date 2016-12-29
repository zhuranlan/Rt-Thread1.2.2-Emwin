#include <rtthread.h>
#include <lwip/netdb.h> /* 为了解析主机名，需要包含netdb.h头文件 */
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */


extern const unsigned char bmpImage[230454];
//extern void LCD_DrawPoint(unsigned short x,unsigned short y,unsigned short color);

#define BUF_SIZE   2048

static unsigned char send_buf[2048];//

void tcpclient(const char *url, int port)
{
    char *recv_data; /*接收缓冲区指针*/
    struct hostent *host; /*用于通过DNS解析服务器端信息*/
    int sockfd;        /* socket描述符*/
    int bytes_received;
    struct sockaddr_in server_addr; /* 存储服务端IP和端口号*/

    /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    host = gethostbyname(url);

    /* 分配用于存放接收数据的缓冲 */
    recv_data = rt_malloc(BUF_SIZE);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }

    /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        /* 创建socket失败 */
        rt_kprintf("Socket error\n");

        /* 释放接收缓冲 */
        rt_free(recv_data);
        return;
    }

    /* 初始化预连接的服务端地址 htons*/
    server_addr.sin_family = AF_INET;  		/* IPv4因特网域*/
    server_addr.sin_port = htons(port);    /* 服务器端的端口 这里进行字节序转换*/
    server_addr.sin_addr = *((struct in_addr *)host->h_addr); /* 主机IP信息*/
    /*
    	注意上面不用inet_addr（“192.168.2.1”）这种形式来做主机IP的网络字节序转换
        是由于笔者用那种方式没调通，暂时来不知道为何
    */
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 连接到服务端 */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        /* 连接失败 */
        rt_kprintf("Connect error\n");

        /*释放接收缓冲 */
        rt_free(recv_data);
        return;
    }

    while (1)
    {
        /* 从sock连接中接收最大BUFSZ - 1字节数据 */
        bytes_received = recv(sockfd, recv_data, BUF_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            /* 接收失败，关闭这个连接 */
            closesocket(sockfd);

            /* 释放接收缓冲 */
            rt_free(recv_data);
            break;
        }

        /* 有接收到数据，把末端清零 */
        recv_data[bytes_received] = '\0';

        if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
        {
            /* 如果是首字母是q或Q，关闭这个连接 */
            closesocket(sockfd);

            /* 释放接收缓冲 */
            rt_free(recv_data);
            break;
        }
        else
        {
            /* 在控制终端显示收到的数据 */
            //rt_kprintf("\nRecieved data = %s " , recv_data);

            if(rt_memcmp(recv_data , (unsigned char*)&"GetImage",8) == 0)
            {
                //获取图片命令：
                //GetImage{4字节图片位置}

                //返回数据：
                //每个数据包为2048字节
                //ImgPagk:{4字节图片大小+4字节该包数据长度+2032图片数据}

                int ImageLen = sizeof(bmpImage);//取得图片大小
                int GetDataIndex = 0;//
                int ImgPackLen = 2032;//

                rt_memcpy((unsigned char*)&GetDataIndex,recv_data + 8,4);//

                if((ImageLen - GetDataIndex) < 2032)
                    ImgPackLen = (ImageLen - GetDataIndex);//

                rt_memcpy((unsigned char*)&send_buf,(unsigned char*)"ImgPagk:",8);//
                rt_memcpy((unsigned char*)&send_buf + 8,(unsigned char*)&ImageLen,4);//
                rt_memcpy((unsigned char*)&send_buf + 12,(unsigned char*)&ImgPackLen,4);//
                rt_memcpy((unsigned char*)&send_buf + 16,(unsigned char*)&bmpImage + GetDataIndex,ImgPackLen);//

                send(sockfd, send_buf, sizeof(send_buf), 0);
            }
            else if(rt_memcmp(recv_data , (unsigned char*)&"SetImage:",8) == 0)
            {
                //设置图片命令：
                //SetImage{2字节x,2字节y,960byte}

                unsigned short StartY,Color565,TrgX,TrgY;//
                int TempCount1;//
                unsigned char* pDataIndex;//

                rt_memcpy((unsigned char*)&StartY,recv_data + 10,2);//

                TempCount1 = 0;//
                for(TrgY=StartY; TrgY<(StartY + 2); TrgY++)
                {
                    for(TrgX=0; TrgX<240; TrgX++)
                    {
                        pDataIndex = recv_data + 12 + (TempCount1 * 2);//
                        rt_memcpy((unsigned char*)&Color565,pDataIndex,2);//
                        //LCD_DrawPoint(TrgX ,TrgY,Color565 );//

                        TempCount1++;//
                    }
                }

                rt_memcpy((unsigned char*)&send_buf,(unsigned char*)"SetImgOK:",8);//
                send(sockfd, send_buf, sizeof(send_buf), 0);
            }
        }

        /* 发送数据到sock连接 */
        //send(sock, send_data, strlen(send_data), 0);
    }
    rt_kprintf("Disonnect\n");
    return;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
/* 输出tcpclient函数到finsh shell中 */
FINSH_FUNCTION_EXPORT(tcpclient, startup tcp client);
#endif
