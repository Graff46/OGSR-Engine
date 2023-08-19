#include "stdafx.h"

#include "actor.h"
#include "../xr_3da/camerabase.h"

#include "ActorEffector.h"
#include "holder_custom.h"
#ifdef DEBUG
#include "PHDebug.h"
#endif
#include "alife_space.h"
#include "hit.h"
#include "PHDestroyable.h"
#include "Car.h"
#include "..\Include/xrRender/Kinematics.h"
#include "..\Include/xrRender/KinematicsAnimated.h"
#include "PHShellSplitter.h"

#include "actor_anim_defs.h"
#include "game_object_space.h"
#include "characterphysicssupport.h"
#include "inventory.h"

#include "script_callback_ex.h"
#include "script_game_object.h"

void CActor::attach_Vehicle(CHolderCustom* vehicle)
{
    if (!vehicle)
        return;

    if (m_holder)
        return;

    CCar* car = smart_cast<CCar*>(vehicle);

    if (car)
    {
        if (((car) && (car->Owner()) && (car->passengers->vacantSits() > 0)) || (car->getActorAsPassenger()))
            isPassenger = true;

        if (((!isPassenger) && (vehicle->Owner())) || ((isPassenger) && (car->passengers->vacantSits() < 1)))
            return;
    }

    PickupModeOff();
    m_holder = vehicle;// isPassenger ? nullptr : vehicle;

    IKinematicsAnimated* V = smart_cast<IKinematicsAnimated*>(Visual());
    R_ASSERT(V);

    if (!vehicle->attach_Actor(this, isPassenger))
    {
        m_holder = NULL;
        return;
    }
    // temp play animation
    
    u16 anim_type = car->DriverAnimationType();
    SVehicleAnimCollection& anims = m_vehicle_anims->m_vehicles_type_collections[anim_type];
    V->PlayCycle(anims.idles[0], FALSE);

    ResetCallbacks();
    u16 head_bone = V->dcast_PKinematics()->LL_BoneID("bip01_head");
    V->dcast_PKinematics()->LL_GetBoneInstance(u16(head_bone)).set_callback(bctPhysics, VehicleHeadCallback, this);

    character_physics_support()->movement()->DestroyCharacter();
    mstate_wishful = 0;
    
    //if (isPassenger)
    m_holderID = car->ID();

    SetWeaponHideState(INV_STATE_CAR, true);

    CStepManager::on_animation_start(MotionID(), 0);

    this->callback(GameObject::eAttachVehicle)(car->lua_game_object(), this->lua_game_object());
}

void CActor::detach_Vehicle()
{
    if (!m_holder)
        return;

    CCar* car = smart_cast<CCar*>(m_holder);

    if (!car)
        return;

    isPassenger = false;

    CPHShellSplitterHolder* sh = car->PPhysicsShell()->SplitterHolder();

    if (sh)
        sh->Deactivate();

    if (!character_physics_support()->movement()->ActivateBoxDynamic(0))
    {
        if (sh)
            sh->Activate();
        return;
    }
    if (sh)
        sh->Activate();

    m_holder->detach_Actor(); //

    Fvector exitPosition = m_holder->ExitPosition();
    Fvector exitVelocity = m_holder->ExitVelocity();

    CGameObject* obj = smart_cast<CGameObject*>(this);
    if (car->passengers->getOccupiedPlaces()->contains(obj))
    {
        exitPosition.set(car->calcExitPosition(&Position()));
        exitVelocity.set( car->ExitVelocity(exitPosition) );

        car->passengers->removePassenger(obj);
    }

    character_physics_support()->movement()->SetPosition(exitPosition);
    character_physics_support()->movement()->SetVelocity(exitVelocity);

    r_model_yaw = -m_holder->Camera()->yaw;
    r_torso.yaw = r_model_yaw;
    r_model_yaw_dest = r_model_yaw;
    m_holder = NULL;
    SetCallbacks();
    IKinematicsAnimated* V = smart_cast<IKinematicsAnimated*>(Visual());
    R_ASSERT(V);
    V->PlayCycle(m_anims->m_normal.legs_idle);
    V->PlayCycle(m_anims->m_normal.m_torso_idle);
    m_holderID = u16(-1);

    //.	SetWeaponHideState(whs_CAR, FALSE);
    SetWeaponHideState(INV_STATE_CAR, false);

    this->callback(GameObject::eDetachVehicle)(car->lua_game_object(), this->lua_game_object());
}

bool CActor::use_Vehicle(CHolderCustom* object)
{
    //	CHolderCustom* vehicle=smart_cast<CHolderCustom*>(object);
    CHolderCustom* vehicle = object;
    Fvector center;
    Center(center);
    if (m_holder)
    {
        if (!vehicle && m_holder->Use(Device.vCameraPosition, Device.vCameraDirection, center))
            detach_Vehicle();
        else
        {
            if (m_holder == vehicle)
                if (m_holder->Use(Device.vCameraPosition, Device.vCameraDirection, center))
                    detach_Vehicle();
        }
        return true;
    }
    else
    {
        if (vehicle)
        {
            if (vehicle->Use(Device.vCameraPosition, Device.vCameraDirection, center))
            {
                if (pCamBobbing)
                {
                    Cameras().RemoveCamEffector(eCEBobbing);
                    pCamBobbing = NULL;
                }

                attach_Vehicle(vehicle);
            }

            else if (auto car = smart_cast<CCar*>(vehicle))
            {
                this->callback(GameObject::eUseVehicle)(car->lua_game_object());
            }

            return true;
        }
        return false;
    }
}

void CActor::on_requested_spawn(CObject* object)
{
    /*CCar* car = smart_cast<CCar*>(object);
    car->PPhysicsShell()->DisableCollision();
    attach_Vehicle(car);
    car->PPhysicsShell()->EnableCollision();*/
}