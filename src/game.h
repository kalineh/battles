
#include "inc.h"

struct Game
{
	void Init(void* awindow);
	void Release();

	void Update();
	void Render();
	void RenderImGui();

	Unit* GetUnit(UnitIndex id);
	UnitIndex GetUnitIndex(Unit* unit);

	Grid* GetGrid();

	Group* GetGroup(int index);

	void* window;

	bool paused;

	Unit* OWNER ARRAY units;
	Grid* OWNER grid;
	Touch* OWNER touch;
	Group* OWNER ARRAY groups;

	int selectedTeam;
	int selectedGroup;
};