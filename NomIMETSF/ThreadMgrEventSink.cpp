#include "stdafx.h"
#include "NomIMETSF.h"

STDAPI NomIMETSF::OnInitDocumentMgr(ITfDocumentMgr* pDocMgr) {
  return S_OK;
}

STDAPI NomIMETSF::OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr) {
  return S_OK;
}

STDAPI NomIMETSF::OnSetFocus(ITfDocumentMgr* pDocMgrFocus,
                             ITfDocumentMgr* pDocMgrPrevFocus) {
  _InitTextEditSink(pDocMgrFocus);

  com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
  com_ptr<ITfContext> pTfContext = _GetUIContextDocument();
  if ((nullptr != pTfContext) &&
      SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr))) {
    if (pCandidateListDocumentMgr != pDocMgrFocus) {
      _HideUI();
    } else {
      _ShowUI();
    }
  }

  return S_OK;
}

STDAPI NomIMETSF::OnPushContext(ITfContext* pContext) {
  return S_OK;
}

STDAPI NomIMETSF::OnPopContext(ITfContext* pContext) {
  return S_OK;
}

BOOL NomIMETSF::_InitThreadMgrEventSink() {
  ITfSource* pSource;
  if (_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource) != S_OK)
    return FALSE;
  if (pSource->AdviseSink(IID_ITfThreadMgrEventSink,
                          (ITfThreadMgrEventSink*)this,
                          &_dwThreadMgrEventSinkCookie) != S_OK) {
    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    pSource->Release();
    return FALSE;
  }
  pSource->Release();
  return TRUE;
}

void NomIMETSF::_UninitThreadMgrEventSink() {
  ITfSource* pSource;
  if (_dwThreadMgrEventSinkCookie == TF_INVALID_COOKIE)
    return;
  if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource))) {
    pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
    pSource->Release();
  }
  _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
