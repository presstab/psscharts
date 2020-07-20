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

#include "linechart.h"
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

LineChart::LineChart(QWidget *parent) : QWidget (parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_axisSections = 0;
    m_yPadding = 0;
    m_fEnableFill = true;
    m_fChangesMade = true;
    m_fIsLineChart = false;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;

    setMouseTracking(true);
}

/**
 * @brief LineChart::ConvertToPlotPoint: convert a datapoint into the actual point it will be painted to
 * @param pair
 * @return
 */
QPointF LineChart::ConvertToPlotPoint(const std::pair<uint32_t, double> &pair) const
{
    QRect rectChart = ChartArea();
    if (m_yPadding > 0) {
        //Y padding will make it so that there is an area on the top and bottom of the ChartArea
        //that does not get drawn in. It makes it so the lines don't go right onto the edges.
        rectChart.setBottom(rectChart.bottom() - m_yPadding);
        rectChart.setTop(rectChart.top() + m_yPadding);
    }

    //compute point-value of X
    int nWidth = rectChart.width();
    double nValueMaxX = (MaxX() - MinX());
    double nValueX = ((pair.first - MinX()) / nValueMaxX);
    nValueX *= nWidth;
    nValueX += rectChart.left();

    //compute point-value of Y
    int nHeight = rectChart.height();
    uint64_t y1 = pair.second * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxY = MaxY()*m_precision;
    uint64_t nMinY = MinY()*m_precision;
    uint64_t nSpanY = (nMaxY - nMinY);
    uint64_t nValueY = (y1 - nMinY);
    nValueY *= nHeight;
    double dValueY = nValueY;
    if (nSpanY == 0) {
        dValueY = rectChart.top() + (nHeight/2);
    } else {
        dValueY /= nSpanY;
        dValueY = rectChart.bottom() - dValueY; // Qt uses inverted Y axis
    }
    return QPointF(nValueX, dValueY);
}

/**
 * @brief LineChart::ConvertFromPlotPoint: convert a point on the chart to the value it represents
 * @return
 */
std::pair<uint32_t, double> LineChart::ConvertFromPlotPoint(const QPointF& point)
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

/**
 * @brief LineChart::ConvertToPlotPoint: convert a datapoint into the actual point it will be painted to
 * @param pair
 * @return
 */
