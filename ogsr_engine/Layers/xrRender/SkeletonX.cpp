// SkeletonX.cpp: implementation of the CSkeletonX class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "../../xr_3da/Render.h"
#include "SkeletonX.h"
#include "SkeletonCustom.h"
#include "../../xr_3da/fmesh.h"
#include "../../xr_3da/xrSkinXW.hpp"

//////////////////////////////////////////////////////////////////////
// Body Part
//////////////////////////////////////////////////////////////////////
void CSkeletonX::AfterLoad(CKinematics* parent, u16 child_idx)
{
    SetParent(parent);
    ChildIDX = child_idx;
}
void CSkeletonX::_Copy(CSkeletonX* B)
{
    Parent = nullptr;
    ChildIDX = B->ChildIDX;
    Vertices1W = B->Vertices1W;
    Vertices2W = B->Vertices2W;
    Vertices3W = B->Vertices3W;
    Vertices4W = B->Vertices4W;
    BonesUsed = B->BonesUsed;

    // caution - overlapped (union)
    cache_DiscardID = B->cache_DiscardID;
    cache_vCount = B->cache_vCount;
    cache_vOffset = B->cache_vOffset;
    RenderMode = B->RenderMode;
    RMS_boneid = B->RMS_boneid;
    RMS_bonecount = B->RMS_bonecount;

    m_Indices = B->m_Indices;
}

