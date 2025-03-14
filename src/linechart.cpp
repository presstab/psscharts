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
#include <QMouseEvent>
#include <QResizeEvent>

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
    m_yPadding = 1;
    m_fEnableFill = true;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;
    m_nYSectionModulus = 0;
    m_fPlotPointsDirty = true;

    m_fDrawVolume = false;
    m_nBarWidth = 5;
    
    // Initialize mouse tracking variables
    m_lastMouseInChartArea = false;
    m_lastMousePos = QPoint(0, 0);

    setMouseTracking(true);
}


// Custom user data
void LineChart::AddProperty(const std::string &strKey, const std::string &strValue)
{
    m_mapProperties.emplace(strKey, strValue);
}

bool LineChart::GetProperty(const std::string &strKey, std::string &strValue)
{
    if (!m_mapProperties.count(strKey))
        return false;
    strValue = m_mapProperties.at(strKey);
    return true;
}

void LineChart::ClearProperties()
{
    m_mapProperties.clear();
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
 * @brief LineChart::ConvertToVolumePoint Convert to a data point to be used in volume bar chart at bottom of graph
 * @param pair
 * @return
 */
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
    m_vSeries.at(nSeries).data.emplace(x, y);
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
    m_vSeries.at(nSeries).data.erase(x);
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
        m_vSeries.at(nSeries).fShow = true;
    }
    m_vSeries.at(nSeries).data = mapPoints;
    ProcessChangedData();
}

void LineChart::RemoveSeries(const uint32_t& nSeries)
{
    //Check that series exists
    if (m_vSeries.size() < nSeries + 1) {
        return;
    }
    m_vSeries.erase(m_vSeries.begin()+nSeries);
}

void LineChart::ClearAll()
{
    m_vSeries.clear();
    m_vVolume.clear();
}

// Keeps data in chart series, but does not paint the line
void LineChart::HideSeries(const uint32_t& nSeries, bool fHide)
{
    //Check that series exists
    if (m_vSeries.size() < nSeries + 1) {
        return;
    }
    m_vSeries.at(nSeries).fShow = !fHide;
}

bool LineChart::SeriesHidden(const uint32_t &nSeries)
{
    if (m_vSeries.size() < nSeries + 1) {
        return false;
    }
    return !m_vSeries.at(nSeries).fShow;
}

void LineChart::SetSeriesLabel(const uint32_t &nSeries, const QString &strLabel)
{
    if (m_vSeries.size() < nSeries + 1) {
        return;
    }

    m_vSeries.at(nSeries).label = strLabel;
}

QString LineChart::SeriesLabel(const uint32_t &nSeries)
{
    if (m_vSeries.size() < nSeries + 1) {
        return "";
    }

    return m_vSeries.at(nSeries).label;
}

void LineChart::SetSeriesRawPrice(const uint32_t& nSeries, const double &price)
{
    if (m_vSeries.size() < nSeries + 1) {
        return;
    }

    m_vSeries.at(nSeries).priceRaw = price;
}

void LineChart::ClearSeriesLabels()
{
    for (unsigned int i = 0; i < m_vSeries.size(); i++) {
        m_vSeries.at(i).label.clear();
        m_vSeries.at(i).priceRaw = 0;
    }
}

/**
 * @brief LineChart::AddVolumePoint : Add a data point for volume for a specific series
 * @param nSeries: index of the series being altered
 * @param x: date of data
 * @param y: volume
 */
void LineChart::AddVolumePoint(const uint32_t& nSeries, const uint32_t& x, const double& y)
{
    if (m_vVolume.size() < nSeries+1) {
        //Series does not exist
        return;
    }
    m_vVolume.at(nSeries).data.emplace(x, y);
    ProcessChangedData();
}

/**
 * @brief LineChart::RemoveVolumePoint: Remove volume data for a specific series
 * @param nSeries: index of series being altered
 * @param x: date of data
 */
