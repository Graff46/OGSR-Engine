#include "stdafx.h"
#include "CarWpnSeat.h"
#include "Car.h"

#include <NpcCarStor.h>
#include <Actor.h>
#include "..\xr_3da\XR_IOConsole.h"
#include "../Include/xrRender/Kinematics.h"

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

		if (pUserData->line_exist("car_weapon", "shown_bone"))
		{
			shownBone = carKi->LL_BoneID(pUserData->r_string("car_weapon", "shown_bone"));

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

	if (shownBone)
	{
		carKi->LL_SetBoneVisible(shownBone, TRUE, TRUE);
		carKi->CalculateBones_Invalidate();
		carKi->CalculateBones();
	}

	if (hiddenBone)
		carKi->LL_SetBoneVisible(hiddenBone, FALSE, TRUE);

	mtrxSeat = carKi->LL_GetTransform(wpnBone);
	mtrxSeat.c.add(posSeat);

	if (npc->ID() == Actor()->ID())
	{
		car->OnCameraChange(0);
		car->setCamParam("car_wpn_cam");

		car->m_sits_transforms.set(mtrxSeat);
	}
	else
	{
		car->passengers->removePassenger(npc);
		place = CarPassengers::Place{ u8(-1) - 1, mtrxSeat, true, car->calcDoorForPlace(&mx.c) };
		car->passengers->getOccupiedPlaces()->emplace(npc, &place);

		NpcCarStor::remove(npc->ID());
		NpcCarStor::add(npc->ID(), car->ID(), u8(-1) - 1);
	}
}