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

#include "chart.h"
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

typedef std::map<uint32_t, double> LineSeries;

namespace PssCharts {

class LineChart : public Chart
{
    Q_OBJECT

protected:
    std::vector<LineSeries> m_vSeries;
    std::vector<LineSeries> m_vVolume;
    QPointF ConvertToPlotPoint(const std::pair<uint32_t, double>& pair) const;
    QPointF ConvertToVolumePoint(const std::pair<uint32_t, double>& pair) const;
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point) override;
    std::vector<QBrush> m_vLineColor; //Line color for each series
    QBrush m_brushFill;
    bool m_fEnableFill; //! Does the line get filled

    bool m_fDrawVolume;
    double m_nBarWidth;
    QColor m_colorVolume;

    QRect MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const;
    void ProcessChangedData() override;

public:
    LineChart(QWidget* parent = nullptr);
    void AddDataPoint(const uint32_t& nSeries, const uint32_t& x, const double& y);
    void RemoveDataPoint(const uint32_t& nSeries, const uint32_t& x);
    void SetDataPoints(const std::map<uint32_t, double>& mapPoints, const uint32_t& nSeries);

    void AddVolumePoint(const uint32_t& nSeries, const uint32_t& x, const double& y);
    void RemoveVolumePoint(const uint32_t& nSeries, const uint32_t& x);
    void SetVolumePoints(const std::map<uint32_t, double>& mapPoints, const uint32_t& nSeries);

    void paintEvent(QPaintEvent *event) override;
    void SetFillBrush(const QBrush& brush);
    void EnableFill(bool fEnable);
    void SetLineBrush(const uint32_t& nSeries, const QBrush& brush);
    void SetLineWidth(int nWidth);
    void GetLineEquation(const QLineF& line, double& nSlope, double& nYIntercept);
    QColor GetSeriesColor(const uint32_t& nSeries) const;
    void EnableVolumeBar(bool fEnable);
    void SetVolumeBarWidth(int nWidth);
};

} //namespace
#endif // LINECHART_H
