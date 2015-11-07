#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

#include <ribbon.h>
#include <uiapplication.h>
#include <uicommandhandler.h>

HRESULT UIApplication::CreateInstance(IUIApplication **ppApplication)
{
    *ppApplication = NULL;

    HRESULT hr = S_OK;

    UIApplication* pApplication = new UIApplication();

    if (pApplication != NULL)
    {
        *ppApplication = static_cast<IUIApplication *>(pApplication);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

STDMETHODIMP_(ULONG) UIApplication::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) UIApplication::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP UIApplication::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IUIApplication))
    {
        *ppv = static_cast<IUIApplication*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP UIApplication::OnCreateUICommand(
    UINT nCmdID,
    UI_COMMANDTYPE typeID,
    IUICommandHandler** ppCommandHandler)
{
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(nCmdID);

    if (NULL == m_pCommandHandler)
    {
        HRESULT hr = UICommandHandler::CreateInstance(&m_pCommandHandler);
        if (FAILED(hr)) return hr;
    }

    return m_pCommandHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
}

STDMETHODIMP UIApplication::OnViewChanged(
    UINT viewId,
    UI_VIEWTYPE typeId,
    IUnknown* pView,
    UI_VIEWVERB verb,
    INT uReasonCode)
{
    UNREFERENCED_PARAMETER(uReasonCode);
    UNREFERENCED_PARAMETER(viewId);

    HRESULT hr = E_NOTIMPL;

    if (UI_VIEWTYPE_RIBBON == typeId)
    {
        switch (verb)
        {
        case UI_VIEWVERB_CREATE:
            hr = S_OK;
            hr = pView->QueryInterface(&StupidPlot::Ribbon::g_pRibbon);
            // hide ribbon
            {
                IPropertyStore *pPropertyStore = NULL;
                hr = StupidPlot::Ribbon::g_pRibbon->QueryInterface(__uuidof(IPropertyStore), (void**)&pPropertyStore);
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT propvar;
                    PropVariantInit(&propvar);
                    UIInitPropertyFromBoolean(UI_PKEY_Viewable, FALSE, &propvar);
                    hr = pPropertyStore->SetValue(UI_PKEY_Viewable, propvar);
                    pPropertyStore->Commit();
                    pPropertyStore->Release();
                }
            }
            break;
        case UI_VIEWVERB_DESTROY:
            hr = S_OK;
            StupidPlot::Ribbon::g_pRibbon = NULL;
            break;
        }
    }

    return hr;
}

STDMETHODIMP UIApplication::OnDestroyUICommand(
    UINT32 nCmdID,
    UI_COMMANDTYPE typeID,
    IUICommandHandler* commandHandler)
{
    UNREFERENCED_PARAMETER(commandHandler);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(nCmdID);

    return E_NOTIMPL;
}