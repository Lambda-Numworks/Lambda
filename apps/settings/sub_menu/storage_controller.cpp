#include "storage_controller.h"
#include "../../apps_container.h"
#include <assert.h>
#include <cmath>

#include <ion/timing.h>
#include <ion/keyboard.h>

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
  ::PopUpController(4, Invocation([](void * context, void * sender) { return true; }, this)),
  m_contentView(this, 4, Invocation([](void * context, void * sender) {
    Ion::FileSystem::format([](int current, int total, void* sndr) {
      ((FormatPopUpController::ContentView*) ((HighContrastButton*) sndr)->parentResponder())->setProgress(current, total);
      AppsContainer::sharedAppsContainer()->redrawWindow();
      Ion::Keyboard::scan();
    }, sender);

    Container::activeApp()->dismissModalViewController(false);
    return true;
  }, this))
{
  m_contentView.setMessage(0, I18n::Message::FormatLaunch1);
  m_contentView.setMessage(1, I18n::Message::FormatLaunch2);
  m_contentView.setMessage(2, I18n::Message::FormatLaunch3);
  m_contentView.setMessage(3, I18n::Message::FormatLaunch4);
}

View * FormatPopUpController::view() {
  return &m_contentView;
}

void FormatPopUpController::didBecomeFirstResponder() {
  m_contentView.setSelectedButton(0);
}

bool FormatPopUpController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Left && m_contentView.selectedButton() == 1) {
    m_contentView.setSelectedButton(0);
    return true;
  }
  if (event == Ion::Events::Right && m_contentView.selectedButton() == 0) {
    m_contentView.setSelectedButton(1);
    return true;
  }
  return false;
}

FormatPopUpController::ContentView::ContentView(Responder * parentResponder, int numberOfLines, Invocation okInvocation) :
  Responder(parentResponder),
  m_cancelButton(
    this, I18n::Message::Cancel,
    Invocation(
      [](void * context, void * sender) {
        Container::activeApp()->dismissModalViewController();
        return true;
      }, this),
    KDFont::SmallFont),
  m_okButton(this, I18n::Message::Ok, okInvocation, KDFont::SmallFont),
  m_warningTextView(KDFont::SmallFont, I18n::Message::Warning, 0.5, 0.5, KDColorWhite, KDColorBlack),
  m_numberOfLines(numberOfLines),
  m_messageTextViews{}
{
  assert(m_numberOfLines <= k_maxNumberOfLines && m_numberOfLines >= 0);
  for (int i = 0; i < m_numberOfLines; i++) {
    m_messageTextViews[i].setFont(KDFont::SmallFont);
    m_messageTextViews[i].setAlignment(0.5f, 0.5f);
    m_messageTextViews[i].setBackgroundColor(KDColorBlack);
    m_messageTextViews[i].setTextColor(KDColorWhite);
  }
}

void FormatPopUpController::ContentView::setSelectedButton(int selectedButton) {
  m_cancelButton.setHighlighted(selectedButton == 0);
  m_okButton.setHighlighted(selectedButton == 1);
  Container::activeApp()->setFirstResponder(selectedButton == 0 ? &m_cancelButton : &m_okButton);
}

int FormatPopUpController::ContentView::selectedButton() {
  return m_cancelButton.isHighlighted() ? 0 : 1;
}

void FormatPopUpController::ContentView::setMessage(int index, I18n::Message message) {
  assert(index >=0 && index < m_numberOfLines);
  m_messageTextViews[index].setMessage(message);
}

int FormatPopUpController::ContentView::numberOfSubviews() const {
  // MessageTextViews + WarningTextView + ProgressBarView + CancelButton + OkButton
  return m_numberOfLines + 4;
}

View * FormatPopUpController::ContentView::subviewAtIndex(int index) {
  int totalSubviews = numberOfSubviews();
  if (index < 0 || index >= totalSubviews) {
    assert(false);
    return nullptr;
  }
  if (index == 0) {
    return &m_warningTextView;
  }
  if (index == totalSubviews - 3) {
    return &m_progressBarView;
  }
  if (index == totalSubviews - 2) {
    return &m_cancelButton;
  }
  if (index == totalSubviews - 1) {
    return &m_okButton;
  }
  return &m_messageTextViews[index-1];
}

void FormatPopUpController::ContentView::layoutSubviews(bool force) {
  KDCoordinate height = bounds().height();
  KDCoordinate width = bounds().width();
  KDCoordinate textHeight = KDFont::SmallFont->glyphSize().height();
  m_warningTextView.setFrame(KDRect(0, k_topMargin, width, textHeight), force);

  // Offset to center text vertically
  const int offset = (k_maxNumberOfLines - m_numberOfLines) / 2;

  for (int i = 0; i < m_numberOfLines; i++) {
    m_messageTextViews[i].setFrame(KDRect(0, k_topMargin + k_paragraphHeight + (i + 1 + offset) * textHeight, width, textHeight), force);
  }

  m_progressBarView.setFrame(KDRect(k_buttonMargin, height - k_buttonMargin - k_buttonHeight - k_progressbarMargin - k_progressbarHeight, width - 2 * k_buttonMargin, k_progressbarHeight), force);

  m_cancelButton.setFrame(KDRect(k_buttonMargin, height - k_buttonMargin - k_buttonHeight, (width - 3 * k_buttonMargin) / 2, k_buttonHeight), force);
  m_okButton.setFrame(KDRect(2 * k_buttonMargin + (width - 3 * k_buttonMargin) / 2, height - k_buttonMargin - k_buttonHeight, (width - 3 * k_buttonMargin) / 2, k_buttonHeight), force);
}

void FormatPopUpController::ContentView::setProgress(int current, int total) {
  m_progressBarView.setValue(current, total);
}

}
