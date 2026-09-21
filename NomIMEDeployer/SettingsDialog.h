#pragma once

#include "resource.h"
#include "NomIMEDeployer.h"
#include "UIStyleSettings.h"
#include <rime_levers_api.h>
#include <vector>

// Brings a suggestion-mode file written by an older version up to date.
void MigrateSuggestionSettings();

class SettingsDialog : public LangDialogImpl<SettingsDialog> {
 public:
  enum { IDD = IDD_SETTINGS };

  SettingsDialog(RimeSwitcherSettings* switcher_settings,
                 UIStyleSettings* ui_style_settings);
  ~SettingsDialog();

  // true when the exact/related suggestion mode was changed and the workspace
  // must be redeployed.
  bool suggestion_changed() const { return suggestion_changed_; }

 protected:
  BEGIN_MSG_MAP(SettingsDialog)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  MESSAGE_HANDLER(WM_CLOSE, OnClose)
  COMMAND_ID_HANDLER(IDOK, OnOK)
  COMMAND_HANDLER(IDC_GET_SCHEMATA, BN_CLICKED, OnGetSchemata)
  COMMAND_HANDLER(IDC_COLOR_SCHEME, LBN_SELCHANGE, OnColorSchemeSelChange)
  NOTIFY_HANDLER(IDC_SCHEMA_LIST, LVN_ITEMCHANGED, OnSchemaListItemChanged)
  END_MSG_MAP()

  LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnOK(WORD, WORD code, HWND, BOOL&);
  LRESULT OnGetSchemata(WORD, WORD, HWND, BOOL&);
  LRESULT OnColorSchemeSelChange(WORD, WORD, HWND, BOOL&);
  LRESULT OnSchemaListItemChanged(int, LPNMHDR, BOOL&);

  // schema (input schema) section
  void PopulateSchemata();
  void ShowSchemaDetails(RimeSchemaInfo* info);

  // appearance section
  void PopulateColorSchemes();
  void PreviewColorScheme(int index);
  void FitColorSchemeList();
  void PopulateFontCombo(CComboBox& combo, const std::wstring& current);

  // language section
  void PopulateLanguage();

  RimeLeversApi* api_;
  RimeSwitcherSettings* switcher_settings_;
  UIStyleSettings* ui_style_settings_;
  bool loaded_;
  bool schema_modified_;
  bool related_initial_;
  bool suggestion_changed_;
  std::vector<ColorSchemeInfo> preset_;
  // the font settings as loaded, including their fallback chains
  std::wstring font_face_orig_;
  std::wstring comment_font_face_orig_;

  CCheckListViewCtrl schema_list_;
  CStatic schema_description_;
  CEdit hotkeys_;
  CButton get_schemata_;

  CButton suggest_exact_;
  CButton suggest_related_;

  CListBox color_schemes_;
  CStatic preview_;
  CImage preview_image_;
  CComboBox font_face_;
  CEdit font_point_;
  CComboBox comment_font_face_;
  CEdit comment_font_point_;
  CButton layout_horizontal_;
  CButton layout_vertical_;
  CButton show_comment_;

  CComboBox language_;
};
