#include "stdafx.h"
#include "SettingsDialog.h"
#include "Configurator.h"
#include "NomIMEDeployer.h"
#include <NomIMEUtility.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>

namespace {

// The SinoNom schema loads this optional file through `__patch:
// sinoime_match:/patch?`. Without it (or with completion off) the schema
// suggests exact matches only; with completion on it also suggests related
// candidates (prefix completion and diacritic-insensitive spelling).
std::filesystem::path MatchFilePath() {
  return NomIMEUserDataPath() / L"sinoime_match.yaml";
}

bool ReadRelatedSuggestions() {
  std::ifstream in(MatchFilePath());
  std::string line;
  while (std::getline(in, line)) {
    size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos || line[first] == '#')
      continue;
    if (line.find("translator/enable_completion") != std::string::npos &&
        line.find("true") != std::string::npos)
      return true;
  }
  return false;
}

bool WriteRelatedSuggestions(bool related) {
  std::ofstream out(MatchFilePath(), std::ios::binary | std::ios::trunc);
  if (!out)
    return false;
  out << "# Managed by NomIME Settings (suggestion mode). Do not edit by hand.\n"
         "patch:\n";
  if (related) {
    out << "  translator/enable_completion: true\n"
           "  translator/exact_match_only: false\n"
           "  speller/algebra/+:\n"
           "    - fuzz/aw/a/\n"
           "    - fuzz/aa/a/\n"
           "    - fuzz/ee/e/\n"
           "    - fuzz/oo/o/\n"
           "    - fuzz/uw/u/\n"
           "    - fuzz/ow/o/\n"
           "    - fuzz/dd/d/\n";
  } else {
    out << "  translator/enable_completion: false\n"
           "  translator/exact_match_only: true\n";
  }
  return !!out;
}

// The font boxes show only the primary font of a `font_face` fallback chain.
// Saving a different primary font keeps the rest of the chain; saving the
// same one leaves the setting untouched.
std::wstring ComposeFontFace(const std::wstring& original,
                             const std::wstring& selected) {
  size_t comma = original.find(L',');
  std::wstring primary = original.substr(0, comma);
  std::wstring tail =
      comma == std::wstring::npos ? std::wstring() : original.substr(comma);
  if (selected == primary)
    return original;
  return selected + tail;
}

// {system, vie, eng, chs, cht} registry values understood by get_language_id().
const wchar_t* const kLanguageValues[] = {L"system", L"vie", L"eng", L"chs", L"cht"};
const wchar_t* const kLanguageNames[] = {L"Theo hệ thống / System", L"Tiếng Việt",
                                         L"English", L"简体中文", L"繁體中文"};
const int kLanguageCount = 5;

int CALLBACK EnumFontFamProc(const LOGFONT* lf,
                             const TEXTMETRIC*,
                             DWORD,
                             LPARAM lParam) {
  std::vector<std::wstring>* names =
      reinterpret_cast<std::vector<std::wstring>*>(lParam);
  if (lf->lfFaceName[0] != L'@')  // skip vertical-writing font variants
    names->push_back(lf->lfFaceName);
  return 1;
}

}  // namespace

// Earlier versions wrote `derive` rules, which cannot tell related spellings
// from exact ones. Rewrites such a file in the current format.
void MigrateSuggestionSettings() {
  bool has_derive = false;
  {
    std::ifstream in(MatchFilePath());
    std::string line;
    while (std::getline(in, line)) {
      if (line.find("derive/") != std::string::npos)
        has_derive = true;
    }
  }
  if (has_derive)
    WriteRelatedSuggestions(ReadRelatedSuggestions());
}

SettingsDialog::SettingsDialog(RimeSwitcherSettings* switcher_settings,
                               UIStyleSettings* ui_style_settings)
    : switcher_settings_(switcher_settings),
      ui_style_settings_(ui_style_settings),
      loaded_(false),
      schema_modified_(false),
      related_initial_(false),
      suggestion_changed_(false) {
  api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
}

SettingsDialog::~SettingsDialog() {
  preview_image_.Destroy();
}

