/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "entitymailboxabstract.hpp"
#include "pyscript/pickler.hpp"
#include "helper/debug_helper.hpp"
#include "network/packet.hpp"
#include "network/bundle.hpp"
#include "network/network_interface.hpp"
#include "server/components.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"

namespace KBEngine{


SCRIPT_METHOD_DECLARE_BEGIN(EntityMailboxAbstract)
SCRIPT_METHOD_DECLARE("__reduce_ex__",				reduce_ex__,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(EntityMailboxAbstract)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityMailboxAbstract)
SCRIPT_GET_DECLARE("id",							pyGetID,				0,					0)	
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityMailboxAbstract, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
EntityMailboxAbstract::EntityMailboxAbstract(PyTypeObject* scriptType, 
											const Mercury::Address* pAddr, 
											COMPONENT_ID componentID, 
											ENTITY_ID eid, 
											uint16 utype, 
											ENTITY_MAILBOX_TYPE type):
ScriptObject(scriptType, false),
componentID_(componentID),
addr_((pAddr == NULL) ? Mercury::Address::NONE : *pAddr),
type_(type),
id_(eid),
utype_(utype)
{
}

//-------------------------------------------------------------------------------------
EntityMailboxAbstract::~EntityMailboxAbstract()
{
}

//-------------------------------------------------------------------------------------
void EntityMailboxAbstract::newMail(Mercury::Bundle& bundle)
{
	// 如果是server端的mailbox
	if(g_componentType != CLIENT_TYPE && g_componentType != BOTS_TYPE)
	{
		if(componentID_ == 0)	// 客户端
		{
			bundle.newMessage(ClientInterface::onRemoteMethodCall);
		}
		else					// 服务器组件
		{
			Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID_);

			if(cinfos != NULL)
			{
				// 找到对应的组件投递过去， 如果这个mailbox还需要中转比如 e.base.cell ， 则由baseapp转往cellapp
				if(cinfos->componentType == BASEAPP_TYPE)
				{
					bundle.newMessage(BaseappInterface::onEntityMail);
				}
				else
				{
					bundle.newMessage(CellappInterface::onEntityMail);
				}
			}
			else
			{
				ERROR_MSG(boost::format("EntityMailboxAbstract::newMail: not found component(%1%)!\n") 
					% componentID_);
			}
		}

		bundle << id_;
		
		// 如果是发往客户端的包则无需附加这样一个类型
		if(componentID_ > 0)
			bundle << type_;
	}
	else
	{
		// 如果是客户端上的mailbox调用服务端方法只存在调用cell或者base
		switch(type_)
		{
		case MAILBOX_TYPE_BASE:
			bundle.newMessage(BaseappInterface::onRemoteMethodCall);
			break;
		case MAILBOX_TYPE_CELL:
			bundle.newMessage(BaseappInterface::onRemoteCallCellMethodFromClient);
			break;
		default:
			KBE_ASSERT(false && "no support!\n");
			break;
		};

		bundle << id_;
	}
}

//-------------------------------------------------------------------------------------
bool EntityMailboxAbstract::postMail(Mercury::Bundle& bundle)
{
	KBE_ASSERT(Components::getSingleton().pNetworkInterface() != NULL);
	Mercury::Channel* pChannel = getChannel();

	if(pChannel && !pChannel->isDead())
	{
		bundle.send(*Components::getSingleton().pNetworkInterface(), pChannel);
		return true;
	}
	else
	{
		ERROR_MSG(boost::format("EntityMailboxAbstract::postMail: invalid channel(%1%)!\n") %
			addr_.c_str());
	}

	return false;
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailboxAbstract::__py_reduce_ex__(PyObject* self, PyObject* protocol)
{
	EntityMailboxAbstract* emailbox = static_cast<EntityMailboxAbstract*>(self);
	
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Mailbox");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	
	PyObject* args1 = PyTuple_New(4);
	PyTuple_SET_ITEM(args1, 0, PyLong_FromLong(emailbox->id()));
	PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLongLong(emailbox->componentID()));
	PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(emailbox->utype()));

	int16 mbType = static_cast<int16>(emailbox->type());
	
	PyTuple_SET_ITEM(args1, 3, PyLong_FromLong(mbType));
	PyTuple_SET_ITEM(args, 1, args1);

	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailboxAbstract::pyGetID()
{ 
	return PyLong_FromLong(id()); 
}

//-------------------------------------------------------------------------------------

}
