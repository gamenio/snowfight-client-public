#ifndef __TIMER_H__
#define __TIMER_H__

#include "common/Common.h"

NS_BEGIN

class IntervalTimer
{
public:

	IntervalTimer()
		: m_interval(0), m_current(0)
	{
	}

	void update(float diff)
	{
		if (m_interval <= 0)
			return;

		m_current += diff;
	}

	bool passed()
	{
		if (m_interval <= 0)
			return false;

		return m_current >= m_interval;
	}

	void setPassed()
	{
		m_current = m_interval;
	}

	void reset()
	{
		m_current = 0;
	}


	void setInterval(float interval)
	{
		m_current = 0;
		m_interval = interval;
	}

	float getInterval() const { return m_interval; }
	float getCurrent() const { return m_current; }

private:
	float m_interval;
	float m_current;
};

class DelayTimer
{
public:
    DelayTimer() :
        m_duration(0),
        m_current(0)
    {
    }
    
    void update(float diff)
    {
        if (m_current <= 0)
            return;
        
        m_current -= diff;
    }
    
    bool passed() const
    {
        return m_current <= 0;
    }

	void setPassed() { m_current = 0; }
    
    void reset()
    {
        m_current = m_duration;
    }
    
    void setDuration(float duration)
    {
        m_current = duration;
        m_duration = duration;
    }
    float getDuration() const { return m_duration; }
    
    //float getCurrent() const { return m_current; }
	float getRemainder() const { return std::max(0.f, m_current); }
	float getElapsed() const { return m_duration - this->getRemainder(); }
    
private:
    float m_duration;
    float m_current;
};


NS_END

#endif //__TIMER_H__