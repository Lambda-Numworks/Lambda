#ifndef SETTINGS_STORAGE_CONTROLLER_H
#define SETTINGS_STORAGE_CONTROLLER_H

#include <escher/pop_up_controller.h>

#include "generic_sub_controller.h"
#include "selectable_view_with_messages.h"

namespace Settings {

class FormatPopUpController : public ::PopUpController {
public:
  FormatPopUpController();
};

class StorageController : public GenericSubController {
public:
  StorageController(Responder * parentResponder);
  View * view() override { return &m_view; }
  void viewWillAppear() override;
  TELEMETRY_ID("Storage");
  bool handleEvent(Ion::Events::Event event) override;
  HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;
private:
  constexpr static int k_totalNumberOfCell = 2;
  char m_SizeBuffer[16];
  bool m_showSize;
  SelectableViewWithMessages m_view;
  MessageTableCellWithBuffer m_cells[k_totalNumberOfCell];
  FormatPopUpController m_formatPopUpController;
};

}

#endif
