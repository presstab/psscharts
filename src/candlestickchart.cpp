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

#include "candlestickchart.h"
#include "stringutil.h"

#include <QDateTime>
#include <QLineF>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>

/* ----------------------------------------------- |
 * |              TOP TITLE AREA                   |
 * |             ______________________________    |
 * |     |   Y  |                             | R  |
 * |     |   L  |                             | I  |
 * |  Y  |   A  |                             | G  |
 * |  T  |   B  |                             | H  |
 * |  I  |   E  |                             | T  |
 * |  T  |   L  |                             |    |
 * |  L  |      |        CHART AREA           | M  |
 * |  E  |   A  |                             | A  |
 * |     |   R  |                             | R  |
 * |  A  |   E  |                             | G  |
 * |  R  |   A  |                             | I  |
 * |  E  |      |                             | N  |
 * |  A  |      |_____________________________|    |
 * |     |      |    X LABEL AREA             |    |
 * |     |      |                             |    |
 * ----------------------------------------------
 *
*/
namespace PssCharts {

CandlestickChart::CandlestickChart(QWidget *parent) : PssChart (ChartType::CANDLESTICK, parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_axisSections = 0;
    m_yPadding = 0;
    m_nCandleSpacing = 2;
    m_nCandleLineWidth = 2;
    m_nCandleWidth = 8;
    m_nCandleMaxWidth = 20;
    m_nCandleMinWidth = 1;
    m_nCandleTimePeriod = 60;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;
    m_strOHLC = "O:0\tH:0\tL:0\tC:0\t0%";
    m_colorUpCandle = Qt::green;
    m_colorDownCandle = Qt::red;
    m_colorUpCandleLine = Qt::darkGreen;
    m_colorDownCandleLine = Qt::darkRed;
    m_colorUpTail = Qt::darkGreen;
    m_colorDownTail = Qt::darkRed;
    m_colorUpDash = Qt::darkGreen;
    m_colorDownDash = Qt::darkRed;
    m_fFillCandle = true;
    m_fDrawWick = true;
    m_fDrawOutline = true;
    m_fDisplayCandleDash = false;
    m_fDisplayOHLC = true;
    m_fontOHLC.setPointSize(8);

    setMouseTracking(true);
}

/**
 * @brief CandlestickChart::ConvertToPlotPoint: convert a datapoint into the actual point it will be painted to
 * @param pair
 * @return
 */
std::pair<uint32_t, Candle> CandlestickChart::ConvertToCandlePlotPoint(const std::pair<uint32_t, Candle> &pair)
{
    QRect rectChart = ChartArea();
    if (m_yPadding > 0) {
        //Y padding will make it so that there is an area on the top and bottom of the ChartArea
        //that does not get drawn in. It makes it so the lines don't go right onto the edges.
        rectChart.setBottom(rectChart.bottom() - m_yPadding);
        rectChart.setTop(rectChart.top() + m_yPadding);
    }

    //compute point-value of X
    m_nCandles = 0;
    int nWidth = rectChart.width();
    double nValueMaxX = (MaxX() - MinX());
    double nValueX = ((pair.first - MinX()) / nValueMaxX);
    nValueX *= nWidth;
    nValueX += rectChart.left() - (2*m_nCandles + 1) * m_nCandleWidth; // factor in space candles take
    if(nValueX < rectChart.left()) {
        // outside of chart
        return std::pair<uint32_t, Candle>();
    } else {
        m_nCandles++;
    }

    //compute point-value of Open
    int nHeight = rectChart.height();
    uint64_t nOpen = pair.second.m_open * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxOpen = MaxY()*m_precision;
    uint64_t nMinOpen = MinY()*m_precision;
    uint64_t nSpanOpen = (nMaxOpen - nMinOpen);
    uint64_t nValueOpen = (nOpen - nMinOpen);
    nValueOpen *= nHeight;
    double dValueOpen = nValueOpen;
    if (nSpanOpen == 0) {
        dValueOpen = rectChart.top() + (nHeight/2);
    } else {
        dValueOpen /= nSpanOpen;
        dValueOpen = rectChart.bottom() - dValueOpen; // Qt uses inverted Y axis
    }

    //compute point-value of High
    uint64_t nHigh = pair.second.m_high * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxHigh = MaxY()*m_precision;
    uint64_t nMinHigh = MinY()*m_precision;
    uint64_t nSpanHigh = (nMaxHigh - nMinHigh);
    uint64_t nValueHigh = (nHigh - nMinHigh);
    nValueHigh *= nHeight;
    double dValueHigh = nValueHigh;
    if (nSpanHigh == 0) {
        dValueHigh = rectChart.top() + (nHeight/2);
    } else {
        dValueHigh /= nSpanHigh;
        dValueHigh = rectChart.bottom() - dValueHigh; // Qt uses inverted Y axis
    }

    //compute point-value of Low
    uint64_t nLow = pair.second.m_low * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxLow = MaxY()*m_precision;
    uint64_t nMinLow = MinY()*m_precision;
    uint64_t nSpanLow = (nMaxLow - nMinLow);
    uint64_t nValueLow = (nLow - nMinLow);
    nValueLow *= nHeight;
    double dValueLow = nValueLow;
    if (nSpanLow == 0) {
        dValueLow = rectChart.top() + (nHeight/2);
    } else {
        dValueLow /= nSpanLow;
        dValueLow = rectChart.bottom() - dValueLow; // Qt uses inverted Y axis
    }

    //compute point-value of Close
    uint64_t nClose = pair.second.m_close * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxClose = MaxY()*m_precision;
    uint64_t nMinClose = MinY()*m_precision;
    uint64_t nSpanClose = (nMaxClose - nMinClose);
    uint64_t nValueClose = (nClose - nMinClose);
    nValueClose *= nHeight;
    double dValueClose = nValueClose;
    if (nSpanClose == 0) {
        dValueClose = rectChart.top() + (nHeight/2);
    } else {
        dValueClose /= nSpanClose;
        dValueClose = rectChart.bottom() - dValueClose; // Qt uses inverted Y axis
    }

    // Have to manually add values to avoid exception since Qt uses inverted Y axis
    Candle candle;
    candle.m_open = dValueOpen;
    candle.m_high = dValueHigh;
    candle.m_low = dValueLow;
    candle.m_close = dValueClose;
    return std::pair<uint32_t, Candle>(nValueX, candle);
}

/**
 * @brief CandlestickChart::ConvertFromPlotPoint: get the timestamp for a candle on the chart
 * @return
 */
uint32_t CandlestickChart::ConvertCandlePlotPointTime(const QPointF& point)
{
    QRect rectChart = ChartArea();
    auto t1 = (point.x() - rectChart.left()) / rectChart.width();
    auto t2 = t1 * (MaxX() - MinX());
    return  t2 + MinX();
}

/**
 * @brief CandlestickChart::ConvertFromPlotPoint: get the timestamp for a candle on the chart
 * @return
 */
std::pair<uint32_t, double> CandlestickChart::ConvertFromPlotPoint(const QPointF& point)
{
    std::pair<uint32_t, double> pairValues;
    QRect rectChart = ChartArea();

    //Y
    auto t = rectChart.bottom() - point.y();
    auto t1 = t / rectChart.height();
    auto t2 = t1 * (MaxY() - MinY());
    pairValues.second = t2 + MinY();

    //X
    t1 = (point.x() - rectChart.left()) / rectChart.width();
    t2 = t1 * (MaxX() - MinX());
    pairValues.first = t2 + MinX();

    return pairValues;
}

std::map<uint32_t, Candle> CandlestickChart::ConvertLineToCandlestickData(const std::map<uint32_t, double> lineChartData, uint32_t candleTimePeriod)
{
    uint32_t startTime = 0;
    Candle currentCandle;
    std::map<uint32_t, Candle> candleData;
    bool fFirstRun = true;
    for (const std::pair<uint32_t, double> pair : lineChartData) {
        if ((pair.first - startTime) > candleTimePeriod || fFirstRun) {
            if (!fFirstRun) {
                candleData.emplace(std::make_pair(startTime, currentCandle));
            }
            startTime = pair.first;
            currentCandle.m_open = pair.second;
            currentCandle.m_low = pair.second;
            currentCandle.m_high = pair.second;
            currentCandle.m_close = pair.second;
            fFirstRun = false;
        } else {
            currentCandle.m_low = std::min(pair.second, currentCandle.m_low);
            currentCandle.m_high = std::max(pair.second, currentCandle.m_high);
            currentCandle.m_close = pair.second;
        }
    }
    candleData.emplace(std::make_pair(startTime, currentCandle));
    return candleData;
}

void CandlestickChart::SetDataPoints(std::map<uint32_t, Candle>& mapPoints)
{
    m_mapPoints = mapPoints;
    ProcessChangedData();
}

void CandlestickChart::ProcessChangedData()
{
    m_pairXRange = {0, 0};
    m_pairYRange = {0, 0};
    bool fFirstRun = true;
    m_nCandles = 0;
    std::map<uint32_t, Candle>::reverse_iterator rit;
    for (rit = m_mapPoints.rbegin(); rit != m_mapPoints.rend(); ++rit) {
        // determine if candles are out of bounds
        QRect rectChart = ChartArea();
        int nWidth = rectChart.width();
        double nValueX = (2*m_nCandles + 1) * m_nCandleWidth + 2*m_nCandleSpacing;
        if(nValueX > nWidth) {
            break;
        } else {
            m_nCandles++;
        }

        //Set min and max for x and y
        if (fFirstRun || rit->first < m_pairXRange.first)
            m_pairXRange.first = rit->first;
        if (fFirstRun || rit->first > m_pairXRange.second)
            m_pairXRange.second = rit->first;
        if (fFirstRun || rit->second.m_low < m_pairYRange.first)
            m_pairYRange.first = rit->second.m_low;
        if (fFirstRun || rit->second.m_high > m_pairYRange.second)
            m_pairYRange.second = rit->second.m_high;
        fFirstRun = false;
    }
    // Add y-axis buffer for candlestick data
    double buffer = m_yPadding * (m_pairYRange.second - m_pairYRange.first) / 20;
    m_pairYRange.first -= buffer;
    m_pairYRange.second += buffer;
    m_fChangesMade = true;
}

void CandlestickChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    //Fill in the background first
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(m_brushBackground);
    painter.fillRect(rect(), m_brushBackground);

