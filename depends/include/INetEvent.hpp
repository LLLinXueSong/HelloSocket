#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_
#include "CellClient.hpp"
class INetEvent
{
public:
	//客户端加入
	virtual void OnNetJoin(CellClient* pClient) = 0;
	//有客户端离开时通知
	virtual void OnLeave(CellClient* pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(CellClient* pClient) = 0;

};
#endif // !_I_NET_EVENT_HPP_
