#pragma once

#define NOMIME_CODE_NAME "NomIME"
#define NOMIME_REG_KEY L"Software\\SinoNom\\NomIME"
#define RIME_REG_KEY L"Software\\SinoNom"

#define STRINGIZE(x) #x
#define VERSION_STR(x) STRINGIZE(x)
#define NOMIME_VERSION VERSION_STR(VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH)