    //If there is only one data point, then return without drawing anything but the background
    if (m_mapPoints.size() <= 1) {
        return;
    }

    //If auto precision is enabled, determine the precision to use
    if (m_settingsYLabels.AutoPrecisionEnabled()) {
        double nDifference = MaxY() - MinY();
        if (m_settingsYLabels.labeltype == AxisLabelType::AX_NUMBER) {
            //Determine by difference in decimal
            m_settingsYLabels.SetPrecision(PrecisionHint(nDifference), /*fDisableAuto*/false);
        }
    }

    //If axis labels are dynamic, get the sizing
    if (m_axisSections > 0) {
        if (m_settingsYLabels.fDynamicSizing) {
            QString strLabel = PrecisionToString(MaxY(), m_settingsYLabels.Precision());
            QFontMetrics fm(m_settingsYLabels.font);
            m_settingsYLabels.sizeDynamicDimension.setWidth(fm.horizontalAdvance(strLabel)+3);
            m_settingsYLabels.sizeDynamicDimension.setHeight(fm.height());
        }

        if (m_settingsXLabels.fDynamicSizing) {
            QString strLabel;
            if (m_settingsXLabels.labeltype == AxisLabelType::AX_TIMESTAMP)
                strLabel = TimeStampToString(MaxX());
            else
                strLabel = PrecisionToString(MaxX(), m_settingsXLabels.Precision());

            QFontMetrics fm(m_settingsXLabels.font);
            m_settingsXLabels.sizeDynamicDimension.setHeight(fm.height()+3);
            m_settingsXLabels.sizeDynamicDimension.setWidth(fm.horizontalAdvance(strLabel)+3);
        }
    }

