#include "TestService.h"

#include <tclap/CmdLine.h>

#include "common/utils/TimeUtil.h"
#include "PlayTest.h"

#define	THREAD_COUNT								5

static const float UPDATE_SLEEP_TIME = 1.0f / 60;

struct TestCase
{
    std::string name;
    std::function<TestBase*(float)> callback;
};

std::vector<TestCase> gTests = {
    { "PlayTest", [](float duration) { return new PlayTest(duration); } },
};

bool findTest(std::string const& name, TestCase& result)
{
    std::string dstName = name;
    std::transform(dstName.begin(), dstName.end(), dstName.begin(), ::tolower);
    for(auto it = gTests.begin(); it != gTests.end(); ++it)
    {
        std::string copy = name;
        std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
        if(copy == dstName)
        {
            result = *it;
            return true;
        }

    }
    return false;
}

TestService* TestService::instance()
{
	static TestService instance;
	return &instance;
}

TestService::TestService():
	m_isStopped(false),
	m_isStopping(false)
{
}

TestService::~TestService()
{
	this->shutdownThreadPool();
}

void TestService::mainLoop(float delta)
{
	if (m_isStopped)
		return;

	if (m_isStopping)
	{
		for (auto it = m_testTasks.begin(); it != m_testTasks.end(); ++it)
		{
			TestTask& task = *it;
			task.stop();
		}

		m_isStopping = false;
		m_isStopped = true;
	}

	for (auto it = m_testTasks.begin(); it != m_testTasks.end();)
	{
		TestTask& task = *it;
		if (!task.update(delta))
			it = m_testTasks.erase(it);
		else
			++it;

		if(m_testTasks.empty())
			TESTLOG("========> Test have been completed.");
	}

	PoolManager::getInstance()->getCurrentPool()->clear();
}

void TestService::signalHandler(const asio::error_code& error, int signalNumber)
{
	if (error)
	{
		std::string message = error.message();
		TESTLOG("signalHandler msg=%s", error.message().c_str());
	}
	m_isStopping = true;
}

void TestService::shutdownThreadPool()
{
	if (!m_ioService.stopped())
		m_ioService.stop();

	for (auto& thread : m_threadPool)
	{
		thread.join();
	}
	m_threadPool.clear();
}

// Round test
// loadtest -n playtest -t 1000 --min-interval 2 --max-interval 5
// Login logout test
// loadtest -n playtest -t 1000 --min-interval 1 --max-interval 5 --min-duration 5 --max-duration 30
int TestService::run(int argc, const char* argv[])
{
    srand((unsigned)time(NULL));
    
    try {
        TCLAP::CmdLine cmd("Load test service", ' ');
        cmd.setExceptionHandling(false);
        
        TCLAP::ValueArg<std::string> nameArg("n", "name","Specify test name.", true, "", "test-name");
        TCLAP::ValueArg<int32> timesArg("t", "times", "Specify test times.", false, 1, "test-times");
        TCLAP::ValueArg<float> delayArg("", "delay","Specify test delay time.", false, 0.f, "seconds");
        TCLAP::ValueArg<float> maxIntervalArg("","max-interval", "Specify maximum interval between tests.", false, 0.f, "seconds");
        TCLAP::ValueArg<float> minIntervalArg("","min-interval", "Specify minimum interval between tests.", false, 0.f, "seconds");
        TCLAP::ValueArg<float> maxDurationArg("","max-duration", "Specify maximum duration of each test.", false, 0.f, "seconds");
        TCLAP::ValueArg<float> minDurationArg("","min-duration", "Specify minimum duration of each test.", false, 0.f, "seconds");
        cmd.add(minDurationArg);
        cmd.add(maxDurationArg);
        cmd.add(minIntervalArg);
        cmd.add(maxIntervalArg);
        cmd.add(delayArg);
        cmd.add(timesArg);
        cmd.add(nameArg);
        
        cmd.parse(argc, argv);
        
        std::string name = nameArg.getValue();
        int32 times = timesArg.getValue();
        float delay = delayArg.getValue();
        float maxInterval = maxIntervalArg.getValue();
        float minInterval = minIntervalArg.getValue();
        float maxDuration = maxDurationArg.getValue();
        float minDuration = minDurationArg.getValue();
        

        TestCase test;
        if(findTest(name, test))
        {
            this->runTest(test.name, test.callback, times, delay, maxInterval, minInterval, maxDuration, minDuration);
        }
        else
        {
            TESTLOG("Specify test '%s' does not exist", name.c_str());
            return EXIT_FAILURE;
        }
        
    }
    catch (TCLAP::ArgException& ae)  // catch any exceptions
    {
        TESTLOG("error: %s for arg %s", ae.error().c_str(), ae.argId().c_str());
        return EXIT_FAILURE;
    }
    catch(TCLAP::ExitException& ee)
    {
        return ee.getExitStatus();
    }
    
	static asio::signal_set signals(m_ioService, SIGINT, SIGTERM);
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
    signals.add(SIGBREAK);
#endif
	signals.async_wait(std::bind(&TestService::signalHandler, this, std::placeholders::_1, std::placeholders::_2));

	for (int32 i = 0; i < THREAD_COUNT; ++i)
		m_threadPool.emplace_back([this]() {
		m_ioService.run();
		TESTLOG("IOService exit...");
	});



	NSTime prevTime = time_util::getUptimeMillis();
	while (!m_isStopped)
	{
		NSTime currTime = time_util::getUptimeMillis();
		NSTime diff = currTime - prevTime;
		prevTime = currTime;

		float delta = diff / 1000.0f;
		this->mainLoop(delta);
		if (delta < UPDATE_SLEEP_TIME)
		{
			NSTime sleepTime  = (UPDATE_SLEEP_TIME - delta) * 1000000;
			std::this_thread::sleep_for(std::chrono::microseconds(sleepTime));
		}
		//else
		//	TESTLOG("===========================================>delta: %f", delta);

	}

	this->shutdownThreadPool();

	return EXIT_SUCCESS;
}

void TestService::runTest(std::string const& testName, std::function<TestBase*(float)> callback, int32 times, float delay, float maxInterval, float minInterval, float maxDuration, float minDuration)
{
	TESTLOG("========> Running '%s' %d times, delay %.1fs, interval %.1f-%.1fs, duration %.1f-%.1fs", testName.c_str(), times, delay, minInterval, maxInterval, minDuration, maxDuration);

	maxDuration = std::max(maxDuration, minDuration);
	maxInterval = std::max(maxInterval, minInterval);
	float timePoint = delay;
	for (int32 i = 0; i < times; ++i)
	{
        float dur = CCRANDOM_0_1() * (maxDuration - minDuration) + minDuration;
		m_testTasks.emplace_back((i + 1), timePoint, dur, callback);
        
        float inr = CCRANDOM_0_1() * (maxInterval - minInterval) + minInterval;
        timePoint += inr;
	}
}

void TestService::asyncTask(std::function<void()> callback)
{
	m_ioService.post([callback]() {
		callback();
	});
}
