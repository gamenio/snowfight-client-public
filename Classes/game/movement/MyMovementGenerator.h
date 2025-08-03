#ifndef __MY_MOVEMENT_GENERATOR_H__
#define __MY_MOVEMENT_GENERATOR_H__

#include "common/Common.h"
#include "game/entities/MovementInfo.h"
#include "game/movement/MovementGenerator.h"

NS_BEGIN

class MyCharacter;

class MyMovementGenerator : public MovementGenerator
{
public:
	MyMovementGenerator(MyCharacter* owner);
	~MyMovementGenerator();

	void update(float delta) override;

	void moveBy(float direction);
	void finish() override;

private:
	void sendMoveStart();
	void sendMoveStop();

	MyCharacter* m_owner;
};

NS_END

#endif // __MY_MOVEMENT_GENERATOR_H__