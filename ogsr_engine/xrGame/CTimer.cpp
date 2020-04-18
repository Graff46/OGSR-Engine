#include "stdafx.h"
#include "CTimer.h"
#include <thread>

void CWTimer(int msec, const luabind::functor<void>& callback)
{
	std::thread([=]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(std::chrono::milliseconds(msec)));
		callback();
		}).detach();	
};
