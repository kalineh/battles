
#include "inc.h"

struct Group
{
	enum FormationType
	{
		FormationType_None,
		FormationType_Box,
		FormationType_Wedge,
		FormationType_Circle,
	};

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
	void CommandFormationCircle(float ratio, float loose);

	v2 CalcCentroid();
	float CalcUnitSlowestMovement();
	float CalcUnitLargestRadius();
	int CalcUnitAliveCount();
	float CalcUnitAverageResolve();
	float CalcUnitAverageHealth();

	MemberIndex PositionToMemberIndexBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	MemberIndex PositionToMemberIndexWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	v2 FormationPositionBox(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	v2 FormationPositionWedge(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	v2 FormationPositionCircle(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	MemberIndex FindNearestUnoccupied(MemberIndex queryMemberIndex);
	UnitIndex FindNearestUnit(v2 pos, UnitIndex* ARRAY source, UnitIndex failureIndex);

	Unit* ARRAY units;
	UnitIndex* OWNER ARRAY members;
	UnitIndex* OWNER ARRAY slots;

	FormationType formationType;
	v2 groupPos;
	v2 commandPos;
	v2 displacementAggregate;
	float damageAggregate;
	float disarrayRatio;
	float commandAngle;
	float formationRatio;
	float formationLoose;
};
