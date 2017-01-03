
#include "inc.h"

void Game::Init()
{
	const int UnitCount = 16;

	units = NULL;
	stb_arr_setlen(units, UnitCount);
	for (int i = 0; i < UnitCount; ++i)
		units[i] = Unit::CreateTestUnit();

	grid = (Grid*)stb_malloc(this, sizeof(Grid));
	grid->Init(this, v2inew(8, 8), v2zero(), v2new(8.0f, 8.0f));
}

void Game::Release()
{
	units = NULL;
	grid = NULL;
}

void Game::Update()
{
	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = GetUnit(i);
		UnitID unitID = GetUnitID(unit);
		unit->fatigue += 0.01f * (float)unitID;
	}
}

void Game::Render()
{
	static bool guiOpen = true;

	ImGui::Begin("Game", &guiOpen);
	ImGui::Text("%.2f, %.2f", units[0].pos.x, units[0].pos.y);
	ImGui::Text("Units: %d", stb_arr_len(units));
	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = &units[i];
		UnitID unitID = GetUnitID(unit);
		ImGui::Text("%d: %d: fatigue: %.2f", i, unitID, unit->fatigue);
	}

	ImGui::End();
}

Unit* Game::GetUnit(UnitID id)
{
	assert(id >= 0);
	assert(id < stb_arr_len(units));
	return units + id;
}

UnitID Game::GetUnitID(Unit* unit)
{
	int offset = (int)(unit - units);
	int index = offset;

	assert(index >= 0);
	assert(index < stb_arr_len(units));

	return index;
}

Grid* Game::GetGrid()
{
	return grid;
}

