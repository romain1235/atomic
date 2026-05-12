#include "list_atomic_cell.h"
#include <poincare/integer.h>
#include <poincare/number.h>
#include <escher/palette.h>
#include <algorithm>

namespace Atomic {

ListAtomicCell::ListAtomicCell()
{
}

void ListAtomicCell::drawRect(KDContext * ctx, KDRect rect) const {
  // Fond général
  ctx->fillRect(rect, Palette::BackgroundApps);

  // Couleurs
  KDColor color = Palette::AtomColor[m_atom.type];
  KDColor highlighted = Palette::AtomColorHighlighted[m_atom.type];

  // Carré principal (50x50 centré verticalement)
  int squareSize = 50;
  int squareX = rect.left() + (rect.width() - squareSize) / 2;
  int squareY = rect.top() + (rect.height() - squareSize) / 2;
  ctx->fillRect(KDRect(squareX + 1, squareY + 1, squareSize - 2, squareSize - 2), color);
  ctx->strokeRect(KDRect(squareX, squareY, squareSize, squareSize), highlighted);

  // Z (protons) et A (nucléons)
  char nucleons[4];
  Poincare::Integer(m_atom.num + m_atom.neutrons).serialize(nucleons, 4);
  char protons[4];
  Poincare::Integer(m_atom.num).serialize(protons, 4);

  KDSize nucleonsSize = KDFont::SmallFont->stringSize(nucleons);
  KDSize protonsSize = KDFont::SmallFont->stringSize(protons);
  KDSize symbolSize = KDFont::LargeFont->stringSize(m_atom.symbol);

  int totalWidth = std::max(nucleonsSize.width(), protonsSize.width()) + symbolSize.width();
  int nucleonsPosition = squareX + (squareSize - totalWidth + std::max(nucleonsSize.width(), protonsSize.width()) - nucleonsSize.width()) / 2;
  int protonsPosition = squareX + (squareSize - totalWidth + std::max(nucleonsSize.width(), protonsSize.width()) - protonsSize.width()) / 2;
  int symbolPosition = std::max(nucleonsPosition + nucleonsSize.width(), protonsPosition + protonsSize.width());

  int nucleonsY = squareY + (squareSize - nucleonsSize.height() - protonsSize.height()) / 2;
  int protonsY = nucleonsY + nucleonsSize.height();
  int symbolY = squareY + (squareSize - symbolSize.height()) / 2;

  // Dessin A (nucléons)
  ctx->drawString(nucleons, KDPoint(nucleonsPosition, nucleonsY), KDFont::SmallFont, highlighted, color);
  // Dessin Z (protons)
  ctx->drawString(protons, KDPoint(protonsPosition, protonsY), KDFont::SmallFont, highlighted, color);
  // Dessin symbole
  ctx->drawString(m_atom.symbol, KDPoint(symbolPosition, symbolY), KDFont::LargeFont, highlighted, color);

}

View * ListAtomicCell::subviewAtIndex(int index) {
  assert(false);
  return nullptr;
}

void ListAtomicCell::setAtom(AtomDef atom) {
  m_atom = atom;
  markRectAsDirty(bounds());
}

KDRect ListAtomicCell::m_atomRect() const {
  return KDRect((bounds().left() + (bounds().width()- k_width) / 2), bounds().top() + k_margin, k_width, bounds().height() - 2*k_margin);
}

void ListAtomicCell::layoutSubviews(bool force) {
}

}
