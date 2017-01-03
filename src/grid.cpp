
#include "inc.h"

void Cell::Init()
{
	unitIDs = NULL;
	stb_arr_setsize(unitIDs, 16);
}

void Cell::Release()
{
	stb_arr_free(unitIDs);
	unitIDs = NULL;
}

void Grid::Init(Game* agame, v2i adim, v2 abl, v2 atr)
{
	game = agame;
	dim = adim;
	bl = abl;
	tr = atr;

	cells = NULL;
	stb_arr_setlen(cells, dim.x * dim.y);
	for (int i = 0; i < stb_arr_len(cells); ++i)
		cells[i].Init();
}

void Grid::Release()
{
	stb_arr_free(cells);
	cells = NULL;
}

int Grid::Query(UnitID* ARRAY OWNER results, v2 qbl, v2 qtr)
{
	int found = 0;

	v2 span = tr - bl;
	v2 cellSize = v2new(span.x / dim.x, span.y / dim.y);

	for (int i = 0; i < stb_arr_len(cells); ++i)
	{
		Cell* cell = &cells[i];
		UnitID* ARRAY cellUnitIDs = cell->unitIDs;

		float row = (float)(i / dim.x);
		float col = (float)(i % dim.x);
		
		v2 cellbl = bl + v2mulv(cellSize, v2new(row, col));
		v2 celltr = tr + v2mulv(cellSize, v2new(row, col));

		if (!rectoverlap(qbl, qtr, cellbl, celltr))
			continue;

		for (int j = 0; j < stb_arr_len(cellUnitIDs); ++j)
		{
			UnitID unitID = cellUnitIDs[j];
			Unit* unit = game->GetUnit(unitID);
			v2 pos = v3xy(unit->pos);
			float rad = unit->data->radius;
			v2 unitbl = v2sub(pos, v2new(rad, rad));
			v2 unittr = v2add(pos, v2new(rad, rad));

			if (!rectoverlap(qbl, qtr, unitbl, unittr))
				continue;

			stb_arr_push(results, unitID);
			found++;
		}
	}

	return found;
}

Cell* Grid::GetCell(v2i loc)
{
	return GetCellIndexed(loc.x + loc.y * dim.x);
}

Cell* Grid::GetCellIndexed(int index)
{
	assert(index < stb_arr_len(cells));
	return cells + index;
}

v2i Grid::GetGridCoord(v2 pos)
{
	pos = pos - bl;

	v2 span = tr - bl;
	v2 cell = v2new(span.x / dim.x, span.y / dim.y);

	v2i coord = v2inew((int)cell.x, (int)cell.y);

	return coord;
}

