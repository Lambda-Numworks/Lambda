#include <escher/progress_bar_view.h>

void ProgressBarView::setBackgroundColor(KDColor backgroundColor) {
  if (m_backgroundColor != backgroundColor) {
    m_backgroundColor = backgroundColor;
    markRectAsDirty(bounds());
  }
}

void ProgressBarView::setForegroundColor(KDColor foregroundColor) {
  if (m_foregroundColor != foregroundColor) {
    m_foregroundColor = foregroundColor;
    markRectAsDirty(bounds());
  }
}

KDSize ProgressBarView::minimalSizeForOptimalDisplay() const  {
  return KDSize(16,4);
}

void ProgressBarView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(bounds(), m_backgroundColor);
  ctx->fillRect(KDRect(bounds().origin(), (int) (((float) bounds().width()) * m_value), bounds().height()), m_foregroundColor);
}

void ProgressBarView::setValue(float value) {
  if (m_value != value) {
    m_value = value;
    markRectAsDirty(bounds());
  }
}

float ProgressBarView::value() const {
  return m_value;
}

void ProgressBarView::setValue(int current, int total) {
  setValue(((float) current) / ((float) total));
}
