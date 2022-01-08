#include "stdafx.h"
#include "UIMainIngameWnd.h"
#include "UICarPanel.h"
#include "UIXmlInit.h"

constexpr LPCSTR CAR_PANEL_XML = "car_panel.xml";
constexpr LPCSTR POINTER_ARROW_TEX = "ui\\hud_map_arrow";

void CUICarPanel::Init			(float x, float y, float width, float height)
{
	CUIXml uiXml;
	bool result = uiXml.Init(CONFIG_PATH, UI_PATH, CAR_PANEL_XML);
	R_ASSERT3(result, "xml file not found", CAR_PANEL_XML);

	CUIXmlInit	xml_init;
	////////////////////////////////////////////////////////////////////
	AttachChild(&UIStaticCarHealth);
	xml_init.InitStatic(uiXml, "car_health_static", 0, &UIStaticCarHealth);

	UIStaticCarHealth.AttachChild(&UICarHealthBar);
	xml_init.InitAutoStaticGroup(uiXml, "car_health_static", 0, &UIStaticCarHealth);
	xml_init.InitProgressBar(uiXml, "car_health_progress_bar", 0, &UICarHealthBar);

	AttachChild(&UISpeedometer);
	xml_init.InitStatic(uiXml, "speedometer", 0, &UISpeedometer);

	UISpeedometer.AttachChild(&UIArrowSpeed);
	xml_init.InitStatic(uiXml, "speedometer:arrow_speed", 0, &UIArrowSpeed);
	//UISpeedometer.InitPointer(POINTER_ARROW_TEX, 0, 0, M_PI*1.f/3.f, -M_PI*1.f/3.f);

	AttachChild(&UITachometer);
	xml_init.InitStatic(uiXml, "tachometer", 0, &UITachometer);

	UITachometer.AttachChild(&UIArrowRPM);
	xml_init.InitStatic(uiXml, "tachometer:arrow_rpm", 0, &UIArrowRPM);
	//UITachometer.InitPointer(POINTER_ARROW_TEX,  0, 0, M_PI*1.f/3.f, -M_PI*1.f/3.f);

	xml_init.InitStatic(uiXml, "gear", 0, &UIStaticGear);
	AttachChild(&UIStaticGear);

	angleSpeed = deg2rad(uiXml.ReadAttribFlt("speedometer", 0, "angle", 90.f));
	angleRPM = deg2rad(uiXml.ReadAttribFlt("tachometer", 0, "angle", 90.f));
	max_speed = uiXml.ReadAttribFlt("speedometer", 0, "max_speed", 100.f) * 0.277;
	rev_max_speed = 1 / max_speed;

	Show(false);
	Enable(false);

	inherited::Init(x,y, width, height);
}

//////////////////////////////////////////////////////////////////////////

void CUICarPanel::SetCarHealth(float value)
{
	float pos = value*100;
	clamp(pos, 0.0f, 100.0f);
	UICarHealthBar.SetProgressPos(pos);
}

//////////////////////////////////////////////////////////////////////////

void CUICarPanel::SetSpeed(float speed)
{
	clamp(speed, 0.f, max_speed);
	UIArrowSpeed.SetHeading(-(speed * rev_max_speed * angleSpeed));
}

//////////////////////////////////////////////////////////////////////////

void CUICarPanel::SetRPM(float rpm)
{
	//clamp(rpm, 0.f, angleRPM);
	UIArrowRPM.SetHeading(-(rpm * angleRPM));
}

void CUICarPanel::SetCarGear(u8 gear)
{
	SetCarGear(std::to_string(gear).c_str());
}

void CUICarPanel::SetCarGear(LPCSTR gear)
{
	UIStaticGear.SetText(gear);
}