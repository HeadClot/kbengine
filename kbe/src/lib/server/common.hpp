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

#ifndef KBE_SERVER_COMMON_HPP
#define KBE_SERVER_COMMON_HPP

// common include
#include "cstdkbe/timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "server/server_errors.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine { 

// 消息转发到某个组件
#define MERCURY_MESSAGE_FORWARD(SEND_INTERFACE, SENDBUNDLE, FORWARDBUNDLE, MYCOMPONENT_ID, FORWARD_COMPONENT_ID)						\
	SENDBUNDLE.newMessage(SEND_INTERFACE::forwardMessage);																				\
	SENDBUNDLE << MYCOMPONENT_ID << FORWARD_COMPONENT_ID;																				\
	FORWARDBUNDLE.finish(true);																											\
	SENDBUNDLE.append(FORWARDBUNDLE);																									\

// cellapp转发消息给客户端
#define MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(ENTITYID, SENDBUNDLE, FORWARDBUNDLE)														\
	SENDBUNDLE.newMessage(BaseappInterface::forwardMessageToClientFromCellapp);															\
	SENDBUNDLE << ENTITYID;																												\
	FORWARDBUNDLE.finish(true);																											\
	SENDBUNDLE.append(FORWARDBUNDLE);																									\

// cellapp转发消息给cellapp
#define MERCURY_ENTITY_MESSAGE_FORWARD_CELLAPP(ENTITYID, SENDBUNDLE, FORWARDBUNDLE)														\
	SENDBUNDLE.newMessage(BaseappInterface::forwardMessageToCellappFromCellapp);															\
	SENDBUNDLE << ENTITYID;																												\
	FORWARDBUNDLE.finish(true);																											\
	SENDBUNDLE.append(FORWARDBUNDLE);	

// cellapp转发消息给客户端开始
#define MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_START(ENTITYID, SENDBUNDLE)																\
	SENDBUNDLE.newMessage(BaseappInterface::forwardMessageToClientFromCellapp);															\
	SENDBUNDLE << ENTITYID;																												\

// cellapp转发消息给客户端消息包追加消息
#define MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND(SENDBUNDLE, FORWARDBUNDLE)															\
	FORWARDBUNDLE.finish(true);																											\
	SENDBUNDLE.append(FORWARDBUNDLE);																									\

// 公共消息
#define COMMON_MERCURY_MESSAGE(COMPONENTTYPE, BUNDLE, MESSAGENAME)											\
		switch(COMPONENTTYPE)																				\
		{																									\
		case CELLAPPMGR_TYPE:																				\
			{																								\
				BUNDLE.newMessage(CellappmgrInterface::MESSAGENAME);										\
			}																								\
			break;																							\
		case BASEAPPMGR_TYPE:																				\
			{																								\
				BUNDLE.newMessage(BaseappmgrInterface::MESSAGENAME);										\
			}																								\
			break;																							\
		case DBMGR_TYPE:																					\
			{																								\
				BUNDLE.newMessage(DbmgrInterface::MESSAGENAME);												\
			}																								\
			break;																							\
		case CELLAPP_TYPE:																					\
			{																								\
				BUNDLE.newMessage(CellappInterface::MESSAGENAME);											\
			}																								\
			break;																							\
		case BASEAPP_TYPE:																					\
			{																								\
				BUNDLE.newMessage(BaseappInterface::MESSAGENAME);											\
			}																								\
			break;																							\
		case LOGINAPP_TYPE:																					\
			{																								\
				BUNDLE.newMessage(LoginappInterface::MESSAGENAME);											\
			}																								\
			break;																							\
		case MESSAGELOG_TYPE:																				\
			{																								\
				BUNDLE.newMessage(MessagelogInterface::MESSAGENAME);										\
			}																								\
			break;																							\
		case RESOURCEMGR_TYPE:																				\
			{																								\
				BUNDLE.newMessage(ResourcemgrInterface::MESSAGENAME);										\
			}																								\
			break;																							\
		case BILLING_TYPE:																					\
			{																								\
				BUNDLE.newMessage(BillingSystemInterface::MESSAGENAME);										\
			}																								\
			break;																							\
		default:																							\
			ERROR_MSG(boost::format("not support componentType=%1%(%2%)\n")									\
					% COMPONENTTYPE % COMPONENT_NAME_EX(COMPONENTTYPE));									\
			KBE_ASSERT(false && "not support componentType!\n");											\
			break;																							\
		};																									\


#define ENTITTAPP_COMMON_MERCURY_MESSAGE(COMPONENTTYPE, BUNDLE, MESSAGENAME)								\
		switch(COMPONENTTYPE)																				\
		{																									\
		case CELLAPP_TYPE:																					\
			{																								\
				BUNDLE.newMessage(CellappInterface::MESSAGENAME);											\
			}																								\
			break;																							\
		case BASEAPP_TYPE:																					\
			{																								\
				BUNDLE.newMessage(BaseappInterface::MESSAGENAME);											\
			}																								\
			break;																							\
		default:																							\
			ERROR_MSG(boost::format("not support componentType=%1%(%2%)")									\
					% COMPONENTTYPE % COMPONENT_NAME_EX(COMPONENTTYPE));									\
			KBE_ASSERT(false && "not support componentType!\n");											\
			break;																							\
		};																									\

/**
将秒转换为tick
@lowerBound: 最少不低于Ntick
*/
int32 secondsToTicks(float seconds, int lowerBound);

/**
	将秒为单位的时间转换为每秒所耗的stamps
*/
inline uint64 secondsToStamps(float seconds)
{
	return (uint64)(seconds * stampsPerSecondD());
}

/*
 账号和密码最大长度
*/
#define ACCOUNT_NAME_MAX_LENGTH 64
#define ACCOUNT_PASSWD_MAX_LENGTH 64

// 登录注册时附带的信息最大长度
#define ACCOUNT_DATA_MAX_LENGTH 1024

// 被用来描述任何只做一次后自动设置为不做的选项
#define KBE_NEXT_ONLY 2

/** c/c++数据类别转换成KBEDataTypeID */
uint16 datatype2id(std::string datatype);

}

#endif // KBE_SERVER_COMMON_HPP
