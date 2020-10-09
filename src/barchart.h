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

#ifndef BARCHART_H
#define BARCHART_H

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

namespace PssCharts {

class BarChart : public Chart
{
    Q_OBJECT

private:
    static const uint32_t VERSION_MAJOR = 0;
    static const uint32_t VERSION_MINOR = 1;
    static const uint32_t VERSION_REVISION = 5;
    static const uint32_t VERSION_BUILD = 0;

protected:
    std::map<uint32_t, double> m_mapPoints;
    QPointF ConvertToPlotPoint(const std::pair<uint32_t, double>& pair);
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point) override;

    void ProcessChangedData() override;

    uint32_t ConvertBarPlotPointTime(const QPointF& point);

    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    bool m_fIsLineChart;
    bool m_fFillBar;
    bool m_fDrawOutline;
    bool m_fEnableFill;

    double m_nBarWidth;
    double m_nBarMaxWidth;
    double m_nBarMinWidth;

    int m_nBarLineWidth;
    int m_nBarSpacing;
    int m_nBars;

    uint32_t m_nBarTimePeriod;

    QBrush m_brushLine;
    QBrush m_brushFill;

public:
    BarChart(QWidget* parent = nullptr);

    QRect ChartArea() const;

    void EnableBarFill(bool fEnable);
    void EnableBarBorder(bool fEnable);

    void SetDataPoints(std::map<uint32_t, double>& mapPoints);
    void SetBarBodyColor(const QColor& upColor, const QColor& downColor = QColor());
    void SetBarColor(const QColor& upColor, const QColor& downColor = QColor());
    void SetLineWidth(int nWidth);
    void SetBarWidth(int nWidth);
    void SetBarWidth(int nWidth, int nMinWidth, int nMaxWidth);
    void SetLineBrush(const QBrush& brush);
    void SetFillBrush(const QBrush& brush);
    void EnableFill(bool fEnable);

signals:
    void barWidthChanged(int dChange);
};

} //namespace
#endif // BARCHART_H
