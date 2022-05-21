#ifndef ESCHER_POGRESS_BAR_VIEW_H
#define ESCHER_POGRESS_BAR_VIEW_H

#include <escher/view.h>
#include <escher/palette.h>
#include <kandinsky/color.h>

class ProgressBarView : public View {
public:
  ProgressBarView(float value = 0.0f, KDColor foregroundColor = Palette::YellowDark, KDColor backgroundColor = Palette::GrayWhite) :
    View(),
    m_value(value),
    m_foregroundColor(foregroundColor),
    m_backgroundColor(backgroundColor)
  {}
  void drawRect(KDContext * ctx, KDRect rect) const override;
  void setBackgroundColor(KDColor backgroundColor);
  void setForegroundColor(KDColor foregroundColor);
  KDSize minimalSizeForOptimalDisplay() const override;
  void setValue(float value);
  float value() const;
  void setValue(int current, int total);
protected:
#if ESCHER_VIEW_LOGGING
  const char * className() const override { return "ProgressBarView"; }
#endif
  float m_value;
  KDColor m_foregroundColor;
  KDColor m_backgroundColor;
};

#endif