void LineChart::RemoveVolumePoint(const uint32_t& nSeries, const uint32_t &x)
{
    if (m_vVolume.size() < nSeries+1) {
        //Series does not exist
        return;
    }
    m_vVolume.at(nSeries).data.erase(x);
    ProcessChangedData();
}

/**
 * @brief LineChart::SetVolumePoints : Set the datapoints for the volume of a specific line series.
 * @param mapPoints
 * @param nSeries : The index of the series that is being changed or added
 */
void LineChart::SetVolumePoints(const std::map<uint32_t, double>& mapPoints, const uint32_t& nSeries)
{
    if (m_vVolume.size() < nSeries+1) {
        //Series does not exist yet
        m_vVolume.resize(nSeries+1);
    }
    m_vVolume.at(nSeries).data = mapPoints;
    ProcessChangedData();
}

void LineChart::ProcessChangedData()
{
    m_pairXRange = {0, 0};
    m_pairYRange = {0, 0};
    bool fFirstRun = true;
    for (const LineSeries& series : m_vSeries) {
        for (const auto& pair : series.data) {
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
    // Add y-axis buffer for the volume bars
    if(m_fDrawVolume) {
        double buffer = m_yPadding * (m_pairYRange.second - m_pairYRange.first) / 10;
        m_pairYRange.first -= buffer;
    }
    m_fChangesMade = true;
    m_fPlotPointsDirty = true; // Mark cached points as dirty when data changes
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

/**
 * @brief LineChart::UpdateCachedPoints Convert data points to screen coordinates and cache them
 */
void LineChart::UpdateCachedPoints()
{
    if (!m_fPlotPointsDirty) {
        return; // Points are already up to date
    }

    // Clear existing cached points
    m_cachedPlotPoints.clear();
    m_cachedVolumePoints.clear();
    
    // Resize containers to match number of series
    m_cachedPlotPoints.resize(m_vSeries.size());
    m_cachedVolumePoints.resize(m_vVolume.size());
    
    // Convert all series data points to screen coordinates
    for (size_t i = 0; i < m_vSeries.size(); i++) {
        const LineSeries& series = m_vSeries.at(i);
        QVector<QPointF>& plotPoints = m_cachedPlotPoints[i];
        
        // Reserve space to avoid reallocations
        plotPoints.reserve(series.data.size());
        
        // Convert each data point to screen coordinates
        for (const auto& pair : series.data) {
            plotPoints.append(ConvertToPlotPoint(pair));
        }
    }
    
    // Convert volume data points
    for (size_t i = 0; i < m_vVolume.size(); i++) {
        const LineSeries& series = m_vVolume.at(i);
        QVector<QPointF>& volumePoints = m_cachedVolumePoints[i];
        
        // Reserve space to avoid reallocations
        volumePoints.reserve(series.data.size());
        
        // Convert each volume data point to screen coordinates
        for (const auto& pair : series.data) {
            volumePoints.append(ConvertToVolumePoint(pair));
        }
    }
    
    m_fPlotPointsDirty = false; // Mark points as up to date
}

void LineChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    //Fill in the background first
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(m_brushBackground);
    painter.fillRect(rect(), m_brushBackground);

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
                strLabel = TimeStampToString(MaxX(), m_settingsXLabels.timeOffset);
            else
                strLabel = PrecisionToString(MaxX(), m_settingsXLabels.Precision());

            QFontMetrics fm(m_settingsXLabels.font);
            m_settingsXLabels.sizeDynamicDimension.setHeight(fm.height()+3);
            m_settingsXLabels.sizeDynamicDimension.setWidth(fm.horizontalAdvance(strLabel)+3);
        }
    }

    QRect rectFull = rect();
    QRect rectChart = ChartArea();

    // Update the cached points if necessary (data changed or resize occurred)
    if (m_fPlotPointsDirty || m_fChangesMade) {
        UpdateCachedPoints();
    }

    // Determine if mouse location is inside of the chart, but only if mouse display is enabled
    QPoint lposMouse;
    bool fMouseInChartArea = false;
    
    if (m_mousedisplay.IsEnabled()) {
        QPoint gposMouse = QCursor::pos();
        QPoint gposChart = mapToGlobal(QPoint(0+WidthYTitleArea()+WidthYLabelArea(),0+HeightTopTitleArea()));
        lposMouse = this->mapFromGlobal(gposMouse);
        
        // Check if the mouse is in the chart area
        if (gposMouse.y() >= gposChart.y() && 
            gposMouse.y() <= rectChart.height() + gposChart.y() &&
            gposMouse.x() >= gposChart.x() && 
            gposMouse.x() <= gposChart.x() + rectChart.width()) {
            fMouseInChartArea = true;
        }
        
        // Update the last mouse position state
        m_lastMousePos = lposMouse;
        m_lastMouseInChartArea = fMouseInChartArea;
    }

    //Clear any existing mouse dots and save current state before clearing
    m_lastMouseDots = m_mousedisplay.GetDots();
    m_mousedisplay.ClearDots();

    //Draw a horizontal line at Y=0 to show gain/loss
    if (m_fDrawZero) {
        painter.save();
        QPointF pointZeroY = ConvertToPlotPoint(std::make_pair(MinX(), 0));
        QPointF pointLeft(rectChart.left(), pointZeroY.y());
        QPointF pointRight(rectChart.right(), pointZeroY.y());
        QPen penZero;
        penZero.setWidthF(2.5);
        penZero.setColor(Qt::black);

        painter.setPen(penZero);
        painter.drawLine(pointLeft, pointRight);
        painter.restore();
    }

    //Draw each series
    for (unsigned int i = 0; i < m_vSeries.size(); i++) {
        const LineSeries& series = m_vSeries.at(i);
        if (!series.fShow || series.data.empty())
            continue;

        // Use cached plot points instead of converting during paint
        QVector<QPointF> qvecPolygon;
        QVector<QLineF> qvecLines;
        
        // Ensure we have cached data for this series
        if (i < m_cachedPlotPoints.size()) {
            const QVector<QPointF>& cachedPoints = m_cachedPlotPoints[i];
            
            // Create polygon starting with bottom-left corner
            qvecPolygon.append(rectChart.bottomLeft());
            
            // Add all cached points to the polygon
            qvecPolygon.append(cachedPoints);
            
            // Add lines between points
            if (cachedPoints.size() > 1) {
                for (int j = 1; j < cachedPoints.size(); j++) {
                    qvecLines.append(QLineF(cachedPoints[j-1], cachedPoints[j]));
                }
            }
            
            // Handle mouse interactions with lines
            if (fMouseInChartArea) {
                bool fMouseSet = false;
                for (const QLineF& line : qvecLines) {
                    if (!fMouseSet && lposMouse.x() >= line.x1() && lposMouse.x() <= line.x2()) {
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
            
            // Get the last data point for labels
            QPointF pointLast;
            double dataLast = 0;
            
            if (!series.data.empty()) {
                auto lastDataIter = series.data.rbegin();
                dataLast = lastDataIter->second;
                
                if (!cachedPoints.empty()) {
                    pointLast = cachedPoints.last();
                }
            }
            
            // Show a label of where the line ends
            QRect rectDraw;
            rectDraw.setTopLeft(pointLast.toPoint());
            rectDraw.setBottomRight(QPoint(pointLast.x()+50, pointLast.y()+10));

            //Center the label on the line
            rectDraw.moveBottom(rectDraw.bottom() - rectDraw.height()/2);

            QPainterPath path;
            path.addRoundedRect(rectDraw, 5, 5);
            painter.fillPath(path, GetSeriesColor(i));

            const auto& fontBefore = painter.font();
            painter.setFont(m_settingsYLabels.font);

            //If there is a label for the series, add it too
            QString strText = QString::number(dataLast, 'f', m_settingsYLabels.Precision());
            if (series.label != "")
                strText = series.label;

            painter.drawText(rectDraw, Qt::AlignCenter, strText);

            //Draw a percent change box if enabled
            if (m_fDrawZero) {
                QRect rectPercent = rectDraw;
                rectPercent.moveLeft(rectDraw.right());
                double percentChange = dataLast * 100;
                strText = QString::number(percentChange, 'f', 3) + QString("%");
                if (m_settingsYLabels.fPriceDisplay) {
                    strText = PrecisionToString(series.priceRaw, PrecisionHint(series.priceRaw));
                }

                QPainterPath path_percent;
                path_percent.addRoundedRect(rectPercent, 5, 5);
                painter.fillPath(path_percent, GetSeriesColor(i));
                painter.drawText(rectPercent, Qt::AlignCenter, strText);
            }
            painter.setFont(fontBefore);

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
    }

    //Draw Volume Bars
    if (m_fDrawVolume) {
        for (unsigned int i = 0; i < m_vVolume.size(); i++) {
            QPen penBar;
            penBar.setBrush(GetSeriesColor(i));
            
            // Use cached volume points if available
            if (i < m_cachedVolumePoints.size()) {
                const QVector<QPointF>& cachedVolumePoints = m_cachedVolumePoints[i];
                
                for (const QPointF& chartBar : cachedVolumePoints) {
                    QPointF pointBar = QPointF(chartBar.x() + i*m_nBarWidth + m_nBarWidth, chartBar.y());
                    QPointF pointOrigin = QPointF(chartBar.x() + i*m_nBarWidth + 1, rectChart.bottom());
                    QRectF rect(pointBar, pointOrigin);
                    QBrush rectBrush = GetSeriesColor(i);
                    painter.setPen(penBar);
                    painter.drawRect(rect);
                    if (m_fEnableFill) {
                        painter.fillRect(rect, rectBrush);
                    }
                }
            }
        }
    }
    painter.save();
    painter.restore();

    //Draw axis sections next so that they get covered up by chart fill
    if (m_axisSections > 0) {
        painter.save();
        m_penAxisSeparater.setWidthF(0.3);
        painter.setPen(m_penAxisSeparater);
        int nSpacingY = rectChart.height() / m_axisSections;
        int nSpacingX = rectChart.width() / m_axisSections;
        std::vector<int> vYPoints;
        std::vector<int> vXPoints;
        for (uint32_t i = 0; i <= m_axisSections; i++) {
            int y = rectChart.top() + i*nSpacingY;
            //todo: finish
            //modulus mode only places lines when mod is 0  (ie multiple of 5, 10, 100, etc)
            uint32_t y_value = ConvertFromPlotPoint(QPointF(0, y)).second;
            if (m_nYSectionModulus > 0) {
                uint32_t remain = y_value % m_nYSectionModulus;
                if (remain) {
                    uint32_t y_line = y_value - remain;
                    auto y_new = ConvertToPlotPoint(std::make_pair(0, double(y_line))).y();
                    auto check = ConvertFromPlotPoint((QPointF(0, y_new))).second;
                    qDebug() << "new point at value: " << QString::number(y_line) << " check value: " << QString::number(check);
                    y = y_new;
                    if (y_line > 100000) {
                        qDebug() << "oiut of range";
                    }
                }
            }
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
    m_fPlotPointsDirty = true;
}

void LineChart::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // Mark cached points as dirty when the widget is resized
    m_fPlotPointsDirty = true;
    
    // Reset mouse tracking on resize
    m_lastMouseInChartArea = false;
    m_lastMouseDots.clear();
    
    Chart::resizeEvent(event);
}

void LineChart::mouseMoveEvent(QMouseEvent *event)
{
    // Only process mouse movement when mouse display is enabled
    if (!m_mousedisplay.IsEnabled()) {
        Chart::mouseMoveEvent(event);
        return;
    }
    
    QRect rectChart = ChartArea();
    QPoint currentPos = event->pos();
    bool currentInChartArea = rectChart.contains(currentPos);
    
    // If nothing has changed in mouse state, don't bother repainting
    if (currentPos == m_lastMousePos && currentInChartArea == m_lastMouseInChartArea) {
        Chart::mouseMoveEvent(event);
        return;
    }
    
    // Calculate regions that need to be repainted
    QRegion updateRegion;
    
    // Create the invalidation region for the previous mouse position
    if (m_lastMouseInChartArea) {
        // Invalidate crosshair lines
        updateRegion += QRect(rectChart.left(), m_lastMousePos.y() - 1, 
                             rectChart.width(), 3); // Horizontal line
        updateRegion += QRect(m_lastMousePos.x() - 1, rectChart.top(), 
                             3, rectChart.height()); // Vertical line
        
        // Invalidate all previous mouse dots and tooltips
        for (const MouseDot& dot : m_lastMouseDots) {
            QPointF dotPos = dot.Pos();
            // Area for the dot (assuming radius 5)
            updateRegion += QRect(dotPos.x() - 6, dotPos.y() - 6, 12, 12);
            
            // Area for the tooltip - this is a conservative estimate
            // Create text to determine size
            auto pairData = ConvertFromPlotPoint(dotPos);
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
            
            QFontMetrics fm(font());
            int tooltipWidth = fm.horizontalAdvance(strLabel) + 10;
            int tooltipHeight = fm.height() + 8;
            
            // Tooltip typically appears below the dot
            updateRegion += QRect(dotPos.x() - tooltipWidth/2, dotPos.y() + 5, 
                                 tooltipWidth, tooltipHeight);
        }
        
        // Also update Y axis label area if mouse display changes the labels
        if (m_settingsYLabels.fEnabled) {
            QRect yLabelArea = YLabelArea();
            updateRegion += QRect(yLabelArea.left(), m_lastMousePos.y() - 10, 
                                 yLabelArea.width(), 20);
        }
        
        // Update X axis label area if mouse display changes the labels
        if (m_settingsXLabels.fEnabled) {
            QRect xLabelArea = XLabelArea();
            updateRegion += QRect(m_lastMousePos.x() - 50, xLabelArea.top(), 
                                 100, xLabelArea.height());
        }
    }
    
    // Add regions for current mouse position
    if (currentInChartArea) {
        // Crosshair lines
        updateRegion += QRect(rectChart.left(), currentPos.y() - 1, 
                             rectChart.width(), 3); // Horizontal line
        updateRegion += QRect(currentPos.x() - 1, rectChart.top(), 
                             3, rectChart.height()); // Vertical line
        
        // Conservative estimate for potential tooltips/dots
        // Add a bit more area around current position
        updateRegion += QRect(currentPos.x() - 75, currentPos.y() - 20, 150, 40);
        
        // Y-axis label area for current mouse position
        if (m_settingsYLabels.fEnabled) {
            QRect yLabelArea = YLabelArea();
            updateRegion += QRect(yLabelArea.left(), currentPos.y() - 10, 
                                 yLabelArea.width(), 20);
        }
        
        // X-axis label area for current mouse position
        if (m_settingsXLabels.fEnabled) {
            QRect xLabelArea = XLabelArea();
            updateRegion += QRect(currentPos.x() - 50, xLabelArea.top(), 
                                 100, xLabelArea.height());
        }
    }
    
    // Store current state for next time
    m_lastMousePos = currentPos;
    m_lastMouseInChartArea = currentInChartArea;
    m_lastMouseDots = m_mousedisplay.GetDots();
    
    // Update only the required regions
    if (!updateRegion.isEmpty()) {
        update(updateRegion);
    }
    
    Chart::mouseMoveEvent(event);
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
    m_fPlotPointsDirty = true;
}

std::vector<std::pair<QString, QColor>> LineChart::GetLegendData()
{
    std::vector<std::pair<QString, QColor>> vLegend;
    for(int i = 0; i < static_cast<int>(m_vSeries.size()); i++) {
        vLegend.emplace_back(QString::number(i), GetSeriesColor(i));
    }
    return vLegend;
}
}//namespace
