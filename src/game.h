
#include "inc.h"

struct Game
{
	void Init(void* awindow);
	void Release();

	void Update();
	void Render();

	Unit* GetUnit(UnitID id);
	UnitID GetUnitID(Unit* unit);

	Grid* GetGrid();

	void* window;

	Unit* OWNER units;
	Grid* OWNER grid;
};