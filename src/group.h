
#include "inc.h"

struct Group
{
	enum FormationType
	{
		FormationType_None,
		FormationType_Box,
		FormationType_Wedge,
	};

	void Init(Unit* ARRAY aunits);
	void Release();

	void Update();

	void AddUnit(UnitID id);
	void RemoveUnit(UnitID id);

	void CommandStop();
	void CommandMoveTo(v2 pos, float angle);

	void CommandFormationNone();
	void CommandFormationBox(float ratio, float loose);
	void CommandFormationWedge();

	Unit* ARRAY units;
	UnitID* OWNER ARRAY members;

	FormationType formationType;
	v2 commandPos;
	float commandAngle;
	float boxRatio;
	float boxLoose;
};
