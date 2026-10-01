#pragma once
#include "Deserializer.h"

class ActionLoader : public nomime::Deserializer {
 public:
  ActionLoader(nomime::ResponseParser* pTarget);
  virtual ~ActionLoader();
  // store data
  virtual void Store(nomime::Deserializer::KeyType const& key,
                     std::wstring const& value);
  // factory method
  static nomime::Deserializer::Ptr Create(nomime::ResponseParser* pTarget);
};
