
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
	const int SpawnCount = 64;
	const int GroupCount = 3;

	window = awindow;

	units = NULL;
	stb_arr_setlen(units, UnitCount);

	for (int i = 0; i < UnitCount; ++i)
		units[i] = Unit::CreateNullUnit();

	for (int i = 0; i < SpawnCount; ++i)
		units[stb_rand() % UnitCount] = (i % 2 == 0) ? Unit::CreateTestLightUnit() : Unit::CreateTestHeavyUnit();
		//units[stb_rand() % UnitCount] = Unit::CreateTestUnit();

	int width, height;
	SDL_GetWindowSize((SDL_Window*)window, &width, &height);

	for (int i = 0; i < UnitCount; ++i)
	{
		Unit* unit = GetUnit((UnitID)i);

		if (!unit->IsValid())
			continue;

		unit->pos = v2new(
			(float)(stb_frand() * (float)width),
			(float)(stb_frand() * (float)height)
		);

		unit->pos = v2new(450 + (unit->team == 1 ? 100 : 0), 300);

		unit->targetPos = unit->pos;
		unit->targetAngle = (float)stb_frand() * TWOPI;
	}

	grid = (Grid*)stb_malloc(this, sizeof(Grid));
	grid->Init(units, v2inew(16, 16), v2zero(), v2new((float)width, (float)height));

	touch = (Touch*)stb_malloc(this, sizeof(Touch));
	touch->Init(units);

	selectedGroup = 0;
	groups = NULL;
	stb_arr_setlen(groups, GroupCount);

	for (int i = 0; i < stb_arr_len(groups); ++i)
		GetGroup(i)->Init(units);

	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = units + i;
		if (!unit->IsValid())
			continue;

		Group* g = GetGroup(stb_rand() % stb_arr_len(groups));
		g->AddUnit((UnitID)i);
	}

	for (int i = 0; i < stb_arr_len(groups); ++i)
		GetGroup(i)->CommandStop();
}

void Game::Release()
{
	stb_arr_free(units);

	grid->Release();
	grid = NULL;

	touch->Release();
	touch = NULL;

	for (int i = 0; i < stb_arr_len(groups); ++i)
		GetGroup(i)->Release();

	stb_arr_free(groups);
}

void Game::Update()
{
	grid->Rebuild();

	touch->Clear();

	for (int i = 0; i < stb_arr_len(groups); ++i)
		GetGroup(i)->Update();

	for (int i = 0; i < stb_arr_len(groups); ++i)
	{
		if (ImGui::IsKeyPressed(SDLK_1 + i))
			selectedGroup = i;
	}

	if (ImGui::IsMouseDown(1) || ImGui::IsKeyPressed(SDLK_SPACE))
	{
		v2 click = v2new(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		Group* group = GetGroup(selectedGroup);
		group->CommandMoveTo(click, 0.0f);
	}

	UnitID* query = NULL;
	stb_arr_setsize(query, 16);

	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = GetUnit(i);

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		int gridCount = grid->Query(&query, unit->pos, unit->pos, unit);
		int touchCount = touch->Collect(unit, query);

		Touch::Entry* entry = touch->GetEntry(i);
		for (int j = 0; j < stb_arrcount(entry->ids); ++j)
		{
			UnitID touchID = entry->ids[j];
			if (touchID != 0)
				unit->ResolveTouch(GetUnit(touchID));
		}

		unit->AI();
		unit->Update();
	}

	stb_arr_free(query);
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
		if (!unit->IsValid())
			continue;

		v4 color = unit->visual->color;

		Touch::Entry* entry = touch->GetEntry(i);
		if (entry->ids[0] != 0)
			color = v4rgb1(0,1,1);

		nvgBeginPath(context);
		nvgCircle(context, unit->pos.x, unit->pos.y, unit->data->radius);
		nvgFillColor(context, nvgRGBAf(color.x, color.y, color.z, color.w));
		nvgFill(context);
		nvgClosePath(context);

		v2 dir = v2fromangle(unit->angle);
		v2 tip = unit->pos + dir * unit->data->radius * 1.25f;

		nvgBeginPath(context);
		nvgMoveTo(context, unit->pos.x, unit->pos.y);
		nvgLineTo(context, tip.x, tip.y);
		nvgStrokeColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
		nvgStroke(context);
		nvgClosePath(context);

		nvgBeginPath(context);
		nvgMoveTo(context, unit->pos.x, unit->pos.y);
		nvgLineTo(context, unit->targetPos.x, unit->targetPos.y);
		nvgStrokeColor(context, nvgRGBAf(color.x, color.y, color.z, color.w * 0.5f));
		nvgStroke(context);
		nvgClosePath(context);
	}

	nvgEndFrame(context);
	
	RenderImGui();
}