    QRect rectFull = rect();
    QRect rectChart = ChartArea();

    // Determine if mouse location is inside of the chart
    QPoint gposMouse = QCursor::pos();
    QPoint gposChart = mapToGlobal(QPoint(0+WidthYTitleArea()+WidthYLabelArea(),0+HeightTopTitleArea()));
    QPoint lposMouse = this->mapFromGlobal(gposMouse);
    bool fMouseInChartArea = false;
    if (m_mousedisplay.IsEnabled() && gposMouse.y() >= gposChart.y()) {
        if (gposMouse.y() <= rectChart.height() + gposChart.y()) {
            //Y is in chart range, check if x is in the chart range too
            if (gposMouse.x() >= gposChart.x() && gposMouse.x() <= gposChart.x() + rectChart.width()) {
                fMouseInChartArea = true;
            }
        }
     }

    // Draw Candlestick Info
    if (m_fDisplayOHLC) {
        QPen penLine;
        penLine.setWidth(m_lineWidth);
        painter.setPen(penLine);
        painter.setFont(m_fontOHLC);
        if(fMouseInChartArea) {
            uint32_t nTime = ConvertCandlePlotPointTime(lposMouse);
            // Calculate the candle the mouse is closest to and change OHLC
            std::map<uint32_t, Candle>::iterator candleUpper = m_mapPoints.upper_bound(nTime - m_nCandleTimePeriod);
            std::map<uint32_t, Candle>::iterator candleLower = m_mapPoints.lower_bound(nTime - m_nCandleTimePeriod);
            int upperDist = std::abs(static_cast<int>(candleUpper->first) - static_cast<int>(nTime));
            int lowerDist = std::abs(static_cast<int>(candleLower->first) - static_cast<int>(nTime));
            Candle currentCandle;
            if (upperDist > lowerDist) {
                currentCandle = candleLower->second;
            } else {
                currentCandle = candleUpper->second;
            }
            m_strOHLC = "O:" + QString::number(currentCandle.m_open) + "\t";
            m_strOHLC += "H:" + QString::number(currentCandle.m_high) + "\t";
            m_strOHLC += "L:" + QString::number(currentCandle.m_low) + "\t";
            m_strOHLC += "C:" + QString::number(currentCandle.m_close) + "\t";
            m_strOHLC += QString::number((currentCandle.m_close - currentCandle.m_open)/ currentCandle.m_open)+ "%";
        }
        QRect rectInfo = rectFull;
        rectInfo.setBottom(rectFull.top() + HeightTopTitleArea());
        painter.drawText(rectInfo, Qt::AlignRight, m_strOHLC);
    }

