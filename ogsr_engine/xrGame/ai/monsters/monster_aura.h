#pragma once

class CBaseMonster;

class monster_aura
{
private:
    CBaseMonster* m_object;

    char m_name[64]{};
    bool this_is_psy_aura{};
    float m_linear_factor{};
    float m_quadratic_factor{};
    float m_max_power{};
    float m_max_distance{};
    bool m_enable_for_dead;

    float m_pp_highest_at{};
    pcstr m_pp_effector_name;
    u32 m_pp_index;

    ref_sound m_sound;
    ref_sound m_detect_sound;
    float m_detect_snd_time;

    bool m_enabled;

public:
    monster_aura(CBaseMonster* object, pcstr name);
    ~monster_aura();

    void load_from_ini(CInifile* ini, pcstr section, bool enable_for_dead_default = false);
    float calculate() const;
    void update_schedule();
    void play_detector_sound();
    void on_monster_death();

    bool enable_for_dead() { return m_enable_for_dead; }
    float max_distance() { return m_max_distance; }

private:
    bool check_work_condition() const;
    void remove_pp_effector();
    float get_post_process_factor() const;
};
