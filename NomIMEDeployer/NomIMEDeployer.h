#pragma once

#include "resource.h"
#include <string>
#include <atlstr.h>
#include <Windows.h>
#include <NomIMEUtility.h>
#include "resource.h"

// Strings/dialogs come from the language selected in NomIME (see
// get_language_id), not from the Windows display language.
inline CString LoadStr(UINT id) {
  return LoadStringLang(GetModuleHandle(NULL), id, get_language_id()).c_str();
}

#define MSG_BY_IDS(idInfo, idCap, uType)                  \
  {                                                       \
    CString info = LoadStr(idInfo);                       \
    CString cap = LoadStr(idCap);                         \
    LANGID langID = get_language_id();                    \
    MessageBoxExW(NULL, info, cap, uType, langID);        \
  }

#define MSG_ID_CAP(info, idCap, uType)                    \
  {                                                       \
    CString cap = LoadStr(idCap);                         \
    LANGID langID = get_language_id();                    \
    MessageBoxExW(NULL, info, cap, uType, langID);        \
  }

// Modal dialog whose template is taken from the language selected in NomIME.
template <class T>
class LangDialogImpl : public ATL::CDialogImpl<T> {
 public:
  INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow(),
                  LPARAM dwInitParam = NULL) {
    HINSTANCE inst = _AtlBaseModule.GetResourceInstance();
    HRSRC res = ::FindResourceExW(inst, RT_DIALOG, MAKEINTRESOURCEW(T::IDD),
                                  get_language_id());
    HGLOBAL mem = res ? ::LoadResource(inst, res) : NULL;
    if (!mem)
      return ATL::CDialogImpl<T>::DoModal(hWndParent, dwInitParam);
    if (!this->m_thunk.Init(NULL, NULL)) {
      ::SetLastError(ERROR_OUTOFMEMORY);
      return -1;
    }
    _AtlWinModule.AddCreateWndData(
        &this->m_thunk.cd, (ATL::CDialogImplBaseT<ATL::CWindow>*)this);
#ifdef _DEBUG
    this->m_bModal = true;
#endif
    return ::DialogBoxIndirectParamW(
        inst, static_cast<LPCDLGTEMPLATEW>(::LockResource(mem)), hWndParent,
        T::StartDialogProc, dwInitParam);
  }
};
