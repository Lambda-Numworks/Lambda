#ifndef POINCARE_COSINE_H
#define POINCARE_COSINE_H

#include <poincare/layout_engine.h>
#include <poincare/static_hierarchy.h>
#include <poincare/approximation_engine.h>
#include <poincare/trigonometry.h>
#include <poincare/expression.h>

namespace Poincare {

class Cosine : public StaticHierarchy<1>::StaticHierarchy  {
  using StaticHierarchy<1>::StaticHierarchy;
  friend class Tangent;
public:
  Type type() const override;
  float characteristicXRange(Context & context, Preferences::AngleUnit angleUnit) const override;
  template<typename T> static std::complex<T> computeOnComplex(const std::complex<T> c, Preferences::AngleUnit angleUnit = Preferences::AngleUnit::Radian);
private:
  /* Layout */
  LayoutRef createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::createPrefixLayout(this, floatDisplayMode, numberOfSignificantDigits, name());
  }
  int writeTextInBuffer(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override {
    return LayoutEngine::writePrefixExpressionTextInBuffer(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, name());
  }
  const char * name() const { return "cos"; }
  /* Simplication */
  ExpressionReference shallowReduce(Context& context, Preferences::AngleUnit angleUnit) override;
  /* Evaluation */
  EvaluationReference<float> approximate(SinglePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return ApproximationEngine::map<float>(this, context, angleUnit,computeOnComplex<float>);
  }
  EvaluationReference<double> approximate(DoublePrecision p, Context& context, Preferences::AngleUnit angleUnit) const override {
    return ApproximationEngine::map<double>(this, context, angleUnit, computeOnComplex<double>);
  }
};

}

#endif
