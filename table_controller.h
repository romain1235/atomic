#ifndef ATOMIC_TABLE_CONTROLLER_H
#define ATOMIC_TABLE_CONTROLLER_H

#include <escher.h>
#include <escher/input_event_handler_delegate.h>
#include "atomic_cell.h"
#include "atom_info.h"
#include "table_lines_view.h"
#include "atoms.h"
#include "list_controller.h"
#include "type_footer_view.h"

namespace Atomic {

class TableController : public ViewController, public SimpleTableViewDataSource, public SelectableTableViewDelegate, public TextFieldDelegate, public InputEventHandlerDelegate {
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
    ColorByElectronegativity,
    ColorByAtomicRadius,
    ColorByElectronAffinity,
    ColorByIonisation,
    ColorByMeltingPoint,
    ColorByBoilingPoint,
    ColorByDensity
  };

  void setColorProperty(ColorProperty p);
  void clearColorProperty();
  void reloadTableData();
  bool moveCursorInMenu(int direction);

  // TextFieldDelegate
  bool textFieldShouldFinishEditing(TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidReceiveEvent(TextField * textField, Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(TextField * textField, const char * text, Ion::Events::Event event) override;
  bool textFieldDidAbortEditing(TextField * textField) override;
  bool textFieldDidHandleEvent(TextField * textField, bool returnValue, bool textSizeDidChange) override;

  // InputEventHandlerDelegate
  Toolbox * toolboxForInputEventHandler(InputEventHandler * handler) override { return nullptr; }
  NestedMenuController * variableBoxForInputEventHandler(InputEventHandler * handler) override { return nullptr; }

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
  void clearSearch();
  void refreshSearchResults();
  int nextSearchResultIndex(int direction) const;
  int scoreForSearch(const AtomDef & atom, const char * query, int queryLength, bool isNumeric) const;
  void updateFooterPropertyDisplay(int atomIndex, bool force = false);
  static bool startsWithIgnoreCase(const char * text, const char * query);
  static bool isNumericString(const char * text);
  StackViewController * stackController() const;
  class ContentView : public View {
  public:
    ContentView(TableController * controller, SelectableTableViewDataSource * selectionDataSource, TextField * searchField);
    SelectableTableView * selectableTableView();
    void drawRect(KDContext * ctx, KDRect rect) const override;
    void setAtom(AtomDef atom);
    void setSearchVisible(bool active);
    void setInfoVisible(bool visible);
    void setPropertyDisplay(const char * label, const char * value, KDColor bgColor, KDColor textColor);
    void clearPropertyDisplay();
    void updateCursorFrame(int col, int row, KDColor color);
    void hideCursor();
  private:
    int numberOfSubviews() const override;
    View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    class CursorView : public View {
    public:
      CursorView() : m_color(KDColorWhite) {}
      void setColor(KDColor c) { m_color = c; }
      void drawRect(KDContext * ctx, KDRect rect) const override {
        ctx->strokeRect(bounds(), m_color);
      }
    private:
      KDColor m_color;
    };
    SelectableTableView m_selectableTableView;
    atomInfo m_info;
    tableLinesView m_lines;
    TypeFooterView m_typeFooter;
    CursorView m_cursor;
    TextField * m_searchFieldPtr;
    bool m_searchVisible = false;
  };
  static constexpr KDCoordinate k_sideMargin = 6;
  static constexpr KDCoordinate k_indicatorMargin = 61;
  static constexpr int k_numberOfColumns = 18;
  static constexpr int k_maxNumberOfCells = 180;
  static constexpr int k_numberOfRows = ((k_maxNumberOfCells - 1) / k_numberOfColumns) + 1;
  static constexpr int k_cellHeight = 17;
  static constexpr int k_cellWidth = 17;
  int m_position;
  char m_searchBuffer[20];
  TextField m_searchField;
  ContentView m_view;
  AtomicCell m_cells[k_maxNumberOfCells];
  bool m_searchMatches[k_maxNumberOfCells];
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
  // Cache for precomputed colors when a property is active
  KDColor m_precomputedBg[k_maxNumberOfCells];
  KDColor m_precomputedText[k_maxNumberOfCells];
  bool m_precomputedColorsValid = false;
  bool m_footerDisplayValid = false;
  const char * m_lastFooterLabel = nullptr;
  char m_lastFooterValue[32] = "";
  KDColor m_lastFooterBg = KDColor::RGB24(0);
  KDColor m_lastFooterFg = KDColor::RGB24(0);
};

}

#endif
