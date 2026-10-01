#include "stdafx.h"
#include "NomIMEServerApp.h"
#include <NomIMEUtility.h>
#include <filesystem>

namespace {
// Public half of the EdDSA (ed25519) keypair used to verify signed update
// packages. The private key never lives in this repository.
const char kUpdateEdDSAPubKey[] =
    "qWwxh7uOOUXuSUam7LzamMMBgz5+/enFYkTk4oAbDXE=";
}  // namespace

NomIMEServerApp::NomIMEServerApp()
    : m_handler(std::make_unique<RimeWithNomIMEHandler>(&m_ui)),
      tray_icon(m_ui) {
  // m_handler.reset(new RimeWithNomIMEHandler(&m_ui));
  m_server.SetRequestHandler(m_handler.get());
  SetupMenuHandlers();
}

NomIMEServerApp::~NomIMEServerApp() {}

int NomIMEServerApp::Run() {
  if (!m_server.Start())
    return -1;

  // win_sparkle_set_appcast_url("http://localhost:8000/nomime/update/appcast.xml");
  win_sparkle_set_registry_path("Software\\SinoNom\\NomIME\\Updates");
  win_sparkle_set_eddsa_public_key(kUpdateEdDSAPubKey);
  // follow the language chosen in Settings (WinSparkle has no Vietnamese)
  const LANGID ui_lang = get_language_id();
  if (ui_lang == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL))
    win_sparkle_set_lang("zh-TW");
  else if (ui_lang == MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED))
    win_sparkle_set_lang("zh-CN");
  else
    win_sparkle_set_lang("en");
  win_sparkle_init();
  m_ui.Create(m_server.GetHWnd());

  m_handler->Initialize();
  m_handler->OnUpdateUI([this]() { tray_icon.Refresh(); });

  tray_icon.Create(m_server.GetHWnd());
  tray_icon.Refresh();

  int ret = m_server.Run();

  m_handler->Finalize();
  m_ui.Destroy();
  tray_icon.RemoveIcon();
  win_sparkle_cleanup();

  return ret;
}

void NomIMEServerApp::SetupMenuHandlers() {
  std::filesystem::path dir = install_dir();
  m_server.AddMenuHandler(ID_NOMIMETRAY_QUIT,
                          [this] { return m_server.Stop() == 0; });
  m_server.AddMenuHandler(ID_NOMIMETRAY_DEPLOY,
                          std::bind(execute, dir / L"NomIMEDeployer.exe",
                                    std::wstring(L"/deploy")));
  m_server.AddMenuHandler(
      ID_NOMIMETRAY_SETTINGS,
      std::bind(execute, dir / L"NomIMEDeployer.exe", std::wstring()));
  m_server.AddMenuHandler(
      ID_NOMIMETRAY_DICT_MANAGEMENT,
      std::bind(execute, dir / L"NomIMEDeployer.exe", std::wstring(L"/dict")));
  m_server.AddMenuHandler(
      ID_NOMIMETRAY_SYNC,
      std::bind(execute, dir / L"NomIMEDeployer.exe", std::wstring(L"/sync")));
  m_server.AddMenuHandler(ID_NOMIMETRAY_WIKI,
                          std::bind(open, L"https://bogo.hannom.org/docs/"));
  m_server.AddMenuHandler(ID_NOMIMETRAY_HOMEPAGE,
                          std::bind(open, L"https://bogo.hannom.org/"));
  m_server.AddMenuHandler(
      ID_NOMIMETRAY_FORUM,
      std::bind(open, L"https://facebook.com/groups/phuongviencac/"));
  m_server.AddMenuHandler(ID_NOMIMETRAY_CHECKUPDATE, check_update);
  m_server.AddMenuHandler(ID_NOMIMETRAY_INSTALLDIR, std::bind(explore, dir));
  m_server.AddMenuHandler(ID_NOMIMETRAY_USERCONFIG,
                          std::bind(explore, NomIMEUserDataPath()));
  m_server.AddMenuHandler(ID_NOMIMETRAY_LOGDIR,
                          std::bind(explore, NomIMELogPath()));
}