std::pair<uint32_t, PssCharts::Candle> LineChart::ConvertToCandlePlotPoint(const std::pair<uint32_t, Candle> &pair) const
{
    QRect rectChart = ChartArea();
    if (m_yPadding > 0) {
        //Y padding will make it so that there is an area on the top and bottom of the ChartArea
        //that does not get drawn in. It makes it so the lines don't go right onto the edges.
        rectChart.setBottom(rectChart.bottom() - m_yPadding);
        rectChart.setTop(rectChart.top() + m_yPadding);
    }

    //compute point-value of X
    int nWidth = rectChart.width();
    double nValueMaxX = (MaxX() - MinX());
    double nValueX = ((pair.first - MinX()) / nValueMaxX);
    nValueX *= nWidth;
    nValueX += rectChart.left();

    //compute point-value of Open
    int nHeight = rectChart.height();
    uint64_t open1 = pair.second.m_open * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxOpen = MaxY()*m_precision;
    uint64_t nMinOpen = MinY()*m_precision;
    uint64_t nSpanOpen = (nMaxOpen - nMinOpen);
    uint64_t nValueOpen = (open1 - nMinOpen);
    nValueOpen *= nHeight;
    double dValueOpen = nValueOpen;
    if (nSpanOpen == 0) {
        dValueOpen = rectChart.top() + (nHeight/2);
    } else {
        dValueOpen /= nSpanOpen;
        dValueOpen = rectChart.bottom() - dValueOpen; // Qt uses inverted Y axis
    }
    //compute point-value of High
    uint64_t high1 = pair.second.m_high * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxHigh = MaxY()*m_precision;
    uint64_t nMinHigh = MinY()*m_precision;
    uint64_t nSpanHigh = (nMaxHigh - nMinHigh);
    uint64_t nValueHigh = (high1 - nMinHigh);
    nValueHigh *= nHeight;
    double dValueHigh = nValueHigh;
    if (nSpanHigh == 0) {
        dValueHigh = rectChart.top() + (nHeight/2);
    } else {
        dValueHigh /= nSpanHigh;
        dValueHigh = rectChart.bottom() - dValueHigh; // Qt uses inverted Y axis
    }
    //compute point-value of Low
    uint64_t Low1 = pair.second.m_low * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxLow = MaxY()*m_precision;
    uint64_t nMinLow = MinY()*m_precision;
    uint64_t nSpanLow = (nMaxLow - nMinLow);
    uint64_t nValueLow = (Low1 - nMinLow);
    nValueLow *= nHeight;
    double dValueLow = nValueLow;
    if (nSpanLow == 0) {
        dValueLow = rectChart.top() + (nHeight/2);
    } else {
        dValueLow /= nSpanLow;
        dValueLow = rectChart.bottom() - dValueLow; // Qt uses inverted Y axis
    }
    //compute point-value of Close
    uint64_t Close1 = pair.second.m_close * m_precision; //Convert to precision/uint64_t to force a certain decimal precision
    uint64_t nMaxClose = MaxY()*m_precision;
    uint64_t nMinClose = MinY()*m_precision;
    uint64_t nSpanClose = (nMaxClose - nMinClose);
    uint64_t nValueClose = (Close1 - nMinClose);
    nValueClose *= nHeight;
    double dValueClose = nValueClose;
    if (nSpanClose == 0) {
        dValueClose = rectChart.top() + (nHeight/2);
    } else {
        dValueClose /= nSpanClose;
        dValueClose = rectChart.bottom() - dValueClose; // Qt uses inverted Y axis
    }
    return std::pair<uint32_t, PssCharts::Candle>(nValueX, Candle(dValueHigh, dValueLow, dValueOpen, dValueClose));
}

/**
 * @brief LineChart::ConvertFromPlotPoint: get the timestamp for a candle on the chart
 * @return
 */
uint32_t LineChart::ConvertCandlePlotPointTime(const QPointF& point)
{
    QRect rectChart = ChartArea();
    auto t1 = (point.x() - rectChart.left()) / rectChart.width();
    auto t2 = t1 * (MaxX() - MinX());
    return  t2 + MinX();
}

int LineChart::HeightTopTitleArea() const
{
    if (m_topTitleHeight != -1)
        return m_topTitleHeight;

    if (m_strTopTitle.isEmpty())
        return 0;

    return 30;
}

void LineChart::SetTopTitleHeight(int height)
{
    m_topTitleHeight = height;
}

int LineChart::HeightXLabelArea() const
{
    if (m_settingsXLabels.fDynamicSizing && m_settingsXLabels.sizeDynamicDimension.height() > 0)
        return m_settingsXLabels.sizeDynamicDimension.height();

    if (m_settingsXLabels.nDimension != -1)
        return m_settingsXLabels.nDimension;

    if (m_settingsXLabels.labeltype == AxisLabelType::AX_NO_LABEL)
        return 0;
    return 30; //fallback
}

void LineChart::SetXLabelHeight(int height)
{
    m_settingsXLabels.nDimension = height;
}

int LineChart::WidthYTitleArea() const
{
    if (m_strTitleY.isEmpty())
        return 0;
    return 30;
}

int LineChart::WidthYLabelArea() const
{
    if (m_settingsYLabels.fDynamicSizing && m_settingsYLabels.sizeDynamicDimension.width() > 0)
        return  m_settingsYLabels.sizeDynamicDimension.width();

    if (m_settingsYLabels.labeltype == AxisLabelType::AX_NO_LABEL)
        return 0;

    return m_settingsYLabels.nDimension;
}

int LineChart::WidthRightMargin() const
{
    if (m_rightMargin != -1)
        return m_rightMargin;

    //No specific instructions given, so size so that the whole x label will fit
    if (m_settingsXLabels.fDynamicSizing && m_settingsXLabels.sizeDynamicDimension.width() > 0) {
        int nWidth = m_settingsXLabels.sizeDynamicDimension.width()/2;
        if (nWidth % 2)
            nWidth += 1;
        return nWidth;
    }

    //Just copy left side for symetry
    if (WidthYTitleArea() + WidthYLabelArea() > 0)
        return WidthYLabelArea();
    return 0;
}

