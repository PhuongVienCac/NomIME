#include "stdafx.h"
#include "NomIMEDeployer.h"
#include "Configurator.h"
#include "SettingsReset.h"
#include "UIStyleSettings.h"
#include "SettingsDialog.h"
#include "DictManagementDialog.h"
#include <NomIMEConstants.h>
#include <NomIMEDataVaultHost.h>
#include <NomIMEIPC.h>
#include <NomIMEIPCData.h>
#include <NomIMEUtility.h>
#pragma warning(disable : 4005)
#include <rime_api.h>
#include <rime_levers_api.h>
#pragma warning(default : 4005)
#include <filesystem>
#include <fstream>
#include "NomIMEDeployer.h"

static void CreateFileIfNotExist(std::string filename) {
  std::filesystem::path file_path = NomIMEUserDataPath() / u8tow(filename);
  DWORD dwAttrib = GetFileAttributes(file_path.c_str());
  if (!(INVALID_FILE_ATTRIBUTES != dwAttrib &&
        0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) {
    std::wofstream o(file_path.c_str(), std::ios::app);
    o.close();
  }
}
Configurator::Configurator() {
  // Before anything reads or recreates them: settings left by an older version
  // would otherwise keep overriding this one's defaults.
  ResetSettingsOnVersionChange();
  CreateFileIfNotExist("default.custom.yaml");
  CreateFileIfNotExist("nomime.custom.yaml");
}

void Configurator::Initialize() {
  MigrateSuggestionSettings();
  // Must precede setup(): the SinoNom schema and tables are served from the
  // embedded vault, not from the data folder.
  nomime::vault::InstallFromModule();
  RIME_STRUCT(RimeTraits, nomime_traits);
  std::string shared_dir = wtou8(NomIMESharedDataPath().wstring());
  std::string user_dir = wtou8(NomIMEUserDataPath().wstring());
  nomime_traits.shared_data_dir = shared_dir.c_str();
  nomime_traits.user_data_dir = user_dir.c_str();
  nomime_traits.prebuilt_data_dir = nomime_traits.shared_data_dir;
  std::string distribution_name = wtou8(get_nomime_ime_name());
  nomime_traits.distribution_name = distribution_name.c_str();
  nomime_traits.distribution_code_name = NOMIME_CODE_NAME;
  nomime_traits.distribution_version = NOMIME_VERSION;
  nomime_traits.app_name = "phuongvien.nomime";
  std::string log_dir = NomIMELogPath().u8string();
  nomime_traits.log_dir = log_dir.c_str();
  RimeApi* rime_api = rime_get_api();
  assert(rime_api);
  rime_api->setup(&nomime_traits);
  LOG(INFO) << "NomIMEDeployer reporting.";
  rime_api->deployer_initialize(NULL);
}

static bool configure_settings(RimeLeversApi* api,
                               RimeSwitcherSettings* switcher_settings,
                               UIStyleSettings* ui_style_settings,
                               bool* reconfigured) {
  RimeCustomSettings* switcher = (RimeCustomSettings*)switcher_settings;
  RimeCustomSettings* style = ui_style_settings->settings();
  if (!api->load_settings(switcher) || !api->load_settings(style))
    return false;
  SettingsDialog dialog(switcher_settings, ui_style_settings);
  if (dialog.DoModal() == IDOK) {
    bool switcher_saved = api->save_settings(switcher);
    bool style_saved = api->save_settings(style);
    if (switcher_saved || style_saved || dialog.suggestion_changed())
      *reconfigured = true;
    return true;
  }
  return false;
}

int Configurator::Run(bool installing) {
  RimeModule* levers = rime_get_api()->find_module("levers");
  if (!levers)
    return 1;
  RimeLeversApi* api = (RimeLeversApi*)levers->get_api();
  if (!api)
    return 1;

  bool reconfigured = false;

  RimeSwitcherSettings* switcher_settings = api->switcher_settings_init();
  UIStyleSettings ui_style_settings;

  bool skip_settings =
      installing &&
      !api->is_first_run((RimeCustomSettings*)switcher_settings) &&
      !api->is_first_run(ui_style_settings.settings());

  skip_settings || configure_settings(api, switcher_settings,
                                      &ui_style_settings, &reconfigured);

  api->custom_settings_destroy((RimeCustomSettings*)switcher_settings);

  if (installing || reconfigured) {
    return UpdateWorkspace(reconfigured);
  }
  return 0;
}

int Configurator::UpdateWorkspace(bool report_errors) {
  HANDLE hMutex = CreateMutex(NULL, TRUE, L"NomIMEDeployerMutex");
  if (!hMutex) {
    LOG(ERROR) << "Error creating NomIMEDeployerMutex.";
    return 1;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    LOG(WARNING) << "another deployer process is running; aborting operation.";
    CloseHandle(hMutex);
    if (report_errors) {
      // MessageBox(NULL,
      // L"正在執行另一項部署任務，方纔所做的修改將在輸入法再次啓動後生效。",
      // L"(Phương Viên - NomIME) ", MB_OK | MB_ICONINFORMATION);
      MSG_BY_IDS(IDS_STR_DEPLOYING_RESTARTREQ, IDS_STR_NOMIME,
                 MB_OK | MB_ICONINFORMATION);
    }
    return 1;
  }

  nomime::Client client;
  if (client.Connect()) {
    LOG(INFO) << "Turning NomIMEServer into maintenance mode.";
    client.StartMaintenance();
  }

  {
    RimeApi* rime = rime_get_api();
    // initialize default config, preset schemas
    rime->deploy();
    // initialize nomime config
    rime->deploy_config_file("nomime.yaml", "config_version");
  }

  CloseHandle(hMutex);  // should be closed before resuming service.

  if (client.Connect()) {
    LOG(INFO) << "Resuming service.";
    client.EndMaintenance();
  }
  return 0;
}

int Configurator::DictManagement() {
  HANDLE hMutex = CreateMutex(NULL, TRUE, L"NomIMEDeployerMutex");
  if (!hMutex) {
    LOG(ERROR) << "Error creating NomIMEDeployerMutex.";
    return 1;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    LOG(WARNING) << "another deployer process is running; aborting operation.";
    CloseHandle(hMutex);
    // MessageBox(NULL, L"正在執行另一項部署任務，請稍候再試。", L"(Phương Viên
    // - NomIME) ", MB_OK | MB_ICONINFORMATION);
    MSG_BY_IDS(IDS_STR_DEPLOYING_WAIT, IDS_STR_NOMIME,
               MB_OK | MB_ICONINFORMATION);
    return 1;
  }

  nomime::Client client;
  if (client.Connect()) {
    LOG(INFO) << "Turning NomIMEServer into maintenance mode.";
    client.StartMaintenance();
  }

  {
    RimeApi* rime = rime_get_api();
    if (RIME_API_AVAILABLE(rime, run_task)) {
      rime->run_task("installation_update");  // setup user data sync dir
    }
    DictManagementDialog dlg;
    dlg.DoModal();
  }

  CloseHandle(hMutex);  // should be closed before resuming service.

  if (client.Connect()) {
    LOG(INFO) << "Resuming service.";
    client.EndMaintenance();
  }
  return 0;
}

int Configurator::SyncUserData() {
  HANDLE hMutex = CreateMutex(NULL, TRUE, L"NomIMEDeployerMutex");
  if (!hMutex) {
    LOG(ERROR) << "Error creating NomIMEDeployerMutex.";
    return 1;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    LOG(WARNING) << "another deployer process is running; aborting operation.";
    CloseHandle(hMutex);
    // MessageBox(NULL, L"正在執行另一項部署任務，請稍候再試。", L"(Phương Viên
    // - NomIME) ", MB_OK | MB_ICONINFORMATION);
    MSG_BY_IDS(IDS_STR_DEPLOYING_WAIT, IDS_STR_NOMIME,
               MB_OK | MB_ICONINFORMATION);
    return 1;
  }

  nomime::Client client;
  if (client.Connect()) {
    LOG(INFO) << "Turning NomIMEServer into maintenance mode.";
    client.StartMaintenance();
  }

  {
    RimeApi* rime = rime_get_api();
    if (!rime->sync_user_data()) {
      LOG(ERROR) << "Error synching user data.";
      CloseHandle(hMutex);
      return 1;
    }
    rime->join_maintenance_thread();
  }

  CloseHandle(hMutex);  // should be closed before resuming service.

  if (client.Connect()) {
    LOG(INFO) << "Resuming service.";
    client.EndMaintenance();
  }
  return 0;
}
