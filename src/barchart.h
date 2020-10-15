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
    static const uint32_t VERSION_REVISION = 6;
    static const uint32_t VERSION_BUILD = 0;

protected:
    std::map<uint32_t, double> m_mapPoints;
    QPointF ConvertToPlotPoint(const std::pair<uint32_t, double>& pair);
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point) override;

    QRect MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const;
    void ProcessChangedData() override;

    uint32_t ConvertBarPlotPointTime(const QPointF& point);

    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    bool m_fEnableOutline;
    bool m_fEnableFill;
    bool m_fEnableHighlightBar;
    bool m_fEnableHighlightOutline;

    double m_nBarWidth;
    double m_nBarMaxWidth;
    double m_nBarMinWidth;

    int m_nBarSpacing;
    int m_nBars;

    QBrush m_brushLine;
    QBrush m_brushLineHighlight;
    QColor m_color;
    QColor m_highlight;

public:
    BarChart(QWidget* parent = nullptr);

    void SetDataPoints(std::map<uint32_t, double>& mapPoints);
    void SetBarColor(const QColor& color);
    void SetLineWidth(int nWidth);
    void SetLineBrush(const QBrush& brush);
    void SetBarHighlightColor(const QColor& color);
    void SetLineHighlightBrush(const QBrush& brush);
    void SetBarWidth(int nWidth);
    void SetBarWidth(int nWidth, int nMinWidth, int nMaxWidth);
    void EnableFill(bool fEnable);
    void EnableBorder(bool fEnable);
    void EnableHighlight(bool fEnable);
    void EnableHighlightBorder(bool fEnable);

signals:
    void barWidthChanged(int dChange);
};

} //namespace
#endif // BARCHART_H
