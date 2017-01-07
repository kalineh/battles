
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
	void UpdateFormation();

	void AddUnit(UnitID id);
	void RemoveUnit(UnitID id);

	void CommandStop();
	void CommandMoveTo(v2 pos, float angle);
	void CommandTeleportTo(v2 pos, float angle);

	void CommandFormationNone();
	void CommandFormationBox(float ratio, float loose);
	void CommandFormationWedge();

	int PositionToIndexBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	int PositionToIndexWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	v2 IndexToPositionBox(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	v2 IndexToPositionWedge(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	Unit* ARRAY units;
	UnitID* OWNER ARRAY members;

	FormationType formationType;
	v2 commandPos;
	float commandAngle;
	float formationRatio;
	float formationLoose;
};
