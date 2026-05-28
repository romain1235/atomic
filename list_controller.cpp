#include "list_controller.h"
#include "table_controller.h"
#include <poincare_layouts.h>
#include <poincare_nodes.h>
#include <poincare/float.h>
#include <string.h>

namespace Atomic {

ListController::ListController(Responder * parentResponder) :
  ViewController(parentResponder),
  m_selectableTableView(this, this, this, this),
  m_parent(parentResponder)
{
  m_selectableTableView.setDecoratorType(ScrollView::Decorator::Type::Bars);
  m_selectableTableView.setMargins(0, 13, 0, 0);
  for (int i = 0; i < k_numberOfCellsWithBuffer; i++) {
    m_cellsWithBuffer[i].setMessageFont(KDFont::LargeFont);
  }
  for (int i = 0; i < k_numberOfCellsWithExpression; i++) {
    m_cellsWithExpression[i].setMessageFont(KDFont::LargeFont);
  }
}

const char * ListController::title() {
  return I18n::translate(m_atom.name);
}

bool ListController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Right || event == Ion::Events::Left) {
    TableController * parentTC = reinterpret_cast<TableController *>(m_parent);
    return parentTC->moveCursorInMenu(event == Ion::Events::Right ? 1 : -1);
  }

  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    int focusRow = selectedRow();
    TableController * parentTC = reinterpret_cast<TableController *>(m_parent);
    // Map rows to properties: 2=num, 3=neutrons, 4=types, 5=mass, 6=electroneg
    if (focusRow >= 2 && focusRow <= 6) {
      static_cast<StackViewController *>(parentResponder())->pop();
      switch (focusRow) {
        case 2:
          parentTC->setColorProperty(TableController::ColorByAtomicNumber);
          break;
        case 3:
          parentTC->setColorProperty(TableController::ColorByNeutrons);
          break;
        case 4:
          parentTC->clearColorProperty();
          break;
        case 5:
          parentTC->setColorProperty(TableController::ColorByMass);
          break;
        case 6:
          parentTC->setColorProperty(TableController::ColorByElectronegativity);
          break;
      }
      parentTC->reloadTableData();
      return true;
    }
    // New property rows (after Electronical row) -> 8..13
    if (focusRow >= 8 && focusRow <= 13) {
      static_cast<StackViewController *>(parentResponder())->pop();
      switch (focusRow) {
        case 8:
          parentTC->setColorProperty(TableController::ColorByAtomicRadius);
          break;
        case 9:
          parentTC->setColorProperty(TableController::ColorByElectronAffinity);
          break;
        case 10:
          parentTC->setColorProperty(TableController::ColorByIonisation);
          break;
        case 11:
          parentTC->setColorProperty(TableController::ColorByMeltingPoint);
          break;
        case 12:
          parentTC->setColorProperty(TableController::ColorByBoilingPoint);
          break;
        case 13:
          parentTC->setColorProperty(TableController::ColorByDensity);
          break;
      }
      parentTC->reloadTableData();
      return true;
    }
  }

  return false;
}

