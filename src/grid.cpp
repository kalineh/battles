
#include "inc.h"

void Cell::Init()
{
	unitIDs = NULL;
	stb_arr_setsize(unitIDs, 32);
	stb_arr_setlen(unitIDs, 0);
}

void Cell::Release()
{
	stb_arr_free(unitIDs);
}

void Cell::Clear()
{
	stb_arr_setlen(unitIDs, 0);
}

void Cell::Add(UnitID id)
{
	stb_arr_push(unitIDs, id);
}

void Grid::Init(Unit* ARRAY aunits, v2i adim, v2 alower, v2 aupper)
{
	units = aunits;
	dim = adim;
	lower = alower;
	upper = aupper;

	cells = NULL;
	stb_arr_setlen(cells, dim.x * dim.y);
	for (int i = 0; i < stb_arr_len(cells); ++i)
		cells[i].Init();
}

void Grid::Release()
{
	stb_arr_free(cells);
}

void Grid::Rebuild()
{
	for (int i = 0; i < stb_arr_len(cells); ++i)
	{
		Cell* cell = cells + i;
		cell->Clear();
	}

	v2 span = upper - lower;
	v2 cellSize = v2new(span.x / dim.x, span.y / dim.y);

	for (int i = 0; i < stb_arr_len(units); ++i)
	{
		Unit* unit = units + i;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		UnitID id = (UnitID)i;
		v2 pos = unit->pos;
		v2i coord = GetGridCoord(pos);
		Cell* cell = GetCell(coord);

		cell->Add(id);
	}
}

void Grid::RenderImGui()
{
	static bool openGridWindow = false;
	static bool sparse = true;
	static bool minimap = true;

	ImGui::Begin("Grid", &openGridWindow);

	ImGui::Checkbox("Sparse", &sparse);
	ImGui::Checkbox("Minimap", &minimap);

	if (minimap)
	{
		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

		for (int y = 0; y < dim.y; ++y)
		{
			for (int x = 0; x < dim.x; ++x)
			{
				int index = x + y * dim.x;
				Cell* cell = GetCellIndexed(index);
				int count = stb_arr_len(cell->unitIDs);

				ImGui::Text("%3d", count);

				if (x != dim.x - 1)
					ImGui::SameLine();
			}
		}

		ImGui::PopStyleVar(4);
		ImGui::Separator();
	}

	ImGui::PushID(this);
	if (ImGui::TreeNode(this, "cells"))
	{
		for (int y = 0; y < dim.y; ++y)
		{
			ImGui::PushID(y);
			if (ImGui::TreeNode(this, "row %d", y))
			{
				for (int x = 0; x < dim.x; ++x)
				{
					int index = x + y * dim.x;
					Cell* cell = GetCellIndexed(index);
					if (sparse && stb_arr_len(cell->unitIDs) == 0)
						continue;

					if (ImGui::TreeNode(cell, "col %d (%d)", x, stb_arr_len(cell->unitIDs)))
					{
						for (int i = 0; i < stb_arr_len(cell->unitIDs); ++i)
						{
							UnitID id = cell->unitIDs[i];
							Unit* unit = units + id;

							ImGui::LabelText("unit", "%d (%s)", id, unit->data->type);
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	ImGui::PopID();

	ImGui::End();
}

int Grid::Query(UnitID** ARRAY results, v2 alower, v2 aupper)
{
	int found = 0;

	v2 span = upper - lower;
	v2 cellSize = v2new(span.x / dim.x, span.y / dim.y);

	v2i lowerCoord = GetGridCoord(alower);
	v2i upperCoord = GetGridCoord(aupper);

	for (int y = lowerCoord.y; y <= upperCoord.y; ++y)
	{
		for (int x = lowerCoord.x; x <= upperCoord.x; ++x)
		{
			int cellIndex = x + y * dim.x;
			Cell* cell = GetCellIndexed(cellIndex);

			v2 cellLower = lower + v2mulv(cellSize, v2new((float)(x + 0), (float)(y + 0)));
			v2 cellUpper = lower + v2mulv(cellSize, v2new((float)(x + 1), (float)(y + 1)));

			if (!rectoverlap(alower, aupper, cellLower, cellUpper))
				continue;

			UnitID* ARRAY cellUnitIDs = cell->unitIDs;

			for (int j = 0; j < stb_arr_len(cellUnitIDs); ++j)
			{
				UnitID unitID = cellUnitIDs[j];
				Unit* unit = units + unitID;
				v2 pos = unit->pos;
				float rad = unit->data->radius;
				v2 unitbl = v2sub(pos, v2new(rad, rad));
				v2 unittr = v2add(pos, v2new(rad, rad));

				stb_arr_push(*results, unitID);
				found++;
			}
		}
	}

	return found;
}

Cell* Grid::GetCell(v2i coord)
{
	return GetCellIndexed(coord.x + coord.y * dim.x);
}

Cell* Grid::GetCellIndexed(int index)
{
	assert(index < stb_arr_len(cells));
	return cells + index;
}

v2i Grid::GetGridCoord(v2 pos)
{
	pos = pos - lower;

	v2 span = upper - lower;
	v2 cell = v2new(span.x / dim.x, span.y / dim.y);
	v2i coord = v2inew((int)(pos.x / cell.x), (int)(pos.y / cell.y));

	coord.x = stb_clamp(coord.x, 0, dim.x - 1);
	coord.y = stb_clamp(coord.y, 0, dim.y - 1);

	return coord;
}