void SettingsDialog::PopulateSchemata() {
  if (!switcher_settings_)
    return;
  RimeSchemaList available = {0};
  api_->get_available_schema_list(switcher_settings_, &available);
  RimeSchemaList selected = {0};
  api_->get_selected_schema_list(switcher_settings_, &selected);
  schema_list_.DeleteAllItems();
  size_t k = 0;
  std::set<RimeSchemaInfo*> recruited;
  for (size_t i = 0; i < selected.size; ++i) {
    const char* schema_id = selected.list[i].schema_id;
    for (size_t j = 0; j < available.size; ++j) {
      RimeSchemaListItem& item(available.list[j]);
      RimeSchemaInfo* info = (RimeSchemaInfo*)item.reserved;
      if (!strcmp(item.schema_id, schema_id) &&
          recruited.find(info) == recruited.end()) {
        recruited.insert(info);
        std::wstring itemwstr = u8tow(item.name);
        schema_list_.AddItem((int)k, 0, itemwstr.c_str());
        schema_list_.SetItemData((int)k, (DWORD_PTR)info);
        schema_list_.SetCheckState((int)k, TRUE);
        ++k;
        break;
      }
    }
  }
  for (size_t i = 0; i < available.size; ++i) {
    RimeSchemaListItem& item(available.list[i]);
    RimeSchemaInfo* info = (RimeSchemaInfo*)item.reserved;
    if (recruited.find(info) == recruited.end()) {
      recruited.insert(info);
      std::wstring itemwstr = u8tow(item.name);
      schema_list_.AddItem((int)k, 0, itemwstr.c_str());
      schema_list_.SetItemData((int)k, (DWORD_PTR)info);
      ++k;
    }
  }
  auto hotkeys_str = api_->get_hotkeys(switcher_settings_);
  if (hotkeys_str) {
    std::wstring txt = u8tow(hotkeys_str);
    hotkeys_.SetWindowTextW(txt.c_str());
  }
  loaded_ = true;
  schema_modified_ = false;
}

void SettingsDialog::ShowSchemaDetails(RimeSchemaInfo* info) {
  if (!info)
    return;
  std::string details;
  if (const char* name = api_->get_schema_name(info)) {
    details += name;
  }
  if (const char* author = api_->get_schema_author(info)) {
    (details += "\n\n") += author;
  }
  if (const char* description = api_->get_schema_description(info)) {
    (details += "\n\n") += description;
  }
  std::wstring txt = u8tow(details.c_str());
  schema_description_.SetWindowTextW(txt.c_str());
}

void SettingsDialog::PopulateColorSchemes() {
  if (!ui_style_settings_)
    return;
  std::string active(ui_style_settings_->GetActiveColorScheme());
  int active_index = -1;
  ui_style_settings_->GetPresetColorSchemes(&preset_);
  for (size_t i = 0; i < preset_.size(); ++i) {
    std::wstring txt = u8tow(preset_[i].name);
    color_schemes_.AddString(txt.c_str());
    if (preset_[i].color_scheme_id == active) {
      active_index = (int)i;
    }
  }
  if (active_index >= 0) {
    color_schemes_.SetCurSel(active_index);
    PreviewColorScheme(active_index);
  }
}

void SettingsDialog::PreviewColorScheme(int index) {
  if (index < 0 || index >= (int)preset_.size())
    return;
  const std::string file_path(
      ui_style_settings_->GetColorSchemePreview(preset_[index].color_scheme_id));
  if (file_path.empty())
    return;
  preview_image_.Destroy();
  // it is from ansi coding, not utf8
  preview_image_.Load(acptow(file_path).c_str());
  if (!preview_image_.IsNull()) {
    preview_.SetBitmap(preview_image_);
  }
}

