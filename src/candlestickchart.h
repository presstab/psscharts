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

#ifndef CANDLESTICKCHART_H
#define CANDLESTICKCHART_H

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

namespace PssCharts {

class CandlestickChart : public QWidget
{
    Q_OBJECT

private:
    static const uint32_t VERSION_MAJOR = 0;
    static const uint32_t VERSION_MINOR = 0;
    static const uint32_t VERSION_REVISION = 5;
    static const uint32_t VERSION_BUILD = 0;

protected:
    std::map<uint32_t, double> m_mapPoints;
    std::map<uint32_t, Candle> m_candlePoints;
    std::pair<double, double> m_pairYRange; // min, max
    std::pair<double, double> m_pairXRange; // min, max
    std::pair<uint32_t, double> ConvertFromPlotPoint(const QPointF& point) override;

    void ProcessChangedData() override;

    //Candlestick stuff
    std::pair<uint32_t, Candle> ConvertToCandlePlotPoint(const std::pair<uint32_t, Candle>& pair);
    uint32_t ConvertCandlePlotPointTime(const QPointF& point);
    std::map<uint32_t, Candle> ConvertLineToCandlestickData(const std::map<uint32_t, double> lineChartData, uint32_t candleTimePeriod);
    QRect ChartArea() const;
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
    PssChart* m_pssChart;

public:
    CandlestickChart(QWidget* parent = nullptr);
    void DrawYLabels(QPainter& painter, const std::vector<int>& vYPoints, bool isMouseDisplay);
    void paintEvent(QPaintEvent *event) override;

    QPixmap grab(const QRect &rectangle = QRect(QPoint(0, 0), QSize(-1, -1)));

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
#endif // CANDLESTICKCHART_H
