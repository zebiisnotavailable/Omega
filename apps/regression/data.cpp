#include "data.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

namespace Regression {

Data::Data() :
  ::Data(),
  CurveViewWindowWithCursor(),
  m_selectedDotIndex(-1)
{
}

/* Raw numeric data */

float Data::xValueAtIndex(int index) {
  assert(index < m_numberOfPairs);
  return m_xValues[index];
}

float Data::yValueAtIndex(int index) {
  assert(index < m_numberOfPairs);
  return m_yValues[index];
}

void Data::setXValueAtIndex(float value, int index) {
  if (index >= k_maxNumberOfPairs) {
    return;
  }
  m_xValues[index] = value;
  if (index >= m_numberOfPairs) {
    m_yValues[index] = 0.0f;
    m_numberOfPairs++;
  }
  initCursorPosition();
  initWindowParameters();
}

void Data::setYValueAtIndex(float value, int index) {
  if (index >= k_maxNumberOfPairs) {
    return;
  }
  m_yValues[index] = value;
  if (index >= m_numberOfPairs) {
    m_xValues[index] = 0.0f;
    m_numberOfPairs++;
  }
  initCursorPosition();
  initWindowParameters();
}

void Data::deletePairAtIndex(int index) {
  m_numberOfPairs--;
  for (int k = index; k < m_numberOfPairs; k++) {
    m_xValues[k] = m_xValues[k+1];
    m_yValues[k] = m_yValues[k+1];
  }
  m_xValues[m_numberOfPairs] = 0.0f;
  m_yValues[m_numberOfPairs] = 0.0f;
  initCursorPosition();
  initWindowParameters();
}

/* Cursor */

int Data::moveCursorVertically(int direction) {
  float yRegressionCurve = yValueForXValue(m_xCursorPosition);
  if (m_selectedDotIndex >= 0) {
    if ((yRegressionCurve - m_yCursorPosition > 0) == (direction > 0)) {
      m_selectedDotIndex = -1;
      m_yCursorPosition = yRegressionCurve;
    } else {
      return -1;
    }
  } else {
    int dotSelected = selectClosestDotRelativelyToCurve(direction);
    if (dotSelected >= 0) {
      m_selectedDotIndex = dotSelected;
    } else {
      return -1;
    }
  }
  bool windowHasMoved = panToMakePointVisible(m_xCursorPosition, m_yCursorPosition, 0.0f, 0.0f);
  return windowHasMoved;
}

int Data::moveCursorHorizontally(int direction) {
  if (m_selectedDotIndex >= 0) {
    int dotSelected = selectNextDot(direction);
    if (dotSelected >= 0) {
      m_selectedDotIndex = dotSelected;
    } else {
      return -1;
    }
  } else {
    m_xCursorPosition = direction > 0 ? m_xCursorPosition + m_xGridUnit/CurveViewWindowWithCursor::k_numberOfCursorStepsInGradUnit :
      m_xCursorPosition - m_xGridUnit/CurveViewWindowWithCursor::k_numberOfCursorStepsInGradUnit;
    m_yCursorPosition = yValueForXValue(m_xCursorPosition);
  }
  bool windowHasMoved = panToMakePointVisible(m_xCursorPosition, m_yCursorPosition, 0.0f, 0.0f);
  return windowHasMoved;
}

int Data::selectedDotIndex() {
  return m_selectedDotIndex;
}

/* Window */

void Data::setDefault() {
  initWindowParameters();
}

/* Calculations */

float Data::xSum() {
  float result = 0;
  for (int k = 0; k < m_numberOfPairs; k++) {
    result += m_xValues[k];
  }
  return result;
}

float Data::ySum() {
  float result = 0;
  for (int k = 0; k < m_numberOfPairs; k++) {
    result += m_yValues[k];
  }
  return result;
}

float Data::xSquaredValueSum() {
  float result = 0;
  for (int k = 0; k < m_numberOfPairs; k++) {
    result += m_xValues[k]*m_xValues[k];
  }
  return result;
}

float Data::ySquaredValueSum() {
  float result = 0;
  for (int k = 0; k < m_numberOfPairs; k++) {
    result += m_yValues[k]*m_yValues[k];
  }
  return result;
}

float Data::xyProductSum() {
  float result = 0;
  for (int k = 0; k < m_numberOfPairs; k++) {
    result += m_xValues[k]*m_yValues[k];
  }
  return result;
}

float Data::xMean() {
  return xSum()/m_numberOfPairs;
}

float Data::yMean() {
  return ySum()/m_numberOfPairs;
}

float Data::xVariance() {
  float mean = xMean();
  return xSquaredValueSum()/m_numberOfPairs - mean*mean;
}

float Data::yVariance() {
  float mean = yMean();
  return ySquaredValueSum()/m_numberOfPairs - mean*mean;
}

float Data::xStandardDeviation() {
  return sqrtf(xVariance());
}

float Data::yStandardDeviation() {
  return sqrtf(yVariance());
}

float Data::covariance() {
  return xyProductSum()/m_numberOfPairs - xMean()*yMean();
}

float Data::slope() {
  return covariance()/xVariance();
}

float Data::yIntercept() {
  return yMean() - slope()*xMean();
}

float Data::yValueForXValue(float x) {
  return slope()*x+yIntercept();
}

float Data::correlationCoefficient() {
  return covariance()/(xStandardDeviation()*yStandardDeviation());
}

float Data::squaredCorrelationCoefficient() {
  float cov = covariance();
  return cov*cov/(xVariance()*yVariance());
}

float Data::maxXValue() {
  float max = -FLT_MAX;
  for (int k = 0; k < m_numberOfPairs; k++) {
    if (m_xValues[k] > max) {
      max = m_xValues[k];
    }
  }
  return max;
}

float Data::maxYValue() {
  float max = -FLT_MAX;
  for (int k = 0; k < m_numberOfPairs; k++) {
    if (m_yValues[k] > max) {
      max = m_yValues[k];
    }
  }
  return max;
}

float Data::minXValue() {
  float min = FLT_MAX;
  for (int k = 0; k < m_numberOfPairs; k++) {
    if (m_xValues[k] < min) {
      min = m_xValues[k];
    }
  }
  return min;
}

float Data::minYValue() {
  float min = FLT_MAX;
  for (int k = 0; k < m_numberOfPairs; k++) {
    if (m_yValues[k] < min) {
      min = m_yValues[k];
    }
  }
  return min;
}

void Data::initCursorPosition() {
  m_xCursorPosition = (m_xMin+m_xMax)/2.0f;
  m_yCursorPosition = yValueForXValue(m_xCursorPosition);
  m_selectedDotIndex = -1;
}

bool Data::computeYaxis() {
  float min = m_yMin;
  float max = m_yMax;
  for (int k = 0; k < m_numberOfPairs; k++) {
    if (m_xMin <= m_xValues[k] && m_xValues[k] <= m_xMax) {
      if (m_yValues[k] < min) {
        min = m_yValues[k];
      }
      if (m_yValues[k] > max) {
        max = m_yValues[k];
      }
    }
  }
  if (min == m_yMin && max == m_yMax) {
    return false;
  }
  m_yMin = min;
  m_yMax = max;
  m_yGridUnit = computeGridUnit(Axis::Y, m_yMin, m_yMax);
  return true;
}

void Data::initWindowParameters() {
  m_xMin = minXValue();
  m_xMax = maxXValue();
  m_yMin = minYValue();
  m_yMax = maxYValue();
  if (m_xMin == m_xMax) {;
    m_xMin = m_xMin - 1.0f;
    m_xMax = m_xMax + 1.0f;
  }
  if (m_yMin == m_yMax) {
    m_yMin = m_yMin - 1.0f;
    m_yMax = m_yMax + 1.0f;
  }
  m_xGridUnit = computeGridUnit(Axis::X, m_xMin, m_xMax);
  m_yGridUnit = computeGridUnit(Axis::Y, m_yMin, m_yMax);
}

int Data::selectClosestDotRelativelyToCurve(int direction) {
  float nextX = INFINITY;
  float nextY = INFINITY;
  int selectedDot = -1;
  /* The conditions to test on all dots are in this order:
  * - the next dot should be within the window abscissa bounds
  * - the next dot is the closest one in abscissa
  * - the next dot is above the selected one if direction == 1 and below
  * otherwise */
  for (int index = 0; index < m_numberOfPairs; index++) {
    if ((m_xMin <= m_xValues[index] && m_xValues[index] <= m_xMax) &&
        (fabsf(m_xValues[index] - m_xCursorPosition) < fabsf(nextX - m_xCursorPosition)) &&
        ((m_yValues[index] - yValueForXValue(m_xValues[index]) >= 0) == (direction > 0))) {
      // Handle edge case: if 2 dots have the same abscissa but different ordinates
      if (nextX != m_xValues[index] || ((nextY - m_yValues[index] >= 0) == (direction > 0))) {
        nextX = m_xValues[index];
        nextY = m_yValues[index];
        selectedDot = index;
      }
    }
  }
  // Compare with the mean dot
  if (m_xMin <= xMean() && xMean() <= m_xMax &&
      (fabsf(xMean() - m_xCursorPosition) < fabsf(nextX - m_xCursorPosition)) &&
      ((yMean() - yValueForXValue(xMean()) >= 0) == (direction > 0))) { 
    if (nextX != xMean() || ((nextY - yMean() >= 0) == (direction > 0))) {
      nextX = xMean();
      nextY = yMean();
      selectedDot = m_numberOfPairs;
    }
  }
  if (!isinf(nextX) && !isinf(nextY)) {
    m_xCursorPosition = nextX;
    m_yCursorPosition = nextY;
    return selectedDot;
  }
  return selectedDot;
}

int Data::selectNextDot(int direction) {
  float nextX = INFINITY;
  float nextY = INFINITY;
  int selectedDot = -1;
  /* We have to scan the data in opposite ways for the 2 directions to ensure to
   * select all dots (even with equal abscissa) */
  if (direction > 0) {
    for (int index = 0; index < m_numberOfPairs; index++) {
      /* The conditions to test are in this order:
       * - the next dot is the closest one in abscissa
       * - the next dot is not the same as the selected one
       * - the next dot is at the right of the selected one */
      if (fabsf(m_xValues[index] - m_xCursorPosition) < fabsf(nextX - m_xCursorPosition) &&
          (index != m_selectedDotIndex) &&
          (m_xValues[index] >= m_xCursorPosition)) {
        // Handle edge case: 2 dots have same abscissa
        if (m_xValues[index] != m_xCursorPosition || (index > m_selectedDotIndex)) {
          nextX = m_xValues[index];
          nextY = m_yValues[index];
          selectedDot = index;
        }
      }
    }
    // Compare with the mean dot
    if (fabsf(xMean() - m_xCursorPosition) < fabsf(nextX - m_xCursorPosition) &&
          (m_numberOfPairs != m_selectedDotIndex) &&
          (xMean() >= m_xCursorPosition)) {
      if (xMean() != m_xCursorPosition || (m_numberOfPairs > m_selectedDotIndex)) {
        nextX = xMean();
        nextY = yMean();
        selectedDot = m_numberOfPairs;
      }
    }
  } else {
    // Compare with the mean dot
    if (fabsf(xMean() - m_xCursorPosition) < fabsf(nextX - m_xCursorPosition) &&
          (m_numberOfPairs != m_selectedDotIndex) &&
          (xMean() <= m_xCursorPosition)) {
      if (xMean() != m_xCursorPosition || (m_numberOfPairs < m_selectedDotIndex)) {
        nextX = xMean();
        nextY = yMean();
        selectedDot = m_numberOfPairs;
      }
    }
    for (int index = m_numberOfPairs-1; index >= 0; index--) {
      if (fabsf(m_xValues[index] - m_xCursorPosition) < fabsf(nextX - m_xCursorPosition) &&
          (index != m_selectedDotIndex) &&
          (m_xValues[index] <= m_xCursorPosition)) {
        // Handle edge case: 2 dots have same abscissa
        if (m_xValues[index] != m_xCursorPosition || (index < m_selectedDotIndex)) {
          nextX = m_xValues[index];
          nextY = m_yValues[index];
          selectedDot = index;
        }
      }
    }
  }
  if (!isinf(nextX) && !isinf(nextY)) {
    m_xCursorPosition = nextX;
    m_yCursorPosition = nextY;
    return selectedDot;
  }
  return selectedDot;
}

}
