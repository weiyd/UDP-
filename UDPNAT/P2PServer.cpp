/*  本源码参考较多网络资料，仅供学习UDP打洞操作，任何利用本代码违法，违规的软件开发将不被认可
*
*	择善教育 - 做最好的教育
*
*  主页 http://www.doedu.com.cn/
*
*  C/C++ 交流群 : 243215922
*
*  左手老师 : 691714544
*
*  学习热线Q : 921700006
*
*/

#pragma comment(lib, "ws2_32.lib")

#include "windows.h"
#include "..\common\msgproto.h"
//#include "..\Exception.h"

USHORT g_nServerPort = SERVER_PORT;//服务器端口

UserList ClientList;//实例化用户信息链表 
//初始化
void InitWinSock()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Windows sockets 2.2 startup");
	}
	else{
		printf("Using %s (Status: %s)\n",
			wsaData.szDescription, wsaData.szSystemStatus);
		printf("with API versions %d.%d to %d.%d\n\n",
			LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion),
			LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));	
	}
}
//生成套接字
SOCKET mksock(int type)//1是TCP、2是UDP、3是原始套接字
{
	SOCKET sock = socket(AF_INET, type, 0);//目前仅支持AF_INET的网络地址格式 0代表不用套接字协议
	if (sock < 0)
	{
        printf("create socket error");
	}
	return sock;
}

stUserListNode GetUser(char *username)//根据用户名提取用户信息
{
	UserList::iterator UserIterator = ClientList.begin();
	for(;	UserIterator!=ClientList.end();++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), username) == 0 )
			return *(*UserIterator);
	}
	return *(*UserIterator);//得到用户名 IP 端口
}

