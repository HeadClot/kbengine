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

#ifndef KBE_DBTASKS_HPP
#define KBE_DBTASKS_HPP

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "cstdkbe/timestamp.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entitydef.hpp"
#include "network/address.hpp"
#include "dbmgr_lib/db_tasks.hpp"

namespace KBEngine{ 

class DBInterface;
class Buffered_DBTasks;
struct ACCOUNT_INFOS;

/*
	数据库线程任务基础类
*/

class DBTask : public DBTaskBase
{
public:
	DBTask(const Mercury::Address& addr, MemoryStream& datas);

	DBTask(const Mercury::Address& addr):
	DBTaskBase(),
	pDatas_(0),
	addr_(addr)
	{
	}

	virtual ~DBTask();

	bool send(Mercury::Bundle& bundle);
protected:
	MemoryStream* pDatas_;
	Mercury::Address addr_;
};

/*
	EntityDBTask
*/
class EntityDBTask : public DBTask
{
public:
	EntityDBTask(const Mercury::Address& addr, MemoryStream& datas, ENTITY_ID entityID, DBID entityDBID):
	DBTask(addr, datas),
	_entityID(entityID),
	_entityDBID(entityDBID),
	_pBuffered_DBTasks(NULL)
	{
	}
	
	EntityDBTask(const Mercury::Address& addr, ENTITY_ID entityID, DBID entityDBID):
	DBTask(addr),
	_entityID(entityID),
	_entityDBID(entityDBID),
	_pBuffered_DBTasks(NULL)
	{
	}
	
	virtual ~EntityDBTask(){}
	
	ENTITY_ID EntityDBTask_entityID()const { return _entityID; }
	DBID EntityDBTask_entityDBID()const { return _entityDBID; }
	
	void pBuffered_DBTasks(Buffered_DBTasks* v){ _pBuffered_DBTasks = v; }
	virtual thread::TPTask::TPTaskState presentMainThread();

	DBTask* tryGetNextTask();
private:
	ENTITY_ID _entityID;
	DBID _entityDBID;
	Buffered_DBTasks* _pBuffered_DBTasks;
};

/**
	执行一条sql语句
*/
class DBTaskExecuteRawDatabaseCommand : public DBTask
{
public:
	DBTaskExecuteRawDatabaseCommand(const Mercury::Address& addr, MemoryStream& datas);
	virtual ~DBTaskExecuteRawDatabaseCommand();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	COMPONENT_ID componentID_;
	COMPONENT_TYPE componentType_;
	std::string sdatas_;
	CALLBACK_ID callbackID_;
	std::string error_;
	MemoryStream execret_;
};


/**
	执行一条sql语句
*/
class DBTaskExecuteRawDatabaseCommandByEntity : public EntityDBTask
{
public:
	DBTaskExecuteRawDatabaseCommandByEntity(const Mercury::Address& addr, MemoryStream& datas, ENTITY_ID entityID);
	virtual ~DBTaskExecuteRawDatabaseCommandByEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	COMPONENT_ID componentID_;
	COMPONENT_TYPE componentType_;
	std::string sdatas_;
	CALLBACK_ID callbackID_;
	std::string error_;
	MemoryStream execret_;
};

/**
	向数据库写entity， 备份entity时也是这个机制
*/
class DBTaskWriteEntity : public EntityDBTask
{
public:
	DBTaskWriteEntity(const Mercury::Address& addr, COMPONENT_ID componentID, 
		ENTITY_ID eid, DBID entityDBID, MemoryStream& datas);

	virtual ~DBTaskWriteEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	COMPONENT_ID componentID_;
	ENTITY_ID eid_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
	CALLBACK_ID callbackID_;
	bool success_;
};

/**
	从数据库中删除entity
*/
class DBTaskRemoveEntity : public EntityDBTask
{
public:
	DBTaskRemoveEntity(const Mercury::Address& addr, COMPONENT_ID componentID, 
		ENTITY_ID eid, DBID entityDBID, MemoryStream& datas);

	virtual ~DBTaskRemoveEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	COMPONENT_ID componentID_;
	ENTITY_ID eid_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
};

/**
	从数据库中删除entity
*/
class DBTaskDeleteBaseByDBID : public DBTask
{
public:
	DBTaskDeleteBaseByDBID(const Mercury::Address& addr, COMPONENT_ID componentID, 
		DBID entityDBID, CALLBACK_ID callbackID, ENTITY_SCRIPT_UID sid);

	virtual ~DBTaskDeleteBaseByDBID();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	COMPONENT_ID componentID_;
	CALLBACK_ID callbackID_;
	DBID entityDBID_;
	ENTITY_SCRIPT_UID sid_;
	bool success_;
	ENTITY_ID entityID_;
	COMPONENT_ID entityInAppID_;
};

/**
	创建一个账号到数据库
*/
class DBTaskCreateAccount : public DBTask
{
public:
	DBTaskCreateAccount(const Mercury::Address& addr, std::string& registerName, std::string& accountName, 
		std::string& password, std::string& postdatas, std::string& getdatas);
	virtual ~DBTaskCreateAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();

