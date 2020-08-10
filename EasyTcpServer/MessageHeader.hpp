#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct netmsg_DataHeader
{
	netmsg_DataHeader() {
		dataLength = sizeof(netmsg_DataHeader);
	}
	short dataLength;
	short cmd;
};
struct netmsg_Login :netmsg_DataHeader
{
	netmsg_Login() {
		dataLength = sizeof(netmsg_Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
	char data[32];
};
struct netmsg_LoginR :netmsg_DataHeader
{
	netmsg_LoginR() {
		dataLength = sizeof(netmsg_LoginR);
		cmd = CMD_LOGIN_RESULT;
		result = 1;
	}
	int result;
	char data[92];
};
struct netmsg_Logout :netmsg_DataHeader
{
	netmsg_Logout() {
		dataLength = sizeof(netmsg_Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
	char PassWord[32];
};
struct netmsg_LogoutR :netmsg_DataHeader
{
	netmsg_LogoutR() {
		dataLength = sizeof(netmsg_LogoutR);
		cmd = CMD_LOGOUT_RESULT;
		result = 1;
	}
	int result;
};
struct netmsg_NewUserJoin :netmsg_DataHeader
{
	netmsg_NewUserJoin() {
		dataLength = sizeof(netmsg_NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
#endif