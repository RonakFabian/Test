#include "Game.h"
#include "System.h"
#include "OrthoCamera.h"
#include "Background.h"
#include "Ship.h"
#include "Asteroid.h"
#include "Explosion.h"
#include "Keyboard.h"
#include "Random.h"
#include "Maths.h"
#include "Bullet.h"
#include "Collision.h"
#include <algorithm>

Game::Game() :
	camera_(0),
	background_(0),
	player_(0),
	collision_(0),
	currentScore_(0)
{
	camera_ = new OrthoCamera();
	camera_->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	camera_->SetFrustum(800.0f, 600.0f, -100.0f, 100.0f);
	background_ = new Background(800.0f, 600.0f);
	collision_ = new Collision();
	maxLives = 3;
	currentLives_ = maxLives;

}

Game::~Game()
{
	delete camera_;
	delete background_;
	delete player_;
	DeleteAllBullets();
	DeleteAllAsteroids();
	DeleteAllExplosions();
	delete collision_;
}

void Game::Update(System* system)
{

	UpdatePlayer(system);
	UpdateAsteroids(system);
	UpdateBullet(system);
	UpdateCollisions();

}

void Game::RenderBackgroundOnly(Graphics* graphics)
{
	camera_->SetAsView(graphics);
	background_->Render(graphics);
}

void Game::RenderEverything(Graphics* graphics)
{
	camera_->SetAsView(graphics);

	background_->Render(graphics);

	if (player_)
	{
		player_->Render(graphics);
	}

	for (AsteroidList::const_iterator asteroidIt = asteroids_.begin(), end = asteroids_.end(); asteroidIt != end; ++asteroidIt)
	{
		(*asteroidIt)->Render(graphics);
	}

	for (BulletList::const_iterator bulletItr = bulletPool_.begin(); bulletItr != bulletPool_.end(); bulletItr++)
	{
		if (*bulletItr)
		{
			(*bulletItr)->Render(graphics);
		}
	}


	for (ExplosionList::const_iterator explosionIt = explosions_.begin(),
		end = explosions_.end();
		explosionIt != end;
		++explosionIt)
	{
		(*explosionIt)->Render(graphics);
	}
}

void Game::InitialiseLevel(int numAsteroids)
{
	DeleteAllAsteroids();
	DeleteAllExplosions();
	DeleteAllBullets();

	SpawnPlayer();
	SpawnAsteroids(numAsteroids);
	maxLives = 3;
	currentScore_ = 0;
	currentLives_ = maxLives;

}

bool Game::IsLevelComplete() const
{
	return (asteroids_.empty() && explosions_.empty());
}

bool Game::IsGameOver() const
{
	return (player_ == 0 && explosions_.empty());
}

void Game::DoCollision(GameEntity* a, GameEntity* b)
{
	Ship* player = static_cast<Ship*>(a == player_ ? a : (b == player_ ? b : 0));
	Bullet* bullet = static_cast<Bullet*>(IsBullet(a) ? a : (IsBullet(b) ? b : 0));
	Asteroid* asteroid = static_cast<Asteroid*>(IsAsteroid(a) ? a : (IsAsteroid(b) ? b : 0));

	if (player && asteroid)
	{
		AsteroidHit(asteroid);

		player->Reset();
		currentLives_--;
		if (currentLives_ <= 0)
		{
			DeletePlayer();
		}
		canShake = true;
		CamTimer = clock();

	}

	if (bullet && asteroid)
	{
		AsteroidHit(asteroid);

		BulletList::const_iterator itr = std::find(bulletPool_.begin(), bulletPool_.end(), bullet);
		DeleteBullet(*itr);
		currentScore_++;
		canShake = true;
		CamTimer = clock();


	}
}

int Game::GetCurrentScore() const
{
	return currentScore_;
}

int Game::GetLives() const
{
	return currentLives_;
}


void Game::operator=(const Game&)
{
}

void Game::SpawnPlayer()
{
	DeletePlayer();
	player_ = new Ship();
	player_->EnableCollisions(collision_, 10.0f);
}

