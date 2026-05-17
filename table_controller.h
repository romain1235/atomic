#ifndef ATOMIC_TABLE_CONTROLLER_H
#define ATOMIC_TABLE_CONTROLLER_H

#include <escher.h>
#include "atomic_cell.h"
#include "atom_info.h"
#include "table_lines_view.h"
#include "atoms.h"
#include "list_controller.h"
#include "type_footer_view.h"

namespace Atomic {

class TableController : public ViewController, public SimpleTableViewDataSource, public SelectableTableViewDelegate {
public:
  TableController(Responder * parentResponder, SelectableTableViewDataSource * selectionDataSource);

  View * view() override;

  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  TELEMETRY_ID("");

  enum ColorProperty {
    ColorByType = 0,
    ColorByAtomicNumber,
    ColorByNeutrons,
    ColorByMass,
    ColorByElectronegativity
  };

  void setColorProperty(ColorProperty p);
  void clearColorProperty();
  void reloadTableData();

  int numberOfRows() const override;
  int numberOfColumns() const override;
  KDCoordinate cellHeight() override;
  KDCoordinate cellWidth() override;
  HighlightCell * reusableCell(int index) override;
  int reusableCellCount() const override;
  void willDisplayCellAtLocation(HighlightCell * cell, int i, int j) override;
private:
  SelectableTableViewDataSource * selectionDataSource() const;
  void setSelection(AtomDef atom);
  void appendCharacterToSearch(char c);
  void removeCharacterFromSearch();
  void clearSearch();
  void refreshSearchResults();
  int nextSearchResultIndex(int direction) const;
  int scoreForSearch(const AtomDef & atom, const char * query, int queryLength, bool isNumeric) const;
  static bool startsWithIgnoreCase(const char * text, const char * query);
  static bool isNumericString(const char * text);
  StackViewController * stackController() const;
  class ContentView : public View {
  public:
    ContentView(TableController * controller, SelectableTableViewDataSource * selectionDataSource);
    SelectableTableView * selectableTableView();
    void drawRect(KDContext * ctx, KDRect rect) const override;
    void setAtom(AtomDef atom);
    void setSearchInput(bool active, const char * text, int cursor);
    void setInfoVisible(bool visible);
    void setPropertyDisplay(const char * label, const char * value, KDColor bgColor, KDColor textColor);
    void clearPropertyDisplay();
  private:
    int numberOfSubviews() const override;
    View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    SelectableTableView m_selectableTableView;
    atomInfo m_info;
    tableLinesView m_lines;
    TypeFooterView m_typeFooter;
  };
  static constexpr KDCoordinate k_sideMargin = 6;
  static constexpr KDCoordinate k_indicatorMargin = 61;
  static constexpr int k_numberOfColumns = 18;
  static constexpr int k_maxNumberOfCells = 180;
  static constexpr int k_numberOfRows = ((k_maxNumberOfCells - 1) / k_numberOfColumns) + 1;
  static constexpr int k_cellHeight = 17;
  static constexpr int k_cellWidth = 17;
  int m_position;
  ContentView m_view;
  AtomicCell m_cells[k_maxNumberOfCells];
  bool m_searchMatches[k_maxNumberOfCells];
  char m_searchBuffer[20];
  int m_searchLength;
  int m_searchCursor;
  int m_searchStartCursor;
  int m_bestSearchResult;
  bool m_searchActive;
  int m_atomIndex[k_numberOfColumns][k_numberOfRows];
  int m_cursor = 0;
  ListController m_list;
  bool m_menuIsOpen = false;
  ColorProperty m_colorProperty = ColorByType;
  bool m_coloringActive = false;
  double m_propertyMin = 0.0;
  double m_propertyMax = 0.0;
};

}

#endif
