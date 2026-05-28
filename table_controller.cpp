#include "table_controller.h"
#include "app.h"
#include "../apps_container.h"
#include <ion/unicode/utf8_decoder.h>
#include <string.h>
#include <poincare/integer.h>
#include <poincare/number.h>
#include <escher/palette.h>
#include <escher/stack_view.h>
#include <float.h>
#include <stdio.h>

extern "C" {
#include <assert.h>
}

namespace Atomic {

static bool isAsciiDigit(char c) {
  return c >= '0' && c <= '9';
}

static bool isAsciiAlnum(char c) {
  return isAsciiDigit(c) || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static char foldCodePointToSearchChar(CodePoint codePoint) {
  uint32_t c = static_cast<uint32_t>(codePoint);
  if (c == 0) {
    return 0;
  }
  if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  }
  if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
    return c;
  }
  if (c >= 0x300 && c <= 0x36F) {
    return 0;
  }
  switch (c) {
    case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5:
    case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:
      return 'a';
    case 0xC7: case 0xE7:
      return 'c';
    case 0xC8: case 0xC9: case 0xCA: case 0xCB:
    case 0xE8: case 0xE9: case 0xEA: case 0xEB:
      return 'e';
    case 0xCC: case 0xCD: case 0xCE: case 0xCF:
    case 0xEC: case 0xED: case 0xEE: case 0xEF:
      return 'i';
    case 0xD1: case 0xF1:
      return 'n';
    case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6:
    case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6:
      return 'o';
    case 0xD9: case 0xDA: case 0xDB: case 0xDC:
    case 0xF9: case 0xFA: case 0xFB: case 0xFC:
      return 'u';
    case 0xDD: case 0xFD: case 0xFF:
      return 'y';
    default:
      return 0;
  }
}

static char nextFoldedSearchChar(const char ** text) {
  UTF8Decoder decoder(*text, *text);
  while (true) {
    CodePoint codePoint = decoder.nextCodePoint();
    *text = decoder.stringPosition();
    if (codePoint == UCodePointNull) {
      return 0;
    }
    char folded = foldCodePointToSearchChar(codePoint);
    if (folded != 0) {
      return folded;
    }
  }
}

struct PropertyDisplayData {
  const char * label;
  char value[32];
  // Renamed: the palette entries represent the "max" color; we also
  // compute a "min" color (slightly tinted toward the theme) for the
  // gradient.
  KDColor propertyColorMax;
  KDColor propertyColorMaxHighlighted;
  KDColor propertyColorMin;
  KDColor propertyColorMinHighlighted;
  double valueForColor;
};

