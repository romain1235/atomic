#include "atom_info.h"
#include "kandinsky/color.h"
#include "kandinsky/rect.h"
#include <poincare/integer.h>
#include <poincare/number.h>
#include <escher/palette.h>
#include <algorithm>

namespace Atomic {

atomInfo::atomInfo() :
  View(),
  m_atomName(KDFont::SmallFont, (I18n::Message)0, 0.0f, 0.0f, Palette::PrimaryText, Palette::BackgroundApps)
{
}

void atomInfo::setCustomColors(KDColor bg, KDColor text) {
  m_hasCustomColors = true;
  m_customBg = bg;
  m_customText = text;
  markRectAsDirty(bounds());
}

void atomInfo::clearCustomColors() {
  if (m_hasCustomColors) {
    m_hasCustomColors = false;
    markRectAsDirty(bounds());
  }
}

void atomInfo::drawRect(KDContext * ctx, KDRect rect) const {
  // Clear the background
  ctx->fillRect(rect, Palette::BackgroundApps);

  // Get the color of the atom (or custom colors if set)
  KDColor color = m_hasCustomColors ? m_customBg : Palette::AtomColor[m_atom.type];

  // Get the border / highlighted color
  KDColor highlighted = m_hasCustomColors ? m_customText : Palette::AtomColorHighlighted[m_atom.type];

  // Draw the background
  ctx->fillRect(KDRect(rect.left() + 1, rect.top() + 1, 48, 48), color);

  // Draw the border
  ctx->strokeRect(KDRect(rect.left(), rect.top(), 50, 50), highlighted);


  // Use cached serialized strings and sizes prepared in setAtom
  KDSize nucleonsSize = m_nucleonsSize;
  KDSize protonsSize = m_protonsSize;
  KDSize symbolSize = m_symbolSize;


  // Compute the position of the nucleons, protons and symbol strings to center them
  // Get the total width of the strings
  int totalWidth = std::max(nucleonsSize.width(), protonsSize.width()) + symbolSize.width();

  // Get the positions
  int nucleonsPosition = (50 - totalWidth + std::max(nucleonsSize.width(), protonsSize.width()) - nucleonsSize.width()) / 2;
  int protonsPosition = (50 - totalWidth + std::max(nucleonsSize.width(), protonsSize.width()) - protonsSize.width()) / 2;
  int symbolPosition = std::max(nucleonsPosition + nucleonsSize.width(), protonsPosition + protonsSize.width());

  // Get the y position of the nucleons and protons, and the symbol
  int nucleonsY = (50 - nucleonsSize.height() - protonsSize.height()) / 2;
  int protonsY = nucleonsY + nucleonsSize.height();
  int symbolY = (50 - symbolSize.height()) / 2;

  // Draw the number of nucleons
  KDPoint coordonates(nucleonsPosition, nucleonsY);
  ctx->drawString(m_nucleonsText, coordonates, KDFont::SmallFont, highlighted, color);

  // Draw the number of protons
  coordonates = KDPoint(protonsPosition, protonsY);
  ctx->drawString(m_protonsText, coordonates, KDFont::SmallFont, highlighted, color);
  // Draw the symbol of the atom
  coordonates = KDPoint(symbolPosition, symbolY);
  ctx->drawString(m_atom.symbol, coordonates, KDFont::LargeFont, highlighted, color);
 
}

int atomInfo::numberOfSubviews() const {
  return 1;
}

View * atomInfo::subviewAtIndex(int index) {
  assert(index == 0);
  return &m_atomName;
}

void atomInfo::layoutSubviews(bool force) {
  // Get the y position of the name
  int nameY = (50 - KDFont::SmallFont->glyphSize().height() * 2) / 2;
  m_atomName.setFrame(KDRect(KDPoint(60, nameY), KDSize(bounds().width() - 60, KDFont::SmallFont->glyphSize().height())), force);
}

void atomInfo::setAtom(AtomDef atom) {
  m_atom = atom;
  m_atomName.setMessage(atom.name);
  // Precompute serialized nucleons/protons and sizes to avoid work in drawRect
  Poincare::Integer(m_atom.num + m_atom.neutrons).serialize(m_nucleonsText, sizeof(m_nucleonsText));
  Poincare::Integer(m_atom.num).serialize(m_protonsText, sizeof(m_protonsText));
  m_nucleonsSize = KDFont::SmallFont->stringSize(m_nucleonsText);
  m_protonsSize = KDFont::SmallFont->stringSize(m_protonsText);
  m_symbolSize = KDFont::LargeFont->stringSize(m_atom.symbol);
  markRectAsDirty(bounds());
}

KDSize atomInfo::minimalSizeForOptimalDisplay() const {
  return KDSize(150, 50);
}

}
