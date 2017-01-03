
#include "inc.h"

struct Cell
{
	void Init();
	void Release();

	UnitID* OWNER ARRAY unitIDs;
};

struct Grid
{
	void Init(Game* agame, v2i adim, v2 abl, v2 atr);
	void Release();

	int Query(UnitID* ARRAY OWNER results, v2 bl, v2 tr);

	Cell* GetCell(v2i loc);
	Cell* GetCellIndexed(int index);
	v2i GetGridCoord(v2 pos);

	Game* game;
	Cell* OWNER ARRAY cells;

	v2i dim;
	v2 bl;
	v2 tr;
};
