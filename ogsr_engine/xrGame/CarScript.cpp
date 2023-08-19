#include "stdafx.h"
#include "alife_space.h"
#include "Car.h"
#include "CarWeapon.h"
#include "script_game_object.h"
#include "xrServer_Objects_ALife_Monsters.h"

using namespace luabind;

bool attachNPC(CCar* car, CScriptGameObject* npc, bool driver = false)
{
    return car->attach_NPC_Vehicle(&npc->object(), driver);
}

void detachNPC(CCar* car, CScriptGameObject* npc)
{
    return car->detach_NPC_Vehicle(&npc->object());
}

luabind::object getPassengers(CCar* car)
{
    luabind::object t = luabind::newtable(ai().script_engine().lua());

    u8 i = 0;
    for (const auto [npc, place] : *car->passengers->getOccupiedPlaces())
        t[++i] = npc->lua_game_object();

    return t;
}

#pragma optimize("s", on)
void CCar::script_register(lua_State* L)
{
    module(L)[class_<CCar, bases<CGameObject, CHolderCustom>>("CCar")
                  .enum_("wpn_action")[value("eWpnDesiredDir", int(CCarWeapon::eWpnDesiredDir)), value("eWpnDesiredPos", int(CCarWeapon::eWpnDesiredPos)),
                                       value("eWpnActivate", int(CCarWeapon::eWpnActivate)), value("eWpnFire", int(CCarWeapon::eWpnFire)),
                                       value("eWpnAutoFire", int(CCarWeapon::eWpnAutoFire)), value("eWpnToDefaultDir", int(CCarWeapon::eWpnToDefaultDir))]
                  .def("Action", &CCar::Action)
                  //		.def("SetParam",		(void (CCar::*)(int,Fvector2)) &CCar::SetParam)
                  .def("SetParam", (void(CCar::*)(int, Fvector)) & CCar::SetParam)
                  .def("CanHit", &CCar::WpnCanHit)
                  .def("FireDirDiff", &CCar::FireDirDiff)
                  .def("IsObjectVisible", &CCar::isObjectVisible)
                  .def("HasWeapon", &CCar::HasWeapon)
                  .def("CurrentVel", &CCar::CurrentVel)
                  .def("GetfHealth", &CCar::GetfHealth)
                  .def("SetfHealth", &CCar::SetfHealth)
                  .def("SetExplodeTime", &CCar::SetExplodeTime)
                  .def("ExplodeTime", &CCar::ExplodeTime)
                  .def("CarExplode", &CCar::CarExplode)

                  .def("GetFuelTank", &CCar::GetFuelTank)
                  .def("GetFuel", &CCar::GetFuel)
                  .def("SetFuel", &CCar::SetFuel)
                  .def("IsLightsOn", &CCar::IsLightsOn)
                  .def("IsEngineOn", &CCar::IsEngineOn)
                  .def("SwitchEngine", &CCar::SwitchEngine)
                  .def("SwitchLights", &CCar::SwitchLights)
                  .def("attach", &attachNPC)
                  .def("detach", &detachNPC)
                  .def_readonly("handbrake", &CCar::brp)
                  .def("set_actor_as_passenger", &CCar::setActorAsPassenger)
                  .def("actor_inside", &CCar::ActorInside)
                  .def("get_owner", [](CCar* car) {return car->Owner() ? car->Owner()->lua_game_object() : nullptr; })
                  .def("get_passengers", &getPassengers)
                  .def("get_passengers_count", [](CCar* car) { return car->passengers->countPlaces(); })
                  .def("get_passengers_vacant_sits", [](CCar* car) { return car->passengers->vacantSits(); })
                  .def(constructor<>())];
}