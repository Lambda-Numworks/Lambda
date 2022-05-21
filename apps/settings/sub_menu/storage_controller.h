#ifndef SETTINGS_STORAGE_CONTROLLER_H
#define SETTINGS_STORAGE_CONTROLLER_H

#include <escher/pop_up_controller.h>
#include <escher/progress_bar_view.h>

#include "generic_sub_controller.h"
#include "selectable_view_with_messages.h"

namespace Settings {

class FormatPopUpController : public ::PopUpController {
public:
  FormatPopUpController();
  View * view() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
protected:
  class ContentView : public View, public Responder {
  public:
    ContentView(Responder * parentResponder, int numberOfLines, Invocation okInvocation);
    void drawRect(KDContext * ctx, KDRect rect) const override { ctx->fillRect(bounds(), KDColorBlack); }
    void setSelectedButton(int selectedButton);
    int selectedButton();
    void setMessage(int index, I18n::Message message);
    void setProgress(int current, int total);
  private:
    constexpr static KDCoordinate k_progressbarMargin = 10;
    constexpr static KDCoordinate k_progressbarHeight = 4;
    constexpr static KDCoordinate k_buttonMargin = 10;
    constexpr static KDCoordinate k_buttonHeight = 20;
    constexpr static KDCoordinate k_topMargin = 8;
    constexpr static KDCoordinate k_paragraphHeight = 20;
    int numberOfSubviews() const override;
    View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    ProgressBarView m_progressBarView;
    HighContrastButton m_cancelButton;
    HighContrastButton m_okButton;
    MessageTextView m_warningTextView;
    const int m_numberOfLines;
    constexpr static int k_maxNumberOfLines = 4;
    MessageTextView m_messageTextViews[k_maxNumberOfLines];
  };
  ContentView m_contentView;
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