void LineChart::AddDataPoint(const uint32_t& x, const double& y)
{
    m_mapPoints.emplace(x, y);
    ProcessChangedData();
}

void LineChart::EnableMouseDisplay(bool fEnable)
{
    m_mousedisplay.SetEnabled(fEnable);
}

void LineChart::RemoveDataPoint(const uint32_t &x)
{
    m_mapPoints.erase(x);
    ProcessChangedData();
}

void LineChart::SetDataPoints(const std::map<uint32_t, double>& mapPoints)
{
    m_mapPoints = mapPoints;
    ProcessChangedData();
}

void LineChart::SetCandleDataPoints(std::map<uint32_t, Candle>& mapPoints)
{
    m_candlePoints = mapPoints;
    ProcessChangedData();
}

void LineChart::ProcessChangedData()
{
    m_pairXRange = {0, 0};
    m_pairYRange = {0, 0};
    bool fFirstRun = true;
    if(m_fIsLineChart) {
        for (const auto& pair : m_mapPoints) {
            //Set min and max for x and y
            if (fFirstRun || pair.first < m_pairXRange.first)
                m_pairXRange.first = pair.first;
            if (fFirstRun || pair.first > m_pairXRange.second)
                m_pairXRange.second = pair.first;
            if (fFirstRun || pair.second < m_pairYRange.first)
                m_pairYRange.first = pair.second;
            if (fFirstRun || pair.second > m_pairYRange.second)
                m_pairYRange.second = pair.second;
            fFirstRun = false;
        }
        m_fChangesMade = true;
    } else {
        for (const auto& candle : m_candlePoints) {
            //Set min and max for x and y
            if (fFirstRun || candle.first < m_pairXRange.first)
                m_pairXRange.first = candle.first;
            if (fFirstRun || candle.first > m_pairXRange.second)
                m_pairXRange.second = candle.first;
            if (fFirstRun || candle.second.m_low < m_pairYRange.first)
                m_pairYRange.first = candle.second.m_low;
            if (fFirstRun || candle.second.m_high > m_pairYRange.second)
                m_pairYRange.second = candle.second.m_high;
            fFirstRun = false;
        }
        m_pairYRange.first = 0;
        m_pairYRange.second = 1;
//        m_pairXRange.first -= m_rectWidth;
//        m_pairXRange.second += m_rectWidth;
        m_fChangesMade = true;
    }
}

/**
 * @brief GetLineEquation Get the slope and y intercept of a line
 * @param line[in]
 * @param nSlope[out]
 * @param nYIntercept[out]
 */
void GetLineEquation(const QLineF& line, double& nSlope, double& nYIntercept)
{
    nSlope = (line.y2() - line.y1()) / (line.x2() - line.x1());
    nYIntercept = line.y1() - (nSlope * line.x1());
}

