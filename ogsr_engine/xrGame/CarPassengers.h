#pragma once
#include "Car.h"

class CarPassengers
{
public: 
	void create(IKinematics* pKinematics);
	bool addPassenger(CGameObject* npc);
	void removePassenger(CGameObject* npc);
	std::map<CGameObject*, const Fmatrix*> getOccupiedPlaces();

private:
	std::list<Fmatrix> list;
	std::map<CGameObject*, const Fmatrix*> occupiedPlaces;
};