    //Draw axis sections next so that they get covered up by chart fill
    if (m_axisSections > 0) {
        painter.save();
        painter.setPen(m_penAxisSeparater);
        int nSpacingY = rectChart.height() / m_axisSections;
        int nSpacingX = rectChart.width() / m_axisSections;
        std::vector<int> vYPoints;
        std::vector<int> vXPoints;
        for (uint32_t i = 0; i <= m_axisSections; i++) {
            int y = rectChart.top() + i*nSpacingY;
            vYPoints.emplace_back(y);
            QPointF lineLeftPoint(rectChart.left(), y);
            QPointF lineRightPoint(rectChart.right(), y);

            //Don't redraw the bottom x-axis
            if (i < m_axisSections)
                painter.drawLine(lineLeftPoint, lineRightPoint);

            int x = rectChart.left() + i * nSpacingX;
            vXPoints.emplace_back(x);
        }
        painter.restore();

        //Draw the Y-axis labels
        if (m_settingsYLabels.fEnabled) {
            painter.setFont(m_settingsYLabels.font);
            painter.setBrush(m_brushLabels);
            painter.setPen(m_brushLabels.color());
            DrawYLabels(painter, vYPoints, /*isMouseDisplay*/false);
            if (m_mousedisplay.IsEnabled() && fMouseInChartArea) {
                DrawYLabels(painter, {lposMouse.y()}, /*isMouseDisplay*/true);
            }
        }

        //Draw the X-axis labels
        if (m_settingsXLabels.fEnabled) {
            painter.setFont(m_settingsXLabels.font);
            painter.setBrush(m_brushLabels);
            DrawXLabels(painter, vXPoints, /*drawIndicatorLine*/true);

            // Give detail about where mouse is located
            if (m_mousedisplay.IsEnabled() && fMouseInChartArea) {
                //Draw the x label
                DrawXLabels(painter, {lposMouse.x()}, /*drawIndicatorLine*/false);
            }
        }
    }