void LineChart::paintEvent(QPaintEvent *event)
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

    if (m_fIsLineChart) {
        //Create the lines that are drawn
        QVector<QPointF> qvecPolygon;
        QVector<QLineF> qvecLines;

        qvecPolygon.append(rectChart.bottomLeft());
        bool fFirstRun = true;
        QPointF pointPrev;
        bool fMouseSet = false;
        for (const std::pair<uint32_t, double>& pair : m_mapPoints) {
            QPointF point = ConvertToPlotPoint(pair);
            qvecPolygon.append(point);
            if (fFirstRun) {
                pointPrev = point;
                fFirstRun = false;
                continue;
            }

            QLineF line(pointPrev, point);
            qvecLines.append(line);
            pointPrev = point;

            if (fMouseInChartArea && !fMouseSet) {
                //Find the line that the mouse x point would belong on
                if (lposMouse.x() >= line.x1() && lposMouse.x() <= line.x2()) {
                    double nLineSlope = 0;
                    double nLineYIntercept = 0;
                    GetLineEquation(line, nLineSlope, nLineYIntercept);
                    double y = nLineSlope * lposMouse.x() + nLineYIntercept;
                    m_mousedisplay.SetDot(QPointF(lposMouse.x(), y));
                    fMouseSet = true;
                }
            }
        }

        // Cleanly close the polygon
        qvecPolygon.append(QPointF(rectChart.right(), rectChart.bottom()));

        if (m_fEnableFill) {
            //Fill in the chart area - Note: this is the most computational part of the painting
            QPolygonF polygon(qvecPolygon);
            painter.setBrush(m_brushFill);
            painter.drawConvexPolygon(polygon); //supposedly faster than "drawPolygon()"
        }

        //Draw the lines
        painter.save();
        QPen penLine;
        penLine.setBrush(m_brushLine);
        penLine.setWidth(m_lineWidth);
        painter.setPen(penLine);
        painter.drawLines(qvecLines);
        painter.restore();
    } else {
        //Create the lines that are drawn
        QVector<QPointF> qvecPolygon;
        QVector<QLineF> qvecLines;
        QVector<QRectF> qvecRects;
        painter.save();
        QPen penUpLine;
        penUpLine.setBrush(m_colorUpTail);
        penUpLine.setWidth(m_nCandleLineWidth);
        QPen penDownLine;
        penDownLine.setBrush(m_colorDownTail);
        penDownLine.setWidth(m_nCandleLineWidth);
        QPen penUpCandle;
        penUpCandle.setBrush(m_colorUpCandleLine);
        penUpCandle.setWidth(m_nCandleLineWidth);
        QPen penDownCandle;
        penDownCandle.setBrush(m_colorDownCandleLine);
        penDownCandle.setWidth(m_nCandleLineWidth);
        qvecPolygon.append(rectChart.bottomLeft());
        for (const std::pair<uint32_t, Candle>& pair : m_candlePoints) {
            std::pair<uint32_t, Candle> chartCandle = ConvertToCandlePlotPoint(pair);
            bool isUpCandle = chartCandle.second.m_close > chartCandle.second.m_open;
            if (isUpCandle) {
                QPointF pointO = QPointF(chartCandle.first - m_rectWidth, chartCandle.second.m_open);
                QPointF pointC = QPointF(chartCandle.first + m_rectWidth, chartCandle.second.m_close);
                QRectF rect(pointO, pointC);
                QBrush rectBrush = m_brushUpCandle;
                painter.setPen(penUpCandle);
                painter.drawRect(rect);
                painter.fillRect(rect, rectBrush);

                pointO = QPointF(chartCandle.first, chartCandle.second.m_open);
                pointC = QPointF(chartCandle.first, chartCandle.second.m_close);
                QPointF pointH = QPointF(chartCandle.first, chartCandle.second.m_high);
                QPointF pointL = QPointF(chartCandle.first, chartCandle.second.m_low);

                QLineF HOline(pointH, pointO);
                QLineF LCline(pointL, pointC);
                painter.setPen(penUpLine);
                painter.drawLine(HOline);
                painter.drawLine(LCline);
            } else {
                QPointF pointO = QPointF(chartCandle.first + m_rectWidth, chartCandle.second.m_open);
                QPointF pointC = QPointF(chartCandle.first - m_rectWidth, chartCandle.second.m_close);
                QRectF rect(pointO, pointC);
                QBrush rectBrush = m_brushDownCandle;
                painter.setPen(penDownCandle);
                painter.drawRect(rect);
                painter.fillRect(rect, rectBrush);

                pointO = QPointF(chartCandle.first, chartCandle.second.m_open);
                pointC = QPointF(chartCandle.first, chartCandle.second.m_close);
                QPointF pointH = QPointF(chartCandle.first, chartCandle.second.m_high);
                QPointF pointL = QPointF(chartCandle.first, chartCandle.second.m_low);

                QLineF HCline(pointH, pointC);
                QLineF LOline(pointL, pointO);
                painter.setPen(penDownLine);
                painter.drawLine(HCline);
                painter.drawLine(LOline);
            }
        }
        painter.save();
        painter.restore();
    }

    // Draw Candlestick Info
    QPen penLine;