static PropertyDisplayData propertyDisplayDataForAtom(const AtomDef & a, TableController::ColorProperty property) {
  PropertyDisplayData data = {
    "",
    "",
    Palette::AtomColor[a.type],
    Palette::AtomColorHighlighted[a.type],
    Palette::BackgroundAppsSecondary,
    Palette::SecondaryText,
    -1
  };

  switch (property) {
    case TableController::ColorByAtomicNumber:
      data.label = I18n::translate(I18n::Message::AtomNum);
      Poincare::Integer(a.num).serialize(data.value, sizeof(data.value));
      data.propertyColorMax = Palette::AtomPropertyNum;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyNumHighlighted;
      data.propertyColorMin = Palette::AtomPropertyNumMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyNumMinHighlighted;
      data.valueForColor = a.num;
      break;
    case TableController::ColorByNeutrons:
      data.label = I18n::translate(I18n::Message::AtomNeutrons);
      Poincare::Integer(a.neutrons).serialize(data.value, sizeof(data.value));
      data.propertyColorMax = Palette::AtomPropertyNeutrons;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyNeutronsHighlighted;
      data.propertyColorMin = Palette::AtomPropertyNeutronsMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyNeutronsMinHighlighted;
      data.valueForColor = a.neutrons;
      break;
    case TableController::ColorByMass:
      data.label = I18n::translate(I18n::Message::AtomMass);
      if (a.mass < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        char tmp[32];
        Poincare::Number::FloatNumber(a.mass).serialize(tmp, sizeof(tmp), Poincare::Preferences::PrintFloatMode::Decimal, 3);
        strlcpy(data.value, tmp, sizeof(data.value));
        strlcat(data.value, " g·mol^-1", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyMass;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyMassHighlighted;
      data.propertyColorMin = Palette::AtomPropertyMassMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyMassMinHighlighted;
      data.valueForColor = a.mass;
      break;
    case TableController::ColorByElectronegativity:
      data.label = I18n::translate(I18n::Message::AtomElectroneg);
      if (a.electroneg < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        Poincare::Number::FloatNumber(a.electroneg).serialize(data.value, sizeof(data.value), Poincare::Preferences::PrintFloatMode::Decimal, 2);
      }
      data.propertyColorMax = Palette::AtomPropertyElectroneg;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyElectronegHighlighted;
      data.propertyColorMin = Palette::AtomPropertyElectronegMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyElectronegMinHighlighted;
      data.valueForColor = a.electroneg;
      break;
    case TableController::ColorByAtomicRadius:
      data.label = I18n::translate(I18n::Message::AtomAtomicRadius);
      if (a.atomicRadius < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        Poincare::Integer(static_cast<int>(a.atomicRadius)).serialize(data.value, sizeof(data.value));
        strlcat(data.value, " pm", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyAtomicRadius;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyAtomicRadiusHighlighted;
      data.propertyColorMin = Palette::AtomPropertyAtomicRadiusMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyAtomicRadiusMinHighlighted;
      data.valueForColor = a.atomicRadius;
      break;
    case TableController::ColorByElectronAffinity:
      data.label = I18n::translate(I18n::Message::AtomElectronAffinity);
      if (a.electronAffinity < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        char tmp[32];
        Poincare::Number::FloatNumber(a.electronAffinity).serialize(tmp, sizeof(tmp), Poincare::Preferences::PrintFloatMode::Decimal, 3);
        strlcpy(data.value, tmp, sizeof(data.value));
        strlcat(data.value, " eV", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyElectronAffinity;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyElectronAffinityHighlighted;
      data.propertyColorMin = Palette::AtomPropertyElectronAffinityMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyElectronAffinityMinHighlighted;
      data.valueForColor = a.electronAffinity;
      break;
    case TableController::ColorByIonisation:
      data.label = I18n::translate(I18n::Message::AtomIonisation);
      if (a.ionisation < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        char tmp[32];
        Poincare::Number::FloatNumber(a.ionisation).serialize(tmp, sizeof(tmp), Poincare::Preferences::PrintFloatMode::Decimal, 3);
        strlcpy(data.value, tmp, sizeof(data.value));
        strlcat(data.value, " eV", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyIonisation;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyIonisationHighlighted;
      data.propertyColorMin = Palette::AtomPropertyIonisationMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyIonisationMinHighlighted;
      data.valueForColor = a.ionisation;
      break;
    case TableController::ColorByMeltingPoint:
      data.label = I18n::translate(I18n::Message::AtomMeltingPoint);
      if (a.meltingPoint < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        Poincare::Integer(static_cast<int>(a.meltingPoint)).serialize(data.value, sizeof(data.value));
        strlcat(data.value, " K", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyMeltingPoint;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyMeltingPointHighlighted;
      data.propertyColorMin = Palette::AtomPropertyMeltingPointMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyMeltingPointMinHighlighted;
      data.valueForColor = a.meltingPoint;
      break;
    case TableController::ColorByBoilingPoint:
      data.label = I18n::translate(I18n::Message::AtomBoilingPoint);
      if (a.boilingPoint < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        Poincare::Integer(static_cast<int>(a.boilingPoint)).serialize(data.value, sizeof(data.value));
        strlcat(data.value, " K", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyBoilingPoint;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyBoilingPointHighlighted;
      data.propertyColorMin = Palette::AtomPropertyBoilingPointMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyBoilingPointMinHighlighted;
      data.valueForColor = a.boilingPoint;
      break;
    case TableController::ColorByDensity:
      data.label = I18n::translate(I18n::Message::AtomDensity);
      if (a.density < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        char tmp[32];
        Poincare::Number::FloatNumber(a.density).serialize(tmp, sizeof(tmp), Poincare::Preferences::PrintFloatMode::Decimal, 3);
        strlcpy(data.value, tmp, sizeof(data.value));
        strlcat(data.value, " g·cm^-3", sizeof(data.value));
      }
      data.propertyColorMax = Palette::AtomPropertyDensity;
      data.propertyColorMaxHighlighted = Palette::AtomPropertyDensityHighlighted;
      data.propertyColorMin = Palette::AtomPropertyDensityMin;
      data.propertyColorMinHighlighted = Palette::AtomPropertyDensityMinHighlighted;
      data.valueForColor = a.density;
      break;
    default:
      break;
  }

  // `propertyColorMin` / `propertyColorMinHighlighted` are set per-property above

  return data;
}

TableController::ContentView::ContentView(TableController * controller, SelectableTableViewDataSource * selectionDataSource, TextField * searchField) :
  m_selectableTableView(controller, controller, selectionDataSource, controller),
  m_searchFieldPtr(searchField),
  m_searchVisible(false)
{
  m_selectableTableView.setVerticalCellOverlap(-1);
  m_selectableTableView.setHorizontalCellOverlap(-1);
  m_selectableTableView.setMargins(k_sideMargin, k_sideMargin, k_sideMargin, k_sideMargin);
  m_selectableTableView.setBackgroundColor(Palette::BackgroundApps);
}

SelectableTableView * TableController::ContentView::selectableTableView() {
  return &m_selectableTableView;
}


void TableController::ContentView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(bounds(), Palette::BackgroundApps);
}

int TableController::ContentView::numberOfSubviews() const {
  return 6;
}

View * TableController::ContentView::subviewAtIndex(int index) {
  switch(index) {
    case 0:
      return &m_selectableTableView;
    case 1:
      return &m_cursor;
    case 2:
      return &m_info;
    case 3:
      return &m_lines;
    case 4:
      return &m_typeFooter;
    case 5:
      return m_searchFieldPtr;
    default:
      assert(false);
      return nullptr;
  }
}

void TableController::ContentView::layoutSubviews(bool force) {
  m_selectableTableView.setFrame(KDRect(bounds().left(), bounds().top() + 20 , bounds().width(), bounds().height() - 20), force);
  m_info.setFrame(KDRect(KDPoint(48, 15),m_info.minimalSizeForOptimalDisplay()), force);
  m_lines.setFrame(KDRect(KDPoint(40, 99 + 20), m_lines.minimalSizeForOptimalDisplay()), force);
  KDCoordinate footerH = m_typeFooter.minimalSizeForOptimalDisplay().height();
  KDRect footerRect = KDRect(0, bounds().height() - footerH, bounds().width(), footerH);
  if (m_searchVisible) {
    m_typeFooter.setFrame(KDRectZero, force);
    m_searchFieldPtr->setFrame(KDRect(6, footerRect.y() + 2, bounds().width() - 12, footerH - 4), force);
  } else {
    m_typeFooter.setFrame(footerRect, force);
    m_searchFieldPtr->setFrame(KDRectZero, force);
  }
}

void TableController::ContentView::setAtom(AtomDef atom) {
  m_info.setAtom(atom);
  m_typeFooter.setType(atom.type);
}

void TableController::ContentView::setSearchVisible(bool active) {
  if (m_searchVisible == active) {
    return;
  }
  m_searchVisible = active;
  layoutSubviews(true);
  markRectAsDirty(bounds());
}

void TableController::ContentView::setInfoVisible(bool visible) {
  if (visible) {
    m_info.setFrame(KDRect(KDPoint(48, 15), m_info.minimalSizeForOptimalDisplay()), true);
  } else {
    m_info.setFrame(KDRect(0, 0, 0, 0), true);
  }
}

void TableController::ContentView::setPropertyDisplay(const char * label, const char * value, KDColor bgColor, KDColor textColor) {
  m_typeFooter.setPropertyDisplay(label, value, bgColor, textColor);
  m_info.setCustomColors(bgColor, textColor);
}

void TableController::ContentView::clearPropertyDisplay() {
  m_typeFooter.clearPropertyDisplay();
  m_info.clearCustomColors();
}

void TableController::ContentView::updateCursorFrame(int col, int row, KDColor color) {
  KDPoint offset = m_selectableTableView.contentOffset();
  constexpr int tableTop = 20;
  int x = TableController::k_sideMargin - 1 + col * TableController::k_cellWidth - offset.x();
  int y = tableTop + TableController::k_sideMargin - 1 + row * TableController::k_cellHeight - offset.y();
  m_cursor.setColor(color);
  m_cursor.setFrame(KDRect(x, y, TableController::k_cellWidth + 1, TableController::k_cellHeight + 1), true);
}

void TableController::ContentView::hideCursor() {
  m_cursor.setFrame(KDRectZero, true);
}

TableController::TableController(Responder * parentResponder, SelectableTableViewDataSource * selectionDataSource) :
  ViewController(parentResponder),
  m_searchField(this, m_searchBuffer, sizeof(m_searchBuffer), sizeof(m_searchBuffer), this, this, KDFont::LargeFont, 0.0f, 0.5f, Palette::PrimaryText, Palette::BackgroundApps),
  m_view(this, selectionDataSource, &m_searchField),
  m_searchStartCursor(-1),
  m_bestSearchResult(-1),
  m_searchActive(false),
  m_list(this)
{
  m_searchBuffer[0] = '\0';

  // Build a fast lookup table from (x,y) -> atomsdefs index to avoid O(n)
  // scans in willDisplayCellAtLocation and during selection.
  for (int x = 0; x < k_numberOfColumns; x++) {
    for (int y = 0; y < k_numberOfRows; y++) {
      m_atomIndex[x][y] = -1;
    }
  }
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  for (int i = 0; i < count; i++) {
    const AtomDef &a = atomsdefs[i];
    if (a.x < k_numberOfColumns && a.y < k_numberOfRows) {
      m_atomIndex[a.x][a.y] = i;
    }
    m_searchMatches[i] = true;
  }
  for (int i = count; i < k_maxNumberOfCells; i++) {
    m_searchMatches[i] = false;
  }
}

bool TableController::handleEvent(Ion::Events::Event event) {
  if (m_menuIsOpen && event != Ion::Events::OK && event != Ion::Events::EXE) {
    // Bloquer tous les événements sauf la fermeture du menu
    return false;
  }

  if (!m_searchActive && event == Ion::Events::Paste) {
    m_searchStartCursor = m_cursor;
    m_searchActive = true;
    m_searchBuffer[0] = '\0';
    m_searchField.setEditing(true);
    m_searchField.reinitDraftTextBuffer();
    m_view.setSearchVisible(true);
    m_view.setInfoVisible(true);
    Container::activeApp()->setFirstResponder(&m_searchField);
    m_searchField.handleEvent(event);
    return true;
  }

  if (event.hasText() && strlen(event.text()) == 1) {
    char c = event.text()[0];
    if (isAsciiAlnum(c)) {
      if (!m_searchActive) {
        // Start search mode
        m_searchStartCursor = m_cursor;
        m_searchActive = true;
        m_searchBuffer[0] = '\0';
        m_searchField.setEditing(true);
        m_searchField.reinitDraftTextBuffer();
        m_view.setSearchVisible(true);
        m_view.setInfoVisible(true);
        Container::activeApp()->setFirstResponder(&m_searchField);
      }
      m_searchField.handleEventWithText(event.text());
      return true;
    }
  }

  if (event == Ion::Events::Right && m_cursor < static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef) - 1)) {
    if (m_searchActive) {
      return true;
    }
    AtomDef atom = atomsdefs[++m_cursor];
    setSelection(atom);
    return true;
  }
  if (event == Ion::Events::Left && m_cursor > 0) {
    if (m_searchActive) {
      return true;
    }
    AtomDef atom = atomsdefs[--m_cursor];
    setSelection(atom);
    return true;
  }
  if (event == Ion::Events::Up) {
    if (m_searchActive) {
      int nextIndex = nextSearchResultIndex(-1);
      if (nextIndex >= 0) {
        m_cursor = nextIndex;
        setSelection(atomsdefs[m_cursor]);
      }
      return true;
    }
    int row = selectionDataSource()->selectedRow();
    int column = selectionDataSource()->selectedColumn();
    if (row > 0) {
      // row 8 (lanthanide/actinide) jumps back to row 6
      int targetRow = (row == 8) ? 6 : row - 1;
      int idx = m_atomIndex[column][targetRow];
      if (idx >= 0) {
        m_cursor = idx;
        setSelection(atomsdefs[m_cursor]);
      }
    }
    return true;
  }
  if (event == Ion::Events::Down) {
    if (m_searchActive) {
      int nextIndex = nextSearchResultIndex(1);
      if (nextIndex >= 0) {
        m_cursor = nextIndex;
        setSelection(atomsdefs[m_cursor]);
      }
      return true;
    }
    int row = selectionDataSource()->selectedRow();
    int column = selectionDataSource()->selectedColumn();
    if (row < 9) {
      // row 6 jumps over the lanthanide/actinide gap to row 8
      int targetRow = (row == 6) ? 8 : row + 1;
      int idx = m_atomIndex[column][targetRow];
      if (idx >= 0) {
        m_cursor = idx;
        setSelection(atomsdefs[m_cursor]);
      }
    }
    return true;
  }
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    if (m_searchActive) {
      int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
      if (m_cursor >= 0 && m_cursor < count && m_searchMatches[m_cursor]) {
        // Keep current selection chosen with Up/Down among search results.
      } else if (m_bestSearchResult >= 0) {
        m_cursor = m_bestSearchResult;
      } else if (m_searchStartCursor >= 0) {
        m_cursor = m_searchStartCursor;
      }
      setSelection(atomsdefs[m_cursor]);
      clearSearch();
      return true;
    }
    m_menuIsOpen = true;
    // Ensure list has the current atom before pushing
    m_list.setAtom(atomsdefs[m_cursor]);
    // If a property coloring is active, pass the precomputed colors to the list modal
    if (m_coloringActive && m_precomputedColorsValid) {
      int idx = m_cursor;
      if (idx >= 0 && idx < static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef))) {
        m_list.setPropertyColors(m_precomputedBg[idx], m_precomputedText[idx]);
      } else {
        m_list.clearPropertyColors();
      }
    } else {
      m_list.clearPropertyColors();
    }
    stackController()->push(&m_list);
    m_list.unhighlightTopCells();
    return true;
  }
  return false;
}

bool TableController::moveCursorInMenu(int direction) {
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  if (count <= 0 || direction == 0) {
    return false;
  }

  int nextCursor = m_cursor + (direction < 0 ? -1 : 1);
  if (nextCursor < 0 || nextCursor >= count) {
    return false;
  }

  m_cursor = nextCursor;
  const AtomDef & atom = atomsdefs[m_cursor];
  m_list.setAtom(atom);
  if (m_coloringActive && m_precomputedColorsValid) {
    m_list.setPropertyColors(m_precomputedBg[m_cursor], m_precomputedText[m_cursor]);
  } else {
    m_list.clearPropertyColors();
  }
  updateFooterPropertyDisplay(m_cursor);
  // Update the app's stack header title for the ListController.
  // TableController has no title() → 0 stack headers for it.
  // ListController has title() → its header is at subview(0) of the ControllerView.
  View * listHeader = stackController()->view()->subview(0);
  if (listHeader) {
    static_cast<StackView *>(listHeader)->setNamedController(&m_list);
  }
  // Re-fire the selection delegate (ensures scroll-to-row-0 for row 1, restores FR)
  m_list.refreshNavigation();
  return true;
}

void TableController::didBecomeFirstResponder() {
  m_menuIsOpen = false;
  clearSearch();
  if (selectionDataSource()->selectedRow() == -1) {
    setSelection(atomsdefs[0]);
  } else {
    setSelection(atomsdefs[m_cursor]);
  }
}

View * TableController::view() {
  return &m_view;
}

int TableController::numberOfRows() const {
  return ((k_maxNumberOfCells - 1) / k_numberOfColumns) + 1;
}

int TableController::numberOfColumns() const {
  return k_numberOfColumns;
}

KDCoordinate TableController::cellHeight() {
  return k_cellHeight;
}

KDCoordinate TableController::cellWidth() {
  return k_cellWidth;
}

HighlightCell * TableController::reusableCell(int index) {
  assert(index < k_maxNumberOfCells);
  return &m_cells[index];
}

int TableController::reusableCellCount() const {
  return k_maxNumberOfCells;
}

void TableController::willDisplayCellAtLocation(HighlightCell * cell, int i, int j) {
  AtomicCell* c = static_cast<AtomicCell*>(cell);
  if (i >= 0 && i < k_numberOfColumns && j >= 0 && j < k_numberOfRows) {
    int index = m_atomIndex[i][j];
    if (index >= 0) {
      c->setVisible(true);
      c->setAtom(atomsdefs[index]);
      c->setSearchState(m_searchActive, m_searchMatches[index]);
      // Apply custom coloring if requested
      if (m_coloringActive && m_precomputedColorsValid) {
        // Fast path: use precomputed colors
        c->setCustomColor(m_precomputedBg[index]);
        c->setCustomTextColor(m_precomputedText[index]);
      } else if (m_coloringActive) {
        // Fallback: compute colors only if necessary
        const AtomDef & a = atomsdefs[index];
        KDColor bgColor = Palette::AtomColor[a.type];
        KDColor textColor = Palette::AtomColorHighlighted[a.type];
        c->setCustomColor(bgColor);
        c->setCustomTextColor(textColor);
      } else {
        // No property coloring: reset to default atom type colors
        c->clearCustomColor();
        c->clearCustomTextColor();
      }
    } else {
      c->setVisible(false);
    }
  }
}

void TableController::setColorProperty(ColorProperty p) {
  m_colorProperty = p;
  if (p == ColorByType) {
    m_coloringActive = false;
    m_view.clearPropertyDisplay();
    reloadTableData();
    return;
  }
  // Compute min and max for the selected property
  double minv = DBL_MAX;
  double maxv = -DBL_MAX;
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  for (int i = 0; i < count; i++) {
    const AtomDef & a = atomsdefs[i];
    double v = 0.0;
    switch (p) {
      case ColorByAtomicNumber: v = a.num; break;
      case ColorByNeutrons: v = a.neutrons; break;
      case ColorByMass: v = a.mass; break;
      case ColorByElectronegativity: v = a.electroneg; break;
      case ColorByIonisation: v = a.ionisation; break;
      case ColorByAtomicRadius: v = a.atomicRadius; break;
      case ColorByElectronAffinity: v = a.electronAffinity; break;
      case ColorByMeltingPoint: v = a.meltingPoint; break;
      case ColorByBoilingPoint: v = a.boilingPoint; break;
      case ColorByDensity: v = a.density; break;
      default: v = -1; break;
    }
    if (v < 0) { continue; }
    if (v < minv) { minv = v; }
    if (v > maxv) { maxv = v; }
  }
  if (minv == DBL_MAX || maxv == -DBL_MAX) {
    m_coloringActive = false;
  } else {
    m_propertyMin = minv;
    m_propertyMax = maxv;
    m_coloringActive = true;
  }
  // Precompute colors for each atom for the selected property to avoid blending per-cell during draw
  if (m_coloringActive && m_propertyMax > m_propertyMin) {
    KDColor propertyColorMax = KDColor::RGB24(0);
    KDColor propertyColorMaxHighlighted = KDColor::RGB24(0);
    KDColor propertyColorMin = KDColor::RGB24(0);
    KDColor propertyColorMinHighlighted = KDColor::RGB24(0);
    switch (p) {
      case ColorByAtomicNumber:
        propertyColorMax = Palette::AtomPropertyNum;
        propertyColorMaxHighlighted = Palette::AtomPropertyNumHighlighted;
        propertyColorMin = Palette::AtomPropertyNumMin;
        propertyColorMinHighlighted = Palette::AtomPropertyNumMinHighlighted;
        break;
      case ColorByNeutrons:
        propertyColorMax = Palette::AtomPropertyNeutrons;
        propertyColorMaxHighlighted = Palette::AtomPropertyNeutronsHighlighted;
        propertyColorMin = Palette::AtomPropertyNeutronsMin;
        propertyColorMinHighlighted = Palette::AtomPropertyNeutronsMinHighlighted;
        break;
      case ColorByMass:
        propertyColorMax = Palette::AtomPropertyMass;
        propertyColorMaxHighlighted = Palette::AtomPropertyMassHighlighted;
        propertyColorMin = Palette::AtomPropertyMassMin;
        propertyColorMinHighlighted = Palette::AtomPropertyMassMinHighlighted;
        break;
      case ColorByElectronegativity:
        propertyColorMax = Palette::AtomPropertyElectroneg;
        propertyColorMaxHighlighted = Palette::AtomPropertyElectronegHighlighted;
        propertyColorMin = Palette::AtomPropertyElectronegMin;
        propertyColorMinHighlighted = Palette::AtomPropertyElectronegMinHighlighted;
        break;
      case ColorByAtomicRadius:
        propertyColorMax = Palette::AtomPropertyAtomicRadius;
        propertyColorMaxHighlighted = Palette::AtomPropertyAtomicRadiusHighlighted;
        propertyColorMin = Palette::AtomPropertyAtomicRadiusMin;
        propertyColorMinHighlighted = Palette::AtomPropertyAtomicRadiusMinHighlighted;
        break;
      case ColorByElectronAffinity:
        propertyColorMax = Palette::AtomPropertyElectronAffinity;
        propertyColorMaxHighlighted = Palette::AtomPropertyElectronAffinityHighlighted;
        propertyColorMin = Palette::AtomPropertyElectronAffinityMin;
        propertyColorMinHighlighted = Palette::AtomPropertyElectronAffinityMinHighlighted;
        break;
      case ColorByIonisation:
        propertyColorMax = Palette::AtomPropertyIonisation;
        propertyColorMaxHighlighted = Palette::AtomPropertyIonisationHighlighted;
        propertyColorMin = Palette::AtomPropertyIonisationMin;
        propertyColorMinHighlighted = Palette::AtomPropertyIonisationMinHighlighted;
        break;
      case ColorByMeltingPoint:
        propertyColorMax = Palette::AtomPropertyMeltingPoint;
        propertyColorMaxHighlighted = Palette::AtomPropertyMeltingPointHighlighted;
        propertyColorMin = Palette::AtomPropertyMeltingPointMin;
        propertyColorMinHighlighted = Palette::AtomPropertyMeltingPointMinHighlighted;
        break;
      case ColorByBoilingPoint:
        propertyColorMax = Palette::AtomPropertyBoilingPoint;
        propertyColorMaxHighlighted = Palette::AtomPropertyBoilingPointHighlighted;
        propertyColorMin = Palette::AtomPropertyBoilingPointMin;
        propertyColorMinHighlighted = Palette::AtomPropertyBoilingPointMinHighlighted;
        break;
      case ColorByDensity:
        propertyColorMax = Palette::AtomPropertyDensity;
        propertyColorMaxHighlighted = Palette::AtomPropertyDensityHighlighted;
        propertyColorMin = Palette::AtomPropertyDensityMin;
        propertyColorMinHighlighted = Palette::AtomPropertyDensityMinHighlighted;
        break;
      default:
        break;
    }
    // `propertyColorMin` / `propertyColorMinHighlighted` are read from the theme

    int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
    for (int i = 0; i < count; i++) {
      const AtomDef & a = atomsdefs[i];
      double v = -1;
      switch (p) {
        case ColorByAtomicNumber: v = a.num; break;
        case ColorByNeutrons: v = a.neutrons; break;
        case ColorByMass: v = a.mass; break;
        case ColorByElectronegativity: v = a.electroneg; break;
        case ColorByAtomicRadius: v = a.atomicRadius; break;
        case ColorByElectronAffinity: v = a.electronAffinity; break;
        case ColorByIonisation: v = a.ionisation; break;
        case ColorByMeltingPoint: v = a.meltingPoint; break;
        case ColorByBoilingPoint: v = a.boilingPoint; break;
        case ColorByDensity: v = a.density; break;
        default: v = -1; break;
      }
      if (v < 0) {
        // N/A -> keep theme secondary background / secondary text
        m_precomputedBg[i] = Palette::BackgroundAppsSecondary;
        m_precomputedText[i] = Palette::SecondaryText;
      } else {
        // Map value to [0,1] range and blend between min and max colors
        double ratio = (v - m_propertyMin) / (m_propertyMax - m_propertyMin);
        if (ratio >= 1.0) {
          m_precomputedBg[i] = propertyColorMax;
          m_precomputedText[i] = propertyColorMaxHighlighted;
        } else {
          int alpha = static_cast<int>(ratio * 255.0 + 0.5);
          if (alpha < 0) { alpha = 0; }
          if (alpha > 255) { alpha = 255; }
          m_precomputedBg[i] = KDColor::blend(propertyColorMax, propertyColorMin, static_cast<uint8_t>(alpha));
          m_precomputedText[i] = KDColor::blend(propertyColorMaxHighlighted, propertyColorMinHighlighted, static_cast<uint8_t>(alpha));
        }
      }
    }
    m_precomputedColorsValid = true;
  } else {
    m_precomputedColorsValid = false;
  }
  updateFooterPropertyDisplay(m_cursor, true);
  reloadTableData();
}

void TableController::clearColorProperty() {
  m_coloringActive = false;
  m_colorProperty = ColorByType;
  m_view.clearPropertyDisplay();
  reloadTableData();
}

void TableController::reloadTableData() {
  m_view.selectableTableView()->reloadData(false);
  // Refresh cursor color after cell colors may have changed
  if (m_cursor >= 0) {
    const AtomDef & a = atomsdefs[m_cursor];
    KDColor cursorColor = Palette::AtomColorHighlighted[a.type];
    if (m_coloringActive) {
      cursorColor = m_precomputedColorsValid ? m_precomputedText[m_cursor]
                                             : Palette::AtomColorHighlighted[a.type];
    }
    m_view.updateCursorFrame(a.x, a.y, cursorColor);
  }
}

SelectableTableViewDataSource * TableController::selectionDataSource() const {
  return App::app()->snapshot();
}

void TableController::setSelection(AtomDef atom) {
  int atomIndex = (atom.x >= 0 && atom.x < k_numberOfColumns && atom.y >= 0 && atom.y < k_numberOfRows) ? m_atomIndex[atom.x][atom.y] : -1;
  if (atomIndex >= 0) {
    m_cursor = atomIndex;
  }

  SelectableTableView * table = m_view.selectableTableView();
  if (table->selectedColumn() != atom.x || table->selectedRow() != atom.y) {
    table->selectCellAtLocation(atom.x, atom.y, false);
  }

  m_view.setAtom(atom);
  // Compute cursor color matching willDisplayCellAtLocation logic
  KDColor cursorColor = Palette::AtomColorHighlighted[atom.type];
  if (m_coloringActive && atomIndex >= 0) {
    cursorColor = m_precomputedColorsValid ? m_precomputedText[atomIndex]
                                           : Palette::AtomColorHighlighted[atomsdefs[atomIndex].type];
  }
  m_view.updateCursorFrame(atom.x, atom.y, cursorColor);
  // m_list is updated lazily in handleEvent (OK) and moveCursorInMenu
  updateFooterPropertyDisplay(m_cursor);
}

void TableController::updateFooterPropertyDisplay(int atomIndex, bool force) {
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  if (!m_coloringActive || atomIndex < 0 || atomIndex >= count) {
    if (force || m_footerDisplayValid) {
      m_view.clearPropertyDisplay();
      m_footerDisplayValid = false;
      m_lastFooterLabel = nullptr;
      m_lastFooterValue[0] = '\0';
    }
    return;
  }

  PropertyDisplayData data = propertyDisplayDataForAtom(atomsdefs[atomIndex], m_colorProperty);

  KDColor bg = Palette::BackgroundApps;
  KDColor fg = Palette::PrimaryText;
  if (data.valueForColor < 0) {
    // N/A -> keep theme secondary background / secondary text
    bg = Palette::BackgroundAppsSecondary;
    fg = Palette::SecondaryText;
  } else if (m_propertyMax > m_propertyMin) {
    double ratio = (data.valueForColor - m_propertyMin) / (m_propertyMax - m_propertyMin);
    if (ratio >= 1.0) {
      bg = data.propertyColorMax;
      fg = data.propertyColorMaxHighlighted;
    } else {
      int alpha = static_cast<int>(ratio * 255.0 + 0.5);
      if (alpha < 0) { alpha = 0; }
      if (alpha > 255) { alpha = 255; }
      bg = KDColor::blend(data.propertyColorMax, data.propertyColorMin, static_cast<uint8_t>(alpha));
      fg = KDColor::blend(data.propertyColorMaxHighlighted, data.propertyColorMinHighlighted, static_cast<uint8_t>(alpha));
    }
  }

  if (!force
      && m_footerDisplayValid
      && m_lastFooterLabel == data.label
      && strcmp(m_lastFooterValue, data.value) == 0
      && m_lastFooterBg == bg
      && m_lastFooterFg == fg) {
    return;
  }

  m_view.setPropertyDisplay(data.label, data.value, bg, fg);
  m_footerDisplayValid = true;
  m_lastFooterLabel = data.label;
  strlcpy(m_lastFooterValue, data.value, sizeof(m_lastFooterValue));
  m_lastFooterBg = bg;
  m_lastFooterFg = fg;
}

void TableController::clearSearch() {
  bool hadSearch = m_searchActive || strlen(m_searchBuffer) > 0;
  m_searchBuffer[0] = '\0';
  m_searchStartCursor = -1;
  m_searchActive = false;
  m_bestSearchResult = -1;
  if (m_searchField.isEditing()) {
    m_searchField.setEditing(false);
  }
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  for (int i = 0; i < count; i++) {
    m_searchMatches[i] = true;
  }
  m_view.setInfoVisible(true);
  m_view.setSearchVisible(false);
  m_view.selectableTableView()->reloadData(false);
  if (Container::activeApp()->firstResponder() != this) {
    Container::activeApp()->setFirstResponder(this);
  }
  if (hadSearch) {
    setSelection(atomsdefs[m_cursor]);
  }
}

void TableController::refreshSearchResults() {
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  int searchLen = static_cast<int>(strlen(m_searchBuffer));

  if (searchLen == 0) {
    clearSearch();
    return;
  }

  bool numericSearch = isNumericString(m_searchBuffer);
  bool wasSearchActive = m_searchActive;
  m_searchActive = true;
  m_bestSearchResult = -1;
  int bestScore = -1;
  for (int i = 0; i < count; i++) {
    int score = scoreForSearch(atomsdefs[i], m_searchBuffer, searchLen, numericSearch);
    bool isMatch = score >= 0;
    bool wasMatch = m_searchMatches[i];
    m_searchMatches[i] = isMatch;
    if (score > bestScore) {
      bestScore = score;
      m_bestSearchResult = i;
    }
    if (wasSearchActive && isMatch != wasMatch) {
      m_view.selectableTableView()->reloadCellAtLocation(atomsdefs[i].x, atomsdefs[i].y);
    }
  }
  if (m_bestSearchResult >= 0) {
    m_cursor = m_bestSearchResult;
    setSelection(atomsdefs[m_cursor]);
    m_view.setInfoVisible(true);
  } else {
    m_view.setInfoVisible(false);
    m_view.selectableTableView()->deselectTable();
    // No matches -> hide the cursor overlay
    m_view.hideCursor();
  }
  if (!wasSearchActive) {
    m_view.selectableTableView()->reloadData(false);
  }
}

bool TableController::textFieldShouldFinishEditing(TextField * textField, Ion::Events::Event event) {
  return event == Ion::Events::OK || event == Ion::Events::EXE;
}

bool TableController::textFieldDidReceiveEvent(TextField * textField, Ion::Events::Event event) {
  if (event == Ion::Events::Up) {
    int nextIndex = nextSearchResultIndex(-1);
    if (nextIndex >= 0) {
      m_cursor = nextIndex;
      setSelection(atomsdefs[m_cursor]);
    }
    return true;
  }
  if (event == Ion::Events::Down) {
    int nextIndex = nextSearchResultIndex(1);
    if (nextIndex >= 0) {
      m_cursor = nextIndex;
      setSelection(atomsdefs[m_cursor]);
    }
    return true;
  }
  return false;
}

bool TableController::textFieldDidFinishEditing(TextField * textField, const char * text, Ion::Events::Event event) {
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  if (m_cursor >= 0 && m_cursor < count && m_searchMatches[m_cursor]) {
    // Keep current selection chosen with Up/Down
  } else if (m_bestSearchResult >= 0) {
    m_cursor = m_bestSearchResult;
  } else if (m_searchStartCursor >= 0) {
    m_cursor = m_searchStartCursor;
  }
  setSelection(atomsdefs[m_cursor]);
  clearSearch();
  return true;
}

bool TableController::textFieldDidAbortEditing(TextField * textField) {
  clearSearch();
  return true;
}

bool TableController::textFieldDidHandleEvent(TextField * textField, bool returnValue, bool textSizeDidChange) {
  if (textSizeDidChange) {
    const char * draft = textField->draftTextBuffer();
    strlcpy(m_searchBuffer, draft, sizeof(m_searchBuffer));
    refreshSearchResults();
  }
  return returnValue;
}

int TableController::nextSearchResultIndex(int direction) const {
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  if (count <= 0 || m_bestSearchResult < 0) {
    return -1;
  }

  int start = (m_cursor >= 0 && m_cursor < count) ? m_cursor : m_bestSearchResult;
  int stepDirection = direction < 0 ? -1 : 1;
  for (int step = 1; step <= count; step++) {
    int index = (start + stepDirection * step) % count;
    if (index < 0) {
      index += count;
    }
    if (m_searchMatches[index]) {
      return index;
    }
  }
  return -1;
}

int TableController::scoreForSearch(const AtomDef & atom, const char * query, int queryLength, bool isNumeric) const {
  if (isNumeric) {
    char atomNumberString[4] = {'\0'};
    int atomNum = atom.num;
    if (atomNum >= 100) {
      atomNumberString[0] = '0' + atomNum / 100;
      atomNumberString[1] = '0' + (atomNum / 10) % 10;
      atomNumberString[2] = '0' + atomNum % 10;
    } else if (atomNum >= 10) {
      atomNumberString[0] = '0' + atomNum / 10;
      atomNumberString[1] = '0' + atomNum % 10;
    } else {
      atomNumberString[0] = '0' + atomNum;
    }
    if (strcmp(atomNumberString, query) == 0) {
      return 1000;
    }
    if (startsWithIgnoreCase(atomNumberString, query)) {
      return 700 - (static_cast<int>(strlen(atomNumberString)) - queryLength);
    }
    return -1;
  }

  const char * name = I18n::translate(atom.name);
  bool symbolStartsWith = startsWithIgnoreCase(atom.symbol, query);
  bool nameStartsWith = startsWithIgnoreCase(name, query);
  bool symbolExact = symbolStartsWith && static_cast<int>(strlen(atom.symbol)) == queryLength;
  bool nameExact = nameStartsWith && static_cast<int>(strlen(name)) == queryLength;

  if (queryLength < 2) {
    if (symbolExact) {
      return 1000;
    }
    if (symbolStartsWith) {
      return 900;
    }
    if (nameExact) {
      return 850;
    }
    if (nameStartsWith) {
      return 750;
    }
    return -1;
  }

  if (symbolExact) {
    return 1200;
  }
  if (symbolStartsWith) {
    return 1000;
  }
  if (nameExact) {
    return 1000;
  }
  if (nameStartsWith) {
    return 900;
  }
  return -1;
}

bool TableController::startsWithIgnoreCase(const char * text, const char * query) {
  const char * textCursor = text;
  const char * queryCursor = query;
  while (true) {
    char queryChar = nextFoldedSearchChar(&queryCursor);
    if (queryChar == 0) {
      return true;
    }
    char textChar = nextFoldedSearchChar(&textCursor);
    if (textChar == 0) {
      return false;
    }
    if (textChar != queryChar) {
      return false;
    }
  }
}

bool TableController::isNumericString(const char * text) {
  if (text == nullptr || text[0] == '\0') {
    return false;
  }
  while (*text != '\0') {
    if (!isAsciiDigit(*text)) {
      return false;
    }
    text++;
  }
  return true;
}

StackViewController * TableController::stackController() const {
  return (StackViewController *)parentResponder();
}

}
