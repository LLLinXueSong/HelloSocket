#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_
#include "CellClient.hpp"
class INetEvent
{
public:
	//�ͻ��˼���
	virtual void OnNetJoin(CellClient* pClient) = 0;
	//�пͻ����뿪ʱ֪ͨ
	virtual void OnLeave(CellClient* pClient) = 0;
	//�ͻ�����Ϣ�¼�
	virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, netmsg_DataHeader* header) = 0;
	//recv�¼�
	virtual void OnNetRecv(CellClient* pClient) = 0;

};
#endif // !_I_NET_EVENT_HPP_