//    penLine.setBrush(m_brushLine);
//    penLine.setWidth(m_lineWidth);
    painter.setPen(penLine);
    painter.setFont(m_fontYTitle);
    QString strData;
    uint32_t nTime = ConvertCandlePlotPointTime(lposMouse);
    std::map<uint32_t, Candle>::iterator itCandle;
    itCandle = m_candlePoints.lower_bound(nTime);
    Candle currentCandle = itCandle->second;
    if (fMouseInChartArea) {
        strData += "O:" + QString::number(currentCandle.m_open) + "\t";
        strData += "H:" + QString::number(currentCandle.m_high) + "\t";
        strData += "L:" + QString::number(currentCandle.m_low) + "\t";
        strData += "C:" + QString::number(currentCandle.m_close);
    }
    QRect rectInfo = rectFull;
    rectInfo.setBottom(rectFull.top() + HeightTopTitleArea());
    painter.drawText(rectInfo, Qt::AlignRight, strData);

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
        if (m_fIsLineChart) {
            painter.drawText(rectTopTitle, Qt::AlignCenter, m_strTopTitle);
        } else {
            painter.drawText(rectTopTitle, Qt::AlignLeft, m_strTopTitle);
        }
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

        //Draw a dot on the line series where the mouse X point is
        if(m_fIsLineChart) {
            QPainterPath pathDot;
            QPointF pointCircleCenter(m_mousedisplay.DotPos());
            pathDot.addEllipse(pointCircleCenter, 5, 5);
            painter.setBrush(m_brushLine.color());
            painter.fillPath(pathDot, m_brushLine.color());

            //Add a border to the dot
    //        QPen pen;
    //        pen.setColor(m_mousedisplay.LabelBackgroundColor());
    //        pen.setWidth(2);
    //        painter.setPen(pen);
    //        painter.drawPath(pathDot);

            //Draw a small tooltip looking item showing the point's data (x,y)
            auto pairData = ConvertFromPlotPoint(pointCircleCenter);
            const uint32_t& nX = pairData.first;
            const double& nY = pairData.second;
            QString strLabel = "(";
            if (m_settingsXLabels.labeltype == AxisLabelType::AX_TIMESTAMP) {
                strLabel += TimeStampToString(nX);
            } else {
                strLabel += PrecisionToString(nX, m_settingsXLabels.Precision());
            }
            strLabel += ", ";
            strLabel += PrecisionToString(nY, m_settingsYLabels.Precision());
            strLabel += ")";

            //Create the background of the tooltip
            QRect rectDraw = MouseOverTooltipRect(painter, rectFull, pointCircleCenter, strLabel);

            QPainterPath pathBackground;
            pathBackground.addRoundedRect(rectDraw, 5, 5);
            painter.fillPath(pathBackground, m_mousedisplay.LabelBackgroundColor());

            //Draw the text of the tooltip
            painter.setBrush(m_brushLabels);
            painter.setPen(Qt::black);
            painter.drawText(rectDraw, Qt::AlignCenter, strLabel);
        }
    }
    m_fChangesMade = false;
}

/**
 * @brief LineChart::MouseOverTooltipRect Get the boundaries of the tooltip that is drawn for the mouseover data.
 * @param painter: The chart widget's painter.
 * @param rectFull: The QRect of the entire drawing area of the chart widget.
 * @param pointCircleCenter: the center of the dot that is being drawn for the mouseover.
 * @param strLabel: the label text that is being placed in the tooltip.
 * @return QRect with the coordinates that the tooltip should be drawn in.
 */
