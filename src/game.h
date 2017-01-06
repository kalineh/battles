
#include "inc.h"

struct Game
{
	void Init(void* awindow);
	void Release();

	void Update();
	void Render();
	void RenderImGui();

	Unit* GetUnit(UnitID id);
	UnitID GetUnitID(Unit* unit);

	Grid* GetGrid();

	Group* GetGroup(int index);

	void* window;

	Unit* OWNER ARRAY units;
	Grid* OWNER grid;
	Touch* OWNER touch;
	Group* OWNER ARRAY groups;

	int selectedGroup;
};