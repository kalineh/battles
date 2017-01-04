
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
	void Init(Game* agame, v2i adim, v2 abl, v2 atr);
	void Release();

	void Fill(Unit* ARRAY units);
	void RenderImGui();

	int Query(UnitID* ARRAY OWNER results, v2 bl, v2 tr);

	Cell* GetCell(v2i coord);
	Cell* GetCellIndexed(int index);
	v2i GetGridCoord(v2 pos);

	Game* game;
	Cell* OWNER ARRAY cells;

	v2i dim;
	v2 bl;
	v2 tr;
};
