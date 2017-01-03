
#include "inc.h"

struct Group
{
	void Init();
	void Release();

	void AddUnit(UnitID id);
	void RemoveUnit(UnitID id);

	void CommandTo(v2 pos);
	void FormationBox(float ratio, float loose);

	UnitID* OWNER ARRAY members;

	v2 command;
	float boxRatio;
	float boxLoose;
};
