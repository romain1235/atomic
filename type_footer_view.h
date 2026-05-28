#ifndef ATOMIC_TYPE_FOOTER_VIEW_H
#define ATOMIC_TYPE_FOOTER_VIEW_H

#include <escher.h>
#include "atoms.h"

namespace Atomic {

class TypeFooterView : public View {
public:
  TypeFooterView() : m_type(UNKNOWN) {}
  void setType(AtomType type) { if (m_type == type) { return; } m_type = type; markRectAsDirty(bounds()); }
  void setPropertyDisplay(const char * label, const char * value, KDColor bgColor, KDColor textColor) {
    m_showProperty = true;
    int i = 0;
    for (; label[i] != '\0' && i < static_cast<int>(sizeof(m_propertyLabel) - 1); i++) {
      m_propertyLabel[i] = label[i];
    }
    m_propertyLabel[i] = '\0';
    int j = 0;
    for (; value[j] != '\0' && j < static_cast<int>(sizeof(m_propertyValue) - 1); j++) {
      m_propertyValue[j] = value[j];
    }
    m_propertyValue[j] = '\0';
    m_propertyBgColor = bgColor;
    m_propertyTextColor = textColor;
    markRectAsDirty(bounds());
  }
  void clearPropertyDisplay() {
    if (m_showProperty) {
      m_showProperty = false;
      markRectAsDirty(bounds());
    }
  }
  void drawRect(KDContext * ctx, KDRect rect) const override {
    const char* typeStr = I18n::translate(AtomicI18nForType[m_type]);
    KDSize typeSize = KDFont::SmallFont->stringSize(typeStr);
    ctx->fillRect(bounds(), Palette::BackgroundApps);
    int x = 8;
    int y = bounds().height() - typeSize.height() - 4;
    // If a property is shown, display it instead of the type
    if (m_showProperty) {
      ctx->fillRect(bounds(), Palette::BackgroundApps);
      // Draw label + ':'
      char labelWithColon[40];
      int li = 0;
      for (; m_propertyLabel[li] != '\0' && li < static_cast<int>(sizeof(labelWithColon) - 2); li++) {
        labelWithColon[li] = m_propertyLabel[li];
      }
      labelWithColon[li++] = ':';
      labelWithColon[li] = '\0';
      KDSize labelSize = KDFont::SmallFont->stringSize(labelWithColon);
      ctx->drawString(labelWithColon, KDPoint(x, y), KDFont::SmallFont, m_propertyTextColor, Palette::BackgroundApps);

      // Draw value, possibly with '^' exponent marker (e.g. "g·mol^-1")
      // Split base and exponent at '^' if present
      char base[48];
      char exponent[16];
      int bi = 0;
      int ei = 0;
      bool hasCaret = false;
      for (int j = 0; m_propertyValue[j] != '\0' && j < static_cast<int>(sizeof(m_propertyValue)); j++) {
        if (m_propertyValue[j] == '^') {
          hasCaret = true;
          int k = j + 1;
          while (m_propertyValue[k] != '\0' && ei < static_cast<int>(sizeof(exponent) - 1)) {
            exponent[ei++] = m_propertyValue[k++];
          }
          exponent[ei] = '\0';
          break;
        }
        if (bi < static_cast<int>(sizeof(base) - 1)) {
          base[bi++] = m_propertyValue[j];
        }
      }
      base[bi] = '\0';

      KDCoordinate valueX = x + labelSize.width() + 4;
      if (!hasCaret) {
        ctx->drawString(base, KDPoint(valueX, y), KDFont::SmallFont, m_propertyTextColor, Palette::BackgroundApps);
        return;
      }
      // Draw base then exponent shifted up
      KDSize baseSize = KDFont::SmallFont->stringSize(base);
      ctx->drawString(base, KDPoint(valueX, y), KDFont::SmallFont, m_propertyTextColor, Palette::BackgroundApps);
      int supShift = KDFont::SmallFont->glyphSize().height() / 2;
      ctx->drawString(exponent, KDPoint(valueX + baseSize.width(), y - supShift), KDFont::SmallFont, m_propertyTextColor, Palette::BackgroundApps);
      return;
    }
    ctx->drawString(typeStr, KDPoint(x, y), KDFont::SmallFont, Palette::AtomColorHighlighted[m_type], Palette::BackgroundApps);
  }
  KDSize minimalSizeForOptimalDisplay() const override {
    return KDSize(320, KDFont::LargeFont->glyphSize().height() + 8);
  }
private:
  AtomType m_type;
  bool m_showProperty = false;
  char m_propertyLabel[32] = {'\0'};
  char m_propertyValue[32] = {'\0'};
  KDColor m_propertyBgColor = KDColor::RGB24(0x000000);
  KDColor m_propertyTextColor = KDColor::RGB24(0xffffff);
};

}

#endif
