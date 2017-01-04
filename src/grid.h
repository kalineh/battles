
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
	void Init(Unit* aunits, v2i adim, v2 abl, v2 atr);
	void Release();

	void Rebuild();

	void RenderImGui();

	int Query(UnitID* ARRAY OWNER results, v2 bl, v2 tr);

	Cell* GetCell(v2i coord);
	Cell* GetCellIndexed(int index);
	v2i GetGridCoord(v2 pos);

	Unit* ARRAY units;
	Cell* OWNER ARRAY cells;

	v2i dim;
	v2 bl;
	v2 tr;
};
