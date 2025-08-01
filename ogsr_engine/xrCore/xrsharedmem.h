#pragma once

#pragma pack(push, 4)
//////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4200)
struct XRCORE_API smem_value
{
    std::atomic<u32> dwReference{};

    u32 dwCRC;
    u32 dwSize; // size in bytes !!!

    u8 value[];
};

// predicate for insertion - just a quick estimate
IC bool smem_search(const smem_value* A, const smem_value* B)
{
    if (A->dwCRC < B->dwCRC)
        return true;
    if (A->dwCRC > B->dwCRC)
        return false;
    return A->dwSize < B->dwSize;
};
#pragma warning(default : 4200)

//////////////////////////////////////////////////////////////////////////
class XRCORE_API smem_container
{
private:
    typedef xr_vector<smem_value*> cdb;
    xrCriticalSection cs;
    cdb container;
    bool bDisable{};

public:
    smem_value* dock(u32 dwSize, void* ptr);
    void clean();
    void dump();
    u32 stat_economy();
    bool disabled() const { return bDisable; }
#ifdef PROFILE_CRITICAL_SECTIONS
    smem_container() : cs(MUTEX_PROFILE_ID(smem_container)) {}
#endif // PROFILE_CRITICAL_SECTIONS

    smem_container(bool disable) { bDisable = disable; }
    ~smem_container();
};
XRCORE_API extern smem_container* g_pSharedMemoryContainer;

//////////////////////////////////////////////////////////////////////////
template <class T>
class ref_smem
{
private:
    smem_value* p_;

protected:
    // ref-counting
    void _dec()
    {
        if (0 == p_)
            return;
        --p_->dwReference;
        if (0 == p_->dwReference)
        {
            // Msg("ref_smem: deleting %d bytes of shared memory", p_->dwSize);
            if (g_pSharedMemoryContainer->disabled())
                xr_free(p_);

            p_ = 0;
        }
    }

public:
    void _set(ref_smem const& rhs)
    {
        smem_value* v = rhs.p_;

        if (v)
            ++v->dwReference;

        _dec();
        p_ = v;
    }
    const smem_value* _get() const { return p_; }

public:
    // construction
    ref_smem() { p_ = 0; }
    ref_smem(ref_smem<T> const& rhs)
    {
        p_ = 0;
        _set(rhs);
    }
    ~ref_smem() { _dec(); }

    void create(u32 dwLength, T* ptr)
    {
        smem_value* v = g_pSharedMemoryContainer->dock(dwLength * sizeof(T), ptr);

        if (v)
            ++v->dwReference;

        _dec();
        p_ = v;
    }

    // assignment & accessors
    ref_smem<T>& operator=(ref_smem<T> const& rhs)
    {
        _set(rhs);
        return static_cast<ref_smem<T>&>(*this);
    }
    T* operator*() const { return p_ ? (T*)p_->value : 0; }
    bool operator!() const { return p_ == 0; }
    T& operator[](size_t id) { return ((T*)(p_->value))[id]; }
    const T& operator[](size_t id) const { return ((T*)(p_->value))[id]; }
    // misc func
    u32 size()
    {
        if (0 == p_)
            return 0;
        else
            return p_->dwSize / sizeof(T);
    }
    void swap(ref_smem<T>& rhs)
    {
        smem_value* tmp = p_;
        p_ = rhs.p_;
        rhs.p_ = tmp;
    }
    bool equal(ref_smem<T>& rhs) { return p_ == rhs.p_; }
    u32 ref_count()
    {
        if (0 == p_)
            return 0;
        else
            return p_->dwReference;
    }
};

// res_ptr == res_ptr
// res_ptr != res_ptr
// const res_ptr == ptr
// const res_ptr != ptr
// ptr == const res_ptr
// ptr != const res_ptr
// res_ptr < res_ptr
// res_ptr > res_ptr
template <class T>
IC bool operator==(ref_smem<T> const& a, ref_smem<T> const& b)
{
    return a._get() == b._get();
}
template <class T>
IC bool operator!=(ref_smem<T> const& a, ref_smem<T> const& b)
{
    return a._get() != b._get();
}
template <class T>
IC bool operator<(ref_smem<T> const& a, ref_smem<T> const& b)
{
    return a._get() < b._get();
}
template <class T>
IC bool operator>(ref_smem<T> const& a, ref_smem<T> const& b)
{
    return a._get() > b._get();
}

// externally visible standart functionality
template <class T>
IC void swap(ref_smem<T>& lhs, ref_smem<T>& rhs)
{
    lhs.swap(rhs);
}

#pragma pack(pop)
