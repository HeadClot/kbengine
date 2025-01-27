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

	
/*
	CallbackMgr( 回调管理器 )
		由于一些回调操作都是异步的， 我们通过一个管理器将这些回调管理起来， 并对外返回一个
		标识该回调的唯一id， 外部可以通过该id来触发这个回调。
		
	用法:
	typedef CallbackMgr<std::tr1::function<void(Base*, int64, bool)>> CALLBACK_MGR;
	CALLBACK_MGR callbackMgr;
	void xxx(Base*, int64, bool){}
	CALLBACK_ID callbackID = callbackMgr.save(&xxx); // 可以使用bind来绑定一个类成员函数
*/

#ifndef KBE_CALLBACKMGR_HPP
#define KBE_CALLBACKMGR_HPP

// common include	
#include "Python.h"
#include "idallocate.hpp"
#include "serverconfig.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "cstdkbe/timer.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "pyscript/pickler.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

template<typename T>
class CallbackMgr
{
public:
	typedef std::map< CALLBACK_ID, std::pair< T, uint64 > > CALLBACKS;

	CallbackMgr()
	{
	}

	~CallbackMgr()
	{
		finalise();
	}	
	
	void finalise()
	{
		cbMap_.clear();
	}


	void addToStream(KBEngine::MemoryStream& s);

	void createFromStream(KBEngine::MemoryStream& s);

	/** 
		向管理器添加一个回调 
	*/
	CALLBACK_ID save(T callback, uint64 timeout = 0/*secs*/)
	{
		if(timeout == 0)
			timeout = uint64(ServerConfig::getSingleton().callback_timeout_);

		CALLBACK_ID cbID = idAlloc_.alloc();
		cbMap_.insert(typename CALLBACKS::value_type(cbID, 
			std::pair< T, uint64 >(callback, timestamp() + (timeout * stampsPerSecond()))));

		tick();
		return cbID;
	}
	
	/** 
		通过callbackID取走回调 
	*/
	T take(CALLBACK_ID cbID)
	{
		typename CALLBACKS::iterator itr = cbMap_.find(cbID);
		if(itr != cbMap_.end()){
			T t = itr->second.first;
			idAlloc_.reclaim(itr->first);
			cbMap_.erase(itr);
			return t;
		}
		
		tick();
		return NULL;
	}

	/**
		tick
	*/
	void tick()
	{
		if(timestamp() - lastTimestamp_ < (ServerConfig::getSingleton().callback_timeout_ * stampsPerSecond()))
			return;

		lastTimestamp_ = timestamp(); 
		typename CALLBACKS::iterator iter = cbMap_.begin();
		for(; iter!= cbMap_.end(); )
		{
			if(lastTimestamp_ > iter->second.second)
			{
				if(processTimeout(iter->first, iter->second.first))
				{
					idAlloc_.reclaim(iter->first);
					cbMap_.erase(iter++);
					continue;
				}
			}

			++iter;
		}
	}

	/**
		超时的callback
	*/
	bool processTimeout(CALLBACK_ID cbID, T callback)
	{
		INFO_MSG(boost::format("CallbackMgr::processTimeout: %1% timeout!\n") % cbID);
		return true;
	}
protected:
	CALLBACKS cbMap_;	// 所有的回调都存储在这个map中
	IDAllocate<CALLBACK_ID> idAlloc_;									// 回调的id分配器
	uint64 lastTimestamp_;
};

template<>
inline void CallbackMgr<PyObjectPtr>::addToStream(KBEngine::MemoryStream& s)
{
	uint32 size = cbMap_.size();

	s << idAlloc_.lastID() << size;

	CALLBACKS::iterator iter = cbMap_.begin();
	for(; iter != cbMap_.end(); iter++)
	{
		s << iter->first;
		s.appendBlob(script::Pickler::pickle(iter->second.first.get()));
		s << iter->second.second;
	}
}

template<>
inline void CallbackMgr<PyObjectPtr>::createFromStream(KBEngine::MemoryStream& s)
{
	CALLBACK_ID v;
	s >> v;

	idAlloc_.lastID(v);

	uint32 size;
	s >> size;

	for(uint32 i=0; i<size; i++)
	{
		CALLBACK_ID cbID;
		s >> cbID;

		std::string data;
		s.readBlob(data);

		PyObject* pyCallback = NULL;
		
		if(data.size() > 0)
			pyCallback = script::Pickler::unpickle(data);
		
		uint64 timeout;
		s >> timeout;

		if(pyCallback == NULL || cbID == 0)
		{
			ERROR_MSG(boost::format("CallbackMgr::createFromStream: pyCallback(%1%) is error!\n") % cbID);
			continue;
		}

		cbMap_.insert(CallbackMgr<PyObjectPtr>::CALLBACKS::value_type(cbID, 
			std::pair< PyObjectPtr, uint64 >(pyCallback, timeout)));

		Py_DECREF(pyCallback);
	}
}

template<>
inline void CallbackMgr<PyObject*>::finalise()
{
	std::map< CALLBACK_ID, std::pair< PyObject*, uint64 > >::iterator iter = cbMap_.begin();
	for(; iter!= cbMap_.end(); iter++)
	{
		Py_DECREF(iter->second.first);
	}

	cbMap_.clear();
}	

template<>
inline bool CallbackMgr<PyObject*>::processTimeout(CALLBACK_ID cbID, PyObject* callback)
{
	std::string name = callback->ob_type->tp_name;
	INFO_MSG(boost::format("CallbackMgr::processTimeout: callbackID:%1%, callback(%2%) timeout!\n") % cbID % 
		name);

	Py_DECREF(callback);
	return true;
}

typedef CallbackMgr<PyObjectPtr> PY_CALLBACKMGR;

}

#endif // KBE_CALLBACKMGR_HPP
