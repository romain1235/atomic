#ifndef ATOMIC_LIST_CONTROLLER_H
#define ATOMIC_LIST_CONTROLLER_H

#include <escher.h>
#include <apps/i18n.h>
#include "atoms.h"
#include "list_atomic_cell.h"
#include "message_table_cell_with_expression_with_copy.h"

namespace Atomic {

// Detail panel for a single element. Pushed on the app's StackViewController
// (push navigation) rather than displayed as a fullscreen modal.
class ListController : public ViewController, public ListViewDataSource, public SelectableTableViewDataSource, public SelectableTableViewDelegate {
public:
  ListController(Responder * parentResponder);

  // ViewController
  const char * title() override;
  View * view() override { return &m_selectableTableView; }
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;

  // ListViewDataSource
  int numberOfRows() const override;
  KDCoordinate rowHeight(int j) override;
  HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  int typeAtLocation(int i, int j) override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;

  // SelectableTableViewDelegate
  void tableViewDidChangeSelection(SelectableTableView * t, int previousSelectedCellX, int previousSelectedCellY, bool withinTemporarySelection) override;

  void setAtom(AtomDef atom);
  void unhighlightTopCells();
  void setPropertyColors(KDColor bg, KDColor text);
  void clearPropertyColors();
  void refreshNavigation();

private:
  class Electronical {
  public :
    static Poincare::Layout createElectronical(AtomDef atom);
  private:
    static Poincare::Layout computeLayer(CodePoint c, int subLayoutNumber, int number);
    struct rowsSubLayers {
      bool s;
      int sNumber;
      bool f;
      int fNumber;
      bool d;
      int dNumber;
      bool p;
      int pNumber;
    };
    struct exceptionStruct {
      int num;
      bool s;
      int sContent;
      bool f;
      int fContent;
      bool d;
      int dContent;
      bool p;
      int pContent;
    };
    const static rowsSubLayers rows[];
    const static exceptionStruct exceptions[18];
  };

  constexpr static int k_atomicCellRowHeight = 110;
  constexpr static int k_classicalRowHeight = 30;

  SelectableTableView m_selectableTableView;
  ListAtomicCell m_atomicCell;
  constexpr static int k_numberOfCellsWithBuffer = 2;
  MessageTableCellWithBuffer m_cellsWithBuffer[k_numberOfCellsWithBuffer];
  constexpr static int k_numberOfCellsWithExpression = 11;
  MessageTableCellWithExpressionWithCopy m_cellsWithExpression[k_numberOfCellsWithExpression];
  constexpr static int k_numberOfRow = 1 + k_numberOfCellsWithBuffer + k_numberOfCellsWithExpression;
  AtomDef m_atom;
  Responder * m_parent;
  // Cached expensive electron config layout — recomputed only when atom changes
  Poincare::Layout m_cachedElectronicalLayout;
  int m_cachedElectronicalAtomNum = -1;
  // Optional property colors to apply to the atomic preview cell
  bool m_hasPropertyColors = false;
  KDColor m_propertyBg = KDColor::RGB24(0x000000);
  KDColor m_propertyText = KDColor::RGB24(0xffffff);
};

}

#endif

