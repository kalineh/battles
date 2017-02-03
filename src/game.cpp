
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

	const int UnitCount = 1000;
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

	selectedTeam = 0;
	selectedGroup = 0;
	selectedUnit = InvalidUnitIndex;

	hoverUnitFriendly = InvalidUnitIndex;
	hoverUnitHostile = InvalidUnitIndex;

	cursorState = CursorState_None,
	cursorPos = v2zero();
	cursorAnchor = v2zero();

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
			group->Init(teams, groups, units);
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

	int groupSelectionOffset = 0;
	if (selectedTeam == 1)
		groupSelectionOffset = stb_arr_len(GetTeam(0)->groups);

	for (int i = 0; i < stb_arr_len(groups); ++i)
	{
		int groupsPerTeam = stb_arr_len(groups) / stb_arr_len(teams);
		int teamIndex = i / groupsPerTeam;
		int groupIndex = i % groupsPerTeam;

		v2 pos = v2new(
			(0.35f + 0.3f * (1.0f / (float)groupsPerTeam) * groupIndex) * width,
			(0.4f + 0.2f * (float)teamIndex) * height
		);

		GetGroup(i)->CommandMoveToInstant(pos, 0.0f);
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

	UpdateInput();

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

void Game::UpdateInput()
{
	cursorPos = v2new(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

	// keyboard input

	if (ImGui::IsKeyPressed(SDLK_TAB))
	{
		selectedTeam = !selectedTeam;
		if (selectedTeam == 0)
			selectedGroup = 0;
		else
			selectedGroup = stb_arr_len(GetTeam(0)->groups);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F1))
	{
		selectedTeam = 0;
		selectedGroup = 0;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F2))
	{
		selectedTeam = 1;
		selectedGroup = stb_arr_len(GetTeam(0)->groups);
	}

	if (ImGui::IsKeyPressed(SDLK_p))
	{
		paused = !paused;
	}

	if (ImGui::IsKeyDown(SDLK_x))
	{
		UnitIndex* debugKillQuery = NULL;
		stb_arr_setsize(debugKillQuery, 4);
		grid->Query(&debugKillQuery, cursorPos, cursorPos, NULL);
		for (int i = 0; i < stb_arr_len(debugKillQuery); ++i)
		{
			UnitIndex id = debugKillQuery[i];
			Unit* unit = GetUnit(id);
			if (circleoverlap(cursorPos, 0.0f, unit->pos, unit->data->radius))
				unit->health = 0.0f;
		}
	}

	static UnitIndex debugMoveIndex = InvalidUnitIndex;
	if (ImGui::IsKeyDown(SDLK_z))
	{
		if (debugMoveIndex == InvalidUnitIndex)
		{
			UnitIndex* debugMoveQuery = NULL;
			stb_arr_setsize(debugMoveQuery, 4);
			grid->Query(&debugMoveQuery, cursorPos, cursorPos, NULL);
			for (int i = 0; i < stb_arr_len(debugMoveQuery); ++i)
			{
				UnitIndex id = debugMoveQuery[i];
				Unit* unit = GetUnit(id);
				if (circleoverlap(cursorPos, 0.0f, unit->pos, unit->data->radius))
				{
					debugMoveIndex = id;
					break;
				}
			}
		}

		Unit* unit = GetUnit(debugMoveIndex);
		if (unit->IsValid())
			unit->pos = cursorPos;
	}
	else
		debugMoveIndex = InvalidUnitIndex;

	int groupSelectionOffset = 0;
	if (selectedTeam == 1)
		groupSelectionOffset = stb_arr_len(GetTeam(0)->groups);

	int groupSelectMax = stb_arr_len(GetTeam(selectedTeam)->groups);

	for (int i = 0; i < groupSelectMax; ++i)
	{
		if (ImGui::IsKeyPressed(SDLK_1 + i))
			selectedGroup = groupSelectionOffset + i;
	}

	// cursor input

	UnitIndex* hoverUnitFriendlyQuery = NULL;
	stb_arr_setsize(hoverUnitFriendlyQuery, 4);
	grid->Query(&hoverUnitFriendlyQuery, cursorPos, cursorPos, NULL);
	float hoverUnitFriendlyQueryNearestSql = 10000.0f;
	UnitIndex hoverUnitFriendlyQueryNearestUnitIndex = InvalidUnitIndex;
	for (int i = 0; i < stb_arr_len(hoverUnitFriendlyQuery); ++i)
	{
		UnitIndex id = hoverUnitFriendlyQuery[i];
		Unit* unit = GetUnit(id);
		if (unit->team != selectedTeam)
			continue;

		if (circleoverlap(cursorPos, 0.0f, unit->pos, unit->data->radius + GROUP_SELECT_SEARCH_RADIUS))
		{
			v2 ofs = unit->pos - cursorPos;
			float sql = v2lensq(ofs);
			if (sql < hoverUnitFriendlyQueryNearestSql)
			{
				hoverUnitFriendlyQueryNearestSql = sql;
				hoverUnitFriendlyQueryNearestUnitIndex = id;
			}
		}
	}

	hoverUnitFriendly = hoverUnitFriendlyQueryNearestUnitIndex;

	UnitIndex* hoverUnitHostileQuery = NULL;
	stb_arr_setsize(hoverUnitHostileQuery, 4);
	grid->Query(&hoverUnitHostileQuery, cursorPos, cursorPos, NULL);
	float hoverUnitHostileQueryNearestSql = 10000.0f;
	UnitIndex hoverUnitHostileQueryNearestUnitIndex = InvalidUnitIndex;
	for (int i = 0; i < stb_arr_len(hoverUnitHostileQuery); ++i)
	{
		UnitIndex id = hoverUnitHostileQuery[i];
		Unit* unit = GetUnit(id);
		if (unit->team == selectedTeam)
			continue;

		if (circleoverlap(cursorPos, 0.0f, unit->pos, unit->data->radius + GROUP_SELECT_SEARCH_RADIUS))
		{
			v2 ofs = unit->pos - cursorPos;
			float sql = v2lensq(ofs);
			if (sql < hoverUnitHostileQueryNearestSql)
			{
				hoverUnitHostileQueryNearestSql = sql;
				hoverUnitHostileQueryNearestUnitIndex = id;
			}
		}
	}

	hoverUnitHostile = hoverUnitHostileQueryNearestUnitIndex;

	switch (cursorState)
	{
	case CursorState_None:
	case CursorState_HoverFriendly:
	case CursorState_HoverHostile:
		if (hoverUnitFriendly != InvalidUnitIndex)
			cursorState = CursorState_HoverFriendly;
		else if (hoverUnitHostile != InvalidUnitIndex)
			cursorState = CursorState_HoverHostile;
		else
			cursorState = CursorState_None;
		break;

	case CursorState_MoveCommand:
		break;
	case CursorState_DragSelect:
		break;
	case CursorState_Pan:
		break;
	}

	switch (cursorState)
	{
	case CursorState_None:
		if (ImGui::IsMouseClicked(0))
		{
			cursorState = CursorState_DragSelect;
			cursorAnchor = cursorPos;
		}
		else if (ImGui::IsMouseClicked(1))
			if (selectedGroup != InvalidGroupIndex)
			{
				cursorState = CursorState_MoveCommand;
				cursorAnchor = cursorPos;
			}
			else
				cursorState = CursorState_Pan;
		break;

	case CursorState_HoverFriendly:
		if (ImGui::IsMouseClicked(0))
			selectedGroup = GetUnit(hoverUnitFriendly)->group;
		if (ImGui::IsMouseClicked(1))
			if (selectedGroup != InvalidGroupIndex)
			{
				cursorState = CursorState_MoveCommand;
				cursorAnchor = cursorPos;
			}
		break;

	case CursorState_HoverHostile:
		if (ImGui::IsMouseClicked(1))
		{
			if (selectedGroup != InvalidGroupIndex)
			{
				Group* group = GetGroup(selectedGroup);
				GroupIndex target = GetUnit(hoverUnitHostile)->group;
				group->CommandMoveAttack(target);
			}
		}
		break;

	case CursorState_MoveCommand:
		if (ImGui::IsMouseReleased(1))
		{
			Group* group = GetGroup(selectedGroup);
			v2 ofs = cursorPos - cursorAnchor;

			float len = v2lensafe(ofs);
			float angle = group->commandAngle;

			if (len > GROUP_MOVE_COMMAND_ROTATE_MIN)
				angle = v2toangle(ofs) + HALFPI;

			group->CommandMoveTo(cursorAnchor, angle);

			if (hoverUnitFriendly != InvalidUnitIndex)
				cursorState = CursorState_HoverFriendly;
			else if (hoverUnitHostile != InvalidUnitIndex)
				cursorState = CursorState_HoverHostile;
			else
				cursorState = CursorState_None;

			cursorAnchor = v2zero();
		}
		else
		{
			hoverUnitFriendly = InvalidUnitIndex;
			hoverUnitHostile = InvalidUnitIndex;
		}
		break;

	case CursorState_DragSelect:
		if (ImGui::IsMouseReleased(0))
		{
			if (hoverUnitFriendly != InvalidUnitIndex)
				cursorState = CursorState_HoverFriendly;
			else if (hoverUnitHostile != InvalidUnitIndex)
				cursorState = CursorState_HoverHostile;
			else
				cursorState = CursorState_None;
		}
		else
		{
			hoverUnitFriendly = InvalidUnitIndex;
			hoverUnitHostile = InvalidUnitIndex;
		}
		break;

	case CursorState_Pan:
		if (ImGui::IsMouseReleased(0))
		{
			if (hoverUnitFriendly != InvalidUnitIndex)
				cursorState = CursorState_HoverFriendly;
			else if (hoverUnitHostile != InvalidUnitIndex)
				cursorState = CursorState_HoverHostile;
			else
				cursorState = CursorState_None;
		}
		else
		{
			hoverUnitFriendly = InvalidUnitIndex;
			hoverUnitHostile = InvalidUnitIndex;
		}
		break;
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

	int hoverGroupIndexFriendly = InvalidGroupIndex;
	if (hoverUnitFriendly != InvalidUnitIndex)
	{
		Unit* unit = GetUnit(hoverUnitFriendly);
		hoverGroupIndexFriendly = unit->group;
	}

	int hoverGroupIndexHostile = InvalidGroupIndex;
	if (hoverUnitHostile != InvalidUnitIndex)
	{
		Unit* unit = GetUnit(hoverUnitHostile);
		hoverGroupIndexHostile = unit->group;
	}

	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = GetUnit(i);
		if (!unit->IsValid())
			continue;

		v4 color = unit->visual->color;
		v4 border = v4new(1.0f, 1.0f, 1.0f, 1.0f);

		Touch::Entry* entry = touch->GetEntry(i);
		if (entry->indexes[0] != 0)
			border = v4rgb1(0.7f, 0.7f, 0.7f);

		if (!unit->IsAlive())
		{
			color.w = 0.1f;
			border.w = 0.1f;
		}

		if (unit->team != selectedTeam)
			color = v4desaturatergb(color);

		if (unit->group == selectedGroup)
		{
			nvgBeginPath(context);
			nvgCircle(context, unit->pos.x, unit->pos.y, unit->data->radius + 1.5f);
			nvgFillColor(context, nvgRGBAf(border.x, border.y, border.z, border.w));
			nvgFill(context);
			nvgClosePath(context);
		}

		if (unit->group == hoverGroupIndexFriendly)
		{
			nvgBeginPath(context);
			nvgCircle(context, unit->pos.x, unit->pos.y, unit->data->radius + 1.5f);
			nvgFillColor(context, nvgRGBAf(border.x, border.y, border.z, border.w * 0.25f));
			nvgFill(context);
			nvgClosePath(context);
		}

		if (unit->group == hoverGroupIndexHostile)
		{
			nvgBeginPath(context);
			nvgCircle(context, unit->pos.x, unit->pos.y, unit->data->radius + 1.5f);
			nvgFillColor(context, nvgRGBAf(border.x, 0.1f, 0.1f, border.w * 0.25f));
			nvgFill(context);
			nvgClosePath(context);
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
		nvgStrokeColor(context, nvgRGBAf(0.2f, 0.2f, 0.2f, color.w));
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
		nvgCircle(context, group->groupPos.x, group->groupPos.y, 10.0f);
		nvgFillColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.25f));
		nvgFill(context);
		nvgClosePath(context);

		switch (group->commandType)
		{
		case Group::CommandType_Move:
			nvgBeginPath(context);
			nvgMoveTo(context, group->groupPos.x, group->groupPos.y);
			nvgLineTo(context, group->commandPos.x, group->commandPos.y);
			nvgStrokeWidth(context, 4.0f);
			nvgStrokeColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
			nvgStroke(context);
			nvgClosePath(context);
			break;

		case Group::CommandType_Attack:
			nvgBeginPath(context);
			nvgMoveTo(context, group->groupPos.x, group->groupPos.y);
			nvgLineTo(context, group->commandPos.x, group->commandPos.y);
			nvgStrokeWidth(context, 4.0f);
			nvgStrokeColor(context, nvgRGBAf(1.0f, 0.2f, 0.2f, 0.7f));
			nvgStroke(context);
			nvgClosePath(context);
			break;
		}

		nvgBeginPath(context);
		nvgMoveTo(context, group->groupPos.x, group->groupPos.y);
		nvgLineTo(context, group->groupPos.x + group->displacementAggregate.x, group->groupPos.y + group->displacementAggregate.y);
		nvgStrokeWidth(context, 4.0f);
		nvgStrokeColor(context, nvgRGBAf(1.0f, 0.0f, 0.0f, 0.5f));
		nvgStroke(context);
		nvgClosePath(context);
	}
	
	v4 cursorDecorationColor = v4new(1, 1, 1, 1);
	switch (cursorState)
	{
	case CursorState_None:
		break;

	case CursorState_MoveCommand: cursorDecorationColor = v4new(0.5f, 0.5f, 0.5f, 1.0f); break;
	case CursorState_HoverFriendly: cursorDecorationColor = v4new(0.5f, 1.0f, 0.5f, 1.0f); break;
	case CursorState_HoverHostile: cursorDecorationColor = v4new(1.0f, 0.5f, 0.5f, 1.0f); break;
	case CursorState_DragSelect: cursorDecorationColor = v4new(0.5f, 0.5f, 0.5f, 1.0f); break;
	case CursorState_Pan: cursorDecorationColor = v4new(0.5f, 0.5f, 0.5f, 0.0f); break;
	}

	nvgBeginPath(context);
	nvgCircle(context, cursorPos.x, cursorPos.y, CURSOR_DECORATION_RADIUS);
	nvgStrokeWidth(context, 2.0f);
	nvgStrokeColor(context, nvgRGBAf(cursorDecorationColor.x, cursorDecorationColor.y, cursorDecorationColor.z, cursorDecorationColor.w));
	nvgStroke(context);
	nvgClosePath(context);

	if (cursorState == CursorState_MoveCommand)
	{
		Group* group = GetGroup(selectedGroup);

		nvgBeginPath(context);
		nvgMoveTo(context, cursorAnchor.x, cursorAnchor.y);
		nvgLineTo(context, cursorPos.x, cursorPos.y);
		nvgStrokeWidth(context, 4.0f);
		nvgStrokeColor(context, nvgRGBAf(1.0f, 1.0f, 0.0f, 0.5f));
		nvgStroke(context);
		nvgClosePath(context);

		v2 ofs = cursorPos - cursorAnchor;
		float len = v2lensafe(ofs);
		float groupAngle = group->commandAngle;
		if (len > GROUP_MOVE_COMMAND_ROTATE_MIN)
			groupAngle = v2toangle(ofs) + HALFPI;

		int aliveUnitCount = group->CalcUnitAliveCount();
		float largestUnitRadius = group->CalcUnitLargestRadius();
		int aliveUnitCursor = 0;

		for (int i = 0; i < stb_arr_len(group->members); ++i)
		{
			UnitIndex unitIndex = group->members[i];
			Unit* unit = GetUnit(unitIndex);

			if (!unit->IsValid())
				continue;
			if (!unit->IsAlive())
				continue;

			v2 pos = v2zero();
			float angle = 0.0f;

			switch (group->formationType)
			{
			case Group::FormationType_Box:
				pos = group->FormationPositionBox(aliveUnitCursor, cursorAnchor, groupAngle, aliveUnitCount, largestUnitRadius, group->formationRatio, group->formationLoose);
				angle = group->FormationAngleBox(aliveUnitCursor, cursorAnchor, groupAngle, aliveUnitCount, largestUnitRadius, group->formationRatio, group->formationLoose);
				break;
			}

			nvgBeginPath(context);
			nvgCircle(context, pos.x, pos.y, largestUnitRadius);
			nvgFillColor(context, nvgRGBAf(i * 0.05f, 1.0f, 1.0f, 0.25f));
			nvgFill(context);
			nvgClosePath(context);

			aliveUnitCursor++;
		}
	}

	if (cursorState == CursorState_DragSelect)
	{
		v2 boxA = cursorPos;
		v2 boxB = cursorAnchor;

		nvgBeginPath(context);
		nvgMoveTo(context, boxA.x, boxA.y);
		nvgLineTo(context, boxB.x, boxA.y);
		nvgLineTo(context, boxB.x, boxB.y);
		nvgLineTo(context, boxA.x, boxB.y);
		nvgLineTo(context, boxA.x, boxA.y);
		nvgStrokeWidth(context, 2.0f);
		nvgStrokeColor(context, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
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
	static bool openTeamWindow = false;
	static bool openHoverWindow = false;

	ImGui::Begin("Game", &openGameWindow);

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

	ImGui::PushID("alive");
	if (ImGui::TreeNode(units, "All Units (Alive)"))
	{
		for (int i = 0; i < stb_arr_len(units); ++i)
		{
			Unit* unit = &units[i];
			UnitIndex unitIndex = GetUnitIndex(unit);

			if (!unit->IsValid())
				continue;
			if (!unit->IsAlive())
				continue;

			if (ImGui::TreeNode(unit, "%s(%d:%d:%d)", unit->data->type, unit->team, unit->group, unitIndex))
			{
				RenderImGuiUnit(i);
				ImGui::TreePop();
			}
		}
		
		ImGui::TreePop();
	}
	ImGui::PopID();

	ImGui::PushID("dead");
	if (ImGui::TreeNode(units, "All Units (Dead)"))
	{
		for (int i = 0; i < stb_arr_len(units); ++i)
		{
			Unit* unit = &units[i];
			UnitIndex unitIndex = GetUnitIndex(unit);

			if (!unit->IsValid())
				continue;
			if (unit->IsAlive())
				continue;

			if (ImGui::TreeNode(unit, "%s(%d:%d:%d)", unit->data->type, unit->team, unit->group, unitIndex))
			{
				RenderImGuiUnit(i);
				ImGui::TreePop();
			}
		}
		
		ImGui::TreePop();
	}
	ImGui::PopID();


	ImGui::End();

	grid->RenderImGui();

	//ImGui::ShowMetricsWindow(&openMetricsWindow);

	if (ImGui::Begin("Team", &openTeamWindow))
	{
		ImGui::Text("Team");
		ImGui::SameLine();
		ImGui::RadioButton("0", &selectedTeam, 0);
		ImGui::SameLine();
		ImGui::RadioButton("1", &selectedTeam, 1);
		ImGui::Separator();

		ImGui::Text("Group");
		ImGui::SameLine();

		int groupSelectionOffset = 0;
		if (selectedTeam == 1)
			groupSelectionOffset = stb_arr_len(GetTeam(0)->groups);

		for (int i = groupSelectionOffset; i < stb_arr_len(groups); ++i)
		{
			Group* group = groups + i;
			if (group->team != selectedTeam)
				continue;

			char txt[8] = { 0 };
			sprintf(txt, "%d", i - groupSelectionOffset);
			ImGui::PushID(i);
			ImGui::RadioButton(txt, &selectedGroup, i);
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::NewLine();
		ImGui::Separator();

		RenderImGuiGroup(selectedGroup);
		ImGui::Indent();
	}

	ImGui::End();

	if (ImGui::Begin("Hover", &openHoverWindow))
	{
		if (hoverUnitFriendly != InvalidUnitIndex)
			RenderImGuiUnit(hoverUnitFriendly);
	}

	ImGui::End();
}

void Game::RenderImGuiGroup(GroupIndex groupIndex)
{
	Group* group = GetGroup(groupIndex);

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
			ImGui::SliderFloat("Angle", &group->commandAngle, 0.0f, TWOPI);			
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
		group->CommandMoveToInstant(group->commandPos, group->commandAngle);
	if (ImGui::Button("Teleport Debug") || ImGui::IsKeyPressed(SDLK_y))
		group->CommandMoveToInstant(v2new(450.0f, 400.0f), 0.0f);

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

