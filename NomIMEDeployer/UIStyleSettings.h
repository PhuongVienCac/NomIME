#pragma once

#include <string>
#include <vector>
#include <rime_levers_api.h>

struct ColorSchemeInfo {
  std::string color_scheme_id;
  std::string name;
  std::string author;
};

class UIStyleSettings {
 public:
  UIStyleSettings();

  bool GetPresetColorSchemes(std::vector<ColorSchemeInfo>* result);
  std::string GetColorSchemePreview(const std::string& color_scheme_id);
  std::string GetActiveColorScheme();
  bool SelectColorScheme(const std::string& color_scheme_id);

  std::wstring GetFontFace();
  void SetFontFace(const std::wstring& font_face);
  int GetFontPoint();
  void SetFontPoint(int font_point);
  std::wstring GetCommentFontFace();
  void SetCommentFontFace(const std::wstring& font_face);
  int GetCommentFontPoint();
  void SetCommentFontPoint(int font_point);
  bool GetHorizontal();
  void SetHorizontal(bool horizontal);
  bool GetShowComment();
  void SetShowComment(bool show_comment);

  RimeCustomSettings* settings() { return settings_; }

 private:
  RimeLeversApi* api_;
  RimeCustomSettings* settings_;
};
