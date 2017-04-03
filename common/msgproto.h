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

#pragma once
#include <list>

// 定义iMessageType的值
#define LOGIN 1
#define LOGOUT 2
#define P2PTRANS 3
#define GETALLUSER  4

// 服务器端口
#define SERVER_PORT 6060

// Client登录时向服务器发送的消息
struct stLoginMessage
{
	char userName[10];//用户名
	char password[10];//密码
};

// Client注销时发送的消息
struct stLogoutMessage
{
	char userName[10];//用户名
};

// Client向服务器请求另外一个Client(userName)向自己方向发送UDP打洞消息
struct stP2PTranslate
{
	char userName[10];//用户名
};

// Client向服务器发送的消息格式
struct stMessage
{
	int iMessageType;//发送信息类型
	union _message
	{
		stLoginMessage loginmember;//登录信息
		stLogoutMessage logoutmember;//退出信息
		stP2PTranslate translatemessage;//转换信息
	}message;
};

// 客户节点信息
struct stUserListNode
{
	char userName[10];//用户名
	unsigned int ip;//用户IP
	unsigned short port;//用户端口
};

// Server向Client发送的消息
struct stServerToClient
{
	int iMessageType;
	union _message
	{
		stUserListNode user;
	}message;

};

//======================================
// 下面的协议用于客户端之间的通信
//======================================
#define P2PMESSAGE 100               // 发送消息
#define P2PMESSAGEACK 101            // 收到消息的应答
#define P2PSOMEONEWANTTOCALLYOU 102  // 服务器向客户端发送的消息
                                     // 希望此客户端发送一个UDP打洞包
#define P2PTRASH        103          // 客户端发送的打洞包，接收端应该忽略此消息

// 客户端之间发送消息格式
struct stP2PMessage
{
	int iMessageType;
	int iStringLen;         // or IP address
	unsigned short Port; 
};

using namespace std;
typedef list<stUserListNode *> UserList;