QRect LineChart::MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const
{
    QFontMetrics fm(painter.font());
    int nWidthText = fm.horizontalAdvance(strLabel) + 4;

    //Place the tooltip right below the dot being displayed.
    QPoint pointTopLeft(pointCircleCenter.x() - nWidthText/2, pointCircleCenter.y()+10);

    QRect rectDraw;
    rectDraw.setTopLeft(pointTopLeft);
    rectDraw.setWidth(nWidthText);
    rectDraw.setHeight(fm.height() + 4);

    //The tooltip is outside of the drawing zone, shift it into the drawing zone
    if (rectDraw.left() < rectFull.left())
        rectDraw.moveRight(rectFull.left()+3);
    if (rectDraw.right() > rectFull.right())
        rectDraw.moveLeft(rectFull.right() - rectDraw.width() - 3);

    return rectDraw;
}

void LineChart::DrawXLabels(QPainter& painter, const std::vector<int>& vXPoints, bool fDrawIndicatorLine)
{
    QFontMetrics fm(painter.font());

    QRect rectXLabels = XLabelArea();
    for (int x : vXPoints) {
        QPointF pointDraw(x, rectXLabels.top());
        auto pairPoints = ConvertFromPlotPoint(pointDraw);
        const uint32_t& nValue = pairPoints.first;

        QString strLabel;
        if (m_settingsXLabels.labeltype == AxisLabelType::AX_TIMESTAMP) {
            strLabel = TimeStampToString(nValue);
        } else {
            strLabel = PrecisionToString(nValue, m_settingsXLabels.Precision());
        }

        QRect rectDraw;
        rectDraw.setTopLeft(pointDraw.toPoint());
        int nWidthText = fm.horizontalAdvance(strLabel);
        rectDraw.setBottomRight(QPoint(pointDraw.x() + nWidthText, rectXLabels.bottom()));

        //Center the label on the line
        rectDraw.moveLeft(rectDraw.left() - rectDraw.width() / 2);
        if (!fDrawIndicatorLine) {
            //Assume this is for the mouse display if not using an indicator line
            QPainterPath path;
            path.addRoundedRect(rectDraw, 5, 5);
            painter.fillPath(path, m_mousedisplay.LabelBackgroundColor());
        }

        painter.drawText(rectDraw, Qt::AlignCenter, strLabel);

        //Draw a line on the axis showing the location of this x value
        if (fDrawIndicatorLine)
            painter.drawLine(QLine(QPoint(rectDraw.center().x(), rectDraw.top()),
                                   QPoint(rectDraw.center().x(), rectDraw.top() + 5)));
    }
}

void LineChart::DrawYLabels(QPainter &painter, const std::vector<int> &vYPoints, bool isMouseDisplay)
{
    QFontMetrics fm(painter.font());

    QRect rectYLabels = YLabelArea();
    for (int y : vYPoints) {
        QPointF pointDraw(rectYLabels.left(), y);
        auto pairPoints = ConvertFromPlotPoint(pointDraw);
        const double& nValue = pairPoints.second;

        QString strLabel = PrecisionToString(nValue, m_settingsYLabels.Precision());

        int nWidthText = fm.horizontalAdvance(strLabel);

        QRect rectDraw;
        rectDraw.setTopLeft(pointDraw.toPoint());
        rectDraw.setBottomRight(QPoint(rectYLabels.right() - 3, rectDraw.y() + fm.height()));

        //Center the label onto the line
        rectDraw.moveTop(rectDraw.top() - fm.height() / 2);

        Qt::AlignmentFlag hAlign = (nWidthText > rectYLabels.width() ? Qt::AlignLeft : Qt::AlignRight);

        if (isMouseDisplay) {
            QPainterPath path;
            path.addRoundedRect(rectDraw, 5, 5);
            painter.fillPath(path, m_mousedisplay.LabelBackgroundColor());
        }

        painter.drawText(rectDraw, hAlign, strLabel);
    }
}

QPixmap LineChart::grab(const QRect &rectangle)
{
    auto rectValid = rectangle.height() != -1;
    bool fRectSizeMatch = (rectValid ? rectangle.size() == m_pixmapCache.size() : true);
    bool fWidgetSizeMatch = this->size() == m_pixmapCache.size();
    if (!m_fChangesMade && fRectSizeMatch && fWidgetSizeMatch)
        return m_pixmapCache;

    m_pixmapCache = QWidget::grab(rectangle);
    return m_pixmapCache;
}

void LineChart::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    repaint();
}

