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

#include "psschart.h"
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

Chart::Chart(QWidget *parent) : QWidget (parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_chartType = ChartType::LINE;
    m_axisSections = 0;
    m_yPadding = 0;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;

    setMouseTracking(true);
}

Chart::Chart(ChartType type, QWidget* parent) : QWidget (parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_chartType = type;
    m_axisSections = 0;
    m_yPadding = 0;
    m_fChangesMade = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;

    setMouseTracking(true);
}

std::string convertToUpper(std::string const &strInput)
{
    std::string strOutput = "";
    std::locale locale;
    for (std::string::size_type i = 0; i < strInput.length(); ++i)
        strOutput += std::toupper(strInput[i], locale);

    return strOutput;
}

std::string Chart::ChartTypeToString(const ChartType type)
{
    switch (type) {
        case ChartType::LINE:
            return "LINE";
        case ChartType::CANDLESTICK:
            return "CANDLESTICK";
        default:
            return "ERROR";
    }
}
ChartType Chart::ChartTypeFromString(std::string strType)
{
    strType = convertToUpper(strType);
    if(strType == "LINE")
        return ChartType::LINE;
    else if(strType == "CANDLESTICK")
        return ChartType::CANDLESTICK;
    else
        return ChartType::ERROR;
}

int Chart::HeightTopTitleArea() const
{
    if (m_topTitleHeight != -1)
        return m_topTitleHeight;

    if (m_strTopTitle.isEmpty())
        return 0;

    return 30;
}

void Chart::SetTopTitleHeight(int height)
{
    m_topTitleHeight = height;
}

int Chart::HeightXLabelArea() const
{
    if (m_settingsXLabels.fDynamicSizing && m_settingsXLabels.sizeDynamicDimension.height() > 0)
        return m_settingsXLabels.sizeDynamicDimension.height();

    if (m_settingsXLabels.nDimension != -1)
        return m_settingsXLabels.nDimension;

    if (m_settingsXLabels.labeltype == AxisLabelType::AX_NO_LABEL)
        return 0;
    return 30; //fallback
}

void Chart::SetXLabelHeight(int height)
{
    m_settingsXLabels.nDimension = height;
}

int Chart::WidthYTitleArea() const
{
    if (m_strTitleY.isEmpty())
        return 0;
    return 30;
}

int Chart::WidthYLabelArea() const
{
    if (m_settingsYLabels.fDynamicSizing && m_settingsYLabels.sizeDynamicDimension.width() > 0)
        return  m_settingsYLabels.sizeDynamicDimension.width();

    if (m_settingsYLabels.labeltype == AxisLabelType::AX_NO_LABEL)
        return 0;

    return m_settingsYLabels.nDimension;
}

int Chart::WidthRightMargin() const
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

void Chart::EnableMouseDisplay(bool fEnable)
{
    m_mousedisplay.SetEnabled(fEnable);
}

