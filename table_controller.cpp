#include "table_controller.h"
#include "app.h"
#include "../apps_container.h"
#include <ion/unicode/utf8_decoder.h>
#include <string.h>
#include <escher/palette.h>

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
  return 5;
}

View * TableController::ContentView::subviewAtIndex(int index) {
  switch(index) {
    case 0:
      return &m_selectableTableView;
    case 1:
      return &m_ok;
    case 2:
      return &m_info;
    case 3:
      return &m_lines;
    case 4:
      return &m_typeFooter;
    default:
      assert(false);
      return nullptr;
  }
}

void TableController::ContentView::layoutSubviews(bool force) {
  m_selectableTableView.setFrame(KDRect(bounds().top(), bounds().left() + 20 , bounds().width(), bounds().height() - 20), force);
  m_ok.setFrame(KDRect(295, 200, m_ok.minimalSizeForOptimalDisplay()), force);
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
      if (m_bestSearchResult >= 0) {
        m_cursor = m_bestSearchResult;
      } else if (m_searchStartCursor >= 0) {
        m_cursor = m_searchStartCursor;
      }
      setSelection(atomsdefs[m_cursor]);
      clearSearch();
      return true;
    }
    m_menuIsOpen = true;
    Container::activeApp()->displayModalViewController(&m_list, 0.f, 0.f, Metric::CommonTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
    m_list.unhighlightTopCells();
    return true;
  }
  return false;
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
      return;
    }
  }
  c->setVisible(false);
}

SelectableTableViewDataSource * TableController::selectionDataSource() const {
  return App::app()->snapshot();
}

void TableController::setSelection(AtomDef atom) {
  m_view.selectableTableView()->selectCellAtLocation(atom.x,atom.y,false);
  m_view.setAtom(atom);
  m_list.setAtom(atomsdefs[m_cursor]);
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
