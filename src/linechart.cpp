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
#include <QPainterPath>

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

LineChart::LineChart(QWidget *parent) : Chart(ChartType::LINE, parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_axisSections = 0;
    m_yPadding = 0;
    m_fEnableFill = true;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;

    m_fDrawVolume = true;
    m_colorVolume = Qt::magenta;
    m_nBarWidth = 5;

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

QPointF LineChart::ConvertToVolumePoint(const std::pair<uint32_t, double> &pair) const
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

    //compute point-value of Volume
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
        dValueY /= 10*nSpanY;
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
 * @brief LineChart::AddDataPoint : Add a data point to a specific line series
 * @param nSeries : The index of the series that is being altered.
 * @param x
 * @param y
 */
void LineChart::AddDataPoint(const uint32_t& nSeries, const uint32_t& x, const double& y)
{
    if (m_vSeries.size() < nSeries+1) {
        //Series does not exist
        return;
    }
    m_vSeries.at(nSeries).emplace(x, y);
    ProcessChangedData();
}

/**
 * @brief LineChart::RemoveDataPoint : Remove a data point from a specific line series
 * @param nSeries : The index of the series that is being altered.
 * @param x
 */
void LineChart::RemoveDataPoint(const uint32_t& nSeries, const uint32_t &x)
{
    if (m_vSeries.size() < nSeries+1) {
        //Series does not exist
        return;
    }
    m_vSeries.at(nSeries).erase(x);
    ProcessChangedData();
}

/**
 * @brief LineChart::SetDataPoints : Set the datapoints of a specific line series.
 * @param mapPoints
 * @param nSeries : The index of the series that is being changed or added.
 */
void LineChart::SetDataPoints(const std::map<uint32_t, double>& mapPoints, const uint32_t& nSeries)
{
    if (m_vSeries.size() < nSeries+1) {
        //Series does not exist yet
        m_vSeries.resize(nSeries+1);
    }
    m_vSeries.at(nSeries) = mapPoints;
    ProcessChangedData();
}

/**
 * @brief LineChart::AddVolumePoint
 * @param nSeries
 * @param x
 * @param y
 */
void LineChart::AddVolumePoint(const uint32_t& nSeries, const uint32_t& x, const double& y)
{
    if (m_vVolume.size() < nSeries+1) {
        //Series does not exist
        return;
    }
    m_vVolume.at(nSeries).emplace(x, y);
    ProcessChangedData();
}

/**
 * @brief LineChart::RemoveVolumePoint
 * @param nSeries
 * @param x
 */
void LineChart::RemoveVolumePoint(const uint32_t& nSeries, const uint32_t &x)
{
    if (m_vVolume.size() < nSeries+1) {
        //Series does not exist
        return;
    }
    m_vVolume.at(nSeries).erase(x);
    ProcessChangedData();
}


/**
 * @brief LineChart::SetDataPoints : Set the datapoints of a specific line series.
 * @param mapPoints
 * @param nSeries : The index of the series that is being changed or added.
 */
void LineChart::SetVolumePoints(const std::map<uint32_t, double>& mapPoints, const uint32_t& nSeries)
{
    if (m_vVolume.size() < nSeries+1) {
        //Series does not exist yet
        m_vVolume.resize(nSeries+1);
    }
    m_vVolume.at(nSeries) = mapPoints;
    ProcessChangedData();
}

void LineChart::ProcessChangedData()
{
    m_pairXRange = {0, 0};
    m_pairYRange = {0, 0};
    bool fFirstRun = true;
    for (const LineSeries& series : m_vSeries) {
        for (const auto& pair : series) {
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
    }
    m_fChangesMade = true;
}

/**
 * @brief GetLineEquation Get the slope and y intercept of a line
 * @param line[in]
 * @param nSlope[out]
 * @param nYIntercept[out]
 */
void LineChart::GetLineEquation(const QLineF& line, double& nSlope, double& nYIntercept)
{
    nSlope = (line.y2() - line.y1()) / (line.x2() - line.x1());
    nYIntercept = line.y1() - (nSlope * line.x1());
}

QColor LineChart::GetSeriesColor(const uint32_t& nSeries) const
{
    QColor color = Qt::black;
    //Default colors if not set
    if (nSeries == 1)
        color = Qt::red;
    else if (nSeries == 2)
        color = Qt::blue;
    else if (nSeries == 3)
        color = Qt::green;
    else if (nSeries == 4)
        color = Qt::yellow;

    if (m_vLineColor.size() >= nSeries + 1)
        color = m_vLineColor.at(nSeries).color();
    return color;
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
//    if (m_mapPoints.size() <= 1) {
//        return;
//    }

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

    //Clear any existing mouse dots
    m_mousedisplay.ClearDots();

    //Draw each series
    for (unsigned int i = 0; i < m_vSeries.size(); i++) {
        const LineSeries& series = m_vSeries.at(i);

        //Create the lines that are drawn
        QVector<QPointF> qvecPolygon;
        QVector<QLineF> qvecLines;

        qvecPolygon.append(rectChart.bottomLeft());
        bool fFirstRun = true;
        QPointF pointPrev;
        bool fMouseSet = false;

        for (const std::pair<uint32_t, double>& pair : series) {
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
                    fMouseSet = true;
                    QColor color = GetSeriesColor(i);
                    m_mousedisplay.AddDot(QPointF(lposMouse.x(), y), color);
                }
            }
        }

        // Cleanly close the polygon
        qvecPolygon.append(QPointF(rectChart.right(), rectChart.bottom()));

        /**Todo - Support fill chart when there are multiple line series**/
        if (m_fEnableFill && m_vSeries.size() == 1) {
            //Fill in the chart area - Note: this is the most computational part of the painting
            QPolygonF polygon(qvecPolygon);
            painter.setBrush(m_brushFill);
            painter.drawConvexPolygon(polygon); //supposedly faster than "drawPolygon()"
        }

        //Draw the lines
        painter.save();

        QBrush brush = GetSeriesColor(i);

        QPen penLine;
        penLine.setBrush(brush);
        penLine.setWidth(m_lineWidth);
        painter.setPen(penLine);
        painter.drawLines(qvecLines);
        painter.restore();
    }

    //Draw Volume Bars
    if(m_fDrawVolume) {
        for (unsigned int i = 0; i < m_vSeries.size(); i++) {
            QPen penBar;
            penBar.setBrush(GetSeriesColor(i));
            penBar.setWidth(m_lineWidth);
            QPen penHighlight;
            penHighlight.setBrush(GetSeriesColor(i));
            penHighlight.setWidth(m_lineWidth);
            for (const std::pair<uint32_t, double> pair : m_vSeries.at(i)) {
                QPointF chartBar = ConvertToVolumePoint(pair);
                QPointF pointBar = QPointF(chartBar.x() + m_nBarWidth, chartBar.y());
                QPointF pointOrigin = QPointF(chartBar.x() - m_nBarWidth, rectChart.bottom());
                QRectF rect(pointBar, pointOrigin);
                QBrush rectBrush = m_colorVolume;
                painter.drawRect(rect);
                if (m_fEnableFill) {
                    painter.fillRect(rect, rectBrush);
                }
            }
        }
    }
    painter.save();
    painter.restore();

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
        painter.setPen(m_colorTopTitle);
        QRect rectTopTitle = rectFull;
        rectTopTitle.setBottom(rectFull.top() + HeightTopTitleArea());
        rectTopTitle.setLeft(2*WidthYTitleArea());
        painter.drawText(rectTopTitle, Qt::AlignCenter, m_strTopTitle);
        painter.restore();
    }

    //Draw y title
    if (!m_strTitleY.isEmpty()) {
        painter.save();
        painter.setFont(m_fontYTitle);
        painter.setPen(m_colorYTitle);
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

        std::vector<MouseDot> vDots = m_mousedisplay.GetDots();
        for (const MouseDot& mousedot : vDots) {
            //Draw a dot on the line series where the mouse X point is
            QPainterPath pathDot;
            QPointF pointCircleCenter(mousedot.Pos());
            pathDot.addEllipse(pointCircleCenter, 5, 5);

            //Set the mouse dot color equal to the line series it is drawn on
            QBrush brushLine = mousedot.Color();

            painter.setBrush(brushLine.color());
            painter.fillPath(pathDot, brushLine.color());

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

void LineChart::SetFillBrush(const QBrush &brush)
{
    m_brushFill = brush;
    m_fChangesMade = true;
}

void LineChart::SetLineBrush(const uint32_t& nSeries, const QBrush &brush)
{
    if (m_vLineColor.size() < nSeries + 1)
        m_vLineColor.resize(nSeries+1);

    m_vLineColor.at(nSeries) = brush;
    m_fChangesMade = true;
}

void LineChart::SetLineWidth(int nWidth)
{
    m_lineWidth = nWidth;
    m_fChangesMade = true;
}

void LineChart::SetVolumeBarWidth(int nWidth)
{
    m_nBarWidth = nWidth;
    m_fChangesMade = true;
}

/**
 * Set the linechart to fill in the area between the line and the bottom of the chart.
 * Default is enabled.
 * @param fEnable
 */
void LineChart::EnableFill(bool fEnable)
{
    m_fEnableFill = fEnable;
    m_fChangesMade = true;
}

void LineChart::EnableVolumeBar(bool fEnable)
{
    m_fDrawVolume = fEnable;
    m_fChangesMade = true;
}

void LineChart::SetVolumeColor(const QColor &color)
{
    m_colorVolume = color;
    m_fChangesMade = true;
}
}//namespace
