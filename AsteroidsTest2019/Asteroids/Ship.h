#ifndef SHIP_H_INCLUDED
#define SHIP_H_INCLUDED

#include "GameEntity.h"
#include "time.h"

class Graphics;

class Ship : public GameEntity
{
public:
	Ship();

	void SetControlInput(float acceleration,
		float rotation);

	void Update(System* system);
	void Render(Graphics* graphics) const;

	XMVECTOR GetForwardVector() const;
	XMVECTOR GetVelocity() const;
	float GetRateOfFire() const;
	clock_t GetLastBulletShotTime() const;

	void Reset();
	void ResetTime();

private:

	float accelerationControl_;
	float rotationControl_;

	XMFLOAT3 velocity_;
	XMFLOAT3 forward_;
	float rotation_;

	float rateOfFire_;
	clock_t lastBulletShotTime_;



};

#endif // SHIP_H_INCLUDED
