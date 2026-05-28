#include "atomic_cell.h"
#include <assert.h>
#include <escher/palette.h>

namespace Atomic {



AtomicCell::AtomicCell() :
  HighlightCell(),
  m_visible(true),
  m_searchActive(false),
  m_searchMatch(true)
{
}

KDColor AtomicCell::colorForType(AtomType type) const {
  return Palette::AtomColor[type];
}

void AtomicCell::drawRect(KDContext * ctx, KDRect rect) const {
  if (m_visible) {
    KDColor color = colorForType(m_atom.type);
    if (m_hasCustomColor) {
      color = m_customColor;
    }
    KDColor textColor = m_hasCustomTextColor ? m_customTextColor : Palette::AtomColorHighlighted[m_atom.type];
    if (m_searchActive && !m_searchMatch) {
      color = Palette::BackgroundAppsSecondary;
      textColor = Palette::SecondaryText;
    }
    ctx->fillRect(rect, color);

    if (m_searchActive && !m_searchMatch) {
      return;
    }

    // Get text size in pixels
    KDSize textSize = KDFont::SmallFont->stringSize(m_atom.symbol);

    // Center text in cell
    KDPoint textPosition(bounds().topLeft().x() + (bounds().width() - textSize.width()) / 2, bounds().topLeft().y() + (bounds().height() - textSize.height()) / 2);
    ctx->drawString(m_atom.symbol, textPosition, KDFont::SmallFont, textColor, color);
  } else {
    ctx->fillRect(rect, Palette::BackgroundApps);
  }
}

int AtomicCell::numberOfSubviews() const {
  return 0;
}

View * AtomicCell::subviewAtIndex(int index) {
  return nullptr;
}

void AtomicCell::layoutSubviews(bool force) {
}

void AtomicCell::setVisible(bool visible) {
  if (m_visible != visible) {
    m_visible = visible;
    markRectAsDirty(bounds());
  }
}

void AtomicCell::setAtom(AtomDef atom) {
  m_atom = atom;
  markRectAsDirty(bounds());
}

void AtomicCell::setSearchState(bool searchActive, bool isMatch) {
  if (m_searchActive != searchActive || m_searchMatch != isMatch) {
    m_searchActive = searchActive;
    m_searchMatch = isMatch;
    markRectAsDirty(bounds());
  }
}

void AtomicCell::reloadCell() {
  markRectAsDirty(bounds());
}

void AtomicCell::setCustomColor(KDColor color) {
  if (!m_hasCustomColor || m_customColor != color) {
    m_hasCustomColor = true;
    m_customColor = color;
    markRectAsDirty(bounds());
  }
}

void AtomicCell::clearCustomColor() {
  if (m_hasCustomColor) {
    m_hasCustomColor = false;
    markRectAsDirty(bounds());
  }
}

void AtomicCell::setCustomTextColor(KDColor color) {
  if (!m_hasCustomTextColor || m_customTextColor != color) {
    m_hasCustomTextColor = true;
    m_customTextColor = color;
    markRectAsDirty(bounds());
  }
}

void AtomicCell::clearCustomTextColor() {
  if (m_hasCustomTextColor) {
    m_hasCustomTextColor = false;
    markRectAsDirty(bounds());
  }
}


}
