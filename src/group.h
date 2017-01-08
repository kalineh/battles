
#include "inc.h"

struct Group
{
	enum FormationType
	{
		FormationType_None,
		FormationType_Box,
		FormationType_Wedge,
	};

	typedef int MemberIndex;

	void Init(Unit* ARRAY aunits);
	void Release();

	void Update();
	void UpdateFormation();

	void AddUnit(UnitIndex index);
	void RemoveUnit(UnitIndex index);

	void CommandStop();
	void CommandMoveTo(v2 pos, float angle);
	void CommandTeleportTo(v2 pos, float angle);

	void CommandFormationNone();
	void CommandFormationBox(float ratio, float loose);
	void CommandFormationWedge();

	float CalcUnitLargestRadius();
	int CalcUnitAliveCount();

	MemberIndex PositionToMemberIndexBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	MemberIndex PositionToMemberIndexWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	v2 MemberIndexToPositionBox(MemberIndex memberIndex, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	v2 MemberIndexToPositionWedge(MemberIndex memberIndex, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	MemberIndex FindNearestUnoccupied(MemberIndex queryMemberIndex);

	Unit* ARRAY units;
	UnitIndex* OWNER ARRAY members;

	FormationType formationType;
	v2 commandPos;
	float commandAngle;
	float formationRatio;
	float formationLoose;
};
