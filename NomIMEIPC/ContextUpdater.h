#pragma once
#include "Deserializer.h"

class ContextUpdater : public nomime::Deserializer {
 public:
  ContextUpdater(nomime::ResponseParser* pTarget);
  virtual ~ContextUpdater();
  virtual void Store(nomime::Deserializer::KeyType const& key,
                     std::wstring const& value);

  void _StoreText(nomime::Text& target,
                  Deserializer::KeyType k,
                  std::wstring const& value);
  void _StoreCand(Deserializer::KeyType k, std::wstring const& value);

  static nomime::Deserializer::Ptr Create(nomime::ResponseParser* pTarget);
};

class StatusUpdater : public nomime::Deserializer {
 public:
  StatusUpdater(nomime::ResponseParser* pTarget);
  virtual ~StatusUpdater();
  virtual void Store(nomime::Deserializer::KeyType const& key,
                     std::wstring const& value);

  static nomime::Deserializer::Ptr Create(nomime::ResponseParser* pTarget);
};
