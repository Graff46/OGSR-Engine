#include "stdafx.h"
#include "CarWpnSeat.h"
#include "Car.h"

#include <NpcCarStor.h>
#include <Actor.h>
#include "..\xr_3da\XR_IOConsole.h"
#include "../Include/xrRender/Kinematics.h"
#include "CarWeapon.h"

CarWpnSeat::CarWpnSeat(CCar* objCar, IKinematics* K)
{
	CInifile* pUserData = K->LL_UserData();

	if (pUserData->section_exist("car_weapon"))
	{
		exist = true;

		car = objCar;
		carKi = K;

		wpnBone = carKi->LL_BoneID(pUserData->r_string("car_weapon", "bone_seat"));

		if (pUserData->line_exist("car_weapon", "pos_offset"))
			posSeat = pUserData->r_fvector3("car_weapon", "pos_offset");

		if (pUserData->line_exist("car_weapon", "hidden_bone"))
			hiddenBone = carKi->LL_BoneID(pUserData->r_string("car_weapon", "hidden_bone"));

		if (pUserData->line_exist("car_weapon", "show_bone"))
		{
			shownBone = carKi->LL_BoneID(pUserData->r_string("car_weapon", "show_bone"));

			carKi->LL_SetBoneVisible(shownBone, FALSE, TRUE);
		}
	}
}

void CarWpnSeat::onSeat(CGameObject* npc)
{
	if (!exist) 
	{
		Console->Show();
		Msg("! This car [%s] not have wpn!", car->Name());
		return;
	}

	if (!car->passengers->getOccupiedPlaces()->contains(npc))
        return;

	car->Action(CCarWeapon::eWpnActivate, 1);

	if (shownBone)
	{
		carKi->LL_SetBoneVisible(shownBone, TRUE, TRUE);
		carKi->CalculateBones_Invalidate();
		carKi->CalculateBones();
	}

	if (hiddenBone)
		carKi->LL_SetBoneVisible(hiddenBone, FALSE, TRUE);

	mtrxSeat.set(carKi->LL_GetTransform(wpnBone));
	//mtrxSeat.c.add(posSeat);

	npc->XFORM().mul_43(car->XFORM(), mtrxSeat);
    npc->setVisible(TRUE);

	afterSeatId = car->passengers->getSeatId(npc);
	owner = npc;

	car->passengers->removePassenger(npc);

    if (actorOwner = npc->ID() == Actor()->ID())
    {
        cameraBone = carKi->LL_BoneID("wpn_cam");

        car->OnCameraChange(0);
        car->setCamParam("car_wpn_cam");
        car->cam_Update(Device.fTimeDelta, Actor()->currentFOV());
    }

	place = CarPassengers::Place{ u8(-1) - 1, mtrxSeat, true, car->calcDoorForPlace(&mtrxSeat.c) };
	car->passengers->getOccupiedPlaces()->emplace(npc, &place);

	NpcCarStor::remove(npc->ID());
	NpcCarStor::add(npc->ID(), car->ID(), u8(-1) - 1);
}

void CarWpnSeat::leaveSeat(u8 seatId)
{
	if (!(exist && owner)) return;


	if (seatId == u8(-1))
		seatId = afterSeatId;

	car->Action(CCarWeapon::eWpnActivate, 0);

	car->passengers->addPassenger(owner, seatId);

	if (actorOwner)
	{
        CInifile* pUserData = carKi->LL_UserData();

        car->setCamParam("car_firsteye_cam");

		u16 id;
		if (pUserData->line_exist("car_definition", "driver_place"))
			id = carKi->LL_BoneID(pUserData->r_string("car_definition", "driver_place"));
		else
			id = carKi->LL_GetBoneRoot();

		car->m_sits_transforms.set(carKi->LL_GetTransform(id));
	}

	actorOwner = false;
	owner = nullptr;

	if (hiddenBone)
	{
		carKi->LL_SetBoneVisible(hiddenBone, TRUE, TRUE);
		carKi->CalculateBones_Invalidate();
		carKi->CalculateBones();
	}

	if (shownBone)
		carKi->LL_SetBoneVisible(shownBone, FALSE, TRUE);
}

ALife::_OBJECT_ID CarWpnSeat::getOwnerID()
{
	return owner ? owner->ID() : ALife::_OBJECT_ID(-1);
}

Fvector3 CarWpnSeat::getCameraOffset() 
{ 
	return carKi->LL_GetTransform(cameraBone).c;
}