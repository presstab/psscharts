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

LineChart::LineChart(QWidget *parent) : QWidget (parent)
{
    setAutoFillBackground(true);
    m_settingsYLabels.labeltype = AxisLabelType::AX_NO_LABEL;
    m_settingsXLabels.labeltype = AxisLabelType::AX_NO_LABEL;
    m_settingsYLabels.nDimension = 30;
    m_settingsXLabels.nDimension = 30;
    m_settingsYLabels.nPrecision = 2;
    m_settingsYLabels.fEnabled = false;
    m_settingsXLabels.fEnabled = false;
    m_axisSections = 0;
    m_yPadding = 0;
    m_fEnableFill = true;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;
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
    if (m_settingsYLabels.labeltype == AxisLabelType::AX_NO_LABEL)
        return 0;
    return m_settingsYLabels.nDimension;
}

int LineChart::WidthRightMargin() const
{
    if (m_rightMargin != -1)
        return m_rightMargin;

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

void LineChart::ProcessChangedData()
{
    m_pairXRange = {0, 0};
    m_pairYRange = {0, 0};
    bool fFirstRun = true;
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
}

void LineChart::paintEvent(QPaintEvent *event)
{
    //Fill in the background first
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(m_brushBackground);
    painter.fillRect(rect(), m_brushBackground);

    //If there is only one data point, then return without drawing anything but the background
    if (m_mapPoints.size() <= 1) {
        return;
    }
    QVector<QPointF> qvecPolygon;
    QVector<QLineF> qvecLines;
    QRect rectFull = rect();
    QRect rectChart = ChartArea();
    qvecPolygon.append(rectChart.bottomLeft());
    bool fFirstRun = true;
    QPointF pointPrev;
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
    }

    // Cleanly close the polygon
    qvecPolygon.append(QPointF(rectChart.right(), rectChart.bottom()));

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

        painter.setFont(m_settingsYLabels.font);
        painter.setBrush(m_brushLabels);

        QFontMetrics fm(painter.font());

        //Draw the Y-axis labels
        if (m_settingsYLabels.fEnabled) {
            QRect rectYLabels = YLabelArea();
            for (int y : vYPoints) {
                QPointF pointDraw(rectYLabels.left(), y);
                auto pairPoints = ConvertFromPlotPoint(pointDraw);
                const double& nValue = pairPoints.second;

                QString strLabel = QString::fromStdString(PrecisionToString(nValue, m_settingsYLabels.nPrecision));

                int nWidthText = fm.horizontalAdvance(strLabel);

                QRect rectDraw;
                rectDraw.setTopLeft(pointDraw.toPoint());
                rectDraw.setBottomRight(QPoint(rectYLabels.right() - 3, rectDraw.y() + fm.height()));

                //Center the label onto the line
                rectDraw.moveTop(rectDraw.top() - fm.height() / 2);

                Qt::AlignmentFlag hAlign = (nWidthText > rectYLabels.width() ? Qt::AlignLeft : Qt::AlignRight);
                painter.drawText(rectDraw, hAlign, strLabel);
            }
        }

        //Draw the X-axis labels
        if (m_settingsXLabels.fEnabled) {
            QRect rectXLabels = XLabelArea();
            for (int x : vXPoints) {
                QPointF pointDraw(x, rectXLabels.top());
                auto pairPoints = ConvertFromPlotPoint(pointDraw);
                const uint32_t& nValue = pairPoints.first;

                QString strLabel;
                if (m_settingsXLabels.labeltype == AxisLabelType::AX_TIMESTAMP) {
                    QDateTime datetime;
                    datetime.setSecsSinceEpoch(nValue);
                    QDate date = datetime.date();
                    strLabel = QString("%1/%2/%3").arg(QString::number(date.month()))
                            .arg(QString::number(date.day()))
                            .arg(QString::number(date.year()));
                }

                QRect rectDraw;
                rectDraw.setTopLeft(pointDraw.toPoint());
                int nWidthText = fm.horizontalAdvance(strLabel);
                rectDraw.setBottomRight(QPoint(pointDraw.x() + nWidthText, rectXLabels.bottom()));

                //Center the label on the line
                rectDraw.moveLeft(rectDraw.left() - rectDraw.width() / 2);
                painter.drawText(rectDraw, Qt::AlignCenter, strLabel);
                painter.drawLine(QLine(QPoint(rectDraw.center().x(), rectDraw.top()),
                                       QPoint(rectDraw.center().x(), rectDraw.top() + 5)));
            }
        }
    }

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
        painter.drawText(rectTopTitle, Qt::AlignCenter, m_strTopTitle);
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

    m_fChangesMade = false;
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
    m_settingsYLabels.nPrecision = precision;
    m_settingsXLabels.nPrecision = precision;
}

void LineChart::SetRightMargin(int margin)
{
    m_rightMargin = margin;
}
