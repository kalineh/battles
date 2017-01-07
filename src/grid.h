
#include "inc.h"

struct Cell
{
	void Init();
	void Release();

	void Clear();
	void Add(UnitIndex id);

	UnitIndex* OWNER ARRAY unitIndexes;
};

struct Grid
{
	void Init(Unit* aunits, v2i adim, v2 alower, v2 aupper);
	void Release();

	void Rebuild();

	void RenderImGui();

	int Query(UnitIndex** ARRAY results, v2 alower, v2 aupper, Unit* ignore = NULL);

	Cell* GetCell(v2i coord);
	Cell* GetCellIndexed(int index);
	v2i GetGridCoord(v2 pos);

	Unit* ARRAY units;
	Cell* OWNER ARRAY cells;

	v2i dim;
	v2 lower;
	v2 upper;
};
