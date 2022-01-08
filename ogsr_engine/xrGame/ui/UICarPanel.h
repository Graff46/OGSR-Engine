#pragma once

#include "uiwindow.h"
#include <ui/UIProgressBar.h>
//#include "uipointergage.h"


class CUICarPanel : public CUIWindow
{
private:
	typedef CUIWindow inherited;

	CUIStatic			UIStaticCarHealth;
	CUIProgressBar		UICarHealthBar;
	CUIStatic			UISpeedometer;
	CUIStatic			UITachometer;
	CUIStatic			UIArrowSpeed;
	CUIStatic			UIArrowRPM;
	CUIStatic			UIStaticGear;
	float				angleSpeed;
	float				angleRPM;
	float				max_speed;
	float				rev_max_speed;
public: 

	// Установить 
	void				SetCarGear		(u8 gear);
	void				SetCarGear		(LPCSTR gear);
	void				SetCarHealth	(float value);
	void				SetSpeed		(float speed);
	void				SetRPM			(float rmp);
	void				Init			(float x, float y, float width, float height);
};