void ListController::didBecomeFirstResponder() {
  m_selectableTableView.reloadData(false);
  m_selectableTableView.selectCellAtLocation(0, 1);
  // Ensure the selectable table view receives input events
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

void ListController::setPropertyColors(KDColor bg, KDColor text) {
  m_propertyBg = bg;
  m_propertyText = text;
  m_hasPropertyColors = true;
  m_selectableTableView.reloadData(false);
  m_selectableTableView.forceRedraw();
}

void ListController::clearPropertyColors() {
  if (m_hasPropertyColors) {
    m_hasPropertyColors = false;
    m_selectableTableView.reloadData(false);
    m_selectableTableView.forceRedraw();
  }
}

void ListController::refreshNavigation() {
  // Re-announce the current selection to the delegate (withinTemporarySelection=false)
  // so that scroll-to-row-0 fires for row 1, and the first responder is properly set.
  int row = m_selectableTableView.selectedRow();
  if (row <= 0) row = 1;
  m_selectableTableView.selectCellAtLocation(0, row);
  // Ensure first responder stays on the selectable table after programmatic selection
  Container::activeApp()->setFirstResponder(&m_selectableTableView);
}

int ListController::numberOfRows() const {
  return k_numberOfRow;
}

KDCoordinate ListController::rowHeight(int j) {
  if (j == 0) {
    return k_atomicCellRowHeight;
  }
  if (j == 7) {
    return 35;
  }
  return k_classicalRowHeight;
}


HighlightCell * ListController::reusableCell(int index, int type) {
  switch (type) {
    case 0:
      {
        assert(index == 0);
        return &m_atomicCell;
      }
    case 1:
      {
        assert(index < k_numberOfCellsWithBuffer);
        return &m_cellsWithBuffer[index];
      }
    case 2:
      {
        assert(index < k_numberOfCellsWithExpression);
        return &m_cellsWithExpression[index];
      }
    default:
      {
        assert(false);
        return nullptr;
      }
  }
}

void ListController::tableViewDidChangeSelection(SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY, bool withinTemporarySelection) {
  if (withinTemporarySelection) {
    return;
  }
  // Forbid selecting ListAtomicCell
  if (t->selectedRow() == 0) {
    t->selectCellAtLocation(0, 1);
  }
  /* But scroll to the top when we select the first
   * cell in order display the ListAtomicCell. */
  if (t->selectedRow() == 1) {
    t->scrollToCell(0, 0);
  }
}

int ListController::reusableCellCount(int type) {
  switch(type) {
    case 0:
      return 1;
    case 1:
      return k_numberOfCellsWithBuffer;
    case 2:
      return k_numberOfCellsWithExpression;
    default:
      assert(false);
      return 0;
  }
}

int ListController::typeAtLocation(int i, int j) {
  if (j == 0) {
    return 0;
  } else if (j == 1 || j == 4) {
    return 1;
  } else {
    return 2;
  }
}

void ListController::setAtom(AtomDef atom) {
  // Guard: skip recomputation if this atom is already loaded
  if (m_cachedElectronicalAtomNum == atom.num) {
    return;
  }
  m_atom = atom;
  m_cachedElectronicalAtomNum = atom.num;
  m_cachedElectronicalLayout = Electronical::createElectronical(atom);
  m_selectableTableView.reloadData(false);
  m_selectableTableView.forceRedraw();
}

void ListController::unhighlightTopCells() {
  m_cellsWithExpression[0].setHighlighted(false);
  m_cellsWithExpression[1].setHighlighted(false);
  // FIXME This fix is ugly (just supposing that there's the 2 first cellsWithExpression that can be seen when scrolling on 1st cell)
  // This assert is supposed to be triggered if the view is modified and this fix is not working anymore...
  assert(rowHeight(0) + rowHeight(1) + rowHeight(2) + rowHeight(3) >= m_selectableTableView.bounds().height());
}

void ListController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  switch (index) {
    case 0 : {
      m_atomicCell.setAtom(m_atom);
      if (m_hasPropertyColors) {
        m_atomicCell.setCustomColor(m_propertyBg);
        m_atomicCell.setCustomTextColor(m_propertyText);
      } else {
        m_atomicCell.clearCustomColor();
        m_atomicCell.clearCustomTextColor();
      }
      return;
    }
    case 1: {
      MessageTableCellWithBuffer * myCell = (MessageTableCellWithBuffer *)cell;
      myCell->setMessage(I18n::Message::AtomSymbol);
      myCell->setAccessoryText(m_atom.symbol);
      myCell->setAccessoryFont(KDFont::SmallFont);
      return;
    }
    case 2: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomNum);
      myCell->setLayoutWithCopy(Poincare::Integer(m_atom.num).createLayout());
      return;
    }
    case 3: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomNeutrons);
      myCell->setLayoutWithCopy(Poincare::Integer(m_atom.neutrons).createLayout());
      return;
    }
    case 4: {
      MessageTableCellWithBuffer * myCell = (MessageTableCellWithBuffer *)cell;
      myCell->setMessage(I18n::Message::AtomTypes);
      myCell->setAccessoryText(I18n::translate(AtomicI18nForType[static_cast<int>(m_atom.type)]));
      myCell->setAccessoryFont(KDFont::SmallFont);
      return;
    }
    case 5: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomMass);
      if (m_atom.mass < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayout = Poincare::FloatNode<double>(m_atom.mass).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7);
      // Build unit layout: " g·mol" with superscript -1 (small font)
      Poincare::Layout unitBase = Poincare::LayoutHelper::String(" g·mol", strlen(" g·mol"), KDFont::SmallFont);
      Poincare::Layout unitExp = Poincare::VerticalOffsetLayout::Builder(Poincare::LayoutHelper::String("-1", 2, KDFont::SmallFont), Poincare::VerticalOffsetLayoutNode::Position::Superscript);
      Poincare::Layout unitLayout = Poincare::HorizontalLayout::Builder(unitBase, unitExp);
      Poincare::HorizontalLayout fullLayout = Poincare::HorizontalLayout::Builder(numLayout, unitLayout);
      myCell->setLayoutWithCopy(fullLayout);
      return;
    }
    case 6: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomElectroneg);
      if (m_atom.electroneg < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      myCell->setLayoutWithCopy(Poincare::FloatNode<double>(m_atom.electroneg).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 5));
      return;
    }
    case 7: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomEC);
      myCell->setLayoutWithCopy(m_cachedElectronicalLayout);
      return;
    }
    case 8: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomAtomicRadius);
      if (m_atom.atomicRadius < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayoutAR = Poincare::FloatNode<double>(m_atom.atomicRadius).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 3);
      Poincare::HorizontalLayout fullLayoutAR = Poincare::HorizontalLayout::Builder(numLayoutAR, Poincare::LayoutHelper::String(" pm", strlen(" pm"), KDFont::SmallFont));
      myCell->setLayoutWithCopy(fullLayoutAR);
      return;
    }
    case 9: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomElectronAffinity);
      if (m_atom.electronAffinity < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayoutEA = Poincare::FloatNode<double>(m_atom.electronAffinity).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 5);
      Poincare::HorizontalLayout fullLayoutEA = Poincare::HorizontalLayout::Builder(numLayoutEA, Poincare::LayoutHelper::String(" eV", strlen(" eV"), KDFont::SmallFont));
      myCell->setLayoutWithCopy(fullLayoutEA);
      return;
    }
    case 10: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomIonisation);
      if (m_atom.ionisation < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayoutI = Poincare::FloatNode<double>(m_atom.ionisation).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 5);
      Poincare::HorizontalLayout fullLayoutI = Poincare::HorizontalLayout::Builder(numLayoutI, Poincare::LayoutHelper::String(" eV", strlen(" eV"), KDFont::SmallFont));
      myCell->setLayoutWithCopy(fullLayoutI);
      return;
    }
    case 11: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomMeltingPoint);
      if (m_atom.meltingPoint < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayoutM = Poincare::Integer(static_cast<int>(m_atom.meltingPoint)).createLayout();
      Poincare::HorizontalLayout fullLayoutM = Poincare::HorizontalLayout::Builder(numLayoutM, Poincare::LayoutHelper::String(" K", strlen(" K"), KDFont::SmallFont));
      myCell->setLayoutWithCopy(fullLayoutM);
      return;
    }
    case 12: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomBoilingPoint);
      if (m_atom.boilingPoint < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayoutB = Poincare::Integer(static_cast<int>(m_atom.boilingPoint)).createLayout();
      Poincare::HorizontalLayout fullLayoutB = Poincare::HorizontalLayout::Builder(numLayoutB, Poincare::LayoutHelper::String(" K", strlen(" K"), KDFont::SmallFont));
      myCell->setLayoutWithCopy(fullLayoutB);
      return;
    }
    case 13: {
      MessageTableCellWithExpressionWithCopy * myCell = (MessageTableCellWithExpressionWithCopy *)cell;
      myCell->setMessage(I18n::Message::AtomDensity);
      if (m_atom.density < 0) {
        myCell->setLayoutWithCopy(Poincare::LayoutHelper::String("N/A", strlen("N/A")));
        return;
      }
      Poincare::Layout numLayoutD = Poincare::FloatNode<double>(m_atom.density).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 6);
      // Build unit layout: " g·cm" with superscript -3
      Poincare::Layout unitBaseD = Poincare::LayoutHelper::String(" g·cm", strlen(" g·cm"), KDFont::SmallFont);
      Poincare::Layout unitExpD = Poincare::VerticalOffsetLayout::Builder(Poincare::LayoutHelper::String("-3", 2, KDFont::SmallFont), Poincare::VerticalOffsetLayoutNode::Position::Superscript);
      Poincare::Layout unitLayoutD = Poincare::HorizontalLayout::Builder(unitBaseD, unitExpD);
      Poincare::HorizontalLayout fullLayoutD = Poincare::HorizontalLayout::Builder(numLayoutD, unitLayoutD);
      myCell->setLayoutWithCopy(fullLayoutD);
      return;
    }
    default: {
      assert(false);
    }
  }
}

