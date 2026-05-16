#ifndef ATOMIC_TYPE_FOOTER_VIEW_H
#define ATOMIC_TYPE_FOOTER_VIEW_H

#include <escher.h>
#include "atoms.h"

namespace Atomic {

class TypeFooterView : public View {
public:
  TypeFooterView() : m_type(UNKNOWN) {}
  void setType(AtomType type) { m_type = type; markRectAsDirty(bounds()); }
  void setSearchInput(bool active, const char * text, int cursor) {
    m_searchActive = active;
    if (text == nullptr) {
      m_searchText[0] = '\0';
    } else {
      int i = 0;
      for (; text[i] != '\0' && i < static_cast<int>(sizeof(m_searchText) - 1); i++) {
        m_searchText[i] = text[i];
      }
      m_searchText[i] = '\0';
    }
    int length = static_cast<int>(strlen(m_searchText));
    if (cursor < 0) {
      m_searchCursor = 0;
    } else if (cursor > length) {
      m_searchCursor = length;
    } else {
      m_searchCursor = cursor;
    }
    markRectAsDirty(bounds());
  }
  void drawRect(KDContext * ctx, KDRect rect) const override {
    if (m_searchActive) {
      KDSize textSize = KDFont::LargeFont->stringSize(m_searchText);
      ctx->fillRect(bounds(), Palette::BackgroundApps);
      KDRect inputRect(6, 2, bounds().width() - 12, bounds().height() - 4);
      ctx->fillRect(inputRect, Palette::ExpressionInputBackground);
      int x = inputRect.x() + 4;
      int y = bounds().height() - textSize.height() - 5;
      ctx->drawString(m_searchText, KDPoint(x, y), KDFont::LargeFont, Palette::PrimaryText, Palette::ExpressionInputBackground);
      char beforeCursor[20];
      int i = 0;
      for (; i < m_searchCursor && i < static_cast<int>(sizeof(beforeCursor) - 1) && m_searchText[i] != '\0'; i++) {
        beforeCursor[i] = m_searchText[i];
      }
      beforeCursor[i] = '\0';
      int cursorX = x + KDFont::LargeFont->stringSize(beforeCursor).width();
      int cursorHeight = KDFont::LargeFont->glyphSize().height();
      ctx->fillRect(KDRect(cursorX, y, 1, cursorHeight), Palette::PrimaryText);
      return;
    }

    const char* typeStr = I18n::translate(AtomicI18nForType[m_type]);
    KDSize typeSize = KDFont::SmallFont->stringSize(typeStr);
    ctx->fillRect(bounds(), Palette::BackgroundApps);
    int x = 8;
    int y = bounds().height() - typeSize.height() - 4;
    ctx->drawString(typeStr, KDPoint(x, y), KDFont::SmallFont, Palette::AtomColorHighlighted[m_type], Palette::BackgroundApps);
  }
  KDSize minimalSizeForOptimalDisplay() const override {
    return KDSize(320, KDFont::LargeFont->glyphSize().height() + 8);
  }
private:
  AtomType m_type;
  bool m_searchActive = false;
  int m_searchCursor = 0;
  char m_searchText[20] = {'\0'};
};

}

#endif
