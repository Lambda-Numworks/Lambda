#include "storage_controller.h"
#include <assert.h>
#include <cmath>

namespace Settings {

StorageController::StorageController(Responder * parentResponder) :
  GenericSubController(parentResponder),
  m_view(&m_selectableTableView), m_showSize(false)
{
  for (int i = 0; i < k_totalNumberOfCell; i++) {
    m_cells[i].setMessageFont(KDFont::LargeFont);
    m_cells[i].setAccessoryFont(KDFont::SmallFont);
    m_cells[i].setAccessoryTextColor(Palette::GrayDark);
  }
}

void StorageController::viewWillAppear() {
  GenericSubController::viewWillAppear();
  m_view.setMessages(nullptr, 0);
}

bool StorageController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE) {
    MessageTableCellWithBuffer * myCell = (MessageTableCellWithBuffer *)m_selectableTableView.selectedCell();
    switch (selectedRow()) {
      case 0:
        m_showSize = !m_showSize;
        myCell->setAccessoryText(m_showSize ? Ion::FileSystem::storageSize(m_SizeBuffer) : Ion::FileSystem::storageUsage(m_SizeBuffer));
        return true;
      case 1:
        Container::activeApp()->displayModalViewController(&m_formatPopUpController, 0.f, 0.f, Metric::ExamPopUpTopMargin, Metric::PopUpRightMargin, Metric::ExamPopUpBottomMargin, Metric::PopUpLeftMargin);
        return true;
    }
    return false;
  }
  return GenericSubController::handleEvent(event);
}

HighlightCell * StorageController::reusableCell(int index, int type) {
  assert(type == 0);
  assert(index >= 0 && index < k_totalNumberOfCell);
  return &m_cells[index];
}

int StorageController::reusableCellCount(int type) {
  assert(type == 0);
  return k_totalNumberOfCell;
}

void StorageController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  GenericSubController::willDisplayCellForIndex(cell, index);
  MessageTableCellWithBuffer * myCell = (MessageTableCellWithBuffer *)cell;
  static const char * messages[] = {
    Ion::FileSystem::storageUsage(m_SizeBuffer),
    ""
  };
  assert(index >= 0 && index < 2);
  myCell->setAccessoryText(messages[index]);
}

FormatPopUpController::FormatPopUpController() :
  ::PopUpController(
    4,
    Invocation(
      [](void * context, void * sender) {
        Ion::FileSystem::format();
        Container::activeApp()->dismissModalViewController(false);
        return true;
      }, this)
  )
{
  m_contentView.setMessage(0, I18n::Message::FormatLaunch1);
  m_contentView.setMessage(1, I18n::Message::FormatLaunch2);
  m_contentView.setMessage(2, I18n::Message::FormatLaunch3);
  m_contentView.setMessage(3, I18n::Message::FormatLaunch4);
}

}