Poincare::Layout ListController::Electronical::createElectronical(AtomDef atom) {
  Poincare::Layout layouts[6];

  int y = (atom.y < 8) ?  atom.y : atom.y - 3;

  if (atom.y > 0) {
    char previousAtom[5] = {'[', ' ', ' ', ']', '\0'};
    for (AtomDef a : atomsdefs) {
      int ay = (a.y < 8) ? a.y : a.y - 3;
      if (ay == y - 1 && a.x == 17) {
        memcpy(&previousAtom[1], a.symbol, 2);
      }
    };
    layouts[0] = Poincare::LayoutHelper::String(previousAtom, strlen(previousAtom));
  }

  bool isException = false;
  exceptionStruct exceptionContent = Electronical::exceptions[0]; //We initialize it with random value to silence compilator warning (-Wmaybe-uninitialized)
  for(exceptionStruct e : exceptions) {
    if (e.num == atom.num) {
      isException = true;
      exceptionContent = e;
    }
  }

  int indexAtRow = -1;
  if (!isException) {
    for (AtomDef a : atomsdefs) {
      int ay = (a.y < 8) ? a.y : a.y - 3;
      if (ay == y) {
        indexAtRow = atom.num - a.num + 1;
        break;
      }
    }
    assert(indexAtRow != -1);
  }

  int s=0, f=0, d=0, p=0;
  Electronical::rowsSubLayers row = Electronical::rows[y];

  if (!isException) {
    bool sEnabled = row.s, fEnabled = row.f, dEnabled = row.d, pEnabled = row.p;
    int toOrder = indexAtRow;
    for (int i = 0; i < indexAtRow; i++) {
      if (sEnabled && s < 2) {
        s++;
        toOrder--;
      } else if (fEnabled && f < 14) {
        f++;
        toOrder--;
      } else if (dEnabled && d < 10) {
        d++;
        toOrder--;
      } else if (pEnabled && p < 6) {
        p++;
        toOrder--;
      }
    }
    assert(toOrder == 0);
  } else {
    s = exceptionContent.s ? (exceptionContent.sContent) : 0;
    f = exceptionContent.f ? (exceptionContent.fContent) : 0;
    d = exceptionContent.d ? (exceptionContent.dContent) : 0;
    p = exceptionContent.p ? (exceptionContent.pContent) : 0;
  }

  int index = (layouts[0].isUninitialized()) ? 0 : 1; 
  // FIXME The 4 conditionnal blocs following are crashing with the 2 "additional" rows (Lanthanide and actinide)
  if (s != 0) {
    layouts[index] = computeLayer('s', row.sNumber, s);
    index++;
  }
  if (f != 0) {
    layouts[index] = computeLayer('f', row.fNumber, f);
    index++;
  }
  if (d != 0) {
    layouts[index] = computeLayer('d', row.dNumber, d);
    index++;
  }
  if (p != 0) {
    layouts[index] = computeLayer('p', row.pNumber, p);
    index++;
  }

  Poincare::HorizontalLayout result = Poincare::HorizontalLayout::Builder(); // FIXME The display is totally broken
  for(int i = 0; i < 5; i++) {
    if(layouts[i].isUninitialized()) {
      break;
    }
    result.addChildAtIndex(layouts[i], i, i, nullptr);
  }

  return result;
}

