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

#pragma once
#include <list>

// ����iMessageType��ֵ
#define LOGIN 1
#define LOGOUT 2
#define P2PTRANS 3
#define GETALLUSER  4

// �������˿�
#define SERVER_PORT 6060

// Client��¼ʱ����������͵���Ϣ
struct stLoginMessage
{
	char userName[10];//�û���
	char password[10];//����
};

// Clientע��ʱ���͵���Ϣ
struct stLogoutMessage
{
	char userName[10];//�û���
};

// Client���������������һ��Client(userName)���Լ�������UDP����Ϣ
struct stP2PTranslate
{
	char userName[10];//�û���
};

// Client����������͵���Ϣ��ʽ
struct stMessage
{
	int iMessageType;//������Ϣ����
	union _message
	{
		stLoginMessage loginmember;//��¼��Ϣ
		stLogoutMessage logoutmember;//�˳���Ϣ
		stP2PTranslate translatemessage;//ת����Ϣ
	}message;
};

// �ͻ��ڵ���Ϣ
struct stUserListNode
{
	char userName[10];//�û���
	unsigned int ip;//�û�IP
	unsigned short port;//�û��˿�
};

// Server��Client���͵���Ϣ
struct stServerToClient
{
	int iMessageType;
	union _message
	{
		stUserListNode user;
	}message;

};

//======================================
// �����Э�����ڿͻ���֮���ͨ��
//======================================
#define P2PMESSAGE 100               // ������Ϣ
#define P2PMESSAGEACK 101            // �յ���Ϣ��Ӧ��
#define P2PSOMEONEWANTTOCALLYOU 102  // ��������ͻ��˷��͵���Ϣ
                                     // ϣ���˿ͻ��˷���һ��UDP�򶴰�
#define P2PTRASH        103          // �ͻ��˷��͵Ĵ򶴰������ն�Ӧ�ú��Դ���Ϣ

// �ͻ���֮�䷢����Ϣ��ʽ
struct stP2PMessage
{
	int iMessageType;
	int iStringLen;         // or IP address
	unsigned short Port; 
};

using namespace std;
typedef list<stUserListNode *> UserList;

