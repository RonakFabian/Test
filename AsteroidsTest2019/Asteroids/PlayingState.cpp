#include "PlayingState.h"
#include "System.h"
#include "Game.h"
#include "FontEngine.h"
#include "Graphics.h"
#include <string>


PlayingState::PlayingState()
{
}

PlayingState::~PlayingState()
{
}

void PlayingState::OnActivate(System* system, StateArgumentMap& args)
{
	Game* game = system->GetGame();

	level_ = args["Level"].asInt;
	game->InitialiseLevel(level_);
}

void PlayingState::OnUpdate(System* system)
{
	Game* game = system->GetGame();
	game->Update(system);
	if (game->IsLevelComplete())
	{
		StateArgumentMap args;
		args["Level"].asInt = level_ + 1;
		system->SetNextState("LevelStart", args);
	}
	else if (game->IsGameOver())
	{
		system->SetNextState("GameOver");
	}
}

void PlayingState::OnRender(System* system)
{
	Game* game = system->GetGame();
	game->RenderEverything(system->GetGraphics());

	Graphics* graphics = system->GetGraphics();
	FontEngine* fontEngine = graphics->GetFontEngine();


	fontEngine->DrawText("Score: " + std::to_string(game->GetCurrentScore()), 680, 30, 0xffffffff, FontEngine::FONT_TYPE_SMALL);

}

void PlayingState::OnDeactivate(System* system)
{
}
