#pragma once
#include <NomIMEUI.h>
#include <NomIMEIPC.h>
#include "SystemTraySDK.h"

#define WM_NOMIME_TRAY_NOTIFY (NOMIME_IPC_LAST_COMMAND + 100)

class NomIMETrayIcon : public CSystemTray {
 public:
  enum NomIMETrayMode {
    INITIAL,
    ZHUNG,
    ASCII,
    DISABLED,
  };

  NomIMETrayIcon(nomime::UI& ui);

  BOOL Create(HWND hTargetWnd);
  void Refresh();

 protected:
  virtual void CustomizeMenu(HMENU hMenu);

  nomime::UIStyle& m_style;
  nomime::Status& m_status;
  NomIMETrayMode m_mode;
  std::wstring m_schema_zhung_icon;
  std::wstring m_schema_ascii_icon;
  bool m_disabled;
};
