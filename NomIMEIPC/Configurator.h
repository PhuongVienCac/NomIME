#pragma once
#include "Deserializer.h"

class Configurator : public nomime::Deserializer {
 public:
  Configurator(nomime::ResponseParser* pTarget);
  virtual ~Configurator();
  // store data
  virtual void Store(nomime::Deserializer::KeyType const& key,
                     std::wstring const& value);
  // factory method
  static nomime::Deserializer::Ptr Create(nomime::ResponseParser* pTarget);
};