Poincare::Layout ListController::Electronical::computeLayer(CodePoint c, int subLayoutNumber, int number) {
  return Poincare::HorizontalLayout::Builder(
      Poincare::Rational::Builder(subLayoutNumber).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7),
      Poincare::CodePointLayout::Builder(c),
      Poincare::VerticalOffsetLayout::Builder(
        Poincare::Rational::Builder(number).createLayout(Poincare::Preferences::PrintFloatMode::Decimal, 7),
        Poincare::VerticalOffsetLayoutNode::Position::Superscript)
    );
}

const ListController::Electronical::rowsSubLayers ListController::Electronical::rows[] = {
  { true,  1,  false, -1,  false, -1,  false, -1 }, // 1s²
  { true,  2,  false, -1,  false, -1,   true,  2 }, // 2s² 2p⁶
  { true,  3,  false, -1,  false, -1,   true,  3 }, // 3s² 3p⁶
  { true,  4,  false, -1,   true,  3,   true,  4 }, // 4s² 3d¹⁰ 4p⁶
  { true,  5,  false, -1,   true,  4,   true,  5 }, // 5s² 4d¹⁰ 5p⁶
  { true,  6,  true,   4,   true,  5,   true,  6 }, // 6s² 4f¹⁴ 5d¹⁰ 6p⁶
  { true,  7,  true,   5,   true,  6,   true,  7 }, // 6s² 4f¹⁴ 5d¹⁰ 6p⁶
};

