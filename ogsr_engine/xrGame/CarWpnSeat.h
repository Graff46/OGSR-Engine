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
	void leaveSeat(u8 seatId = u8(-1));
	ALife::_OBJECT_ID getOwnerID();
	inline bool ownerExist() { return !!owner; };
    Fvector3 getCameraOffset();

private:
	CCar* car;
	u16 wpnBone;
    u16 cameraBone;
	Fmatrix mtrxSeat;
	Fvector3 posSeat;
	CarPassengers::Place place;
	u16 shownBone;
	u16 hiddenBone;
	IKinematics* carKi;
	u8 afterSeatId;
	CGameObject* owner = nullptr;

public:
	bool exist = false;
	bool actorOwner = false;
};
