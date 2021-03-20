#include "ScoreboardState.h"
#include "System.h"
#include "Graphics.h"
#include "FontEngine.h"
#include "Game.h"
#include <string>
#include <iostream>
#include <fstream>
#include <direct.h>
#include <sstream>


ScoreboardState::ScoreboardState() :
	delay_(0),
	isHighScore(false)
{
}

void ScoreboardState::OnActivate(System* system, StateArgumentMap& args)
{
	isHighScore = false;
	delay_ = 120;
	UpdateHighScore(system);
}

void ScoreboardState::OnUpdate(System* system)
{
	if (--delay_ == 0)
	{
		system->SetNextState("MainMenu");
	}
}

void ScoreboardState::OnRender(System* system)
{

	Game* game = system->GetGame();
	Graphics* graphics = system->GetGraphics();
	FontEngine* fontEngine = graphics->GetFontEngine();

	game->RenderBackgroundOnly(graphics);

	if (isHighScore)
	{


		int textWidth = fontEngine->CalculateTextWidth("New Highscore!:", FontEngine::FONT_TYPE_MEDIUM);
		int textX = (800 - textWidth) / 2;
		int textY = (600 - 48) / 2;

		fontEngine->DrawText("New Highscore!: ", textX, textY, 0xff00ffff, FontEngine::FONT_TYPE_MEDIUM);
		fontEngine->DrawText(std::to_string(game->GetCurrentScore()), textX+100 ,textY+50, 0xff00ffff, FontEngine::FONT_TYPE_MEDIUM);
	}
	else
	{
		int textWidth = fontEngine->CalculateTextWidth("Score:   ", FontEngine::FONT_TYPE_MEDIUM);
		int textX = (800 - textWidth) / 2;
		int textY = (600 - 48) / 2;

		fontEngine->DrawText("Score: "+ std::to_string(game->GetCurrentScore()), textX-30, textY, 0xff00ffff, FontEngine::FONT_TYPE_LARGE);
		fontEngine->DrawText("Highscore: "+std::to_string(highScore_), textX+20 , textY + 50, 0xffffffff, FontEngine::FONT_TYPE_SMALL);



	}

}

void ScoreboardState::OnDeactivate(System* system)
{

}
void ScoreboardState::UpdateHighScore(System* system)
{
	Game* game = system->GetGame();
	char direc[100];
	_getcwd(direc, 100);
	std::string path(direc);

	char data[50];

	std::ifstream fin;
	fin.open(path + "\\HighScore.txt");

	fin >> data;
	std::string s(data);
	std::stringstream scoreString(s);
	scoreString >> highScore_;

	fin.close();

	if (game->GetCurrentScore() > highScore_)
	{
		highScore_ = game->GetCurrentScore();

		std::ofstream fout;
		fout.open(path + "\\HighScore.txt");

		std::string newHighScore = std::to_string(game->GetCurrentScore());
		strcpy(data, newHighScore.c_str());
		fout << data << std::endl;

		fout.close();

		isHighScore = true;
	}

}
