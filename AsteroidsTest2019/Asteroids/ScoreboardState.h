#pragma once

#include "GameState.h"
class ScoreboardState : public GameState
{
public:

	ScoreboardState();
	void OnActivate(System* system, StateArgumentMap& args);
	void OnUpdate(System* system);
	void OnRender(System* system);
	void OnDeactivate(System* system);

private:

	int delay_;
	int highScore_;
	bool isHighScore;
	void UpdateHighScore(System* system);
	
};

