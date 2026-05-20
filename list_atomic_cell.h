#ifndef ATOMIC_LIST_ATOMIC_CELL_H
#define ATOMIC_LIST_ATOMIC_CELL_H

#include <escher/bordered.h>
#include <escher/highlight_cell.h>
#include "atoms.h"

namespace Atomic {

class ListAtomicCell : public Bordered, public HighlightCell {
public:
  ListAtomicCell();
  void setHighlighted(bool highlight) override { return; }
  void drawRect(KDContext * ctx, KDRect rect) const override;
  void setAtom(AtomDef atom);
  void setCustomColor(KDColor color);
  void clearCustomColor();
  void setCustomTextColor(KDColor color);
  void clearCustomTextColor();
private:
  constexpr static int k_width = 60;
  constexpr static int k_margin = 10;
  constexpr static int k_padding = 3;
  int numberOfSubviews() const override { return 0; }
  View * subviewAtIndex(int index) override;
  void layoutSubviews(bool force = false) override;
  KDRect m_atomRect() const;
  AtomDef m_atom;
  char m_nucleonsText[4] = {'\0'};
  char m_protonsText[4] = {'\0'};
  KDSize m_nucleonsSize = KDSizeZero;
  KDSize m_protonsSize = KDSizeZero;
  KDSize m_symbolSize = KDSizeZero;
  bool m_hasCustomColor = false;
  KDColor m_customColor;
  bool m_hasCustomTextColor = false;
  KDColor m_customTextColor;
};

}

#endif

