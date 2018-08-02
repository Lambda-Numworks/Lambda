#ifndef POINCARE_RANDINT_H
#define POINCARE_RANDINT_H

#include <poincare/static_hierarchy.h>
#include <poincare/layout_engine.h>

namespace Poincare {

class Randint : public StaticHierarchy<2> {
  using StaticHierarchy<2>::StaticHierarchy;
public:
  Type type() const override;
private:
  /* Layout */
  LayoutRef createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::createPrefixLayout(this, floatDisplayMode, numberOfSignificantDigits, name());
  }
  int writeTextInBuffer(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::writePrefixExpressionTextInBuffer(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, name());
  }
  const char * name() const { return "randint"; }
  /* Evaluation */
  EvaluationReference<float> approximate(SinglePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return templateApproximate<float>(context, angleUnit);
  }
  EvaluationReference<double> approximate(DoublePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return templateApproximate<double>(context, angleUnit);
  }
  template <typename T> EvaluationReference<T> templateApproximate()Context& context, Preferences::AngleUnit angleUnit) const;
};

}

#endif
