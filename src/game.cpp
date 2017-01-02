
#include "game.h"

void Game::Init()
{
	static Unit test;
	static UnitData data;

	test.data = &data;
	test.pos = v3new(1, 2, 3);

	units = NULL;

	stb_arr_push(units, test);
}

void Game::Release()
{
}

void Game::Update()
{
}

void Game::Render()
{
	ImGui::Text("%.2f, %.2f", units[0].pos.x, units[0].pos.y);
}
