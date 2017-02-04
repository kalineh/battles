
#include "inc.h"

struct Game
{
	void Init(void* awindow);
	void Release();

	void Update();
	void UpdateInput();
	void Render();
	void RenderImGui();
	void RenderImGuiTeam(TeamIndex teamIndex);
	void RenderImGuiGroup(GroupIndex groupIndex);
	void RenderImGuiUnit(UnitIndex unitIndex);

	Team* GetTeam(TeamIndex teamIndex);
	Group* GetGroup(GroupIndex groupIndex);
	Unit* GetUnit(UnitIndex unitIndex);

	UnitIndex GetUnitIndex(Unit* unit);

	Grid* GetGrid();

	void* window;

	bool paused;

	Team* OWNER ARRAY teams;
	Group* OWNER ARRAY groups;
	Unit* OWNER ARRAY units;
	Grid* OWNER grid;
	Touch* OWNER touch;

	TeamIndex selectedTeam;
	GroupIndex selectedGroup;
	UnitIndex selectedUnit;

	UnitIndex hoverUnitFriendly;
	UnitIndex hoverUnitHostile;

	enum CursorState
	{
		CursorState_None,
		CursorState_MoveCommand,
		CursorState_HoverFriendly,
		CursorState_HoverHostile,
		CursorState_DragSelect,
		CursorState_Pan,
	};

	CursorState cursorState;
	v2 cursorPos;
	v2 cursorAnchor;
	float commandFormationRatio;
};