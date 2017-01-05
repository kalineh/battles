
#include "inc.h"

struct Cell
{
	void Init();
	void Release();

	void Clear();
	void Add(UnitID id);

	UnitID* OWNER ARRAY unitIDs;
};

struct Grid
{
	void Init(Unit* aunits, v2i adim, v2 alower, v2 aupper);
	void Release();

	void Rebuild();

	void RenderImGui();

	int Query(UnitID** ARRAY results, v2 alower, v2 aupper);

	Cell* GetCell(v2i coord);
	Cell* GetCellIndexed(int index);
	v2i GetGridCoord(v2 pos);

	Unit* ARRAY units;
	Cell* OWNER ARRAY cells;

	v2i dim;
	v2 lower;
	v2 upper;
};
