
#include "inc.h"

struct Game
{
	void Init();
	void Release();

	void Update();
	void Render();

	Unit* GetUnit(UnitID id);
	UnitID GetUnitID(Unit* unit);

	Grid* GetGrid();

	Unit* OWNER units;
	Grid* OWNER grid;
};