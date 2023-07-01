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

		void setProps(bool valOccupied, u8 doorId = u8(-1)) {
			exitDoorId = doorId;
			occupied = valOccupied;
		};

		Fvector* position() {
			return &xform.c;
		};
	};

	xr_unordered_map<u16, Place> list;
	xr_unordered_map<CAI_Stalker*, Place*> occupiedPlaces;

public:
	CarPassengers(CCar* obj);

	void create(IKinematics* pKinematics);
	const Fmatrix* addPassenger(CAI_Stalker* npc);
	void removePassenger(CAI_Stalker* npc);
	xr_unordered_map<CAI_Stalker*, Place*>* getOccupiedPlaces();
};