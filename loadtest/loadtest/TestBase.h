#ifndef __TEST_BASE_H__
#define __TEST_BASE_H__

#include "common/Common.h"
#include "common/utils/MessageDispatcher.h"

USING_NS;

class TestBase: public std::enable_shared_from_this<TestBase>
{
public:
	TestBase();
	virtual ~TestBase();

	virtual bool runTest() { return true; }

	virtual bool isFinished() const { return m_isFinished; }
	virtual void finish() { m_isFinished = true; }
	virtual void update(float dt);

	virtual std::string getTitle() const { return ""; }
	int32 getId() const { return m_id; }
	void setId(int32 id) { m_id = id; }

    void outlog(const char* format, ...);

	MessageDispatcher* getMainDispatcher() const { return m_mainDispatcher; }

	virtual bool isAsyncRunning() const { return false; }
	// 执行异步任务
	void asyncTask(std::function<void()> callback);

protected:
	bool m_isFinished;
	int32 m_id;

	MessageDispatcher* m_mainDispatcher;
};	

#endif // __TEST_BASE_H__