void Game::DeletePlayer()
{
	delete player_;
	player_ = 0;
}

void Game::UpdatePlayer(System* system)
{
	if (player_ == 0)
		return;

	Keyboard* keyboard = system->GetKeyboard();

	float acceleration = 0.0f;
	if (keyboard->IsKeyHeld(VK_UP) || keyboard->IsKeyHeld('W'))
	{
		acceleration = 1.0f;
	}
	else if (keyboard->IsKeyHeld(VK_DOWN) || keyboard->IsKeyHeld('S'))
	{
		acceleration = -1.0f;
	}

	float rotation = 0.0f;
	if (keyboard->IsKeyHeld(VK_RIGHT) || keyboard->IsKeyHeld('D'))
	{
		rotation = -1.0f;
	}
	else if (keyboard->IsKeyHeld(VK_LEFT) || keyboard->IsKeyHeld('A'))
	{
		rotation = 1.0f;
	}

	player_->SetControlInput(acceleration, rotation);
	player_->Update(system);
	WrapEntity(player_);

	if (keyboard->IsKeyPressed(VK_SPACE))
	{
		XMVECTOR playerForward = player_->GetForwardVector();
		XMVECTOR bulletPosition = player_->GetPosition() + playerForward * 10.0f;

		if ((float(clock() - player_->GetLastBulletShotTime()) / CLOCKS_PER_SEC) > player_->GetRateOfFire())
		{

			SpawnBullet(bulletPosition, playerForward);
			player_->ResetTime();

		}

	}


	if (((clock() - CamTimer) / CLOCKS_PER_SEC) > .075f)
	{
		canShake = false;
		CamTimer = clock();
		camera_->SetFrustum(800.0f, 600.0f, -100.0f, 100.0f);
	}


	if (keyboard->IsKeyPressed(VK_TAB))
	{
		if ((float(clock() - player_->GetLastBulletShotTime()) / CLOCKS_PER_SEC) > player_->GetRateOfFire() * 2)
		{
			XMVECTOR playerForward = player_->GetForwardVector();
			XMVECTOR bulletPosition = player_->GetPosition() + playerForward * 10.0f;

			for (AsteroidList::const_iterator asteroidIt = asteroids_.begin(), end = asteroids_.end(); asteroidIt != end; ++asteroidIt)
			{
				XMVECTOR forward = (*asteroidIt)->GetPosition() - player_->GetPosition();
				XMVECTOR normalisedDirection = XMVector3Normalize(forward);
				SpawnBullet(bulletPosition, forward, 10);
			}
			player_->ResetTime();

		}


	}
	if (keyboard->IsKeyPressed(VK_SHIFT))
	{
		DeletePlayer();

	}

	if (canShake)
	{
		CameraShake();
	}

}

void Game::UpdateAsteroids(System* system)
{
	for (AsteroidList::const_iterator asteroidIt = asteroids_.begin(),
		end = asteroids_.end();
		asteroidIt != end;
		++asteroidIt)
	{
		(*asteroidIt)->Update(system);
		WrapEntity(*asteroidIt);
	}
}

void Game::UpdateBullet(System* system)
{
	BulletList::iterator bulletItr;
	for (bulletItr = bulletPool_.begin();
		bulletItr != bulletPool_.end();
		bulletItr++)
	{
		if (*bulletItr)
		{
			(*bulletItr)->Update(system);


			WrapEntity(*bulletItr);

			if (((static_cast<float>(clock()) - (*bulletItr)->GetTimeElapsed()) / CLOCKS_PER_SEC) > (*bulletItr)->GetMaxLifeTime())
			{
				DeleteBullet(*bulletItr);
				return;
			}
		}
	}
}

void Game::WrapEntity(GameEntity* entity) const
{
	XMFLOAT3 entityPosition;
	XMStoreFloat3(&entityPosition, entity->GetPosition());
	entityPosition.x = Maths::WrapModulo(entityPosition.x, -400.0f, 400.0f);
	entityPosition.y = Maths::WrapModulo(entityPosition.y, -300.0f, 300.0f);
	entity->SetPosition(XMLoadFloat3(&entityPosition));
}