// Shrinks the scheme list to the height of the preview picture and pulls
// everything below it up, so the section takes no more room than it needs.
void SettingsDialog::FitColorSchemeList() {
  const int picture_h = preview_image_.IsNull() ? 232 : preview_image_.GetHeight();
  CRect list_rc, preview_rc, client_rc;
  color_schemes_.GetWindowRect(&list_rc);
  ScreenToClient(&list_rc);
  preview_.GetWindowRect(&preview_rc);
  color_schemes_.GetClientRect(&client_rc);
  const int item_h = color_schemes_.GetItemHeight(0);
  if (item_h <= 0)
    return;
  const int frame_h = list_rc.Height() - client_rc.Height();
  const int rows = (std::max)(1, (picture_h - frame_h + item_h / 2) / item_h);
  const int new_h = rows * item_h + frame_h;
  const int delta = list_rc.Height() - new_h;
  if (delta <= 0)
    return;
  const CRect old_list = list_rc;
  color_schemes_.SetWindowPos(NULL, 0, 0, old_list.Width(), new_h,
                              SWP_NOMOVE | SWP_NOZORDER);
  preview_.SetWindowPos(NULL, 0, 0, preview_rc.Width(), new_h,
                        SWP_NOMOVE | SWP_NOZORDER);
  for (HWND child = GetWindow(GW_CHILD); child;
       child = ::GetWindow(child, GW_HWNDNEXT)) {
    if (child == color_schemes_.m_hWnd || child == preview_.m_hWnd)
      continue;
    CRect rc;
    ::GetWindowRect(child, &rc);
    ScreenToClient(&rc);
    if (rc.top >= old_list.bottom) {
      ::SetWindowPos(child, NULL, rc.left, rc.top - delta, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER);
    } else if (rc.top < old_list.top && rc.bottom >= old_list.bottom) {
      // the group box around the section
      ::SetWindowPos(child, NULL, 0, 0, rc.Width(), rc.Height() - delta,
                     SWP_NOMOVE | SWP_NOZORDER);
    }
  }
  CRect window_rc;
  GetWindowRect(&window_rc);
  SetWindowPos(NULL, 0, 0, window_rc.Width(), window_rc.Height() - delta,
               SWP_NOMOVE | SWP_NOZORDER);
}

void SettingsDialog::PopulateFontCombo(CComboBox& combo,
                                       const std::wstring& current) {
  std::vector<std::wstring> names;
  HDC hdc = ::GetDC(NULL);
  LOGFONT lf = {0};
  lf.lfCharSet = DEFAULT_CHARSET;
  ::EnumFontFamiliesExW(hdc, &lf, EnumFontFamProc, (LPARAM)&names, 0);
  ::ReleaseDC(NULL, hdc);
  std::sort(names.begin(), names.end());
  names.erase(std::unique(names.begin(), names.end()), names.end());
  for (const auto& name : names) {
    combo.AddString(name.c_str());
  }
  // font_face may hold a fallback chain; show only the primary font here.
  std::wstring primary = current;
  size_t comma = primary.find(L',');
  if (comma != std::wstring::npos)
    primary = primary.substr(0, comma);
  combo.SetWindowTextW(primary.c_str());
}

void SettingsDialog::PopulateLanguage() {
  for (int i = 0; i < kLanguageCount; ++i) {
    language_.AddString(kLanguageNames[i]);
  }
  std::wstring lang;
  RegGetStringValue(HKEY_CURRENT_USER, L"Software\\SinoNom\\NomIME",
                    L"Language", lang);
  int sel = 1;  // Vietnamese unless another language was chosen
  for (int i = 0; i < kLanguageCount; ++i) {
    if (lang == kLanguageValues[i]) {
      sel = i;
      break;
    }
  }
  language_.SetCurSel(sel);
}

