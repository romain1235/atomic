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
  KDColor propertyColor;
  KDColor propertyColorHighlighted;
  double valueForColor;
};

static PropertyDisplayData propertyDisplayDataForAtom(const AtomDef & a, TableController::ColorProperty property) {
  PropertyDisplayData data = {
    "",
    "",
    Palette::AtomColor[a.type],
    Palette::AtomColorHighlighted[a.type],
    -1
  };

  switch (property) {
    case TableController::ColorByAtomicNumber:
      data.label = I18n::translate(I18n::Message::AtomNum);
      Poincare::Integer(a.num).serialize(data.value, sizeof(data.value));
      data.propertyColor = Palette::AtomPropertyNum;
      data.propertyColorHighlighted = Palette::AtomPropertyNumHighlighted;
      data.valueForColor = a.num;
      break;
    case TableController::ColorByNeutrons:
      data.label = I18n::translate(I18n::Message::AtomNeutrons);
      Poincare::Integer(a.neutrons).serialize(data.value, sizeof(data.value));
      data.propertyColor = Palette::AtomPropertyNeutrons;
      data.propertyColorHighlighted = Palette::AtomPropertyNeutronsHighlighted;
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
      data.propertyColor = Palette::AtomPropertyMass;
      data.propertyColorHighlighted = Palette::AtomPropertyMassHighlighted;
      data.valueForColor = a.mass;
      break;
    case TableController::ColorByElectronegativity:
      data.label = I18n::translate(I18n::Message::AtomElectroneg);
      if (a.electroneg < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        Poincare::Number::FloatNumber(a.electroneg).serialize(data.value, sizeof(data.value), Poincare::Preferences::PrintFloatMode::Decimal, 2);
      }
      data.propertyColor = Palette::AtomPropertyElectroneg;
      data.propertyColorHighlighted = Palette::AtomPropertyElectronegHighlighted;
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
      data.propertyColor = Palette::AtomPropertyAtomicRadius;
      data.propertyColorHighlighted = Palette::AtomPropertyAtomicRadiusHighlighted;
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
      data.propertyColor = Palette::AtomPropertyElectronAffinity;
      data.propertyColorHighlighted = Palette::AtomPropertyElectronAffinityHighlighted;
      data.valueForColor = a.electronAffinity;
      break;
    case TableController::ColorByMeltingPoint:
      data.label = I18n::translate(I18n::Message::AtomMeltingPoint);
      if (a.meltingPoint < 0) {
        strlcpy(data.value, "N/A", sizeof(data.value));
      } else {
        Poincare::Integer(static_cast<int>(a.meltingPoint)).serialize(data.value, sizeof(data.value));
        strlcat(data.value, " K", sizeof(data.value));
      }
      data.propertyColor = Palette::AtomPropertyMeltingPoint;
      data.propertyColorHighlighted = Palette::AtomPropertyMeltingPointHighlighted;
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
      data.propertyColor = Palette::AtomPropertyBoilingPoint;
      data.propertyColorHighlighted = Palette::AtomPropertyBoilingPointHighlighted;
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
      data.propertyColor = Palette::AtomPropertyDensity;
      data.propertyColorHighlighted = Palette::AtomPropertyDensityHighlighted;
      data.valueForColor = a.density;
      break;
    default:
      break;
  }

  return data;
}

TableController::ContentView::ContentView(TableController * controller, SelectableTableViewDataSource * selectionDataSource) :
  m_selectableTableView(controller, controller, selectionDataSource, controller)
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
  return 4;
}

View * TableController::ContentView::subviewAtIndex(int index) {
  switch(index) {
    case 0:
      return &m_selectableTableView;
    case 1:
      return &m_info;
    case 2:
      return &m_lines;
    case 3:
      return &m_typeFooter;
    default:
      assert(false);
      return nullptr;
  }
}

void TableController::ContentView::layoutSubviews(bool force) {
  m_selectableTableView.setFrame(KDRect(bounds().top(), bounds().left() + 20 , bounds().width(), bounds().height() - 20), force);
  m_info.setFrame(KDRect(KDPoint(48, 15),m_info.minimalSizeForOptimalDisplay()), force);
  m_lines.setFrame(KDRect(KDPoint(40, 99 + 20), m_lines.minimalSizeForOptimalDisplay()), force);
  m_typeFooter.setFrame(KDRect(0, bounds().height() - m_typeFooter.minimalSizeForOptimalDisplay().height(),
    bounds().width(), m_typeFooter.minimalSizeForOptimalDisplay().height()), force);
}

