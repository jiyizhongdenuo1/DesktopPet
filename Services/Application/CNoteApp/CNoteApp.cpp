
#include "CNoteApp.h"
#include "DTranslation.h"
#include "IDataCache.h"
#include "CNoteBusiness.h"

CNoteApp::CNoteApp()
        : m_pBusiness(std::make_unique<CNoteBusiness>())
{

}

CNoteApp::~CNoteApp() = default;

void CNoteApp::DoSecend()
{
    if (!m_pBusiness || !m_pCache)
    {
        return ;
    }
    auto p_Snapshot = m_pCache->GetSnapshot();
    std::vector<INT32> vec_TipCache;
    if (p_Snapshot)
    {
        vec_TipCache = m_pBusiness->ProcessTip(p_Snapshot);
        for (auto s32Tip : vec_TipCache)
        {
            if (s32Tip < p_Snapshot->size() && s32Tip >= 0)
            {
                ST_NOTE_DATA st_Modify = p_Snapshot->at(s32Tip);
                const time_t t_LastRemind = TodayWithTimeOfDay(st_Modify.m_s64RemindTime);
                if (t_LastRemind > 0)
                {
                    st_Modify.m_S64LastRemindTime = t_LastRemind;
                }
                m_pCache->UpdateNoteDataCache(st_Modify);

                for (const auto &fn : m_vecfnTip)
                {
                    fn(p_Snapshot->at(s32Tip));
                }

            }
        }
    }
}

void CNoteApp::RegisterTipCallback(std::function<void (const ST_NOTE_DATA &)> fnTip)
{
    if (fnTip)
    {
        m_vecfnTip.push_back(fnTip);
    }
}

void CNoteApp::SetCache(std::shared_ptr<IDataCache> pCache)
{
    if (!pCache)
    {
        return;
    }
    m_pCache = pCache;
}