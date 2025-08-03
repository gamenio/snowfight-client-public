#include "TestBase.h"

#include "TestService.h"

TestBase::TestBase() :
	m_isFinished(false),
	m_id(0),
	m_mainDispatcher(new MessageDispatcher())
{
}

TestBase::~TestBase()
{
	CC_SAFE_DELETE(m_mainDispatcher);
}

void TestBase::update(float dt)
{
	m_mainDispatcher->dispatch();
}

void TestBase::outlog(const char* format, ...)
{
	va_list args;
	char buffer[MAX_LOG_LENGTH];

	va_start(args, format);
	int nret = vsnprintf(buffer, MAX_LOG_LENGTH, format, args);
	va_end(args);

	NS_ASSERT(nret >= 0 && nret < MAX_LOG_LENGTH);
	testlog("[%d] %s", this->getId(), buffer);
}

void TestBase::asyncTask(std::function<void()> callback)
{
	auto self(this->shared_from_this());
	TestService::instance()->asyncTask([self, this, callback]() {
		callback();
	});
}
