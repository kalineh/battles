
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
	printf("game: init start\n");

	const int UnitCount = 10000;
	const int TeamCount = 2;
	const int GroupCountMin = 2;
	const int GroupCountMax = 3;
	const int GroupUnitCountMin = 12;
	const int GroupUnitCountMax = 13;

	assert(UnitCount > (TeamCount * GroupCountMax * GroupUnitCountMax));

	window = awindow;

	paused = false;

	teams = NULL;
	stb_arr_setlen(teams, TeamCount);

	for (int i = 0; i < TeamCount; ++i)
		GetTeam(i)->Init();

	units = NULL;
	stb_arr_setlen(units, UnitCount);

	for (int i = 0; i < UnitCount; ++i)
		units[i] = Unit::CreateUnit(NULL);

	int width, height;
	SDL_GetWindowSize((SDL_Window*)window, &width, &height);

	grid = (Grid*)stb_malloc(this, sizeof(Grid));
	grid->Init(units, v2inew(16, 16), v2zero(), v2new((float)width, (float)height));

	touch = (Touch*)stb_malloc(this, sizeof(Touch));
	touch->Init(units);

	selectedGroup = 0;
	selectedTeam = 0;

	int groupCountPerTeam = stb_rand() % (GroupCountMax - GroupCountMin) + GroupCountMin;
	groups = NULL;
	stb_arr_setlen(groups, groupCountPerTeam * TeamCount);

	const char* groupTypes[] = {
		"Light",
		"Heavy",
	};

	int unitWriteCursor = 0;

	for (int t = 0; t < TeamCount; ++t)
	{
		printf("game: spawn team %d\n", t);

		for (int i = groupCountPerTeam * t; i < groupCountPerTeam * (t + 1); ++i)
		{
			const char* unitType = groupTypes[i % stb_arrcount(groupTypes)];
			int unitCount = stb_rand() % (GroupUnitCountMax - GroupUnitCountMin) + GroupUnitCountMin;
			printf("game: * group %d (%s x%d)\n", i, unitType, unitCount);

			teams[t].AddGroup(i);

			Group* group = GetGroup(i);
			group->Init(units);
			group->team = t;
			group->CommandFormationBox(0.5f, 1.25f);

			for (int j = 0; j < unitCount; ++j)
			{
				Unit* unit = GetUnit(unitWriteCursor);
				*unit = Unit::CreateUnit(unitType);
				unit->team = t;
				unit->group = i;
				group->AddUnit(unitWriteCursor);
				unitWriteCursor++;
			}
		}
	}

	for (int i = 0; i < stb_arr_len(groups); ++i)
	{
		v2 pos = v2new(
			((float)stb_frand()) * width,
			((float)stb_frand()) * height
		);

		GetGroup(i)->CommandTeleportTo(pos, 0.0f);
		GetGroup(i)->CommandStop();
	}

	printf("game: init complete\n");
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

	for (int i = 0; i < stb_arr_len(teams); ++i)
		GetTeam(i)->Release();
	stb_arr_free(teams);
}

