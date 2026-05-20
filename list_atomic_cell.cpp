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
  if (m_hasCustomColor) {
    color = m_customColor;
  }
  if (m_hasCustomTextColor) {
    highlighted = m_customTextColor;
  }

  // Carré principal (50x50 centré verticalement)
  int squareSize = 50;
  int squareX = rect.left() + (rect.width() - squareSize) / 2;
  int squareY = rect.top() + (rect.height() - squareSize) / 2;
  ctx->fillRect(KDRect(squareX + 1, squareY + 1, squareSize - 2, squareSize - 2), color);
  ctx->strokeRect(KDRect(squareX, squareY, squareSize, squareSize), highlighted);

  // Z (protons) et A (nucléons)
  // Use cached serialized strings and sizes prepared in setAtom
  KDSize nucleonsSize = m_nucleonsSize;
  KDSize protonsSize = m_protonsSize;
  KDSize symbolSize = m_symbolSize;

  int totalWidth = std::max(nucleonsSize.width(), protonsSize.width()) + symbolSize.width();
  int nucleonsPosition = squareX + (squareSize - totalWidth + std::max(nucleonsSize.width(), protonsSize.width()) - nucleonsSize.width()) / 2;
  int protonsPosition = squareX + (squareSize - totalWidth + std::max(nucleonsSize.width(), protonsSize.width()) - protonsSize.width()) / 2;
  int symbolPosition = std::max(nucleonsPosition + nucleonsSize.width(), protonsPosition + protonsSize.width());

  int nucleonsY = squareY + (squareSize - nucleonsSize.height() - protonsSize.height()) / 2;
  int protonsY = nucleonsY + nucleonsSize.height();
  int symbolY = squareY + (squareSize - symbolSize.height()) / 2;

  // Dessin A (nucléons)
  ctx->drawString(m_nucleonsText, KDPoint(nucleonsPosition, nucleonsY), KDFont::SmallFont, highlighted, color);
  // Dessin Z (protons)
  ctx->drawString(m_protonsText, KDPoint(protonsPosition, protonsY), KDFont::SmallFont, highlighted, color);
  // Dessin symbole
  ctx->drawString(m_atom.symbol, KDPoint(symbolPosition, symbolY), KDFont::LargeFont, highlighted, color);

}

View * ListAtomicCell::subviewAtIndex(int index) {
  assert(false);
  return nullptr;
}

void ListAtomicCell::setAtom(AtomDef atom) {
  m_atom = atom;
  // Precompute serialized texts and sizes to avoid work in drawRect
  Poincare::Integer(m_atom.num + m_atom.neutrons).serialize(m_nucleonsText, sizeof(m_nucleonsText));
  Poincare::Integer(m_atom.num).serialize(m_protonsText, sizeof(m_protonsText));
  m_nucleonsSize = KDFont::SmallFont->stringSize(m_nucleonsText);
  m_protonsSize = KDFont::SmallFont->stringSize(m_protonsText);
  m_symbolSize = KDFont::LargeFont->stringSize(m_atom.symbol);
  // Reset custom colors when changing atom
  m_hasCustomColor = false;
  m_hasCustomTextColor = false;
  markRectAsDirty(bounds());
}

void ListAtomicCell::setCustomColor(KDColor color) {
  if (!m_hasCustomColor || m_customColor != color) {
    m_hasCustomColor = true;
    m_customColor = color;
    markRectAsDirty(bounds());
  }
}

void ListAtomicCell::clearCustomColor() {
  if (m_hasCustomColor) {
    m_hasCustomColor = false;
    markRectAsDirty(bounds());
  }
}

void ListAtomicCell::setCustomTextColor(KDColor color) {
  if (!m_hasCustomTextColor || m_customTextColor != color) {
    m_hasCustomTextColor = true;
    m_customTextColor = color;
    markRectAsDirty(bounds());
  }
}

void ListAtomicCell::clearCustomTextColor() {
  if (m_hasCustomTextColor) {
    m_hasCustomTextColor = false;
    markRectAsDirty(bounds());
  }
}

KDRect ListAtomicCell::m_atomRect() const {
  return KDRect((bounds().left() + (bounds().width()- k_width) / 2), bounds().top() + k_margin, k_width, bounds().height() - 2*k_margin);
}

void ListAtomicCell::layoutSubviews(bool force) {
}

}
