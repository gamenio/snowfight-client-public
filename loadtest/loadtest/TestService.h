#ifndef __TEST_SERVICE_H__
#define __TEST_SERVICE_H__

#include "asio.hpp"

#include "common/Common.h"
#include "common/utils/Timer.h"
#include "Common.h"
#include "TestBase.h"

USING_NS;


class TestTask
{
public:
	TestTask(int32 id, float delay, float duration, std::function<TestBase*(float)> callback) :
		m_id(id),
		m_duration(duration),
		m_test(callback(duration)),
		m_isStopped(false),
		m_isRunning(false)
	{
		m_test->setId(id);
		// auto refCount = m_test.use_count();
		m_delayTimer.setDuration(delay);
	}

	~TestTask()
	{
	}

	void stop()
	{
		if (m_isStopped)
			return;

		m_test->finish();
		m_isStopped = true;
	}

	bool update(float dt)
	{
		try
		{
			if (m_isRunning)
			{
				m_test->update(dt);
				if (m_isStopped || m_test->isFinished())
				{
					TESTLOG("[%d] \"%s\" done.", m_id, m_test->getTitle().c_str());
					return false;
				}
			}
			else
			{
				if (m_isStopped)
					return false;

				m_delayTimer.update(dt);
				if (m_delayTimer.passed())
				{
					TESTLOG("[%d] \"%s\" running, duration %.1fs.", m_id, m_test->getTitle().c_str(), m_duration);
					if (m_test->runTest())
						m_isRunning = true;
					else
					{
						TESTLOG("[%d] \"%s\" run failed.", m_id, m_test->getTitle().c_str());
						return false;
					}

				}
			}
		}
		catch (std::exception const& ex)
		{
			TESTLOG("[%d] \"%s\" error. msg: %s", m_id, m_test->getTitle().c_str(), ex.what());
			return false;
		}
		return true;
	}

private:
	int32 m_id;
	std::shared_ptr<TestBase> m_test;
	DelayTimer m_delayTimer;
	float m_duration;

	bool m_isStopped;
	bool m_isRunning;
};


class TestService
{
public:
	static TestService* instance();

	int run(int argc, const char* argv[]);
	void runTest(std::string const& testName, std::function<TestBase*(float)> callback, int32 times = 1, float delay = 0, float maxInterval = 0, float minInterval = 0, float maxDuration = 0, float minDuration = 0);
	void asyncTask(std::function<void()> callback);

private:
	TestService();
	~TestService();

	void mainLoop(float delta);

	void signalHandler(const asio::error_code& error, int signalNumber);
	void shutdownThreadPool();

	std::atomic<bool> m_isStopping;
	bool m_isStopped;
	std::vector<std::thread> m_threadPool;
	asio::io_service m_ioService;

	std::vector<TestTask> m_testTasks;
	std::vector<TestBase*> m_tests;
};

#endif // __TEST_SERVICE_H__