LRESULT SettingsDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
  schema_list_.SubclassWindow(GetDlgItem(IDC_SCHEMA_LIST));
  schema_list_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT,
                                        LVS_EX_FULLROWSELECT);
  CString schema_name = LoadStr(IDS_STR_SCHEMA_NAME);
  schema_list_.AddColumn(schema_name, 0);
  CRect rc;
  schema_list_.GetClientRect(&rc);
  schema_list_.SetColumnWidth(0, rc.Width() - 20);

  schema_description_.Attach(GetDlgItem(IDC_SCHEMA_DESCRIPTION));
  hotkeys_.Attach(GetDlgItem(IDC_HOTKEYS));
  hotkeys_.EnableWindow(FALSE);
  get_schemata_.Attach(GetDlgItem(IDC_GET_SCHEMATA));
  get_schemata_.EnableWindow(TRUE);
  PopulateSchemata();

  suggest_exact_.Attach(GetDlgItem(IDC_SUGGEST_EXACT));
  suggest_related_.Attach(GetDlgItem(IDC_SUGGEST_RELATED));
  related_initial_ = ReadRelatedSuggestions();
  CheckRadioButton(IDC_SUGGEST_EXACT, IDC_SUGGEST_RELATED,
                   related_initial_ ? IDC_SUGGEST_RELATED : IDC_SUGGEST_EXACT);

  color_schemes_.Attach(GetDlgItem(IDC_COLOR_SCHEME));
  preview_.Attach(GetDlgItem(IDC_PREVIEW));
  PopulateColorSchemes();
  FitColorSchemeList();

  font_face_.Attach(GetDlgItem(IDC_FONT_FACE));
  font_point_.Attach(GetDlgItem(IDC_FONT_POINT));
  comment_font_face_.Attach(GetDlgItem(IDC_COMMENT_FONT_FACE));
  comment_font_point_.Attach(GetDlgItem(IDC_COMMENT_FONT_POINT));
  layout_horizontal_.Attach(GetDlgItem(IDC_LAYOUT_HORIZONTAL));
  layout_vertical_.Attach(GetDlgItem(IDC_LAYOUT_VERTICAL));
  show_comment_.Attach(GetDlgItem(IDC_SHOW_COMMENT));

  if (ui_style_settings_) {
    font_face_orig_ = ui_style_settings_->GetFontFace();
    // settings saved by an earlier build lost the fallback chain
    if (!font_face_orig_.empty() &&
        font_face_orig_.find(L',') == std::wstring::npos)
      font_face_orig_ += L", Hana Pro Ext, Microsoft YaHei, Microsoft YaHei UI, "
                         L"SimSun, NSimSun, Arial";
    comment_font_face_orig_ = ui_style_settings_->GetCommentFontFace();
    PopulateFontCombo(font_face_, font_face_orig_);
    PopulateFontCombo(comment_font_face_, comment_font_face_orig_);
    SetDlgItemInt(IDC_FONT_POINT, ui_style_settings_->GetFontPoint(), FALSE);
    SetDlgItemInt(IDC_COMMENT_FONT_POINT,
                 ui_style_settings_->GetCommentFontPoint(), FALSE);
    CheckRadioButton(IDC_LAYOUT_HORIZONTAL, IDC_LAYOUT_VERTICAL,
                     ui_style_settings_->GetHorizontal()
                         ? IDC_LAYOUT_HORIZONTAL
                         : IDC_LAYOUT_VERTICAL);
    show_comment_.SetCheck(ui_style_settings_->GetShowComment() ? BST_CHECKED
                                                                 : BST_UNCHECKED);
  }

  language_.Attach(GetDlgItem(IDC_LANGUAGE));
  PopulateLanguage();

  CenterWindow();
  BringWindowToTop();
  return TRUE;
}

LRESULT SettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
  EndDialog(IDCANCEL);
  return 0;
}

LRESULT SettingsDialog::OnGetSchemata(WORD, WORD, HWND hWndCtl, BOOL&) {
  HKEY hKey;
  std::wstring hPath;
  if (is_wow64())
    hPath = _T("Software\\WOW6432Node\\SinoNom\\NomIME");
  else
    hPath = _T("Software\\SinoNom\\NomIME");
  LSTATUS ret = RegOpenKey(HKEY_LOCAL_MACHINE, hPath.c_str(), &hKey);
  if (ret == ERROR_SUCCESS) {
    WCHAR value[MAX_PATH];
    DWORD len = sizeof(value);
    DWORD type = 0;
    ret =
        RegQueryValueExW(hKey, L"NomIMERoot", NULL, &type, (LPBYTE)value, &len);
    if (ret == ERROR_SUCCESS && type == REG_SZ) {
      WCHAR parameters[MAX_PATH + 37];
      wcscpy_s<_countof(parameters)>(
          parameters,
          (std::wstring(L"/k \"") + value + L"\\rime-install.bat\"").c_str());
      SHELLEXECUTEINFOW cmd = {sizeof(SHELLEXECUTEINFO),
                               SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC,
                               hWndCtl,
                               L"open",
                               L"cmd",
                               parameters,
                               NULL,
                               SW_SHOW,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL};
      ShellExecuteExW(&cmd);
      WaitForSingleObject(cmd.hProcess, INFINITE);
      CloseHandle(cmd.hProcess);
      api_->load_settings(reinterpret_cast<RimeCustomSettings*>(switcher_settings_));
      PopulateSchemata();
    }
  }
  RegCloseKey(hKey);
  return 0;
}

