#pragma once
#include "ai\stalker\ai_stalker.h"

class CCar;

class CarPassengers
{
public:
	struct Place
	{
		u8 id;
		Fmatrix xform;
		bool occupied;
		u16 exitDoorId;
	};
private:
	CCar* car;

	xr_unordered_map<u8, Place> list;
	xr_unordered_map<CGameObject*, Place*> occupiedPlaces;
	IKinematics* Ki;
public:
	CarPassengers(CCar* obj);

	void create(IKinematics* pKinematics);
	const Fmatrix* addPassenger(CGameObject* npc, u8 seat = u8(-1));
	void removePassenger(CGameObject* npc);
	xr_unordered_map<CGameObject*, Place*>* getOccupiedPlaces();
	xr_unordered_map<CGameObject*, Place*> getOccupiedPlaces2() { return occupiedPlaces; };
	const u8 vacantSits();
    const u8 countPlaces() { return list.size(); };
	const u8 getSeatId(CGameObject* npc);
};