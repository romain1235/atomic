#ifndef ATOMIC_TYPE_FOOTER_VIEW_H
#define ATOMIC_TYPE_FOOTER_VIEW_H

#include <escher.h>
#include "atoms.h"

namespace Atomic {

class TypeFooterView : public View {
public:
  TypeFooterView() : m_type(I18n::Message::AtomTypeUNKNOWN) {}
  void setType(I18n::Message type) { m_type = type; markRectAsDirty(bounds()); }
  void drawRect(KDContext * ctx, KDRect rect) const override {
    const char* typeStr = I18n::translate(m_type);
    KDSize typeSize = KDFont::SmallFont->stringSize(typeStr);
    // Effacer toute la largeur du footer (pas juste la largeur du texte)
    ctx->fillRect(bounds(), Palette::BackgroundApps);
    int x = 8;
    int y = bounds().height() - typeSize.height() - 4;
    ctx->drawString(typeStr, KDPoint(x, y), KDFont::SmallFont, Palette::PrimaryText, Palette::BackgroundApps);
  }
  KDSize minimalSizeForOptimalDisplay() const override {
    // Toujours la largeur max de l'écran (320px sur NumWorks), hauteur adaptée au texte
    return KDSize(250, KDFont::SmallFont->glyphSize().height() + 8);
  }
private:
  I18n::Message m_type;
};

}

#endif
