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

#ifndef PIECHART_H
#define PIECHART_H

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
#include <cmath>
class QColor;
class QPaintEvent;

namespace PssCharts {

enum class PieLabelType
{
    PIE_NO_LABEL,
    PIE_VALUE,
    PIE_PERCENT,
    PIE_LABEL,
    PIE_LABEL_PERCENT,
    PIE_LABEL_VALUE
};

class PieChart : public Chart
{
    Q_OBJECT

    // comparison function for drawing the slices want greatest values listed first
    struct cmpGreaterKey {
        bool operator()(const double& a, const double& b) const {
            return a > b;
        }
    };

protected:
    double pi = 3.141592653589793;

    std::map<std::string, double> m_mapPoints;
    std::multimap<double, std::string, cmpGreaterKey> m_mapData;
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point) override;
    QPointF ConvertToPlotPoint(const std::pair<uint32_t, double>& pair);
    std::map<std::string, QColor> m_mapColors;

    PieLabelType m_labelType;
    QBrush m_brushLine;
    bool m_fEnableFill; //! Does the chart get filled
    int m_size;
    bool m_fDountHole;
    int m_nDountSize;
    int m_nStartingAngle;
    double m_nTotal;
    double m_nRatio;
    bool m_fEnableOutline;
    double m_xLabelPadding;
    double m_yLabelPadding;

    bool m_fEnableHighlight;
    QColor m_colorHighlight;
    bool m_fEnableHighlightOutline;
    QColor m_colorHighlightOutline;

    QRect MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const;
    void ProcessChangedData() override;

public:
    PieChart(QWidget* parent = nullptr);
    void paintEvent(QPaintEvent *event) override;
    QRect ChartArea() const;
    QStringList ChartLabels();

    void AddDataPoint(const std::string& label, const double& value);
    void RemoveDataPoint(const std::string& label);
    void SetDataPoints(const std::map<std::string, double>& mapPoints);

    void EnableFill(bool fEnable);
    void SetLineBrush(const QBrush& brush);
    void SetLineWidth(int nWidth);
    void SetChartSize(int nSize);
    void SetStartingAngle(int nAngle);
    void SetDonutSize(int nSize);
    void EnableDonut(bool fEnable);
    void EnableOutline(bool fEnable);
    void SetLabelType(PieLabelType nType);
    void SetLabelType(std::string nType);
    void SetXLabelPadding(double nPadding);
    void SetYLabelPadding(double nPadding);

    QColor GetColor(std::string label);
    void SetColor(std::string label, QColor qColor);
    std::map<std::string, QColor> GetColorMap();
    std::vector<std::pair<QString, QColor>> GetLegendData();

    void SetHighlight(QColor color);
    void EnableHighlight(bool fEnable);
    void SetHighlightOutline(QColor color);
    void EnableHighlightOutline(bool fEnable);

    std::string LabelTypeToString(const PieLabelType type);
    PieLabelType LabelTypeFromString(std::string strType);
    std::string strToUpper(const std::string& strInput);
};

} //namespace
#endif // PIECHART_H
