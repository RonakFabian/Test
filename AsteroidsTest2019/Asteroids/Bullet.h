#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

#include "GameEntity.h"
#include <time.h>

class Bullet : public GameEntity
{
public:
	Bullet(XMVECTOR position,XMVECTOR direction);

	void Update(System *system);
	void Render(Graphics *graphics) const;
	clock_t GetTimeElapsed() const;
	float GetMaxLifeTime() const;

private:

	XMFLOAT3 velocity_;
	clock_t currentLifeTime;
	float maxLifeTime;

};

#endif // BULLET_H_INCLUDED
