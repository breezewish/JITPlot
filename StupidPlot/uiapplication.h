#pragma once

#include <resource_ribbon.h>

class UIApplication : public IUIApplication
{
public:
    static HRESULT CreateInstance(IUIApplication **ppApplication);

    IFACEMETHODIMP QueryInterface(REFIID iid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    STDMETHOD(OnCreateUICommand)(UINT nCmdID,
        UI_COMMANDTYPE typeID,
        IUICommandHandler** ppCommandHandler);

    STDMETHOD(OnViewChanged)(UINT viewId,
        UI_VIEWTYPE typeId,
        IUnknown* pView,
        UI_VIEWVERB verb,
        INT uReasonCode);

    STDMETHOD(OnDestroyUICommand)(UINT32 commandId,
        UI_COMMANDTYPE typeID,
        IUICommandHandler* commandHandler);

private:
    UIApplication()
        : m_cRef(1)
        , m_pCommandHandler(NULL)
    {
    }

    ~UIApplication()
    {
        if (m_pCommandHandler)
        {
            m_pCommandHandler->Release();
            m_pCommandHandler = NULL;
        }
    }

    LONG m_cRef;
    IUICommandHandler * m_pCommandHandler;
};
