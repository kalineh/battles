
#include <SDL.h>
#include <GL/glew.h>

#pragma warning(push, 0)
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>
#pragma warning(pop)

#include "inc.h"

void Game::Init(void* awindow)
{
	const int UnitCount = 128;

	window = awindow;

	units = NULL;
	stb_arr_setlen(units, UnitCount);
	for (int i = 0; i < UnitCount; ++i)
		units[i] = Unit::CreateTestUnit();

	int width, height;
	SDL_GetWindowSize((SDL_Window*)window, &width, &height);

	for (int i = 0; i < UnitCount; ++i)
	{
		units[i].pos = v3new(
			(float)(stb_frand() * (float)width),
			(float)(stb_frand() * (float)height),
			0.0f
		);
	}

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
	static NVGcontext* context = NULL;

	if (context == NULL)
		context = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

	int width, height;
	SDL_GetWindowSize((SDL_Window*)window, &width, &height);

	nvgBeginFrame(context, width, height, 1.0f);

	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = GetUnit(i);
		UnitID unitID = GetUnitID(unit);
		v4 color = unit->visual->color;

		nvgBeginPath(context);
		nvgCircle(context, unit->pos.x, unit->pos.y, unit->data->radius);
		nvgFillColor(context, nvgRGBAf(color.x, color.y, color.z, color.w));
		nvgFill(context);
	}

	nvgEndFrame(context);

	ImGui::Begin("Game", &guiOpen);
	ImGui::Text("Units: %d", stb_arr_len(units));
	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = &units[i];
		UnitID unitID = GetUnitID(unit);
		ImGui::Text("%d: %d: pos: %.2f,%.2f; ftg: %.2f", i, unitID, unit->pos.x, unit->pos.y, unit->fatigue);
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

