#pragma once
#include "ai\stalker\ai_stalker.h";

class CCar;

class CarPassengers
{
private:
	CCar* car;
	struct Place
	{
		u8 id;
		Fmatrix xform;
		bool occupied;
		u16 exitDoorId;
	};

	xr_unordered_map<u16, Place> list;
	xr_unordered_map<CGameObject*, Place*> occupiedPlaces;

public:
	CarPassengers(CCar* obj);

	void create(IKinematics* pKinematics);
	const Fmatrix* addPassenger(CGameObject* npc);
	void removePassenger(CGameObject* npc);
	xr_unordered_map<CGameObject*, Place*>* getOccupiedPlaces();
	const u8 vacantSits();
};