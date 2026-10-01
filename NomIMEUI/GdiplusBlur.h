#pragma once
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace nomime {
void DoGaussianBlur(Gdiplus::Bitmap* img, float radiusX, float radiusY);
}
