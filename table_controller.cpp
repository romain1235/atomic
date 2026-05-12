#include "table_controller.h"
#include "app.h"
#include "../apps_container.h"
#include <escher/palette.h>

extern "C" {
#include <assert.h>
}

namespace Atomic {

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
  // Dessiner uniquement le fond de la vue
  ctx->fillRect(bounds(), Palette::BackgroundApps);

  // Afficher le type de l'élément sélectionné en bas à gauche
  AtomDef atom = m_info.atom();
  I18n::Message typeMsg = AtomicI18nForType[atom.type];
  KDSize typeSize = KDFont::SmallFont->stringSize(I18n::translate(typeMsg));
  int x = 8;
  int y = 200;
  ctx->drawString(I18n::translate(typeMsg), KDPoint(x, y), KDFont::SmallFont, Palette::PrimaryText, Palette::BackgroundApps);
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
    m_typeFooter.minimalSizeForOptimalDisplay().width(), m_typeFooter.minimalSizeForOptimalDisplay().height()), force);
}

void TableController::ContentView::setAtom(AtomDef atom) {
  m_info.setAtom(atom);
  m_typeFooter.setType(AtomicI18nForType[atom.type]);
}

TableController::TableController(Responder * parentResponder, SelectableTableViewDataSource * selectionDataSource) :
  ViewController(parentResponder),
  m_view(this, selectionDataSource),
  m_list(this)
{
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
  }
}

bool TableController::handleEvent(Ion::Events::Event event) {
  if (m_menuIsOpen && event != Ion::Events::OK && event != Ion::Events::EXE) {
    // Bloquer tous les événements sauf la fermeture du menu
    return false;
  }
  if (event == Ion::Events::Right && m_cursor < static_cast<int>(sizeof(atomsdefs) / sizeof(AtomDef) - 1)) {
    AtomDef atom = atomsdefs[++m_cursor];
    setSelection(atom);
    return true;
  }
  if (event == Ion::Events::Left && m_cursor > 0) {
    AtomDef atom = atomsdefs[--m_cursor];
    setSelection(atom);
    return true;
  }
  if (event == Ion::Events::Up) {
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
    m_menuIsOpen = true;
    Container::activeApp()->displayModalViewController(&m_list, 0.f, 0.f, Metric::CommonTopMargin, Metric::PopUpLeftMargin, 0, Metric::PopUpRightMargin);
    m_list.unhighlightTopCells();
    return true;
  }
  return false;
}

void TableController::didBecomeFirstResponder() {
  m_menuIsOpen = false;
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

StackViewController * TableController::stackController() const {
  return (StackViewController *)parentResponder();
}

}
