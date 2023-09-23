#pragma once
#include "ai\stalker\ai_stalker.h"
#include "CarPassengers.h"

class CCar;
class CInifile;

class CarWpnSeat
{
public:
	CarWpnSeat(CCar* objCar, IKinematics* K);
	void onSeat(CGameObject* npc);

private:
	bool exist = false;
	CCar* car;
	u16 wpnBone;
	Fmatrix mtrxSeat;
	Fvector3 posSeat;
	CarPassengers::Place place;
	u16 shownBone;
	u16 hiddenBone;
	IKinematics* carKi;
};