void Game::RenderImGui()
{
	static bool openMetricsWindow = false;
	static bool openGameWindow = false;
	static bool openUnitsWindow = false;

	ImGui::Begin("Game", &openGameWindow);

	for (int i = 0; i < stb_arr_len(groups); ++i)
	{
		char name[32] = { 0 };
		stb_snprintf(name, 31, "group%d", i);
		if (ImGui::RadioButton(name, &selectedGroup, i))
			selectedGroup = i;
	}

	ImGui::Separator();
	Group* group = GetGroup(selectedGroup);
	int formation = (int)group->formationType;
	bool changed = false;
	changed = changed || ImGui::RadioButton("Formation None", &formation, 0);
	changed = changed || ImGui::RadioButton("Formation Box", &formation, 1);
	changed = changed || ImGui::RadioButton("Formation Wedge", &formation, 2);

	switch (formation)
	{
		case Group::FormationType_Box:
			ImGui::SliderFloat("Box Ratio", &group->boxRatio, 0.0f, 1.0f);			
			ImGui::SliderFloat("Box Loose", &group->boxLoose, -1.0f, 4.0f);			
			break;
	}
	
	if (changed)
	{
		switch (formation)
		{
			case Group::FormationType_None:
				group->CommandFormationNone();
				break;

			case Group::FormationType_Box:
				group->CommandFormationBox(0.5f, 1.0f);
				break;

			case Group::FormationType_Wedge:
				group->CommandFormationWedge();
				break;
		}
	}

	ImGui::End();

	grid->RenderImGui();

	//ImGui::ShowMetricsWindow(&openMetricsWindow);

	if (ImGui::Begin("Units", &openUnitsWindow))
	{
		if (ImGui::TreeNode(units, "All Units (%d)", stb_arr_len(units)))
		{
			for (int i = 0; i < stb_arr_len(units); ++i)
			{
				Unit* unit = &units[i];
				UnitID unitID = GetUnitID(unit);

				if (ImGui::TreeNode(unit, "%s(%d)", unit->data->type, unitID))
				{
					ImGui::LabelText("Type", unit->data->type);

					if (ImGui::TreeNode("Config"))
					{
						ImGui::LabelText("Radius", "%.2f", unit->data->radius);
						ImGui::LabelText("Mass", "%.2f", unit->data->mass);
						ImGui::LabelText("Accel", "%.2f", unit->data->accel);
						ImGui::LabelText("Armor", "%.2f", unit->data->armor);
						ImGui::LabelText("Health", "%.2f", unit->data->health);
						ImGui::LabelText("Fatigue", "%.2f", unit->data->fatigue);
						ImGui::LabelText("Resolve", "%.2f", unit->data->resolve);
						ImGui::LabelText("Flyer", "%s", unit->data->flyer ? "true" : "false");
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Visual"))
					{
						ImGui::LabelText("Color", "%.2f,%.2f,%.2f,%.2f",
							unit->visual->color.x,
							unit->visual->color.y,
							unit->visual->color.z,
							unit->visual->color.w
						);
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Touching"))
					{
						Touch::Entry* entry = touch->GetEntry(i);
						for (int j = 0; j < stb_arrcount(Touch::Entry::ids); ++j)
							ImGui::Text("%d: %d", j, entry->ids[j]);

						ImGui::TreePop();
					}

					ImGui::SliderFloat3("pos", &unit->pos.x, 0.0f, stb_max(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
					ImGui::SliderFloat("angle", &unit->angle, 0.0f, TWOPI);
					ImGui::SliderFloat2("vel", &unit->vel.x, 0.0f, 100.0f);

					ImGui::SliderFloat("health", &unit->health, 0.0f, unit->data->health);
					ImGui::SliderFloat("fatigue", &unit->fatigue, 0.0f, unit->data->fatigue);
					ImGui::SliderFloat("resolve", &unit->resolve, 0.0f, unit->data->resolve);

					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
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

Group* Game::GetGroup(int index)
{
	assert(index >= 0);
	assert(index < stb_arr_len(groups));

	return groups + index;
}

