#pragma once

namespace nomime {
class Deserializr;
}

class Styler : public nomime::Deserializer {
 public:
  Styler(nomime::ResponseParser* pTarget);
  virtual ~Styler();
  // store data
  virtual void Store(nomime::Deserializer::KeyType const& key,
                     std::wstring const& value);
  // factory method
  static nomime::Deserializer::Ptr Create(nomime::ResponseParser* pTarget);
};
