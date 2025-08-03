#include "TestService.h"

#include "asio/impl/src.hpp"
#include "AppDelegate.h"

int main(int argc, const char* argv[])
{
	AppDelegate app;
	return TestService::instance()->run(argc, argv);
}

