// Rain.h: interface for the CRain class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

// refs
class ENGINE_API IRender_DetailModel;
class ENGINE_API CLAItem;

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/LensFlareRender.h"
#include "../Include/xrRender/ThunderboltDescRender.h"
#include "../Include/xrRender/ThunderboltRender.h"

class CEnvironment;

struct SThunderboltDesc
{
    // geom
    // IRender_DetailModel*		l_model;
    FactoryPtr<IThunderboltDescRender> m_pRender;
    // sound
    ref_sound snd;
    // gradient
    struct SFlare
    {
        float fOpacity;
        Fvector2 fRadius;
        shared_str texture;
        shared_str shader;
        // ref_shader				hShader;
        FactoryPtr<IFlareRender> m_pFlare;
        SFlare()
        {
            fOpacity = 0;
            fRadius.set(0.f, 0.f);
        }
    };
    SFlare* m_GradientTop;
    SFlare* m_GradientCenter;
    shared_str name;
    CLAItem* color_anim{};

public:
    SThunderboltDesc();
    ~SThunderboltDesc();

    void load(CInifile& pIni, shared_str const& sect);
    void create_top_gradient(CInifile& pIni, shared_str const& sect);
    void create_center_gradient(CInifile& pIni, shared_str const& sect);

    void load_shoc(CInifile* pIni, shared_str const& sect);
    void create_top_gradient_shoc(CInifile* pIni, shared_str const& sect);
    void create_center_gradient_shoc(CInifile* pIni, shared_str const& sect);
};

struct SThunderboltCollection
{
    DEFINE_VECTOR(SThunderboltDesc*, DescVec, DescIt);
    DescVec palette;
    shared_str section;

public:
    SThunderboltCollection();
    ~SThunderboltCollection();
    void load(CInifile* pIni, CInifile* thunderbolts, LPCSTR sect);
    void load_shoc(CInifile* pIni, LPCSTR sect);
    SThunderboltDesc* GetRandomDesc() { return palette.at(Random.randI(palette.size())); }
};

#define THUNDERBOLT_CACHE_SIZE 8
//
class ENGINE_API CEffect_Thunderbolt
{
    friend class dxThunderboltRender;

protected:
    DEFINE_VECTOR(SThunderboltCollection*, CollectionVec, CollectionVecIt);
    CollectionVec collection;
    SThunderboltDesc* current;

private:
    Fmatrix current_xform;
    Fvector3 current_direction;

    FactoryPtr<IThunderboltRender> m_pRender;
    // ref_geom			  		hGeom_model;
    //  states
    enum EState
    {
        stIdle,
        stWorking
    };
    EState state;

    // ref_geom			  		hGeom_gradient;

    Fvector lightning_center;
    float lightning_size{};
    float lightning_phase{};

    float life_time;
    float current_time{};
    float next_lightning_time;
    BOOL bEnabled;

    // params
    //	Fvector2					p_var_alt;
    //	float						p_var_long;
    //	float						p_min_dist;
    //	float						p_tilt;
    //	float						p_second_prop;
    //	float						p_sky_color;
    //	float						p_sun_color;
    //	float						p_fog_color;
private:
    BOOL RayPick(const Fvector& s, const Fvector& d, float& range);
    void Bolt(shared_str id, float period, float life_time);

public:
    CEffect_Thunderbolt();
    ~CEffect_Thunderbolt();

    void OnFrame(shared_str id, float period, float duration);
    void Render(CBackend& cmd_list);

    shared_str AppendDef(CEnvironment& environment, CInifile* pIni, CInifile* thunderbolts, LPCSTR sect);
    shared_str AppendDef_shoc(CEnvironment& environment, CInifile* pIni, LPCSTR sect);
};
