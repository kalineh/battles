
#include "inc.h"

struct Group
{
	void Init(Unit* ARRAY aunits);
	void Release();

	void AddUnit(UnitID id);
	void RemoveUnit(UnitID id);

	void CommandTo(v2 pos);
	void FormationBox(float ratio, float loose);

	Unit* ARRAY units;
	UnitID* OWNER ARRAY members;

	v2 command;
	float boxRatio;
	float boxLoose;
};
