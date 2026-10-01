#include "stdafx.h"
#include "Deserializer.h"
#include "Styler.h"

using namespace nomime;

Styler::Styler(nomime::ResponseParser* pTarget) : Deserializer(pTarget) {}

Styler::~Styler() {}

void Styler::Store(nomime::Deserializer::KeyType const& key,
                   std::wstring const& value) {
  if (!m_pTarget->p_style)
    return;

  UIStyle& sty = *m_pTarget->p_style;
  std::wstringstream ss(value);
  boost::archive::text_wiarchive ia(ss);

  TryDeserialize(ia, sty);
}

nomime::Deserializer::Ptr Styler::Create(nomime::ResponseParser* pTarget) {
  return Deserializer::Ptr(new Styler(pTarget));
}