void Game::DeleteAllAsteroids()
{
	for (AsteroidList::const_iterator asteroidIt = asteroids_.begin(),
		end = asteroids_.end();
		asteroidIt != end;
		++asteroidIt)
	{
		delete (*asteroidIt);
	}

	asteroids_.clear();
}

void Game::DeleteAllExplosions()
{
	for (ExplosionList::const_iterator explosionIt = explosions_.begin(),
		end = explosions_.end();
		explosionIt != end;
		++explosionIt)
	{
		delete (*explosionIt);
	}

	explosions_.clear();
}

void Game::DeleteAllBullets()
{
	for (BulletList::const_iterator bulletItr = bulletPool_.begin();
		bulletItr != bulletPool_.end();
		bulletItr++)
	{
		delete (*bulletItr);
	}

	bulletPool_.clear();
}

void Game::SpawnBullet(XMVECTOR position, XMVECTOR direction)
{
	Bullet* bullet = new Bullet(position, direction);
	bullet->EnableCollisions(collision_, 3.0f);
	bulletPool_.push_back(bullet);
}

void Game::SpawnBullet(XMVECTOR position, XMVECTOR direction, float projectileSpeed)
{
	Bullet* bullet = new Bullet(position, direction, projectileSpeed);
	bullet->EnableCollisions(collision_, 3.0f);
	bulletPool_.push_back(bullet);
}

void Game::DeleteBullet(Bullet* currentBullet_)
{
	bulletPool_.remove(currentBullet_);
	delete (currentBullet_);
}

void Game::SpawnAsteroids(int numAsteroids)
{
	float halfWidth = 800.0f * 0.5f;
	float halfHeight = 600.0f * 0.5f;
	for (int i = 0; i < numAsteroids; i++)
	{
		float x = Random::GetFloat(-halfWidth, halfWidth);
		float y = Random::GetFloat(-halfHeight, halfHeight);
		XMVECTOR position = XMVectorSet(x, y, 0.0f, 0.0f);
		SpawnAsteroidAt(position, 3);
	}
}

void Game::SpawnAsteroidAt(XMVECTOR position, int size)
{
	const float MAX_ASTEROID_SPEED = 1.0f;

	float angle = Random::GetFloat(Maths::TWO_PI);
	XMMATRIX randomRotation = XMMatrixRotationZ(angle);
	XMVECTOR velocity = XMVectorSet(0.0f, Random::GetFloat(MAX_ASTEROID_SPEED), 0.0f, 0.0f);
	velocity = XMVector3TransformNormal(velocity, randomRotation);

	Asteroid* asteroid = new Asteroid(position, velocity, size);
	asteroid->EnableCollisions(collision_, size * 5.0f);
	asteroids_.push_back(asteroid);
}

bool Game::IsAsteroid(GameEntity* entity) const
{
	return (std::find(asteroids_.begin(),
		asteroids_.end(), entity) != asteroids_.end());
}

bool Game::IsBullet(GameEntity* entity) const
{
	return (std::find(bulletPool_.begin(),
		bulletPool_.end(), entity) != bulletPool_.end());
}

void Game::AsteroidHit(Asteroid* asteroid)
{
	int oldSize = asteroid->GetSize();
	if (oldSize > 1)
	{
		int smallerSize = oldSize - 1;
		XMVECTOR position = asteroid->GetPosition();
		SpawnAsteroidAt(position, smallerSize);
		SpawnAsteroidAt(position, smallerSize);
	}
	DeleteAsteroid(asteroid);
}

void Game::DeleteAsteroid(Asteroid* asteroid)
{
	asteroids_.remove(asteroid);
	delete asteroid;
}

void Game::UpdateCollisions()
{
	collision_->DoCollisions(this);
}

void Game::CameraShake()
{

	camera_->SetFrustum(800.0f + rand() % 10, 600.0f + rand() % 10, -100.0f, 100.0f);

}
