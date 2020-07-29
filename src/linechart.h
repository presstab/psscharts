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

struct Candle {
    double m_open;
    double m_high;
    double m_low;
    double m_close;
    Candle() {
        m_open = 0;
        m_high = 0;
        m_low = 0;
        m_close = 0;
    }
    Candle(double open, double high, double low, double close) {
        if (high < std::max(open, std::max(low, close))) {
            throw "High is not the maximum value";
        }
        if (low > std::min(open, std::min(high, close))) {
            throw "Low is not the minimum value";
        }
        m_open = open;
        m_high = high;
        m_low = low;
        m_close = close;
    }
    bool isNull() {
        return this->m_low == 0.0 && this->m_high == 0.0 && this->m_low == 0.0 && this->m_close == 0.0;
    }
};

enum class AxisLabelType
{
    AX_NO_LABEL,
    AX_TIMESTAMP,
    AX_NUMBER
};

enum class ChartType
{
    LINE,
    CANDLESTICK
};

class LineChart : public QWidget
{
    Q_OBJECT

private:
    static const uint32_t VERSION_MAJOR = 0;
    static const uint32_t VERSION_MINOR = 0;
    static const uint32_t VERSION_REVISION = 4;
    static const uint32_t VERSION_BUILD = 0;

protected:
    std::map<uint32_t, double> m_mapPoints;
    std::map<uint32_t, Candle> m_candlePoints;
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
    MouseDisplay m_mousedisplay;

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
    QRect MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const;

    int WidthYTitleArea() const;
    int WidthYLabelArea() const;
    int WidthRightMargin() const;

    void ProcessChangedData();

    //Candlestick stuff
    std::pair<uint32_t, Candle> ConvertToCandlePlotPoint(const std::pair<uint32_t, Candle>& pair);
    uint32_t ConvertCandlePlotPointTime(const QPointF& point);
    std::map<uint32_t, Candle> ConvertLineToCandlestickData(const std::map<uint32_t, double> lineChartData, uint32_t candleTimePeriod);
    void wheelEvent(QWheelEvent *event) override;
    bool m_fIsLineChart;
    bool m_fFillCandle;
    bool m_fDrawWick;
    bool m_fDrawOutline;
    bool m_fDisplayCandleDash;
    bool m_fDisplayOHLC;
    double m_nCandleWidth;
    double m_nCandleMaxWidth;
    double m_nCandleMinWidth;
    int m_nCandleLineWidth;
    int m_nCandleSpacing;
    int m_nCandles;
    uint32_t m_nCandleTimePeriod;
    QColor m_colorUpCandle;
    QColor m_colorDownCandle;
    QColor m_colorUpCandleLine;
    QColor m_colorDownCandleLine;
    QColor m_colorUpTail;
    QColor m_colorDownTail;
    QColor m_colorUpDash;
    QColor m_colorDownDash;
    QFont m_fontOHLC;
    QString m_strOHLC;

public:
    LineChart(QWidget* parent = nullptr);
    bool ChangesMade() const { return m_fChangesMade; }
    void AddDataPoint(const uint32_t& x, const double& y);
    void DrawXLabels(QPainter& painter, const std::vector<int>& vXPoints, bool fDrawIndicatorLine);
    void DrawYLabels(QPainter& painter, const std::vector<int>& vYPoints, bool isMouseDisplay);
    void EnableMouseDisplay(bool fEnable);
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
    void SetLabelAutoPrecision(bool fEnable);
    void SetYPadding(int nPadding);
    void SetYTitle(const QString& strTitle);
    void SetYTitleFont(const QFont& font);
    void SetAxisOnOff(bool fDrawX, bool fDrawY);
    void SetAxisLabelsOnOff(bool fDrawXLabels, bool fDrawYLabels);
    void SetAxisSectionCount(uint32_t nCount);
    void SetAxisSeparatorPen(const QPen& pen);
    static uint32_t Version();
    static QString VersionString();

    AxisLabelSettings* YLabelSettings() { return &m_settingsYLabels; }
    AxisLabelSettings* XLabelSettings() { return &m_settingsXLabels; }
    MouseDisplay* GetMouseDisplay() { return &m_mousedisplay; }

    QRect ChartArea() const;
    QRect YLabelArea() const;
    QRect XLabelArea() const;
    const double& MaxX() const;
    const double& MaxY() const;
    const double& MinX() const;
    const double& MinY() const;

    QBrush BackgroundBrush() const;

    QPixmap grab(const QRect &rectangle = QRect(QPoint(0, 0), QSize(-1, -1)));
    void mouseMoveEvent(QMouseEvent* event) override;

    // Candlestick
    void SetChartType(const QString& type);
    void SetCandleDataPoints(std::map<uint32_t, Candle>& mapPoints);
    void SetCandleBodyColor(const QColor& upColor, const QColor& downColor = QColor());
    void SetCandleLineColor(const QColor& upColor, const QColor& downColor = QColor());
    void SetTailColor(const QColor& upColor, const QColor& downColor = QColor());
    void SetDashColor(const QColor& upColor, const QColor& downColor = QColor());
    void SetCandleLineWidth(int nWidth);
    void SetCandleWidth(int nWidth);
    void SetCandleTimePeriod(uint32_t nTime);
    void EnableCandleFill(bool fEnable);
    void EnableWick(bool fEnable);
    void EnableCandleBorder(bool fEnable);
    void EnableCandleDash(bool fEnable);
    void EnableOHLCDisplay(bool fEnable);
    void SetOLHCFont(const QFont &font);
signals:
    void candleWidthChanged(int dChange);
};

} //namespace
#endif // LINECHART_H
