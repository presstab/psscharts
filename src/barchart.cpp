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

#include "barchart.h"
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

BarChart::BarChart(QWidget *parent) : Chart (ChartType::BAR, parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_lineWidth = 3;
    m_nBarSpacing = 2;
    m_nBarWidth = 8;
    m_nBarMaxWidth = 20;
    m_nBarMinWidth = 1;

    m_fEnableFill = true;
    m_fEnableOutline = true;
    m_fEnableHighlightBar = true;
    m_fEnableHighlightOutline = true;

    m_axisSections = 0;
    m_yPadding = 0;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;
    m_brushLine = QBrush(QColor(Qt::black));
    m_color = QColor(Qt::green);
    m_brushLineHighlight = QBrush(QColor(Qt::white));
    m_highlight = QColor(Qt::magenta);
    setMouseTracking(true);
}

/**
 * @brief LineChart::ConvertToPlotPoint: convert a datapoint into the actual point it will be painted to
 * @param pair
 * @return
 */
QPointF BarChart::ConvertToPlotPoint(const std::pair<uint32_t, double> &pair)
{
    QRect rectChart = ChartArea();
    if (m_yPadding > 0) {
        //Y padding will make it so that there is an area on the top and bottom of the ChartArea
        //that does not get drawn in. It makes it so the lines don't go right onto the edges.
        rectChart.setBottom(rectChart.bottom() - m_yPadding);
        rectChart.setTop(rectChart.top() + m_yPadding);
    }

    //compute point-value of X
    m_nBars = 0;
    int nWidth = rectChart.width();
    double nValueMaxX = (MaxX() - MinX());
    double nValueX = ((pair.first - MinX()) / nValueMaxX);
    nValueX *= nWidth;
    nValueX += rectChart.left() - (2*m_nBars + 1) * m_nBarWidth; // factor in space bars take
    if(nValueX < rectChart.left()) {
        // outside of chart
        return QPointF(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    } else {
        m_nBars++;
    }

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
 * @brief BarChart::ConvertFromPlotPoint: get the timestamp for a candle on the chart
 * @return
 */
std::pair<uint32_t, double> BarChart::ConvertFromPlotPoint(const QPointF& point)
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

void BarChart::SetDataPoints(std::map<uint32_t, double>& mapPoints)
{
    m_mapPoints = mapPoints;
    ProcessChangedData();
}

void BarChart::ProcessChangedData()
{
    m_pairXRange = {0, 0};
    m_pairYRange = {0, 0};
    bool fFirstRun = true;
    m_nBars = 0;
    std::map<uint32_t, double>::reverse_iterator rit;
    for (rit = m_mapPoints.rbegin(); rit != m_mapPoints.rend(); ++rit) {
        // determine if bars are out of bounds
        QRect rectChart = ChartArea();
        int nWidth = rectChart.width();
        double nValueX = (2*m_nBars + 1) * m_nBarWidth + 2*m_nBarSpacing;
        if(nValueX > nWidth) {
            break;
        } else {
            m_nBars++;
        }

        //Set min and max for x and y
        if (fFirstRun || rit->first < m_pairXRange.first)
            m_pairXRange.first = rit->first;
        if (fFirstRun || rit->first > m_pairXRange.second)
            m_pairXRange.second = rit->first;
        if (rit->second < m_pairYRange.first)
            m_pairYRange.first = rit->second;
        if (rit->second > m_pairYRange.second)
            m_pairYRange.second = rit->second;
        fFirstRun = false;
    }
    m_fChangesMade = true;
}

void BarChart::paintEvent(QPaintEvent *event)
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

    //Draw Bars
    QPen penBar;
    penBar.setBrush(m_brushLine);
    penBar.setWidth(m_lineWidth);
    QPen penHighlight;
    penHighlight.setBrush(m_brushLineHighlight);
    penHighlight.setWidth(m_lineWidth);
    bool fMouseSet = false;
    ProcessChangedData();
    for (const std::pair<uint32_t, double>& pair : m_mapPoints) {
        QPointF chartBar = ConvertToPlotPoint(pair);
        QPointF pointBar = QPointF(chartBar.x() + m_nBarWidth, chartBar.y());
        QPointF pointOrigin = QPointF(chartBar.x() - m_nBarWidth, rectChart.bottom());
        QRectF rect(pointBar, pointOrigin);
        QBrush rectBrush = m_color;
        if (lposMouse.x() >= rect.x() - 2*m_nBarWidth && lposMouse.x() <= rect.x()) {
            m_mousedisplay.AddDot(QPointF(chartBar.x(), chartBar.y()), m_color);
            if(m_fEnableHighlightBar) {
                rectBrush = m_highlight;
            }
            if (m_fEnableHighlightOutline) {
                painter.setPen(penHighlight);
            }
        } else {
            painter.setPen(penBar);
        }
        if(m_fEnableOutline) {
            painter.drawRect(rect);
            if (m_fEnableFill) {
                painter.fillRect(rect, rectBrush);
            }
        } else if (m_fEnableFill) {
            penBar.setColor(m_color);
            painter.drawRect(rect);
            painter.fillRect(rect, rectBrush);
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

        //Draw a small tooltip looking item showing the point's data (x,y)
        MouseDot mouseDot = m_mousedisplay.GetDot(0);
        QPointF pointCircleCenter(mouseDot.Pos());
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
QRect BarChart::MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const
{
    QFontMetrics fm(painter.font());
    int nWidthText = fm.horizontalAdvance(strLabel) + 4;

    //Place the tooltip right above the bar its displayed on.
    QPoint pointTopLeft(pointCircleCenter.x() - nWidthText/2, pointCircleCenter.y() - 20);

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

void BarChart::SetBarColor(const QColor &color) {
    m_color = color;
    m_fChangesMade = true;
}

void BarChart::SetLineBrush(const QBrush &brush)
{
    m_brushLine = brush;
    m_fChangesMade = true;
}

void BarChart::SetBarHighlightColor(const QColor &color) {
    m_highlight = color;
    m_fChangesMade = true;
}

void BarChart::SetLineHighlightBrush(const QBrush &brush)
{
    m_brushLineHighlight = brush;
    m_fChangesMade = true;
}

void BarChart::SetBarWidth(int nWidth)
{
    m_nBarWidth = nWidth;
    m_fChangesMade = true;
}

void BarChart::SetBarWidth(int nWidth, int nMinWidth, int nMaxWidth)
{
    m_nBarWidth = nWidth;
    m_nBarMinWidth = nMinWidth;
    m_nBarMaxWidth = nMaxWidth;
    m_fChangesMade = true;
}

void BarChart::SetLineWidth(int nWidth)
{
    m_lineWidth = nWidth;
    m_fChangesMade = true;
}

void BarChart::EnableFill(bool fEnable)
{
    m_fEnableFill = fEnable;
}

void BarChart::EnableBorder(bool fEnable)
{
    m_fEnableOutline = fEnable;
}

void BarChart::EnableHighlight(bool fEnable)
{
    m_fEnableHighlightBar = fEnable;
}

void BarChart::EnableHighlightBorder(bool fEnable)
{
    m_fEnableHighlightOutline = fEnable;
}

std::vector<std::pair<QString, QColor>> BarChart::GetLegendData()
{
    std::vector<std::pair<QString, QColor>> vLegend;
    vLegend.emplace_back(std::make_pair("data", m_color));
    return vLegend;
}

void BarChart::wheelEvent(QWheelEvent *event)
{
    int dBarWidth = event->angleDelta().y()/120;
    if ((m_nBarWidth > m_nBarMinWidth && dBarWidth < 0)
            || (m_nBarWidth < m_nBarMaxWidth && dBarWidth > 0)) {
        m_nBarWidth += dBarWidth;
        m_fChangesMade = true;
        emit barWidthChanged(dBarWidth);
    }
}

}//namespace
