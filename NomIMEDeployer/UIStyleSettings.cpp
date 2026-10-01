#include "stdafx.h"
#include <NomIMEUtility.h>
#include "UIStyleSettings.h"

UIStyleSettings::UIStyleSettings() {
  api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
  settings_ = api_->custom_settings_init("nomime", "NomIME::UIStyleSettings");
}

bool UIStyleSettings::GetPresetColorSchemes(
    std::vector<ColorSchemeInfo>* result) {
  if (!result)
    return false;
  result->clear();
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  RimeApi* rime = rime_get_api();
  RimeConfigIterator preset = {0};
  if (!rime->config_begin_map(&preset, &config, "preset_color_schemes")) {
    return false;
  }
  while (rime->config_next(&preset)) {
    std::string name_key(preset.path);
    name_key += "/name";
    const char* name = rime->config_get_cstring(&config, name_key.c_str());
    std::string author_key(preset.path);
    author_key += "/author";
    const char* author = rime->config_get_cstring(&config, author_key.c_str());
    if (!name)
      continue;
    ColorSchemeInfo info;
    info.color_scheme_id = preset.key;
    info.name = name;
    if (author)
      info.author = author;
    result->push_back(info);
  }
  return true;
}

// check if a file exists
static inline bool IfFileExist(std::string filename) {
  DWORD dwAttrib = GetFileAttributes(acptow(filename).c_str());
  return (INVALID_FILE_ATTRIBUTES != dwAttrib &&
          0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// get preview image from user dir first, then shared_dir
std::string UIStyleSettings::GetColorSchemePreview(
    const std::string& color_scheme_id) {
  std::string shared_dir = rime_get_api()->get_shared_data_dir();
  std::string user_dir = rime_get_api()->get_user_data_dir();
  std::string filename =
      user_dir + "\\preview\\color_scheme_" + color_scheme_id + ".png";
  if (IfFileExist(filename))
    return filename;
  else
    return (shared_dir + "\\preview\\color_scheme_" + color_scheme_id + ".png");
}

std::string UIStyleSettings::GetActiveColorScheme() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  const char* value =
      rime_get_api()->config_get_cstring(&config, "style/color_scheme");
  if (!value)
    return std::string();
  return std::string(value);
}

bool UIStyleSettings::SelectColorScheme(const std::string& color_scheme_id) {
  api_->customize_string(settings_, "style/color_scheme",
                         color_scheme_id.c_str());
  return true;
}

std::wstring UIStyleSettings::GetFontFace() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  const char* value =
      rime_get_api()->config_get_cstring(&config, "style/font_face");
  return value ? u8tow(value) : std::wstring();
}

void UIStyleSettings::SetFontFace(const std::wstring& font_face) {
  api_->customize_string(settings_, "style/font_face",
                         wtou8(font_face).c_str());
}

int UIStyleSettings::GetFontPoint() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  int value = 0;
  rime_get_api()->config_get_int(&config, "style/font_point", &value);
  return value;
}

void UIStyleSettings::SetFontPoint(int font_point) {
  api_->customize_int(settings_, "style/font_point", font_point);
}

std::wstring UIStyleSettings::GetCommentFontFace() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  const char* value =
      rime_get_api()->config_get_cstring(&config, "style/comment_font_face");
  return value ? u8tow(value) : std::wstring();
}

void UIStyleSettings::SetCommentFontFace(const std::wstring& font_face) {
  api_->customize_string(settings_, "style/comment_font_face",
                         wtou8(font_face).c_str());
}

int UIStyleSettings::GetCommentFontPoint() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  int value = 0;
  rime_get_api()->config_get_int(&config, "style/comment_font_point", &value);
  return value;
}

void UIStyleSettings::SetCommentFontPoint(int font_point) {
  api_->customize_int(settings_, "style/comment_font_point", font_point);
}

bool UIStyleSettings::GetHorizontal() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  Bool value = False;
  rime_get_api()->config_get_bool(&config, "style/horizontal", &value);
  return !!value;
}

void UIStyleSettings::SetHorizontal(bool horizontal) {
  api_->customize_bool(settings_, "style/horizontal", horizontal);
}

bool UIStyleSettings::GetShowComment() {
  RimeConfig config = {0};
  api_->settings_get_config(settings_, &config);
  Bool value = True;
  rime_get_api()->config_get_bool(&config, "style/show_comment", &value);
  return !!value;
}

void UIStyleSettings::SetShowComment(bool show_comment) {
  api_->customize_bool(settings_, "style/show_comment", show_comment);
}