void Game::Update()
{
	grid->Rebuild();

	touch->Clear();

	for (int i = 0; i < stb_arr_len(groups); ++i)
		GetGroup(i)->Update();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F1))
		selectedTeam = 0;
	if (ImGui::IsKeyPressed(SDL_SCANCODE_F2))
		selectedTeam = 1;

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

	if (ImGui::IsKeyPressed(SDLK_p))
	{
		paused = !paused;
	}

	if (ImGui::IsKeyDown(SDLK_x))
	{
		UnitIndex* killQuery = NULL;
		stb_arr_setsize(killQuery, 4);
		v2 mousePos = v2new(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
		grid->Query(&killQuery, mousePos, mousePos, NULL);
		for (int i = 0; i < stb_arr_len(killQuery); ++i)
		{
			UnitIndex id = killQuery[i];
			Unit* unit = GetUnit(id);
			if (circleoverlap(mousePos, 0.0f, unit->pos, unit->data->radius))
				unit->health = 0.0f;
		}
	}

	static UnitIndex moveIndex = InvalidUnitIndex;
	if (ImGui::IsKeyDown(SDLK_z))
	{
		v2 mousePos = v2new(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

		if (moveIndex == InvalidUnitIndex)
		{
			UnitIndex* moveQuery = NULL;
			stb_arr_setsize(moveQuery, 4);
			grid->Query(&moveQuery, mousePos, mousePos, NULL);
			for (int i = 0; i < stb_arr_len(moveQuery); ++i)
			{
				UnitIndex id = moveQuery[i];
				Unit* unit = GetUnit(id);
				if (circleoverlap(mousePos, 0.0f, unit->pos, unit->data->radius))
				{
					moveIndex = id;
					break;
				}
			}
		}

		Unit* unit = GetUnit(moveIndex);
		if (unit->IsValid())
			unit->pos = mousePos;
	}
	else
		moveIndex = InvalidUnitIndex;

	if (paused)
		return;

	UnitIndex* query = NULL;
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

		(void)gridCount;
		(void)touchCount;

		Touch::Entry* entry = touch->GetEntry(i);
		for (int j = 0; j < stb_arrcount(entry->indexes); ++j)
		{
			UnitIndex touchID = entry->indexes[j];
			if (touchID != 0)
			{
				Unit* other = GetUnit(touchID);
				unit->ResolveTouch(other);
				if (unit->team != other->team)
					unit->ResolveCombat(other);
			}
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

	v2 gridLower = grid->lower;
	v2 gridUpper = grid->upper;
	v2 gridSize = grid->upper - gridLower;
	v2 cellSize = v2new(gridSize.x / (float)grid->dim.x, gridSize.y / (float)grid->dim.y);

	nvgBeginPath(context);
	nvgRect(context, gridLower.x, gridLower.y, gridUpper.x - gridLower.x, gridUpper.y - gridLower.y);
	nvgStrokeColor(context, nvgRGBf(1.0f, 1.0f, 1.0f));
	nvgStrokeWidth(context, 2.0f);
	nvgStroke(context);
	nvgClosePath(context);

	for (int y = 0; y < grid->dim.y; ++y)
	{
		for (int x = 0; x < grid->dim.x; ++x)
		{
			v2 cellLower = gridLower + v2new(cellSize.x * (float)(x + 0), cellSize.y * (float)(y + 0));

			nvgBeginPath(context);
			nvgStrokeColor(context, nvgRGBAf(0.25f, 0.25f, 0.25f, 0.25f));
			nvgStrokeWidth(context, 1.0f);
			nvgRect(context, cellLower.x, cellLower.y, cellSize.x, cellSize.y);
			nvgStroke(context);
			nvgClosePath(context);
		}
	}

	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = GetUnit(i);
		if (!unit->IsValid())
			continue;

		v4 color = unit->visual->color;

		Touch::Entry* entry = touch->GetEntry(i);
		if (entry->indexes[0] != 0)
			color = v4rgb1(0, 0.1f, 1);

		if (!unit->IsAlive())
			color.w = 0.1f;

		if (unit->team != selectedTeam)
		{
			color.x = stb_clamp(color.x - 0.25f, 0, 1);
			color.y = stb_clamp(color.y - 0.25f, 0, 1);
			color.z = stb_clamp(color.z - 0.25f, 0, 1);
		}
		if (unit->team == selectedTeam)
		{
			color.x = stb_clamp(color.x + 0.25f, 0, 1);
			color.y = stb_clamp(color.y + 0.25f, 0, 1);
			color.z = stb_clamp(color.z + 0.25f, 0, 1);
		}

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
		nvgStrokeColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, color.w));
		nvgStroke(context);
		nvgClosePath(context);

		nvgBeginPath(context);
		nvgMoveTo(context, unit->pos.x, unit->pos.y);
		nvgLineTo(context, unit->targetPos.x, unit->targetPos.y);
		nvgStrokeColor(context, nvgRGBAf(color.x, color.y, color.z, color.w * 0.5f));
		nvgStroke(context);
		nvgClosePath(context);
	}

	for (int i = 0; i < stb_arr_len(groups); ++i)
	{
		Group* group = groups + i;

		nvgBeginPath(context);
		nvgCircle(context, group->groupPos.x, group->groupPos.y, 7.5f);
		nvgFillColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
		nvgFill(context);
		nvgClosePath(context);

		nvgBeginPath(context);
		nvgMoveTo(context, group->groupPos.x, group->groupPos.y);
		nvgLineTo(context, group->commandPos.x, group->commandPos.y);
		nvgStrokeWidth(context, 4.0f);
		nvgStrokeColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
		nvgStroke(context);
		nvgClosePath(context);

		nvgBeginPath(context);
		nvgMoveTo(context, group->groupPos.x, group->groupPos.y);
		nvgLineTo(context, group->groupPos.x + group->displacementAggregate.x, group->groupPos.y + group->displacementAggregate.y);
		nvgStrokeWidth(context, 4.0f);
		nvgStrokeColor(context, nvgRGBAf(1.0f, 0.0f, 0.0f, 0.5f));
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
	static bool openUnitsWindow2 = false;

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
	ImGui::Text("Formation");
	ImGui::SameLine();
	changed = changed || ImGui::RadioButton("None", &formation, 0); ImGui::SameLine();
	changed = changed || ImGui::RadioButton("Box", &formation, 1); ImGui::SameLine();
	changed = changed || ImGui::RadioButton("Wedge", &formation, 2); ImGui::SameLine();
	changed = changed || ImGui::RadioButton("Circle", &formation, 3); ImGui::SameLine();
	ImGui::NewLine();

	switch (formation)
	{
		case Group::FormationType_Box:
			ImGui::SliderFloat("Box Ratio", &group->formationRatio, 0.0f, 1.0f);			
			ImGui::SliderFloat("Box Loose", &group->formationLoose, -1.0f, 4.0f);			
			break;
		case Group::FormationType_Circle:
			ImGui::SliderFloat("Circle Ratio", &group->formationRatio, 0.0f, 1.0f);			
			ImGui::SliderFloat("Circle Loose", &group->formationLoose, -1.0f, 4.0f);			
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
				group->CommandFormationBox(0.5f, 1.25f);
				break;

			case Group::FormationType_Wedge:
				group->CommandFormationWedge();
				break;

			case Group::FormationType_Circle:
				group->CommandFormationCircle(0.5f, 1.0f);
				break;
		}
	}

	ImGui::LabelText("Alive", "%d/%d", group->CalcUnitAliveCount(), stb_arr_len(group->members));
	ImGui::LabelText("Radius", "%.2f", group->CalcUnitLargestRadius());
	ImGui::LabelText("Disarray", "%.2f", group->disarrayRatio);
	ImGui::LabelText("Displacement", "%.2f,%.2f", group->displacementAggregate.x, group->displacementAggregate.y);

	if (ImGui::Button("Teleport") || ImGui::IsKeyPressed(SDLK_t))
		group->CommandTeleportTo(group->commandPos, group->commandAngle);
	if (ImGui::Button("Teleport Debug") || ImGui::IsKeyPressed(SDLK_y))
		group->CommandTeleportTo(v2new(450.0f, 400.0f), 0.0f);

	ImGui::End();

	grid->RenderImGui();

	//ImGui::ShowMetricsWindow(&openMetricsWindow);

	if (ImGui::Begin("Units", &openUnitsWindow))
	{
		ImGui::Text("Team");
		ImGui::SameLine();
		ImGui::RadioButton("0", &selectedTeam, 0);
		ImGui::SameLine();
		ImGui::RadioButton("1", &selectedTeam, 1);

		ImGui::Text("Group");
		ImGui::SameLine();

		for (int i = 0; i < stb_arr_len(groups); ++i)
		{
			Group* group = groups + i;
			if (group->team != selectedTeam)
				continue;

			char txt[8] = { 0 };
			sprintf(txt, "%d", i);
			ImGui::PushID(i);
			ImGui::RadioButton(txt, &selectedGroup, i);
			ImGui::PopID();
			ImGui::SameLine();
		}

		ImGui::NewLine();

		if (ImGui::TreeNode(units, "All Units (%d)", stb_arr_len(units)))
		{
			for (int i = 0; i < stb_arr_len(units); ++i)
			{
				Unit* unit = &units[i];
				UnitIndex unitIndex = GetUnitIndex(unit);

				if (ImGui::TreeNode(unit, "%s(%d:%d:%d)", unit->data->type, unit->team, unit->group, unitIndex))
				{
					RenderImGuiUnit(i);
					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}

		ImGui::PushID(selectedGroup);
		Group* group = groups + selectedGroup;
		if (ImGui::TreeNode(units, "Group %d:%d Units (%d)", selectedTeam, selectedGroup, stb_arr_len(group->members)))
		{
			for (int i = 0; i < stb_arr_len(group->members); ++i)
			{
				UnitIndex unitIndex = group->members[i];
				Unit* unit = units + unitIndex;

				if (ImGui::TreeNode(unit, "%s(%d:%d:%d)", unit->data->type, unit->team, unit->group, unitIndex))
				{
					RenderImGuiUnit(unitIndex);
					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::End();
}

void Game::RenderImGuiUnit(UnitIndex unitIndex)
{
	Unit* unit = GetUnit(unitIndex);

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
		Touch::Entry* entry = touch->GetEntry(unitIndex);
		for (int j = 0; j < stb_arrcount(Touch::Entry::indexes); ++j)
			ImGui::Text("%d: %d", j, entry->indexes[j]);

		ImGui::TreePop();
	}

	ImGui::SliderFloat3("pos", &unit->pos.x, 0.0f, stb_max(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
	ImGui::SliderFloat("angle", &unit->angle, 0.0f, TWOPI);
	ImGui::SliderFloat2("vel", &unit->vel.x, 0.0f, 100.0f);

	ImGui::SliderFloat("health", &unit->health, 0.0f, unit->data->health);
	ImGui::SliderFloat("fatigue", &unit->fatigue, 0.0f, unit->data->fatigue);
	ImGui::SliderFloat("resolve", &unit->resolve, 0.0f, unit->data->resolve);
}

Team* Game::GetTeam(TeamIndex teamIndex)
{
	assert(teamIndex >= 0);
	assert(teamIndex < stb_arr_len(teams));
	return teams + teamIndex;
}

Group* Game::GetGroup(GroupIndex groupIndex)
{
	assert(groupIndex >= 0);
	assert(groupIndex < stb_arr_len(groups));
	return groups + groupIndex;
}

Unit* Game::GetUnit(UnitIndex unitIndex)
{
	assert(unitIndex >= 0);
	assert(unitIndex < stb_arr_len(units));
	return units + unitIndex;
}

UnitIndex Game::GetUnitIndex(Unit* unit)
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

