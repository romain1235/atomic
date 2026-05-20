#ifndef ATOMIC_INFO_H
#define ATOMIC_INFO_H

#include <escher.h>
#include "atoms.h"

namespace Atomic {

class atomInfo : public View {
public:
  atomInfo();
  void drawRect(KDContext * ctx, KDRect rect) const override;
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void layoutSubviews(bool force = false) override;
  KDSize minimalSizeForOptimalDisplay() const override;
  void setAtom(AtomDef atom);
  AtomDef atom() const { return m_atom; }
  void setCustomColors(KDColor bg, KDColor text);
  void clearCustomColors();
private:
  AtomDef m_atom;
  MessageTextView m_atomName;
  bool m_hasCustomColors = false;
  KDColor m_customBg;
  KDColor m_customText;
  char m_nucleonsText[4] = {'\0'};
  char m_protonsText[4] = {'\0'};
  KDSize m_nucleonsSize = KDSizeZero;
  KDSize m_protonsSize = KDSizeZero;
  KDSize m_symbolSize = KDSizeZero;
};

}

#endif
