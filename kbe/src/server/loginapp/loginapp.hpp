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


#ifndef KBE_LOGINAPP_HPP
#define KBE_LOGINAPP_HPP
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/pendingLoginmgr.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
	
namespace KBEngine{

class HTTPCBHandler;

class Loginapp :	public ServerApp, 
					public Singleton<Loginapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1,
		TIMEOUT_CHECK_STATUS,
		TIMEOUT_MAX
	};

	Loginapp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Loginapp();
	
	bool run();
	
	virtual void onChannelDeregister(Mercury::Channel * pChannel);

	virtual void handleTimeout(TimerHandle handle, void * arg);
	void handleCheckStatusTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	
	virtual void onShutdownBegin();

	virtual void onHello(Mercury::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	/** 网络接口
		某个client向本app告知处于活动状态。
	*/
	void onClientActiveTick(Mercury::Channel* pChannel);

	/** 网络接口
		创建账号
	*/
	bool _createAccount(Mercury::Channel* pChannel, std::string& accountName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE type = ACCOUNT_TYPE_NORMAL);
	void reqCreateAccount(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		创建email账号
	*/
	void reqCreateMailAccount(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		创建账号
	*/
	void onReqCreateAccountResult(Mercury::Channel* pChannel, MemoryStream& s);
	void onReqCreateMailAccountResult(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		重置账号密码申请（忘记密码?）
	*/
	void reqAccountResetPassword(Mercury::Channel* pChannel, std::string& accountName);
	void onReqAccountResetPasswordCB(Mercury::Channel* pChannel, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** 网络接口
		dbmgr账号激活返回
	*/
	void onAccountActivated(Mercury::Channel* pChannel, std::string& code, bool success);

	/** 网络接口
		dbmgr账号绑定email返回
	*/
	void onAccountBindedEmail(Mercury::Channel* pChannel, std::string& code, bool success);

	/** 网络接口
		dbmgr账号重设密码返回
	*/
	void onAccountResetPassword(Mercury::Channel* pChannel, std::string& code, bool success);

	/** 网络接口
		用户登录服务器
		clientType[COMPONENT_CLIENT_TYPE]: 前端类别(手机， web， pcexe端)
		clientData[str]: 前端附带数据(可以是任意的， 比如附带手机型号， 浏览器类型等)
		accountName[str]: 帐号名
		password[str]: 密码
	*/
	void login(Mercury::Channel* pChannel, MemoryStream& s);

	/*
		登录失败
		failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_SRV_OVERLOAD:服务器负载过重, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	void _loginFailed(Mercury::Channel* pChannel, std::string& loginName, SERVER_ERROR_CODE failedcode, std::string& datas, bool force = false);
	
	/** 网络接口
		dbmgr返回的登录账号检测结果
	*/
	void onLoginAccountQueryResultFromDbmgr(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		baseappmgr返回的登录网关地址
	*/
	void onLoginAccountQueryBaseappAddrFromBaseappmgr(Mercury::Channel* pChannel, std::string& loginName, 
		std::string& accountName, uint32 addr, uint16 port);


	/** 网络接口
		dbmgr发送初始信息
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest);

	/** 网络接口
		客户端协议导出
	*/
	void importClientMessages(Mercury::Channel* pChannel);

	/** 网络接口
		错误码描述导出
	*/
	void importServerErrorsDescr(Mercury::Channel* pChannel);

	// 引擎版本不匹配
	virtual void onVersionNotMatch(Mercury::Channel* pChannel);

	// 引擎脚本层版本不匹配
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel);

	/** 网络接口
		baseapp同步自己的初始化信息
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onBaseappInitProgress(Mercury::Channel* pChannel, float progress);
protected:
	TimerHandle							loopCheckTimerHandle_;

	// 记录注册账号还未登陆的请求
	PendingLoginMgr						pendingCreateMgr_;

	// 记录登录到服务器但还未处理完毕的账号
	PendingLoginMgr						pendingLoginMgr_;

	std::string							digest_;

	HTTPCBHandler*						pHttpCBHandler;

	float								initProgress_;
};

}

#endif // KBE_LOGINAPP_HPP