    //Draw Candlesticks
    QPen penCandle;
    penCandle.setWidth(m_nCandleLineWidth);
    ProcessChangedData();
    std::map<uint32_t, Candle>::reverse_iterator rit;
    for (rit = m_mapPoints.rbegin(); rit != m_mapPoints.rend(); ++rit) {
        std::pair<uint32_t, Candle> chartCandle = ConvertToCandlePlotPoint(*rit);
        if (chartCandle.second.isNull()) {
            break;
        }
        QPointF pointO = QPointF(chartCandle.first, chartCandle.second.m_open);
        QPointF pointH = QPointF(chartCandle.first, chartCandle.second.m_high);
        QPointF pointL = QPointF(chartCandle.first, chartCandle.second.m_low);
        QPointF pointC = QPointF(chartCandle.first, chartCandle.second.m_close);
        if(chartCandle.second.m_open > chartCandle.second.m_close) {
            if(m_fDrawWick) {
                QLineF HCline(pointH, pointC);
                QLineF LOline(pointL, pointO);
                penCandle.setColor(m_colorUpTail);
                painter.setPen(penCandle);
                painter.drawLine(HCline);
                painter.drawLine(LOline);
            }
            if(m_fDisplayCandleDash) {
                QLineF Highline(QPointF(chartCandle.first - m_nCandleWidth, chartCandle.second.m_high), QPointF(chartCandle.first + m_nCandleWidth, chartCandle.second.m_high));
                QLineF Lowline(QPointF(chartCandle.first - m_nCandleWidth, chartCandle.second.m_low), QPointF(chartCandle.first + m_nCandleWidth, chartCandle.second.m_low));
                penCandle.setColor(m_colorUpDash);
                painter.setPen(penCandle);
                painter.drawLine(Highline);
                painter.drawLine(Lowline);
            }
            pointO = QPointF(chartCandle.first + m_nCandleWidth, chartCandle.second.m_open);
            pointC = QPointF(chartCandle.first - m_nCandleWidth, chartCandle.second.m_close);
            QRectF rect(pointO, pointC);
            QBrush rectBrush = m_colorUpCandle;
            if(m_fDrawOutline) {
                penCandle.setColor(m_colorUpCandleLine);
                painter.setPen(penCandle);
                painter.drawRect(rect);
                if (m_fFillCandle) {
                    painter.fillRect(rect, rectBrush);
                } else {
                    painter.fillRect(rect, m_brushBackground);
                }
            } else if (m_fFillCandle) {
                penCandle.setColor(m_colorUpCandle);
                painter.setPen(penCandle);
                painter.drawRect(rect);
                painter.fillRect(rect, rectBrush);
            }
        } else {
            if(m_fDrawWick) {
                QLineF HOline(pointH, pointO);
                QLineF LCline(pointL, pointC);
                penCandle.setColor(m_colorDownTail);
                painter.setPen(penCandle);
                painter.drawLine(HOline);
                painter.drawLine(LCline);
            }
            if(m_fDisplayCandleDash) {
                QLineF Highline(QPointF(chartCandle.first - m_nCandleWidth, chartCandle.second.m_high), QPointF(chartCandle.first + m_nCandleWidth, chartCandle.second.m_high));
                QLineF Lowline(QPointF(chartCandle.first - m_nCandleWidth, chartCandle.second.m_low), QPointF(chartCandle.first + m_nCandleWidth, chartCandle.second.m_low));
                penCandle.setColor(m_colorDownDash);
                painter.setPen(penCandle);
                painter.drawLine(Highline);
                painter.drawLine(Lowline);
            }
            pointO = QPointF(chartCandle.first - m_nCandleWidth, chartCandle.second.m_open);
            pointC = QPointF(chartCandle.first + m_nCandleWidth, chartCandle.second.m_close);
            QRectF rect(pointO, pointC);
            QBrush rectBrush = m_colorDownCandle;
            if(m_fDrawOutline) {
                penCandle.setColor(m_colorDownCandleLine);
                painter.setPen(penCandle);
                painter.drawRect(rect);
                if (m_fFillCandle) {
                    painter.fillRect(rect, rectBrush);
                } else {
                    painter.fillRect(rect, m_brushBackground);
                }
            } else if (m_fFillCandle) {
                penCandle.setColor(m_colorDownCandle);
                painter.setPen(penCandle);
                painter.drawRect(rect);
                painter.fillRect(rect, rectBrush);
            }
        }
    }
    painter.save();
    painter.restore();

    //Draw axis
    if (m_fDrawXAxis) {
        QLineF axisX(rectChart.bottomLeft(), rectChart.bottomRight());
        painter.setPen(Qt::black);
        painter.drawLine(axisX);
    }
    if (m_fDrawYAxis) {
        QLineF axisY(rectChart.bottomLeft(), rectChart.topLeft());
        painter.setPen(Qt::black);
        painter.drawLine(axisY);
    }

    //Draw top title
    if (!m_strTopTitle.isEmpty()) {
        painter.save();
        painter.setFont(m_fontTopTitle);
        QRect rectTopTitle = rectFull;
        rectTopTitle.setBottom(rectFull.top() + HeightTopTitleArea());
        rectTopTitle.setLeft(2*WidthYTitleArea());
        painter.drawText(rectTopTitle, Qt::AlignLeft, m_strTopTitle);
        painter.restore();
    }

