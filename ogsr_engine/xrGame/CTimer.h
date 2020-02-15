#pragma once

#include <chrono>
#include <functional>

void CWTimer(int msec, const luabind::functor<void > &callback);