bool fill_array(Fvector4* bones_array, CKinematics* parent, bool need_old, u32 size)
{
    if (!bones_array)
        return false;

    for (u32 mid = 0; mid < size; mid++)
    {
        const Fmatrix& M = (need_old) ? parent->LL_GetTransform_R_old(u16(mid)) : parent->LL_GetTransform_R(u16(mid));

        bones_array->set(M._11, M._21, M._31, M._41);
        bones_array++;
        bones_array->set(M._12, M._22, M._32, M._42);
        bones_array++;
        bones_array->set(M._13, M._23, M._33, M._43);
        bones_array++;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////
void CSkeletonX::_Render(CBackend& cmd_list, ref_geom& hGeom, u32 vCount, u32 iOffset, u32 pCount)
{
    Parent->StoreVisualMatrix(cmd_list.xforms.m_w);
    cmd_list.set_xform_world_old(Parent->mOldWorldMartrix);

    cmd_list.stat.r.s_dynamic.add(vCount);
    switch (RenderMode)
    {
    case RM_SKINNING_SOFT: {
        _Render_soft(cmd_list, hGeom, vCount, iOffset, pCount);
        cmd_list.stat.r.s_dynamic_sw.add(vCount);
    }break;
    case RM_SINGLE: {
        Fmatrix W;
        W.mul_43(cmd_list.xforms.m_w, Parent->LL_GetTransform_R(u16(RMS_boneid)));
        Fmatrix O;
        O.mul_43(cmd_list.xforms.m_w_old, Parent->LL_GetTransform_R_old(u16(RMS_boneid)));
        cmd_list.set_xform_world(W);
        cmd_list.set_xform_world_old(O);
        cmd_list.set_Geometry(hGeom);
        cmd_list.Render(D3DPT_TRIANGLELIST, 0, 0, vCount, iOffset, pCount);
        cmd_list.stat.r.s_dynamic_inst.add(vCount);
    }
    break;
    case RM_SKINNING_1B:
    case RM_SKINNING_2B:
    case RM_SKINNING_3B:
    case RM_SKINNING_4B: {
        // transfer matrices
        const u32 c_bones_array_size = RMS_bonecount * sizeof(Fvector4) * 3;
        const u32 c_bones_array_size_old = c_bones_array_size;

        static const shared_str s_bones_array_const{"sbones_array"}, s_bones_array_const_old{"sbones_array_old"};

        Fvector4* c_bones_array{};
        Fvector4* c_bones_array_old{};
        cmd_list.get_ConstantDirect(s_bones_array_const, c_bones_array_size, reinterpret_cast<void**>(&c_bones_array), nullptr, nullptr);
        cmd_list.get_ConstantDirect(s_bones_array_const_old, c_bones_array_size_old, reinterpret_cast<void**>(&c_bones_array_old), nullptr, nullptr);

        if (!fill_array(c_bones_array, Parent, false, RMS_bonecount) || !fill_array(c_bones_array_old, Parent, true, RMS_bonecount))
        {
            static bool logged{}; // чтоб не спамить в лог по сто раз за кадр.
            if (!logged)
            {
                logged = true;
                Msg("!![%s] Can't get/create some bone array for model [%s] with [%u] bones. Most likely, an incorrect shader is assigned there.", __FUNCTION__,
                    this->Parent->dbg_name.c_str(), RMS_bonecount);
            }
        }

        // render
        cmd_list.set_Geometry(hGeom);
        cmd_list.Render(D3DPT_TRIANGLELIST, 0, 0, vCount, iOffset, pCount);
        if (RM_SKINNING_1B == RenderMode)
            cmd_list.stat.r.s_dynamic_1B.add(vCount);
        else if (RM_SKINNING_2B == RenderMode)
            cmd_list.stat.r.s_dynamic_2B.add(vCount);
        else if (RM_SKINNING_3B == RenderMode)
            cmd_list.stat.r.s_dynamic_3B.add(vCount);
        else if (RM_SKINNING_4B == RenderMode)
            cmd_list.stat.r.s_dynamic_4B.add(vCount);
    }
    break;
    }

    cmd_list.set_xform_world_old(Fidentity);
}

void CSkeletonX::_Render_soft(CBackend& cmd_list, ref_geom& hGeom, u32 vCount, u32 iOffset, u32 pCount)
{
    u32 vOffset = cache_vOffset;

    _VertexStream& _VS = RImplementation.Vertex;
    if (cache_DiscardID != _VS.DiscardID() || vCount >= cache_vCount)
    {
        vertRender* Dest = (vertRender*)_VS.Lock(vCount, hGeom->vb_stride, vOffset);
        cache_DiscardID = _VS.DiscardID();
        cache_vCount = vCount;
        cache_vOffset = vOffset;

        Device.Statistic->RenderDUMP_SKIN.Begin();
        if (*Vertices1W)
        {
            Skin1W(Dest, // dest
                   *Vertices1W, // source
                   vCount, // count
                   Parent->bone_instances // bones
            );
        }
        else if (*Vertices2W)
        {
            Skin2W(Dest, // dest
                   *Vertices2W, // source
                   vCount, // count
                   Parent->bone_instances // bones
            );
        }
        else if (*Vertices3W)
        {
            Skin3W(Dest, // dest
                   *Vertices3W, // source
                   vCount, // count
                   Parent->bone_instances // bones
            );
        }
        else if (*Vertices4W)
        {
            Skin4W(Dest, // dest
                   *Vertices4W, // source
                   vCount, // count
                   Parent->bone_instances // bones
            );
        }
        else
            R_ASSERT(0, "unsupported soft rendering");

        Device.Statistic->RenderDUMP_SKIN.End();
        _VS.Unlock(vCount, hGeom->vb_stride);
    }

    cmd_list.set_Geometry(hGeom);
    cmd_list.Render(D3DPT_TRIANGLELIST, vOffset, 0, vCount, iOffset, pCount);
}

void CSkeletonX::_Load(const char* N, IReader* data, u32& dwVertCount)
{
    xr_vector<u16> bids;

    // Load vertices
    R_ASSERT(data->find_chunk(OGF_VERTICES));

    constexpr u16 hw_bones_cnt = 254;

    u16 sw_bones_cnt = 0;

    u32 dwVertType, it/*, size, crc*/;
    dwVertType = data->r_u32();
    dwVertCount = data->r_u32();

    RenderMode = RM_SKINNING_SOFT;
    Render->shader_option_skinning(-1);

    switch (dwVertType)
    {
    case OGF_VERTEXFORMAT_FVF_1L: // 1-Link
    case 1: {
        //size = dwVertCount * sizeof(vertBoned1W);
        const vertBoned1W* pVO = (vertBoned1W*)data->pointer();

        for (it = 0; it < dwVertCount; ++it)
        {
            const vertBoned1W& VB = pVO[it];
            u16 mid = (u16)VB.matrix;

            if (bids.end() == std::find(bids.begin(), bids.end(), mid))
                bids.push_back(mid);

            sw_bones_cnt = _max(sw_bones_cnt, mid);
        }
        if (1 == bids.size())
        {
            // HW- single bone
            RenderMode = RM_SINGLE;
            RMS_boneid = *bids.begin();
            Render->shader_option_skinning(0);
        }
        else if (sw_bones_cnt <= hw_bones_cnt)
        {
            // HW- one weight
            RenderMode = RM_SKINNING_1B;
            RMS_bonecount = sw_bones_cnt + 1;
            Render->shader_option_skinning(1);
        }
        else
        {
            // software
            //crc = crc32(data->pointer(), size);
            Vertices1W.create(dwVertCount, (vertBoned1W*)data->pointer());
            Render->shader_option_skinning(-1);
        }
    }
    break;
    case OGF_VERTEXFORMAT_FVF_2L: // 2-Link
    case 2: {
        //size = dwVertCount * sizeof(vertBoned2W);
        const vertBoned2W* pVO = (vertBoned2W*)data->pointer();

        for (it = 0; it < dwVertCount; ++it)
        {
            const vertBoned2W& VB = pVO[it];
            sw_bones_cnt = _max(sw_bones_cnt, VB.matrix0);
            sw_bones_cnt = _max(sw_bones_cnt, VB.matrix1);

            if (bids.end() == std::find(bids.begin(), bids.end(), VB.matrix0))
                bids.push_back(VB.matrix0);

            if (bids.end() == std::find(bids.begin(), bids.end(), VB.matrix1))
                bids.push_back(VB.matrix1);
        }
        //.			R_ASSERT(sw_bones_cnt<=hw_bones_cnt);
        if (sw_bones_cnt <= hw_bones_cnt)
        {
            // HW- two weights
            RenderMode = RM_SKINNING_2B;
            RMS_bonecount = sw_bones_cnt + 1;
            Render->shader_option_skinning(2);
        }
        else
        {
            // software
            //crc = crc32(data->pointer(), size);
            Vertices2W.create(dwVertCount, (vertBoned2W*)data->pointer());
            Render->shader_option_skinning(-1);
        }
    }
    break;
    case OGF_VERTEXFORMAT_FVF_3L: // 3-Link
    case 3: {
        //size = dwVertCount * sizeof(vertBoned3W);
        const vertBoned3W* pVO = (vertBoned3W*)data->pointer();

        for (it = 0; it < dwVertCount; ++it)
        {
            const vertBoned3W& VB = pVO[it];
            for (unsigned short i : VB.m)
            {
                sw_bones_cnt = _max(sw_bones_cnt, i);

                if (bids.end() == std::find(bids.begin(), bids.end(), i))
                    bids.push_back(i);
            }
        }
        //.			R_ASSERT(sw_bones_cnt<=hw_bones_cnt);
        if ((sw_bones_cnt <= hw_bones_cnt))
        {
            RenderMode = RM_SKINNING_3B;
            RMS_bonecount = sw_bones_cnt + 1;
            Render->shader_option_skinning(3);
        }
        else
        {
            //crc = crc32(data->pointer(), size);
            Vertices3W.create(dwVertCount, (vertBoned3W*)data->pointer());
            Render->shader_option_skinning(-1);
        }
    }
    break;
    case OGF_VERTEXFORMAT_FVF_4L: // 4-Link
    case 4: {
        //size = dwVertCount * sizeof(vertBoned4W);
        const vertBoned4W* pVO = (vertBoned4W*)data->pointer();

        for (it = 0; it < dwVertCount; ++it)
        {
            const vertBoned4W& VB = pVO[it];

            for (unsigned short i : VB.m)
            {
                sw_bones_cnt = _max(sw_bones_cnt, i);

                if (bids.end() == std::find(bids.begin(), bids.end(), i))
                    bids.push_back(i);
            }
        }
        //.			R_ASSERT(sw_bones_cnt<=hw_bones_cnt);
        if (sw_bones_cnt <= hw_bones_cnt)
        {
            RenderMode = RM_SKINNING_4B;
            RMS_bonecount = sw_bones_cnt + 1;
            Render->shader_option_skinning(4);
        }
        else
        {
            //crc = crc32(data->pointer(), size);
            Vertices4W.create(dwVertCount, (vertBoned4W*)data->pointer());
            Render->shader_option_skinning(-1);
        }
    }
    break;
    default: FATAL("Invalid vertex type in skinned model '%s'", N); break;
    }
    if (bids.size() > 1)
    {
        //crc = crc32(&*bids.begin(), bids.size() * sizeof(u16));
        BonesUsed.create(bids.size(), &*bids.begin());
    }
}

BOOL CSkeletonX::has_visible_bones()
{
    if (RM_SINGLE == RenderMode)
    {
        return Parent->LL_GetBoneVisible((u16)RMS_boneid);
    }

    for (u32 it = 0; it < BonesUsed.size(); it++)
        if (Parent->LL_GetBoneVisible(BonesUsed[it]))
        {
            return TRUE;
        }
    return FALSE;
}

void get_pos_bones(const vertBoned1W& v, Fvector& p, CKinematics* Parent)
{
    const Fmatrix& xform = Parent->LL_GetBoneInstance((u16)v.matrix).mRenderTransform;
    xform.transform_tiny(p, v.P);
}

void get_pos_bones(const vertBoned2W& vert, Fvector& p, CKinematics* Parent)
{
    Fvector P0, P1;

    const Fmatrix& xform0 = Parent->LL_GetBoneInstance(vert.matrix0).mRenderTransform;
    const Fmatrix& xform1 = Parent->LL_GetBoneInstance(vert.matrix1).mRenderTransform;
    xform0.transform_tiny(P0, vert.P);
    xform1.transform_tiny(P1, vert.P);
    p.lerp(P0, P1, vert.w);
}

void get_pos_bones(const vertBoned3W& vert, Fvector& p, CKinematics* Parent)
{
    const Fmatrix& M0 = Parent->LL_GetBoneInstance(vert.m[0]).mRenderTransform;
    const Fmatrix& M1 = Parent->LL_GetBoneInstance(vert.m[1]).mRenderTransform;
    const Fmatrix& M2 = Parent->LL_GetBoneInstance(vert.m[2]).mRenderTransform;

    Fvector P0, P1, P2;
    M0.transform_tiny(P0, vert.P);
    P0.mul(vert.w[0]);
    M1.transform_tiny(P1, vert.P);
    P1.mul(vert.w[1]);
    M2.transform_tiny(P2, vert.P);
    P2.mul(1.0f - vert.w[0] - vert.w[1]);

    p = P0;
    p.add(P1);
    p.add(P2);
}
void get_pos_bones(const vertBoned4W& vert, Fvector& p, CKinematics* Parent)
{
    const Fmatrix& M0 = Parent->LL_GetBoneInstance(vert.m[0]).mRenderTransform;
    const Fmatrix& M1 = Parent->LL_GetBoneInstance(vert.m[1]).mRenderTransform;
    const Fmatrix& M2 = Parent->LL_GetBoneInstance(vert.m[2]).mRenderTransform;
    const Fmatrix& M3 = Parent->LL_GetBoneInstance(vert.m[3]).mRenderTransform;

    Fvector P0, P1, P2, P3;
    M0.transform_tiny(P0, vert.P);
    P0.mul(vert.w[0]);
    M1.transform_tiny(P1, vert.P);
    P1.mul(vert.w[1]);
    M2.transform_tiny(P2, vert.P);
    P2.mul(vert.w[2]);
    M3.transform_tiny(P3, vert.P);
    P3.mul(1.0f - vert.w[0] - vert.w[1] - vert.w[2]);

    p = P0;
    p.add(P1);
    p.add(P2);
    p.add(P3);
}

//-----------------------------------------------------------------------------------------------------
// Wallmarks
//-----------------------------------------------------------------------------------------------------
#include "cl_intersect.h"
BOOL CSkeletonX::_PickBoneSoft1W(IKinematics::pick_result& r, float dist, const Fvector& S, const Fvector& D, u16* indices, CBoneData::FacesVec& faces) const
{
    return pick_bone<vertBoned1W>(Vertices1W, Parent, r, dist, S, D, indices, faces);
}

BOOL CSkeletonX::_PickBoneSoft2W(IKinematics::pick_result& r, float dist, const Fvector& S, const Fvector& D, u16* indices, CBoneData::FacesVec& faces)
{
    return pick_bone<vertBoned2W>(Vertices2W, Parent, r, dist, S, D, indices, faces);
}

BOOL CSkeletonX::_PickBoneSoft3W(IKinematics::pick_result& r, float dist, const Fvector& S, const Fvector& D, u16* indices, CBoneData::FacesVec& faces)
{
    return pick_bone<vertBoned3W>(Vertices3W, Parent, r, dist, S, D, indices, faces);
}

BOOL CSkeletonX::_PickBoneSoft4W(IKinematics::pick_result& r, float dist, const Fvector& S, const Fvector& D, u16* indices, CBoneData::FacesVec& faces)
{
    return pick_bone<vertBoned4W>(Vertices4W, Parent, r, dist, S, D, indices, faces);
}

// Fill Vertices
void CSkeletonX::_FillVerticesSoft1W(const Fmatrix& view, CSkeletonWallmark& wm, const Fvector& normal, float size, u16* indices, CBoneData::FacesVec& faces)
{
    VERIFY(*Vertices1W);
    for (const unsigned short& face : faces)
    {
        Fvector p[3];
        const u32 idx = face * 3;
        CSkeletonWallmark::WMFace F;
        for (u32 k = 0; k < 3; k++)
        {
            vertBoned1W& vert = Vertices1W[indices[idx + k]];
            F.bone_id[k][0] = (u16)vert.matrix;
            F.bone_id[k][1] = F.bone_id[k][0];
            F.bone_id[k][2] = F.bone_id[k][0];
            F.bone_id[k][3] = F.bone_id[k][0];
            F.weight[k][0] = 0.f;
            F.weight[k][1] = 0.f;
            F.weight[k][2] = 0.f;
            const Fmatrix& xform = Parent->LL_GetBoneInstance(F.bone_id[k][0]).mRenderTransform;
            F.vert[k].set(vert.P);
            xform.transform_tiny(p[k], F.vert[k]);
        }
        Fvector test_normal;
        test_normal.mknormal(p[0], p[1], p[2]);
        const float cosa = test_normal.dotproduct(normal);
        if (cosa < EPS)
            continue;
        if (CDB::TestSphereTri(wm.ContactPoint(), size, p))
        {
            Fvector UV;
            for (u32 k = 0; k < 3; k++)
            {
                Fvector2& uv = F.uv[k];
                view.transform_tiny(UV, p[k]);
                uv.x = (1 + UV.x) * .5f;
                uv.y = (1 - UV.y) * .5f;
            }
            wm.m_Faces.push_back(F);
        }
    }
}

void CSkeletonX::_FillVerticesSoft2W(const Fmatrix& view, CSkeletonWallmark& wm, const Fvector& normal, float size, u16* indices, CBoneData::FacesVec& faces)
{
    VERIFY(*Vertices2W);
    for (const unsigned short& face : faces)
    {
        Fvector p[3];
        const u32 idx = face * 3;
        CSkeletonWallmark::WMFace F;
        for (u32 k = 0; k < 3; k++)
        {
            Fvector P0, P1;
            vertBoned2W& vert = Vertices2W[indices[idx + k]];
            F.bone_id[k][0] = vert.matrix0;
            F.bone_id[k][1] = vert.matrix1;
            F.bone_id[k][2] = F.bone_id[k][1];
            F.bone_id[k][3] = F.bone_id[k][1];
            F.weight[k][0] = vert.w;
            F.weight[k][1] = 0.f;
            F.weight[k][2] = 0.f;
            Fmatrix& xform0 = Parent->LL_GetBoneInstance(F.bone_id[k][0]).mRenderTransform;
            Fmatrix& xform1 = Parent->LL_GetBoneInstance(F.bone_id[k][1]).mRenderTransform;
            F.vert[k].set(vert.P);
            xform0.transform_tiny(P0, F.vert[k]);
            xform1.transform_tiny(P1, F.vert[k]);
            p[k].lerp(P0, P1, F.weight[k][0]);
        }
        Fvector test_normal;
        test_normal.mknormal(p[0], p[1], p[2]);
        const float cosa = test_normal.dotproduct(normal);
        if (cosa < EPS)
            continue;
        if (CDB::TestSphereTri(wm.ContactPoint(), size, p))
        {
            Fvector UV;
            for (u32 k = 0; k < 3; k++)
            {
                Fvector2& uv = F.uv[k];
                view.transform_tiny(UV, p[k]);
                uv.x = (1 + UV.x) * .5f;
                uv.y = (1 - UV.y) * .5f;
            }
            wm.m_Faces.push_back(F);
        }
    }
}

void CSkeletonX::_FillVerticesSoft3W(const Fmatrix& view, CSkeletonWallmark& wm, const Fvector& normal, float size, u16* indices, CBoneData::FacesVec& faces)
{
    VERIFY(*Vertices3W);
    for (const unsigned short& face : faces)
    {
        Fvector p[3];
        const u32 idx = face * 3;
        CSkeletonWallmark::WMFace F;

        for (u32 k = 0; k < 3; k++)
        {
            const vertBoned3W& vert = Vertices3W[indices[idx + k]];
            F.bone_id[k][0] = vert.m[0];
            F.bone_id[k][1] = vert.m[1];
            F.bone_id[k][2] = vert.m[2];
            F.bone_id[k][3] = F.bone_id[k][2];
            F.weight[k][0] = vert.w[0];
            F.weight[k][1] = vert.w[1];
            F.weight[k][2] = 0.f;
            vert.get_pos(F.vert[k]);
            get_pos_bones(vert, p[k], Parent);
        }
        Fvector test_normal;
        test_normal.mknormal(p[0], p[1], p[2]);
        const float cosa = test_normal.dotproduct(normal);
        if (cosa < EPS)
            continue;

        if (CDB::TestSphereTri(wm.ContactPoint(), size, p))
        {
            Fvector UV;
            for (u32 k = 0; k < 3; k++)
            {
                Fvector2& uv = F.uv[k];
                view.transform_tiny(UV, p[k]);
                uv.x = (1 + UV.x) * .5f;
                uv.y = (1 - UV.y) * .5f;
            }
            wm.m_Faces.push_back(F);
        }
    }
}

void CSkeletonX::_FillVerticesSoft4W(const Fmatrix& view, CSkeletonWallmark& wm, const Fvector& normal, float size, u16* indices, CBoneData::FacesVec& faces)
{
    VERIFY(*Vertices4W);
    for (const unsigned short& face : faces)
    {
        Fvector p[3];
        const u32 idx = face * 3;
        CSkeletonWallmark::WMFace F;

        for (u32 k = 0; k < 3; k++)
        {
            const vertBoned4W& vert = Vertices4W[indices[idx + k]];
            F.bone_id[k][0] = vert.m[0];
            F.bone_id[k][1] = vert.m[1];
            F.bone_id[k][2] = vert.m[2];
            F.bone_id[k][3] = vert.m[3];
            F.weight[k][0] = vert.w[0];
            F.weight[k][1] = vert.w[1];
            F.weight[k][2] = vert.w[2];
            vert.get_pos(F.vert[k]);
            get_pos_bones(vert, p[k], Parent);
        }
        Fvector test_normal;
        test_normal.mknormal(p[0], p[1], p[2]);
        const float cosa = test_normal.dotproduct(normal);
        if (cosa < EPS)
            continue;

        if (CDB::TestSphereTri(wm.ContactPoint(), size, p))
        {
            Fvector UV;
            for (u32 k = 0; k < 3; k++)
            {
                Fvector2& uv = F.uv[k];
                view.transform_tiny(UV, p[k]);
                uv.x = (1 + UV.x) * .5f;
                uv.y = (1 - UV.y) * .5f;
            }
            wm.m_Faces.push_back(F);
        }
    }
}

void CSkeletonX::_DuplicateIndices(const char* N, IReader* data)
{
    //	We will have trouble with container since don't know were to take readable indices
    VERIFY(!data->find_chunk(OGF_ICONTAINER));
    //	Index buffer replica since we can't read from index buffer in DX10
    // ref_smem<u16>			Indices;
    R_ASSERT(data->find_chunk(OGF_INDICES));
    const u32 iCount = data->r_u32();

    //u32 size = iCount * 2;
    //u32 crc = crc32(data->pointer(), size);
    m_Indices.create(iCount, (u16*)data->pointer());
}