    //Draw y title
    if (!m_strTitleY.isEmpty()) {
        painter.save();
        painter.setFont(m_fontYTitle);
        painter.rotate(-90);

        //The painter rotates around the (0,0) coordinate.
        QRect rectRotated;
        rectRotated.setTopLeft(QPoint(0,0));
        rectRotated.setBottomRight(QPoint(rectFull.height(), WidthYTitleArea()));

        // move "left" (from the painter's perspective) because the rotation takes it out of the paint area.
        rectRotated.moveLeft(-rectFull.height());
        painter.drawText(rectRotated, Qt::AlignCenter, m_strTitleY);
        painter.restore();
    }

    //Draw mouse display
    if (m_mousedisplay.IsEnabled() && fMouseInChartArea) {
        //Cross hair lines
        painter.setPen(m_mousedisplay.Pen());
        QPointF posLeft(rectChart.left(), lposMouse.y());
        QPointF posRight(rectChart.right(), lposMouse.y());
        QLineF lineMouseX(posLeft, posRight);
        painter.drawLine(lineMouseX);
        QPointF posTop(lposMouse.x(), rectChart.top());
        QPointF posBottom(lposMouse.x(), rectChart.bottom());
        painter.drawLine(QLineF(posTop, posBottom));
    }
    m_fChangesMade = false;
}

void CandlestickChart::SetCandleBodyColor(const QColor &upColor, const QColor &downColor) {
    m_colorUpCandle = upColor;
    m_colorDownCandle = downColor;
    if(downColor == QColor()) {
        m_colorDownCandleLine = upColor;
    }
    m_fChangesMade = true;
}

void CandlestickChart::SetCandleLineColor(const QColor& upColor, const QColor& downColor) {
    m_colorUpCandleLine = upColor;
    m_colorDownCandleLine = downColor;
    if(downColor == QColor()) {
        m_colorDownCandleLine = upColor;
    }
    m_fChangesMade = true;
}

void CandlestickChart::SetTailColor(const QColor& upColor, const QColor& downColor) {
    m_colorUpTail = upColor;
    m_colorDownTail = downColor;
    if(downColor == QColor()) {
        m_colorDownTail = upColor;
    }
    m_fChangesMade = true;
}

void CandlestickChart::SetDashColor(const QColor& upColor, const QColor& downColor) {
    m_colorUpDash = upColor;
    m_colorDownDash = downColor;
    if(downColor == QColor()) {
        m_colorDownDash = upColor;
    }
    m_fChangesMade = true;
}

void CandlestickChart::SetCandleLineWidth(int nWidth)
{
    m_nCandleLineWidth = nWidth;
    m_fChangesMade = true;
}

void CandlestickChart::SetCandleWidth(int nWidth)
{
    m_nCandleWidth = nWidth;
    m_fChangesMade = true;
}

void CandlestickChart::SetCandleTimePeriod(uint32_t nTime)
{
    m_nCandleTimePeriod = nTime;
    m_fChangesMade = true;
}

void CandlestickChart::SetOLHCFont(const QFont &font)
{
    m_fontOHLC = font;
    m_fChangesMade = true;
}

/**
 * @brief CandlestickChart::ChartArea : Get the area of the widget that is dedicated to the chart itself
 * @return
 */
QRect CandlestickChart::ChartArea() const
{
    QRect rectFull = this->rect();
    QRect rectChart = rectFull;
    rectChart.setTop(rectFull.top() + HeightTopTitleArea());
    rectChart.setBottom(rectFull.bottom() - HeightXLabelArea());
    rectChart.setLeft(rectFull.left() + WidthYTitleArea() + WidthYLabelArea());
    rectChart.setRight(rectFull.right());
    return rectChart;
}

void CandlestickChart::EnableCandleFill(bool fEnable)
{
    m_fFillCandle = fEnable;
}

void CandlestickChart::EnableWick(bool fEnable)
{
    m_fDrawWick = fEnable;
}

void CandlestickChart::EnableCandleBorder(bool fEnable)
{
    m_fDrawOutline = fEnable;
}

void CandlestickChart::EnableCandleDash(bool fEnable)
{
    m_fDisplayCandleDash = fEnable;
}

void CandlestickChart::EnableOHLCDisplay(bool fEnable)
{
    m_fDisplayOHLC = fEnable;
}

void CandlestickChart::wheelEvent(QWheelEvent *event)
{
    int dCandleWidth = event->angleDelta().y()/120;
    if ((m_nCandleWidth > m_nCandleMinWidth && dCandleWidth < 0)
            || (m_nCandleWidth < m_nCandleMaxWidth && dCandleWidth > 0)) {
        m_nCandleWidth += dCandleWidth;
        m_fChangesMade = true;
        emit candleWidthChanged(dCandleWidth);
    }
}

}//namespace
