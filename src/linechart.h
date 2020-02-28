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

#include "axislabelsettings.h"

#include <QBrush>
#include <QPen>
#include <QPointF>
#include <QString>
#include <QWidget>

#include <list>
#include <set>

class QColor;
class QPaintEvent;

enum class AxisLabelType
{
    AX_NO_LABEL,
    AX_TIMESTAMP,
    AX_NUMBER
};

class LineChart : public QWidget
{
    Q_OBJECT
private:
    const uint8_t VERSION = 1;

protected:
    std::map<uint32_t, double> m_mapPoints;
    std::pair<double, double> m_pairYRange; // min, max
    std::pair<double, double> m_pairXRange; // min, max
    QPointF ConvertToPlotPoint(const std::pair<uint32_t, double>& pair) const;
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point);
    QBrush m_brushBackground;
    QBrush m_brushLine;
    QBrush m_brushFill;
    bool m_fEnableFill; //! Does the line get filled
    QBrush m_brushLabels;
    QPen m_penAxisSeparater;
    int m_lineWidth;
    uint32_t m_precision;

    //Top Title
    QString m_strTopTitle; //! Chart main title
    QFont m_fontTopTitle;

    //Y-Title
    QString m_strTitleY; //! Title for Y-axis
    QFont m_fontYTitle;

    //Axis Tick Labels
    AxisLabelSettings m_settingsYLabels;
    AxisLabelSettings m_settingsXLabels;

    bool m_fDrawXAxis;
    bool m_fDrawYAxis;

    int m_yPadding;
    int m_rightMargin;
    int m_topTitleHeight;

    uint32_t m_axisSections; // Split the axis into this many sections

    QPixmap m_pixmapCache;
    bool m_fChangesMade; // Have changes been made since the last paint

    int HeightTopTitleArea() const;
    int HeightXLabelArea() const;

    int WidthYTitleArea() const;
    int WidthYLabelArea() const;
    int WidthRightMargin() const;

    void ProcessChangedData();

public:
    LineChart(QWidget* parent = nullptr);
    bool ChangesMade() const { return m_fChangesMade; }
    void AddDataPoint(const uint32_t& x, const double& y);
    void RemoveDataPoint(const uint32_t& x);
    void SetDataPoints(const std::map<uint32_t, double>& mapPoints);
    void paintEvent(QPaintEvent *event) override;
    void SetBackgroundBrush(const QBrush& brush);
    void SetFillBrush(const QBrush& brush);
    void EnableFill(bool fEnable);
    void SetLineBrush(const QBrush& brush);
    void SetLineWidth(int nWidth);
    void SetRightMargin(int margin);
    void SetTopTitleHeight(int height);
    void SetTopTitle(const QString& strTitle);
    void SetTopTitleFont(const QFont& font);
    void SetAxisLabelsBrush(const QBrush& brush);
    void SetXLabelType(AxisLabelType labelType);
    void SetYLabelType(AxisLabelType labelType);
    void SetYLabelWidth(int width);
    void SetYLabelFont(const QFont& font);
    void SetXLabelHeight(int height);
    void SetLabelPrecision(int precision);
    void SetYPadding(int nPadding);
    void SetYTitle(const QString& strTitle);
    void SetYTitleFont(const QFont& font);
    void SetAxisOnOff(bool fDrawX, bool fDrawY);
    void SetAxisLabelsOnOff(bool fDrawXLabels, bool fDrawYLabels);
    void SetAxisSectionCount(uint32_t nCount);
    void SetAxisSeparatorPen(const QPen& pen);
    uint8_t Version() { return VERSION; }

    QRect ChartArea() const;
    QRect YLabelArea() const;
    QRect XLabelArea() const;
    const double& MaxX() const;
    const double& MaxY() const;
    const double& MinX() const;
    const double& MinY() const;

    QBrush BackgroundBrush() const;

    QPixmap grab(const QRect &rectangle = QRect(QPoint(0, 0), QSize(-1, -1)));
};

#endif // LINECHART_H
