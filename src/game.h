
#include "inc.h"

struct Game
{
	void Init(void* awindow);
	void Release();

	void Update();
	void Render();
	void RenderImGui();
	void RenderImGuiTeam(TeamIndex teamIndex);
	void RenderImGuiGroup(GroupIndex groupIndex);
	void RenderImGuiUnit(UnitIndex unitIndex);

	Unit* GetUnit(UnitIndex id);
	UnitIndex GetUnitIndex(Unit* unit);

	Grid* GetGrid();

	Group* GetGroup(int index);

	void* window;

	bool paused;

	Team* OWNER ARRAY teams;
	Group* OWNER ARRAY groups;
	Unit* OWNER ARRAY units;
	Grid* OWNER grid;
	Touch* OWNER touch;

	int selectedTeam;
	int selectedGroup;
};