void Chart::DrawXLabels(QPainter& painter, const std::vector<int>& vXPoints, bool fDrawIndicatorLine)
{
    QFontMetrics fm(painter.font());

    QRect rectXLabels = XLabelArea();
    for (int x : vXPoints) {
        QPointF pointDraw(x, rectXLabels.top());
        std::pair<uint32_t, double> pairPoints = ConvertFromPlotPoint(pointDraw);
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

void Chart::DrawYLabels(QPainter &painter, const std::vector<int> &vYPoints, bool isMouseDisplay)
{
    QFontMetrics fm(painter.font());

    QRect rectYLabels = YLabelArea();
    for (int y : vYPoints) {
        QPointF pointDraw(rectYLabels.left(), y);
        std::pair<uint32_t, double> pairPoints = ConvertFromPlotPoint(pointDraw);
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

QPixmap Chart::grab(const QRect &rectangle)
{
    auto rectValid = rectangle.height() != -1;
    bool fRectSizeMatch = (rectValid ? rectangle.size() == m_pixmapCache.size() : true);
    bool fWidgetSizeMatch = this->size() == m_pixmapCache.size();
    if (!m_fChangesMade && fRectSizeMatch && fWidgetSizeMatch)
        return m_pixmapCache;

    m_pixmapCache = QWidget::grab(rectangle);
    return m_pixmapCache;
}

void Chart::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    repaint();
}

void Chart::SetBackgroundBrush(const QBrush &brush)
{
    m_brushBackground = brush;
    QPalette palette = this->palette();
    palette.setBrush(QPalette::ColorRole::Base, brush);
    m_fChangesMade = true;
}

void Chart::SetTopTitle(const QString &strTitle)
{
    m_strTopTitle = strTitle;
    m_fChangesMade = true;
}

void Chart::SetTopTitleFont(const QFont &font)
{
    m_fontTopTitle = font;
    m_fChangesMade = true;
}

void Chart::SetXLabelType(AxisLabelType labelType)
{
    m_settingsXLabels.labeltype = labelType;
    m_fChangesMade = true;
}

void Chart::SetYLabelType(AxisLabelType labelType)
{
    m_settingsYLabels.labeltype = labelType;
    m_fChangesMade = true;
}

void Chart::SetYLabelWidth(int width)
{
    m_settingsYLabels.nDimension = width;
}

void Chart::SetYTitle(const QString &strTitle)
{
    m_strTitleY = strTitle;
    m_fChangesMade = true;
}

void Chart::SetYTitleFont(const QFont &font)
{
    m_fontYTitle = font;
    m_fChangesMade = true;
}

void Chart::SetAxisOnOff(bool fDrawX, bool fDrawY)
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
void Chart::SetAxisLabelsOnOff(bool fDrawXLabels, bool fDrawYLabels)
{
    m_settingsXLabels.fEnabled = fDrawXLabels;
    m_settingsYLabels.fEnabled = fDrawYLabels;
}

/**
 * @brief PssChart::SetAxisSectionCount: Set the number of vertical dividers that are displayed.
 * @param nCount
 */
void Chart::SetAxisSectionCount(uint32_t nCount)
{
    m_axisSections = nCount;
    m_fChangesMade = true;
}

/**
 * @brief PssChart::SetAxisSeparatorPen: The pen that is used to draw divider lines for the axis sections.
 * @param pen
 */
void Chart::SetAxisSeparatorPen(const QPen &pen)
{
    m_penAxisSeparater = pen;
    m_fChangesMade = true;
}

void Chart::SetChartType(const ChartType &type)
{
    m_chartType = type;
    m_fChangesMade = true;
    ProcessChangedData();
}

uint32_t Chart::Version()
{
    uint32_t nVersion = 0;
    nVersion |= VERSION_BUILD;
    nVersion |= (VERSION_REVISION << 8);
    nVersion |= (VERSION_MINOR << 16);
    nVersion |= (VERSION_MAJOR << 24);
    return nVersion;
}

QString Chart::VersionString()
{
    return QString("v%1.%2.%3.%4")
            .arg(QString::number(VERSION_MAJOR))
            .arg(QString::number(VERSION_MINOR))
            .arg(QString::number(VERSION_REVISION))
            .arg(QString::number(VERSION_BUILD));
}

/**
 * @brief PssChart::ChartArea : Get the area of the widget that is dedicated to the chart itself
 * @return
 */
QRect Chart::ChartArea() const
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
 * @brief PssChart::YLabelArea The area that is reserved for Y-axis labels. This is between the Y title
 * area and the left side of the ChartArea.
 * @return
 */
QRect Chart::YLabelArea() const
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
 * @brief PssChart::XLabelArea The area that is reserved for X-axis labels. This is between the bottom
 * of the PssChart QWidget and the bottom of the ChartArea.
 * @return
 */
QRect Chart::XLabelArea() const
{
    QRect rectXLabels;
    QRect rectFull = this->rect();
    QRect rectChart = ChartArea();
    rectXLabels.setTopLeft(rectChart.bottomLeft());
    rectXLabels.setBottomRight(QPoint(rectChart.right(), rectFull.bottom()));
    return rectXLabels;
}

const double &Chart::MaxX() const
{
    return m_pairXRange.second;
}

const double &Chart::MaxY() const
{
    return m_pairYRange.second;
}

const double &Chart::MinX() const
{
    return m_pairXRange.first;
}

const double &Chart::MinY() const
{
    return m_pairYRange.first;
}

QBrush Chart::BackgroundBrush() const
{
    return m_brushBackground;
}

void Chart::SetYLabelFont(const QFont& font)
{
    m_settingsYLabels.font = font;
}

void Chart::SetYPadding(int nPadding)
{
    m_yPadding = nPadding;
}

void Chart::SetAxisLabelsBrush(const QBrush& brush)
{
    m_brushLabels = brush;
}

/**
 * Set the precision that the axis labels will print their numbers at.
 * @param precision
 */
void Chart::SetLabelPrecision(int precision)
{
    m_settingsYLabels.SetPrecision(precision, /*disableAuto*/true);
    m_settingsXLabels.SetPrecision(precision, /*disableAuto*/true);
}

void Chart::SetLabelAutoPrecision(bool fEnable)
{
    m_settingsYLabels.SetAutoPrecision(fEnable);
    m_settingsXLabels.SetAutoPrecision(fEnable);
}

void Chart::SetRightMargin(int margin)
{
    m_rightMargin = margin;
}

}//namespace