int main(int argc, char* argv[])
{
	if ( argc > 1 )
	{
		g_nServerPort = atoi( argv[ 1 ] );
	}

	try{
		InitWinSock();//初始化套接字
		
		SOCKET PrimaryUDP;
		PrimaryUDP = mksock(SOCK_DGRAM);//建立套接字

		sockaddr_in local;
		local.sin_family=AF_INET;//使用TCP/IP协议簇
		local.sin_port= htons(g_nServerPort); //套接字端口号
		local.sin_addr.s_addr = htonl(INADDR_ANY);//监听任意地址
		int nResult=bind(PrimaryUDP,(sockaddr*)&local,sizeof(sockaddr));//进行绑定监听
		if(nResult==SOCKET_ERROR)
			printf("bind error");

		sockaddr_in sender;
		stMessage recvbuf;
		memset(&recvbuf,0,sizeof(stMessage));

		// 开始主循环.
		// 主循环负责下面几件事情:
		// 一:读取客户端登陆和登出消息,记录客户列表
		// 二:转发客户p2p请求
		for(;;)
		{
			int dwSender = sizeof(sender);
			int ret = recvfrom(PrimaryUDP, (char *)&recvbuf, sizeof(stMessage), 0, (sockaddr *)&sender, &dwSender);
			if(ret <= 0)
			{
				printf("recv error");
				continue;
			}
			else
			{
				int messageType = recvbuf.iMessageType;//确定消息类型
				switch(messageType){
				case LOGIN:
					{
						//  将这个用户的信息记录到用户列表中
						stUserListNode *currentuser = new stUserListNode();
						strcpy_s(currentuser->userName,10, recvbuf.message.loginmember.userName);
						currentuser->ip = ntohl(sender.sin_addr.S_un.S_addr);
						currentuser->port = ntohs(sender.sin_port);

						BOOL bFound = FALSE;
						for(UserList::iterator UserIterator=ClientList.begin();
							UserIterator!=ClientList.end();
							++UserIterator)
						{
							if( strcmp( ((*UserIterator)->userName), recvbuf.message.loginmember.userName) == 0 )
							{
								bFound = TRUE;
								break;
							}
						}
						
						if ( !bFound )
						{
							printf("has a user login : %s <-> %s:%ld\n", recvbuf.message.loginmember.userName, inet_ntoa( sender.sin_addr ), ntohs(sender.sin_port) );
							ClientList.push_back(currentuser);
						}
						
						// 发送已经登陆的客户信息
						int nodecount = (int)ClientList.size();
						sendto(PrimaryUDP, (const char*)&nodecount, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));
						UserList::iterator UserIterator = ClientList.begin();
						for( UserIterator=ClientList.begin();
								UserIterator!=ClientList.end();
								++UserIterator)
						{
							sendto(PrimaryUDP, (const char*)(*UserIterator), sizeof(stUserListNode), 0, (const sockaddr*)&sender, sizeof(sender)); 
						}
								
						printf("send user list information to: %s <-> %s:%ld\n", recvbuf.message.loginmember.userName, inet_ntoa( sender.sin_addr ), ntohs(sender.sin_port) );

						break;
					}
				case LOGOUT:
					{
						// 将此客户信息删除
						printf("has a user logout : %s <-> %s:%ld\n", recvbuf.message.logoutmember.userName, inet_ntoa( sender.sin_addr ), ntohs(sender.sin_port) );
						UserList::iterator removeiterator = ClientList.begin();
						for(UserList::iterator UserIterator=ClientList.begin();
							UserIterator!=ClientList.end();
							++UserIterator)
						{
							if( strcmp( ((*UserIterator)->userName), recvbuf.message.logoutmember.userName) == 0 )
							{
								removeiterator = UserIterator;
								break;
							}
						}
						if(removeiterator != ClientList.end())
							ClientList.remove(*removeiterator);
						break;
					}
				case P2PTRANS:
					{
						// 某个客户希望服务端向另外一个客户发送一个打洞消息
						printf("%s:%ld wants to p2p %s\n",inet_ntoa(sender.sin_addr), ntohs(sender.sin_port), recvbuf.message.translatemessage.userName );
						stUserListNode node = GetUser(recvbuf.message.translatemessage.userName);
						sockaddr_in remote;
						remote.sin_family=AF_INET;
						remote.sin_port= htons(node.port); 
						remote.sin_addr.s_addr = htonl(node.ip);

						in_addr tmp;
						tmp.S_un.S_addr = htonl(node.ip);

						stP2PMessage transMessage;
						transMessage.iMessageType = P2PSOMEONEWANTTOCALLYOU;
						transMessage.iStringLen = ntohl(sender.sin_addr.S_un.S_addr);
						transMessage.Port = ntohs(sender.sin_port);
                        
						sendto(PrimaryUDP,(const char*)&transMessage, sizeof(transMessage), 0, (const sockaddr *)&remote, sizeof(remote));
						printf( "tell %s <-> %s:%d to send p2ptrans message to: %s:%ld\n", 
							recvbuf.message.translatemessage.userName, inet_ntoa(remote.sin_addr), node.port, inet_ntoa(sender.sin_addr), ntohs(sender.sin_port) );

						break;
					}
				
				case GETALLUSER:
					{
						int command = GETALLUSER;
						sendto(PrimaryUDP, (const char*)&command, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));

						int nodecount = (int)ClientList.size();
						sendto(PrimaryUDP, (const char*)&nodecount, sizeof(int), 0, (const sockaddr*)&sender, sizeof(sender));

						for(UserList::iterator UserIterator=ClientList.begin();
								UserIterator!=ClientList.end();
								++UserIterator)
						{
							sendto(PrimaryUDP, (const char*)(*UserIterator), sizeof(stUserListNode), 0, (const sockaddr*)&sender, sizeof(sender)); 
						}

						printf("send user list information to: %s <-> %s:%ld\n", recvbuf.message.loginmember.userName, inet_ntoa( sender.sin_addr ), ntohs(sender.sin_port) );

						break;
					}
				}
			}
		}

	}
	catch(exception &e)
	{
		printf("something is error:%s", e.what());
		return 1;
	}

	return 0;
}