void LineChart::SetUpCandleBrush(const QBrush &brush)
{
    m_brushUpCandle = brush;
    QPalette palette = this->palette();
    palette.setBrush(QPalette::ColorRole::Base, brush);
    m_fChangesMade = true;
}

void LineChart::SetDownCandleBrush(const QBrush &brush)
{
    m_brushDownCandle = brush;
    QPalette palette = this->palette();
    palette.setBrush(QPalette::ColorRole::Base, brush);
    m_fChangesMade = true;
}


void LineChart::SetCandleLineColor(const QColor& upColor, const QColor& downColor) {
    m_colorUpCandleLine = upColor;
    m_colorDownCandleLine = downColor;
    if(downColor == QColor()) {
        m_colorDownCandleLine = upColor;
    }
}

void LineChart::SetTailColor(const QColor& upColor, const QColor& downColor) {
    m_colorUpTail = upColor;
    m_colorDownTail = downColor;
    if(downColor == QColor()) {
        m_colorDownTail = upColor;
    }
}

void LineChart::SetBackgroundBrush(const QBrush &brush)
{
    m_brushBackground = brush;
    QPalette palette = this->palette();
    palette.setBrush(QPalette::ColorRole::Base, brush);
    m_fChangesMade = true;
}

void LineChart::SetFillBrush(const QBrush &brush)
{
    m_brushFill = brush;
    m_fChangesMade = true;
}

void LineChart::SetLineBrush(const QBrush &brush)
{
    m_brushLine = brush;
    m_fChangesMade = true;
}

void LineChart::SetLineWidth(int nWidth)
{
    m_lineWidth = nWidth;
    m_fChangesMade = true;
}

void LineChart::SetCandleLineWidth(int nWidth)
{
    m_nCandleLineWidth = nWidth;
    m_fChangesMade = true;
}

void LineChart::SetCandleWidth(int nWidth)
{
    m_rectWidth = nWidth;
    m_fChangesMade = true;
}

void LineChart::SetTopTitle(const QString &strTitle)
{
    m_strTopTitle = strTitle;
    m_fChangesMade = true;
}

void LineChart::SetTopTitleFont(const QFont &font)
{
    m_fontTopTitle = font;
    m_fChangesMade = true;
}

void LineChart::SetXLabelType(AxisLabelType labelType)
{
    m_settingsXLabels.labeltype = labelType;
    m_fChangesMade = true;
}

void LineChart::SetYLabelType(AxisLabelType labelType)
{
    m_settingsYLabels.labeltype = labelType;
    m_fChangesMade = true;
}

void LineChart::SetYLabelWidth(int width)
{
    m_settingsYLabels.nDimension = width;
}

void LineChart::SetYTitle(const QString &strTitle)
{
    m_strTitleY = strTitle;
    m_fChangesMade = true;
}

void LineChart::SetYTitleFont(const QFont &font)
{
    m_fontYTitle = font;
    m_fChangesMade = true;
}

void LineChart::SetAxisOnOff(bool fDrawX, bool fDrawY)
{
    m_fDrawXAxis = fDrawX;
    m_fDrawYAxis = fDrawY;
    m_fChangesMade = true;
}

/**
 * Set the axis labels to be on or off. Default is off.
 * @param fDrawXLabels
 * @param fDrawYLabels
 */
void LineChart::SetAxisLabelsOnOff(bool fDrawXLabels, bool fDrawYLabels)
{
    m_settingsXLabels.fEnabled = fDrawXLabels;
    m_settingsYLabels.fEnabled = fDrawYLabels;
}

/**
 * @brief LineChart::SetAxisSectionCount: Set the number of vertical dividers that are displayed.
 * @param nCount
 */
void LineChart::SetAxisSectionCount(uint32_t nCount)
{
    m_axisSections = nCount;
    m_fChangesMade = true;
}

/**
 * @brief LineChart::SetAxisSeparatorPen: The pen that is used to draw divider lines for the axis sections.
 * @param pen
 */
void LineChart::SetAxisSeparatorPen(const QPen &pen)
{
    m_penAxisSeparater = pen;
    m_fChangesMade = true;
}