LRESULT SettingsDialog::OnColorSchemeSelChange(WORD, WORD, HWND, BOOL&) {
  int index = color_schemes_.GetCurSel();
  if (index >= 0 && index < (int)preset_.size()) {
    ui_style_settings_->SelectColorScheme(preset_[index].color_scheme_id);
    PreviewColorScheme(index);
  }
  return 0;
}

LRESULT SettingsDialog::OnSchemaListItemChanged(int, LPNMHDR p, BOOL&) {
  LPNMLISTVIEW lv = reinterpret_cast<LPNMLISTVIEW>(p);
  if (!loaded_ || !lv || lv->iItem < 0 ||
      lv->iItem >= schema_list_.GetItemCount())
    return 0;
  if ((lv->uNewState & LVIS_STATEIMAGEMASK) !=
      (lv->uOldState & LVIS_STATEIMAGEMASK)) {
    schema_modified_ = true;
  } else if ((lv->uNewState & LVIS_SELECTED) &&
             !(lv->uOldState & LVIS_SELECTED)) {
    ShowSchemaDetails((RimeSchemaInfo*)(schema_list_.GetItemData(lv->iItem)));
  }
  return 0;
}

LRESULT SettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) {
  if (schema_modified_ && switcher_settings_ &&
      schema_list_.GetItemCount() != 0) {
    std::vector<const char*> selection;
    selection.reserve(schema_list_.GetItemCount());
    for (int i = 0; i < schema_list_.GetItemCount(); ++i) {
      if (!schema_list_.GetCheckState(i))
        continue;
      RimeSchemaInfo* info = (RimeSchemaInfo*)(schema_list_.GetItemData(i));
      if (info) {
        selection.push_back(api_->get_schema_id(info));
      }
    }
    if (selection.empty()) {
      MSG_BY_IDS(IDS_STR_ERR_AT_LEAST_ONE_SEL, IDS_STR_NOT_REGULAR,
                 MB_OK | MB_ICONEXCLAMATION);
      return 0;
    }
    api_->select_schemas(switcher_settings_, selection.data(),
                         (int)selection.size());
  }

  if (ui_style_settings_) {
    CString font_face_str;
    font_face_.GetWindowTextW(font_face_str);
    if (font_face_str.GetLength() > 0)
      ui_style_settings_->SetFontFace(
          ComposeFontFace(font_face_orig_, font_face_str.GetString()));

    CString comment_font_face_str;
    comment_font_face_.GetWindowTextW(comment_font_face_str);
    if (comment_font_face_str.GetLength() > 0)
      ui_style_settings_->SetCommentFontFace(ComposeFontFace(
          comment_font_face_orig_, comment_font_face_str.GetString()));

    BOOL translated = FALSE;
    UINT font_point = GetDlgItemInt(IDC_FONT_POINT, &translated, FALSE);
    if (translated)
      ui_style_settings_->SetFontPoint(
          (int)std::clamp<UINT>(font_point, 6, 96));

    translated = FALSE;
    UINT comment_font_point =
        GetDlgItemInt(IDC_COMMENT_FONT_POINT, &translated, FALSE);
    if (translated)
      ui_style_settings_->SetCommentFontPoint(
          (int)std::clamp<UINT>(comment_font_point, 6, 96));

    ui_style_settings_->SetHorizontal(
        IsDlgButtonChecked(IDC_LAYOUT_HORIZONTAL) == BST_CHECKED);
    ui_style_settings_->SetShowComment(show_comment_.GetCheck() ==
                                       BST_CHECKED);
  }

  bool related = IsDlgButtonChecked(IDC_SUGGEST_RELATED) == BST_CHECKED;
  if (related != related_initial_ && WriteRelatedSuggestions(related))
    suggestion_changed_ = true;

  int lang_sel = language_.GetCurSel();
  if (lang_sel >= 0 && lang_sel < kLanguageCount) {
    SetUserLanguagePreference(kLanguageValues[lang_sel]);
  }

  EndDialog(code);
  return 0;
}