void TableController::ContentView::setAtom(AtomDef atom) {
  m_info.setAtom(atom);
  m_typeFooter.setType(atom.type);
}

void TableController::ContentView::setSearchInput(bool active, const char * text, int cursor) {
  m_typeFooter.setSearchInput(active, text, cursor);
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

TableController::TableController(Responder * parentResponder, SelectableTableViewDataSource * selectionDataSource) :
  ViewController(parentResponder),
  m_view(this, selectionDataSource),
  m_searchLength(0),
  m_searchCursor(0),
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

  if (event.hasText() && strlen(event.text()) == 1) {
    char c = event.text()[0];
    if (isAsciiAlnum(c)) {
      appendCharacterToSearch(c);
      return true;
    }
  }

  if (event == Ion::Events::Backspace && m_searchLength > 0) {
    removeCharacterFromSearch();
    return true;
  }

  if (m_searchActive && (event == Ion::Events::Left || event == Ion::Events::Right)) {
    if (event == Ion::Events::Left && m_searchCursor > 0) {
      m_searchCursor--;
    }
    if (event == Ion::Events::Right && m_searchCursor < m_searchLength) {
      m_searchCursor++;
    }
    m_view.setSearchInput(true, m_searchBuffer, m_searchCursor);
    return true;
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
      if (row == 8) {
        row--;
      }
      for(size_t i = 0; i < (sizeof(atomsdefs) / sizeof(AtomDef)); i++) {
        AtomDef atom = atomsdefs[i];
        if (atom.x == column && atom.y == row-1) {
          m_cursor = i;
          setSelection(atom);
          return true;
        }
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
      if (row == 6) {row++;}
      for(size_t i = 0; i < (sizeof(atomsdefs) / sizeof(AtomDef)); i++) {
        AtomDef atom = atomsdefs[i];
        if (atom.x == column && atom.y == row+1) {
          m_cursor = i;
          setSelection(atom);
          return true;
        }
      }
    }
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
    Container::activeApp()->displayModalViewController(&m_list, 0.f, 0.f, 0, 0, 0, 0);
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
  // Update modal header title to reflect the newly selected atom
  // If headers are displayed, subview(0) is the StackView for the title bar.
  View * stackSubview = m_list.view()->subview(0);
  StackView * stackView = static_cast<StackView *>(stackSubview);
  if (stackView) {
    stackView->setNamedController(m_list.topViewController());
  }
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
    KDColor propertyColor = KDColor::RGB24(0);
    KDColor propertyColorHighlighted = KDColor::RGB24(0);
    switch (p) {
      case ColorByAtomicNumber:
        propertyColor = Palette::AtomPropertyNum;
        propertyColorHighlighted = Palette::AtomPropertyNumHighlighted;
        break;
      case ColorByNeutrons:
        propertyColor = Palette::AtomPropertyNeutrons;
        propertyColorHighlighted = Palette::AtomPropertyNeutronsHighlighted;
        break;
      case ColorByMass:
        propertyColor = Palette::AtomPropertyMass;
        propertyColorHighlighted = Palette::AtomPropertyMassHighlighted;
        break;
      case ColorByElectronegativity:
        propertyColor = Palette::AtomPropertyElectroneg;
        propertyColorHighlighted = Palette::AtomPropertyElectronegHighlighted;
        break;
      case ColorByAtomicRadius:
        propertyColor = Palette::AtomPropertyAtomicRadius;
        propertyColorHighlighted = Palette::AtomPropertyAtomicRadiusHighlighted;
        break;
      case ColorByElectronAffinity:
        propertyColor = Palette::AtomPropertyElectronAffinity;
        propertyColorHighlighted = Palette::AtomPropertyElectronAffinityHighlighted;
        break;
      case ColorByMeltingPoint:
        propertyColor = Palette::AtomPropertyMeltingPoint;
        propertyColorHighlighted = Palette::AtomPropertyMeltingPointHighlighted;
        break;
      case ColorByBoilingPoint:
        propertyColor = Palette::AtomPropertyBoilingPoint;
        propertyColorHighlighted = Palette::AtomPropertyBoilingPointHighlighted;
        break;
      case ColorByDensity:
        propertyColor = Palette::AtomPropertyDensity;
        propertyColorHighlighted = Palette::AtomPropertyDensityHighlighted;
        break;
      default:
        break;
    }
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
        case ColorByMeltingPoint: v = a.meltingPoint; break;
        case ColorByBoilingPoint: v = a.boilingPoint; break;
        case ColorByDensity: v = a.density; break;
        default: v = -1; break;
      }
      if (v < 0) {
        m_precomputedBg[i] = Palette::BackgroundAppsSecondary;
        m_precomputedText[i] = Palette::SecondaryText;
      } else {
        double ratio = (v - m_propertyMin) / (m_propertyMax - m_propertyMin);
        if (ratio < 0.0) { ratio = 0.0; }
        if (ratio > 1.0) { ratio = 1.0; }
        uint8_t alpha = static_cast<uint8_t>(ratio * 255);
        m_precomputedBg[i] = KDColor::blend(propertyColor, Palette::BackgroundAppsSecondary, alpha);
        m_precomputedText[i] = KDColor::blend(propertyColorHighlighted, Palette::SecondaryText, alpha);
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
  m_list.setAtom(atomsdefs[m_cursor]);
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
    bg = Palette::BackgroundAppsSecondary;
    fg = Palette::SecondaryText;
  } else if (m_propertyMax > m_propertyMin) {
    double ratio = (data.valueForColor - m_propertyMin) / (m_propertyMax - m_propertyMin);
    if (ratio < 0.0) { ratio = 0.0; }
    if (ratio > 1.0) { ratio = 1.0; }
    uint8_t alpha = static_cast<uint8_t>(ratio * 255);
    bg = KDColor::blend(data.propertyColor, Palette::BackgroundAppsSecondary, alpha);
    fg = KDColor::blend(data.propertyColorHighlighted, Palette::SecondaryText, alpha);
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

void TableController::appendCharacterToSearch(char c) {
  if (m_searchLength == 0) {
    m_searchStartCursor = m_cursor;
  }
  if (m_searchLength >= static_cast<int>(sizeof(m_searchBuffer) - 1)) {
    return;
  }
  for (int i = m_searchLength; i > m_searchCursor; i--) {
    m_searchBuffer[i] = m_searchBuffer[i - 1];
  }
  m_searchBuffer[m_searchCursor] = c;
  m_searchLength++;
  m_searchCursor++;
  m_searchBuffer[m_searchLength] = '\0';
  refreshSearchResults();
}

void TableController::removeCharacterFromSearch() {
  if (m_searchLength <= 0 || m_searchCursor <= 0) {
    return;
  }
  for (int i = m_searchCursor - 1; i < m_searchLength - 1; i++) {
    m_searchBuffer[i] = m_searchBuffer[i + 1];
  }
  m_searchCursor--;
  m_searchLength--;
  m_searchBuffer[m_searchLength] = '\0';
  refreshSearchResults();
}

void TableController::clearSearch() {
  bool hadSearch = m_searchActive || m_searchLength > 0;
  m_searchLength = 0;
  m_searchCursor = 0;
  m_searchBuffer[0] = '\0';
  m_searchStartCursor = -1;
  m_searchActive = false;
  m_bestSearchResult = -1;
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  for (int i = 0; i < count; i++) {
    m_searchMatches[i] = true;
  }
  m_view.setInfoVisible(true);
  m_view.setSearchInput(false, nullptr, 0);
  m_view.selectableTableView()->reloadData(false);
  if (hadSearch) {
    setSelection(atomsdefs[m_cursor]);
  }
}

void TableController::refreshSearchResults() {
  int count = static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef));
  if (m_searchLength <= 0) {
    clearSearch();
    return;
  }

  bool numericSearch = isNumericString(m_searchBuffer);
  bool wasSearchActive = m_searchActive;
  m_searchActive = true;
  m_bestSearchResult = -1;
  int bestScore = -1;
  for (int i = 0; i < count; i++) {
    int score = scoreForSearch(atomsdefs[i], m_searchBuffer, m_searchLength, numericSearch);
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
  }
  m_view.setSearchInput(true, m_searchBuffer, m_searchCursor);
  if (!wasSearchActive) {
    m_view.selectableTableView()->reloadData(false);
  }
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