void LineChart::SetChartType(const QString &type)
{
    m_fIsLineChart = type == "Line";
}

uint32_t LineChart::Version() const
{
    uint32_t nVersion = 0;
    nVersion |= VERSION_BUILD;
    nVersion |= (VERSION_REVISION << 8);
    nVersion |= (VERSION_MINOR << 16);
    nVersion |= (VERSION_MAJOR << 24);
    return nVersion;
}

QString LineChart::VersionString() const
{
    return QString("v%1.%2.%3.%4")
            .arg(QString::number(VERSION_MAJOR))
            .arg(QString::number(VERSION_MINOR))
            .arg(QString::number(VERSION_REVISION))
            .arg(QString::number(VERSION_BUILD));
}

/**
 * @brief LineChart::ChartArea : Get the area of the widget that is dedicated to the chart itself
 * @return
 */
QRect LineChart::ChartArea() const
{
    QRect rectFull = this->rect();
    QRect rectChart = rectFull;
    rectChart.setTop(rectFull.top() + HeightTopTitleArea());
    rectChart.setBottom(rectFull.bottom() - HeightXLabelArea());
    rectChart.setLeft(rectFull.left() + WidthYTitleArea() + WidthYLabelArea());
    rectChart.setRight(rectFull.right() - WidthRightMargin());

    return rectChart;
}

/**
 * @brief LineChart::YLabelArea The area that is reserved for Y-axis labels. This is between the Y title
 * area and the left side of the ChartArea.
 * @return
 */
QRect LineChart::YLabelArea() const
{
    QRect rectFull = this->rect();
    QRect rectChart = ChartArea();
    QRect rectYLabel;
    rectYLabel.setLeft(rectFull.left() + WidthYTitleArea());
    rectYLabel.setRight(rectChart.left());
    rectYLabel.setBottom(rectChart.bottom());
    rectYLabel.setTop(rectChart.top());

    return rectYLabel;
}

/**
 * @brief LineChart::XLabelArea The area that is reserved for X-axis labels. This is between the bottom
 * of the LineChart QWidget and the bottom of the ChartArea.
 * @return
 */
QRect LineChart::XLabelArea() const
{
    QRect rectXLabels;
    QRect rectFull = this->rect();
    QRect rectChart = ChartArea();
    rectXLabels.setTopLeft(rectChart.bottomLeft());
    rectXLabels.setBottomRight(QPoint(rectChart.right(), rectFull.bottom()));
    return rectXLabels;
}

const double &LineChart::MaxX() const
{
    return m_pairXRange.second;
}

const double &LineChart::MaxY() const
{
    return m_pairYRange.second;
}

const double &LineChart::MinX() const
{
    return m_pairXRange.first;
}

const double &LineChart::MinY() const
{
    return m_pairYRange.first;
}

QBrush LineChart::BackgroundBrush() const
{
    return m_brushBackground;
}

void LineChart::SetYLabelFont(const QFont& font)
{
    m_settingsYLabels.font = font;
}

void LineChart::SetYPadding(int nPadding)
{
    m_yPadding = nPadding;
}

void LineChart::SetAxisLabelsBrush(const QBrush& brush)
{
    m_brushLabels = brush;
}

/**
 * Set the linechart to fill in the area between the line and the bottom of the chart.
 * Default is enabled.
 * @param fEnable
 */
void LineChart::EnableFill(bool fEnable)
{
    m_fEnableFill = fEnable;
}

/**
 * Set the precision that the axis labels will print their numbers at.
 * @param precision
 */
void LineChart::SetLabelPrecision(int precision)
{
    m_settingsYLabels.SetPrecision(precision, /*disableAuto*/true);
    m_settingsXLabels.SetPrecision(precision, /*disableAuto*/true);
}

void LineChart::SetLabelAutoPrecision(bool fEnable)
{
    m_settingsYLabels.SetAutoPrecision(fEnable);
    m_settingsXLabels.SetAutoPrecision(fEnable);
}

void LineChart::SetRightMargin(int margin)
{
    m_rightMargin = margin;
}

}//namespace