const ListController::Electronical::exceptionStruct ListController::Electronical::exceptions[18] = {
  {  24,  true,   1, false, -1,  true,  5, false, -1},
  {  29,  true,   1, false, -1,  true, 10, false, -1},

  {  41,  true,   1, false, -1,  true,  4, false, -1},
  {  42,  true,   1, false, -1,  true,  5, false, -1},
  {  44,  true,   1, false, -1,  true,  7, false, -1},
  {  45,  true,   1, false, -1,  true,  8, false, -1},
  {  46, false,  -1, false, -1, false, -1, false, -1},
  {  47,  true,   1, false, -1,  true, 10, false, -1},

  {  57,  true,   2, false, -1,  true,  1, false, -1},
  {  58,  true,   2,  true,  1,  true,  1, false, -1},
  {  78,  true,   1,  true, 14,  true, 19, false, -1},

  {  89,  true,   2, false, -1,  true,  1, false, -1},
  {  90,  true,   2, false, -1,  true,  2, false, -1},
  {  91,  true,   2,  true,  2,  true,  1, false, -1},
  {  92,  true,   2,  true,  3,  true,  1, false, -1},
  {  93,  true,   2,  true,  4,  true,  1, false, -1},
  {  96,  true,   2,  true,  7,  true,  1, false, -1},
  { 103,  true,   2,  true, 14, false, -1,  true,  1},
};



}