	static bool writeAccount(DBInterface* pdbi, const std::string& accountName, 
		const std::string& passwd, const std::string& datas, ACCOUNT_INFOS& info);
protected:
	std::string registerName_; 
	std::string accountName_;
	std::string password_;
	std::string postdatas_, getdatas_;
	bool success_;
	
};

/**
	创建一个email账号
*/
class DBTaskCreateMailAccount : public DBTask
{
public:
	DBTaskCreateMailAccount(const Mercury::Address& addr, std::string& registerName, std::string& accountName, 
		std::string& password, std::string& postdatas, std::string& getdatas);
	virtual ~DBTaskCreateMailAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string registerName_; 
	std::string accountName_;
	std::string password_;
	std::string postdatas_, getdatas_;
	bool success_;
	
};

/**
	创建一个email账号
*/
class DBTaskActivateAccount : public DBTask
{
public:
	DBTaskActivateAccount(const Mercury::Address& addr, std::string& code);
	virtual ~DBTaskActivateAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string code_; 
	bool success_;
	
};

/**
	请求重置账号
*/
class DBTaskReqAccountResetPassword : public DBTask
{
public:
	DBTaskReqAccountResetPassword(const Mercury::Address& addr, std::string& accountName);
	virtual ~DBTaskReqAccountResetPassword();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string code_; 
	std::string email_;
	std::string accountName_;
	bool success_;
	
};

/**
	完成重置账号
*/
class DBTaskAccountResetPassword : public DBTask
{
public:
	DBTaskAccountResetPassword(const Mercury::Address& addr, std::string& accountName, 
		std::string& newpassword, std::string& code);
	virtual ~DBTaskAccountResetPassword();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string code_; 
	std::string accountName_;
	std::string newpassword_;
	bool success_;
	
};

/**
	请求绑定email
*/
class DBTaskReqAccountBindEmail : public DBTask
{
public:
	DBTaskReqAccountBindEmail(const Mercury::Address& addr, ENTITY_ID entityID, std::string& accountName, 
		std::string password,std::string& email);
	virtual ~DBTaskReqAccountBindEmail();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string code_; 
	std::string password_; 
	std::string accountName_;
	std::string email_; 
	bool success_;
	ENTITY_ID entityID_;
	
};

/**
	完成绑定email
*/
class DBTaskAccountBindEmail : public DBTask
{
public:
	DBTaskAccountBindEmail(const Mercury::Address& addr, std::string& accountName, 
		std::string& code);
	virtual ~DBTaskAccountBindEmail();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string code_; 
	std::string accountName_;
	bool success_;
	
};

/**
	设置新密码
*/
class DBTaskAccountNewPassword : public DBTask
{
public:
	DBTaskAccountNewPassword(const Mercury::Address& addr, ENTITY_ID entityID, std::string& accountName, 
		std::string& oldpassword_, std::string& newpassword);
	virtual ~DBTaskAccountNewPassword();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string accountName_;
	std::string oldpassword_, newpassword_;
	bool success_;
	ENTITY_ID entityID_;
};

/**
	baseapp请求查询account信息
*/
class DBTaskQueryAccount : public EntityDBTask
{
public:
	DBTaskQueryAccount(const Mercury::Address& addr, std::string& accountName, std::string& password, 
		COMPONENT_ID componentID, ENTITY_ID entityID, DBID entityDBID, uint32 ip, uint16 port);
	virtual ~DBTaskQueryAccount();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string accountName_;
	std::string password_;
	bool success_;
	MemoryStream s_;
	DBID dbid_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
	std::string error_;
	uint32 ip_;
	uint16 port_;

	uint32 flags_;
	uint64 deadline_;
};

/**
	账号上线
*/
class DBTaskAccountOnline : public EntityDBTask
{
public:
	DBTaskAccountOnline(const Mercury::Address& addr, std::string& accountName,
		COMPONENT_ID componentID, ENTITY_ID entityID);
	virtual ~DBTaskAccountOnline();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string accountName_;
	COMPONENT_ID componentID_;
};


/**
	entity下线
*/
class DBTaskEntityOffline : public EntityDBTask
{
public:
	DBTaskEntityOffline(const Mercury::Address& addr, DBID dbid, ENTITY_SCRIPT_UID sid);
	virtual ~DBTaskEntityOffline();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	ENTITY_SCRIPT_UID sid_;
};


/**
	一个新用户登录， 需要检查合法性
*/
class DBTaskAccountLogin : public DBTask
{
public:
	DBTaskAccountLogin(const Mercury::Address& addr, std::string& loginName, 
		std::string& accountName, std::string& password, bool success, std::string& postdatas, std::string& getdatas);

	virtual ~DBTaskAccountLogin();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	std::string loginName_;
	std::string accountName_;
	std::string password_;
	std::string postdatas_, getdatas_;
	bool success_;
	COMPONENT_ID componentID_;
	ENTITY_ID entityID_;
	DBID dbid_;

	uint32 flags_;
	uint64 deadline_;
};

/**
	baseapp请求查询entity信息
*/
class DBTaskQueryEntity : public EntityDBTask
{
public:
	DBTaskQueryEntity(const Mercury::Address& addr, int8 queryMode, std::string& entityType, DBID dbid, 
		COMPONENT_ID componentID, CALLBACK_ID callbackID, ENTITY_ID entityID);

	virtual ~DBTaskQueryEntity();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	int8 queryMode_;
	std::string entityType_;
	DBID dbid_;
	COMPONENT_ID componentID_;
	CALLBACK_ID callbackID_;
	bool success_;
	MemoryStream s_;
	ENTITY_ID entityID_;
	bool wasActive_;
};

}

#endif // KBE_DBTASKS_HPP
