#pragma once
#include "Deserializer.h"

class Committer : public nomime::Deserializer {
 public:
  Committer(nomime::ResponseParser* pTarget);
  virtual ~Committer();
  // store data
  virtual void Store(nomime::Deserializer::KeyType const& key,
                     std::wstring const& value);
  // factory method
  static nomime::Deserializer::Ptr Create(nomime::ResponseParser* pTarget);
};
