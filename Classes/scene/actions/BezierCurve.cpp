#include "BezierCurve.h"


NS_BEGIN

static const float NEWTON_RAPHSON_ERROR = 0.0001f;
static const int32 NEWTON_RAPHSON_MAX_ITER = 10;

// Bezier curve formulas:
//https://en.wikipedia.org/wiki/B%C3%A9zier_curve
//
// Quadratic
static inline float bezierat(float a, float b, float c, float t)
{
	return powf(1 - t, 2) * a + 2 * t*(1 - t)*b + powf(t, 2) *c;
}
// Cubic
static inline float bezierat(float a, float b, float c, float d, float t)
{
	return (powf(1 - t, 3) * a +
		3 * t*(powf(1 - t, 2))*b +
		3 * powf(t, 2)*(1 - t)*c +
		powf(t, 3)*d);
}

float calcQuadBezierLength(float A, float B, float C, float t)
{
	float Scba = sqrt(C + t * (B + A * t));
	float AB_Sc = (2 * A * t * Scba + B * (Scba - sqrt(C)));
	float bbac = 0.f;
	float bac = B + 2 * sqrt(A) * sqrt(C);
	float baa = B + 2 * A * t + 2 * sqrt(A) * Scba;
	if (bac > 0.f && baa > 0.f)
		bbac = (B * B - 4 * A * C) * (log(bac) - log(baa));

	float Sa = 2 * sqrt(A) * AB_Sc;
	float l = (Sa + bbac) / (8 * pow(A, 1.5f));

	NS_ASSERT(!std::isinf(l));
	NS_ASSERT(!std::isnan(l));

	return l;
}

// https://blog.51cto.com/u_15458423/4808292
float calcUniformSpeedQuadBezierTime(float A, float B, float C, float t, float l)
{
	float t1 = t, t2;
	int32 iter = 0;
	do
	{
		++iter;
		float length = calcQuadBezierLength(A, B, C, t1);
		float speed = sqrt(A * t1 * t1 + B * t1 + C);
		NS_ASSERT(speed > 0);
		t2 = t1 - (length - l) / speed;
		NS_ASSERT(!std::isinf(t2));
		NS_ASSERT(!std::isnan(t2));
//		float a = t1 - t2;
		if (fabs(t1 - t2) < NEWTON_RAPHSON_ERROR || iter >= NEWTON_RAPHSON_MAX_ITER)
			break;
		t1 = t2;
	} while (true);

	return t2;
}

//
// BezierCurve
//

BezierCurve::BezierCurve() :
	m_startPosition(Point::ZERO),
	m_previousPosition(Point::ZERO)
{
}

BezierCurve::~BezierCurve() 
{
}

BezierCurve* BezierCurve::create(float t, BezierCurveConfig const& c)
{
	BezierCurve *bezierBy = new (std::nothrow) BezierCurve();
	if (bezierBy && bezierBy->initWithDuration(t, c))
	{
		bezierBy->autorelease();
		return bezierBy;
	}

	delete bezierBy;
	return nullptr;
}

bool BezierCurve::initWithDuration(float t, BezierCurveConfig const& c)
{
	if (ActionInterval::initWithDuration(t))
	{
		m_config = c;
		return true;
	}

	return false;
}

void BezierCurve::startWithTarget(Node *target)
{
	ActionInterval::startWithTarget(target);
	m_previousPosition = m_startPosition = target->getPosition();
}

void BezierCurve::update(float time)
{
	if (_target)
	{
		float x, y;
		switch (m_config.order)
		{
		case BezierCurveConfig::QUADRATIC:
		{
			float xa = 0;
			float xb = m_config.controlPoints[0].x;
			float xc = m_config.endPosition.x;

			float ya = 0;
			float yb = m_config.controlPoints[0].y;
			float yc = m_config.endPosition.y;

			float ax = xa - 2 * xb + xc;
			float ay = ya - 2 * yb + yc;
			float bx = 2 * xb - 2 * xa;
			float by = 2 * yb - 2 * ya;
			float A = 4 * (ax * ax + ay * ay);
			float B = 4 * (ax * bx + ay * by);
			float C = bx * bx + by * by;
			float l = time * m_config.length;
			float t = calcUniformSpeedQuadBezierTime(A, B, C, time, l);

			x = bezierat(xa, xb, xc, t);
			y = bezierat(ya, yb, yc, t);

			break;
		}
		case BezierCurveConfig::CUBIC:
		{
			float xa = 0;
			float xb = m_config.controlPoints[0].x;
			float xc = m_config.controlPoints[1].x;
			float xd = m_config.endPosition.x;

			float ya = 0;
			float yb = m_config.controlPoints[0].y;
			float yc = m_config.controlPoints[1].y;
			float yd = m_config.endPosition.y;

			x = bezierat(xa, xb, xc, xd, time);
			y = bezierat(ya, yb, yc, yd, time);
			break;
		}
		default:
			break;
		}

		Vec2 newPos = m_startPosition + Vec2(x, y);
        _target->setPosition(newPos);
        
		m_previousPosition = newPos;
	}
}

void BezierCurve::step(float dt)
{
	if (_firstTick)
	{
		_firstTick = false;
	}
	else
	{
		_elapsed += dt;
	}


	float updateDt = MAX(0,                                  // needed for rewind. elapsed could be negative
		MIN(1, _elapsed / _duration)
	);

	this->update(updateDt);

	_done = _elapsed >= _duration;
}

NS_END
