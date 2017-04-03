/*  ��Դ��ο��϶��������ϣ�����ѧϰUDP�򶴲������κ����ñ�����Υ����Υ�����������������Ͽ�
*
*	���ƽ��� - ����õĽ���
*
*  ��ҳ http://www.doedu.com.cn/
*
*  C/C++ ����Ⱥ : 243215922
*
*  ������ʦ : 691714544
*
*  ѧϰ����Q : 921700006
*
*/

#pragma comment(lib, "ws2_32.lib")

#include "windows.h"
#include "..\common\msgproto.h"
//#include "..\Exception.h"

USHORT g_nServerPort = SERVER_PORT;//�������˿�

UserList ClientList;//ʵ�����û���Ϣ���� 
//��ʼ��
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
//�����׽���
SOCKET mksock(int type)//1��TCP��2��UDP��3��ԭʼ�׽���
{
	SOCKET sock = socket(AF_INET, type, 0);//Ŀǰ��֧��AF_INET�������ַ��ʽ 0�������׽���Э��
	if (sock < 0)
	{
        printf("create socket error");
	}
	return sock;
}

stUserListNode GetUser(char *username)//�����û�����ȡ�û���Ϣ
{
	UserList::iterator UserIterator = ClientList.begin();
	for(;	UserIterator!=ClientList.end();++UserIterator)
	{
		if( strcmp( ((*UserIterator)->userName), username) == 0 )
			return *(*UserIterator);
	}
	return *(*UserIterator);//�õ��û��� IP �˿�
}

int main(int argc, char* argv[])
{
	if ( argc > 1 )
	{
		g_nServerPort = atoi( argv[ 1 ] );
	}

	try{
		InitWinSock();//��ʼ���׽���
		
		SOCKET PrimaryUDP;
		PrimaryUDP = mksock(SOCK_DGRAM);//�����׽���

		sockaddr_in local;
		local.sin_family=AF_INET;//ʹ��TCP/IPЭ���
		local.sin_port= htons(g_nServerPort); //�׽��ֶ˿ں�
		local.sin_addr.s_addr = htonl(INADDR_ANY);//���������ַ
		int nResult=bind(PrimaryUDP,(sockaddr*)&local,sizeof(sockaddr));//���а󶨼���
		if(nResult==SOCKET_ERROR)
			printf("bind error");

		sockaddr_in sender;
		stMessage recvbuf;
		memset(&recvbuf,0,sizeof(stMessage));

		// ��ʼ��ѭ��.
		// ��ѭ���������漸������:
		// һ:��ȡ�ͻ��˵�½�͵ǳ���Ϣ,��¼�ͻ��б�
		// ��:ת���ͻ�p2p����
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
				int messageType = recvbuf.iMessageType;//ȷ����Ϣ����
				switch(messageType){
				case LOGIN:
					{
						//  ������û�����Ϣ��¼���û��б���
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
						
						// �����Ѿ���½�Ŀͻ���Ϣ
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
						// ���˿ͻ���Ϣɾ��
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
						// ĳ���ͻ�ϣ�������������һ���ͻ�����һ������Ϣ
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

