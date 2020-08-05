/*
MIT License

Copyright (c) 2020 Paddington Software Services

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef LINECHART_H
#define LINECHART_H

#include "psschart.h"
#include "axislabelsettings.h"
#include "mousedisplay.h"

#include <QBrush>
#include <QPen>
#include <QPointF>
#include <QString>
#include <QWidget>
#include <QWheelEvent>

#include <list>
#include <set>

class QColor;
class QPaintEvent;

namespace PssCharts {

class LineChart : public Chart
{
    Q_OBJECT

private:
    static const uint32_t VERSION_MAJOR = 0;
    static const uint32_t VERSION_MINOR = 1;
    static const uint32_t VERSION_REVISION = 5;
    static const uint32_t VERSION_BUILD = 0;

protected:
    std::map<uint32_t, double> m_mapPoints;
    QPointF ConvertToPlotPoint(const std::pair<uint32_t, double>& pair) const;
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point) override;
    QBrush m_brushLine;
    QBrush m_brushFill;
    bool m_fEnableFill; //! Does the line get filled

    QRect MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const;
    void ProcessChangedData() override;

public:
    LineChart(QWidget* parent = nullptr);
    void AddDataPoint(const uint32_t& x, const double& y);
    void RemoveDataPoint(const uint32_t& x);
    void SetDataPoints(const std::map<uint32_t, double>& mapPoints);
    void paintEvent(QPaintEvent *event) override;
    void SetFillBrush(const QBrush& brush);
    void EnableFill(bool fEnable);
    void SetLineBrush(const QBrush& brush);
    void SetLineWidth(int nWidth);
    void GetLineEquation(const QLineF& line, double& nSlope, double& nYIntercept);
};

} //namespace
#endif // LINECHART_H
