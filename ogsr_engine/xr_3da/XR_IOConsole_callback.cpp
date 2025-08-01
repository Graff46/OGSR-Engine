////////////////////////////////////////////////////////////////////////////
// Module : XR_IOConsole_callback.cpp
// Created : 17.05.2008
// Author : Evgeniy Sokolov
// Description : Console`s callback functions class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include "igame_persistent.h"
#include "XR_IOConsole.h"

#include "line_editor.h"
#include "xr_input.h"
#include "xr_ioc_cmd.h"

#include "Render.h"

void CConsole::Register_callbacks()
{
	ec().assign_callback(DIK_PRIOR, text_editor::ks_free, Callback(this, &CConsole::Prev_log));
	ec().assign_callback(DIK_NEXT, text_editor::ks_free, Callback(this, &CConsole::Next_log));
	ec().assign_callback(DIK_PRIOR, text_editor::ks_Ctrl, Callback(this, &CConsole::Begin_log));
	ec().assign_callback(DIK_NEXT, text_editor::ks_Ctrl, Callback(this, &CConsole::End_log));

	ec().assign_callback(DIK_TAB, text_editor::ks_free, Callback(this, &CConsole::Find_cmd));
	ec().assign_callback(DIK_TAB, text_editor::ks_Shift, Callback(this, &CConsole::Find_cmd_back));
	ec().assign_callback(DIK_TAB, text_editor::ks_Alt, Callback(this, &CConsole::GamePause));

	ec().assign_callback(DIK_UP, text_editor::ks_free, Callback(this, &CConsole::Prev_tip));
	ec().assign_callback(DIK_DOWN, text_editor::ks_free, Callback(this, &CConsole::Next_tip));
	ec().assign_callback(DIK_UP, text_editor::ks_Ctrl, Callback(this, &CConsole::Prev_cmd));
	ec().assign_callback(DIK_DOWN, text_editor::ks_Ctrl, Callback(this, &CConsole::Next_cmd));

	ec().assign_callback(DIK_HOME, text_editor::ks_Alt, Callback(this, &CConsole::Begin_tips));
	ec().assign_callback(DIK_END, text_editor::ks_Alt, Callback(this, &CConsole::End_tips));
	ec().assign_callback(DIK_PRIOR, text_editor::ks_Alt, Callback(this, &CConsole::PageUp_tips));
	ec().assign_callback(DIK_NEXT, text_editor::ks_Alt, Callback(this, &CConsole::PageDown_tips));

	ec().assign_callback(DIK_RETURN, text_editor::ks_free, Callback(this, &CConsole::Execute_cmd));
	ec().assign_callback(DIK_NUMPADENTER, text_editor::ks_free, Callback(this, &CConsole::Execute_cmd));

	ec().assign_callback(DIK_ESCAPE, text_editor::ks_free, Callback(this, &CConsole::Hide_cmd_esc));
	ec().assign_callback(DIK_GRAVE, text_editor::ks_free, Callback(this, &CConsole::Hide_cmd));

	//Screenshot
	ec().assign_callback(DIK_F12, text_editor::ks_free, Callback(this, &CConsole::Screenshot));
}

void CConsole::Screenshot()
{
	Render->Screenshot();
#pragma todo("Simp: ?")
    //if (g_pGamePersistent)
    //    g_pGamePersistent->m_pMainMenu->Screenshot();
}

void CConsole::Prev_log() // DIK_PRIOR=PAGE_UP
{
	scroll_delta++;
	if (scroll_delta > int(LogFile.size()) - 1)
	{
		scroll_delta = LogFile.size() - 1;
	}
}

void CConsole::Next_log() // DIK_NEXT=PAGE_DOWN
{
	scroll_delta--;
	if (scroll_delta < 0)
	{
		scroll_delta = 0;
	}
}

void CConsole::Begin_log() // PAGE_UP+Ctrl
{
	scroll_delta = LogFile.size() - 1;
}

void CConsole::End_log() // PAGE_DOWN+Ctrl
{
	scroll_delta = 0;
}

void CConsole::Find_cmd() // DIK_TAB
{
	shared_str out_str;

	IConsole_Command* cc = find_next_cmd(ec().str_edit(), out_str);
	if (cc && out_str.size())
	{
		ec().set_edit(out_str.c_str());
	}
}

void CConsole::Find_cmd_back() // DIK_TAB+shift
{
	LPCSTR edt = ec().str_edit();

	vecCMD_IT it = Commands.lower_bound(edt);
	if (it != Commands.begin())
	{
		--it;
		IConsole_Command& cc = *(it->second);
		LPCSTR name_cmd = cc.Name();

		ec().set_edit(name_cmd);
	}
}

void CConsole::Prev_cmd() // DIK_UP + Ctrl
{
	prev_cmd_history_idx();
	SelectCommand();
}

void CConsole::Next_cmd() // DIK_DOWN + Ctrl
{
	next_cmd_history_idx();
	SelectCommand();
}

void CConsole::Prev_tip() // DIK_UP
{
	if (xr_strlen(ec().str_edit()) == 0)
	{
		prev_cmd_history_idx();
		SelectCommand();
		return;
	}
	prev_selected_tip();
}

void CConsole::Next_tip() // DIK_DOWN + Ctrl
{
	if (xr_strlen(ec().str_edit()) == 0)
	{
		next_cmd_history_idx();
		SelectCommand();
		return;
	}
	next_selected_tip();
}

void CConsole::Begin_tips()
{
	m_select_tip = 0;
	m_start_tip = 0;
}

void CConsole::End_tips()
{
	m_select_tip = m_tips.size() - 1;
	m_start_tip = m_select_tip - VIEW_TIPS_COUNT + 1;
	check_next_selected_tip();
}

void CConsole::PageUp_tips()
{
	m_select_tip -= VIEW_TIPS_COUNT;
	check_prev_selected_tip();
}

void CConsole::PageDown_tips()
{
	m_select_tip += VIEW_TIPS_COUNT;
	check_next_selected_tip();
}

void CConsole::Execute_cmd() // DIK_RETURN, DIK_NUMPADENTER
{
	if (0 <= m_select_tip && m_select_tip < (int)m_tips.size())
	{
		shared_str const& str = m_tips[m_select_tip].text;
		if (m_tips_mode == 1)
		{
            string_path buf;
            xr_strconcat(buf, str.c_str(), " ");
			ec().set_edit(buf);
		}
		else if (m_tips_mode == 2)
		{
            string_path buf;
            xr_strconcat(buf, m_cur_cmd.c_str(), " ", str.c_str());
			ec().set_edit(buf);
		}
		reset_selected_tip();
	}
	else
	{
		ExecuteCommand(ec().str_edit());
	}
	m_disable_tips = false;
}

void CConsole::Show_cmd()
{
	Show();
}

void CConsole::Hide_cmd()
{
	Hide();
}

void CConsole::Hide_cmd_esc()
{
	if (0 <= m_select_tip && m_select_tip < (int)m_tips.size())
	{
		m_disable_tips = true;
		return;
	}
	Hide();
}

void CConsole::GamePause()
{
}
