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

#include "piechart.h"
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

PieChart::PieChart(QWidget *parent) : Chart(ChartType::PIE, parent)
{
    setAutoFillBackground(true);
    m_settingsXLabels.SetNull();
    m_settingsYLabels.SetNull();

    m_labelType = PieLabelType::PIE_LABEL;
    m_size = 250;
    m_nDountSize = 100;
    m_nStartingAngle = 16*90; // 12 o'clock position
    m_axisSections = 0;
    m_yLabelPadding = 0;
    m_fEnableFill = true;
    m_fChangesMade = true;
    m_fDountHole = true;
    m_rightMargin = -1;
    m_topTitleHeight = -1;
    m_precision = 100000000;

    setMouseTracking(true);
}

std::string PieChart::strToUpper(std::string const &strInput)
{
    std::string strOutput = "";
    std::locale locale;
    for (std::string::size_type i = 0; i < strInput.length(); ++i)
        strOutput += std::toupper(strInput[i], locale);

    return strOutput;
}

std::string PieChart::LabelTypeToString(const PieLabelType type)
{
    switch (type) {
        case PieLabelType::PIE_LABEL:
            return "LINE";
        case PieLabelType::PIE_LABEL_PERCENT:
            return "LABEL AND PERCENTAGE";
        case PieLabelType::PIE_LABEL_VALUE:
            return "LABEL AND VALUE";
        case PieLabelType::PIE_PERCENT:
            return "PERCENTAGE";
        case PieLabelType::PIE_VALUE:
            return "VALUE";
        default:
            return "NONE";
    }
}

PieLabelType PieChart::LabelTypeFromString(std::string strType)
{
    strType = strToUpper(strType);
    if (strType == "LABEL")
        return PieLabelType::PIE_LABEL;
    else if (strType == "LABEL AND PERCENTAGE")
        return PieLabelType::PIE_LABEL_PERCENT;
    else if(strType == "LABEL AND VALUE")
        return PieLabelType::PIE_LABEL_VALUE;
    else if (strType == "PERCENTAGE")
        return PieLabelType::PIE_PERCENT;
    else if (strType == "VALUE")
        return PieLabelType::PIE_VALUE;
    else
        return PieLabelType::PIE_NO_LABEL;
}

/**
 * @brief PieChart::ConvertFromPlotPoint: convert a point on the chart to the value it represents
 * @return
 */
std::pair<uint32_t, double> PieChart::ConvertFromPlotPoint(const QPointF& point)
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

void PieChart::AddDataPoint(const std::string& label, const double& value)
{
    m_mapPoints.emplace(label, value);
    ProcessChangedData();
}

void PieChart::RemoveDataPoint(const std::string &label)
{
    m_mapPoints.erase(label);
    ProcessChangedData();
}

void PieChart::SetDataPoints(const std::map<std::string, double>& mapPoints)
{
    m_mapPoints = mapPoints;
    ProcessChangedData();
}

void PieChart::ProcessChangedData()
{
    m_mapData.clear();
    m_nTotal = 0;
    for (auto pair: m_mapPoints) {
        m_nTotal += pair.second;
        m_mapData.emplace(std::pair<double, std::string>(pair.second, pair.first));
    }
    m_nRatio = 5760 / m_nTotal;
    m_fChangesMade = true;
}

void PieChart::paintEvent(QPaintEvent *event)
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

    // Draw Pie Chart
    QPen penLine;
    penLine.setBrush(m_brushLine);
    penLine.setWidth(m_lineWidth);
    painter.setPen(penLine);
    QRect rectPie;
    QPoint pointCenter = rectChart.center();
    rectPie.setBottom(pointCenter.y() - m_size);
    rectPie.setTop(pointCenter.y() + m_size);
    rectPie.setRight(pointCenter.x() - m_size);
    rectPie.setLeft(pointCenter.x() + m_size);
    std::srand(42);
    double nFilled = 0;
    for(auto pair: m_mapData) {
        if (m_fEnableFill) {
            painter.setBrush(QColor(std::rand()%256, std::rand()%256, std::rand()%256));
        }
        if(m_fEnableOutline) {
            penLine.setColor(painter.brush().color());
        }
        double nSliceAngle = m_nStartingAngle + nFilled;
        double nSliceSpan = pair.first * m_nRatio;
        painter.drawPie(rectPie, nSliceAngle, nSliceSpan);
        nFilled += nSliceSpan;
        double nSliceMidPoint = (90 + ((nSliceAngle + nSliceSpan/2) / 16)) * pi / 180;

        QRect rectText;
        QPoint pointText(pointCenter.x() + (m_xLabelPadding * m_size * std::sin(nSliceMidPoint)), pointCenter.y() + (m_yLabelPadding * m_size * std::cos(nSliceMidPoint)));
        rectText.setHeight(1000);
        rectText.setWidth(1000);
        rectText.moveCenter(pointText);
        QString strLabel;
        switch (m_labelType) {
            case PieLabelType::PIE_LABEL: {
                strLabel = QString::fromStdString(pair.second);
                break;
            }
            case PieLabelType::PIE_VALUE: {
                strLabel = PrecisionToString(pair.first, m_settingsYLabels.Precision());
                break;
            }
            case PieLabelType::PIE_LABEL_VALUE: {
                strLabel = QString::fromStdString(pair.second) + " (" + PrecisionToString(pair.first, m_settingsYLabels.Precision()) + ")";
                break;
            }
            case PieLabelType::PIE_PERCENT: {
                strLabel = PrecisionToString(pair.first/m_nTotal*100, m_settingsYLabels.Precision()) + "%";
                break;
            }
            case PieLabelType::PIE_LABEL_PERCENT: {
                strLabel = QString::fromStdString(pair.second) + " (" + PrecisionToString(pair.first/m_nTotal*100, m_settingsYLabels.Precision()) + "%)";
                break;
            }
            default:
                strLabel = "";
                break;
        }
        painter.drawText(rectText, Qt::AlignCenter, strLabel);
    }

    if(m_fDountHole) {
        painter.setBrush(m_brushBackground);
        painter.drawEllipse(pointCenter, m_nDountSize, m_nDountSize);
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

/**
 * @brief PieChart::MouseOverTooltipRect Get the boundaries of the tooltip that is drawn for the mouseover data.
 * @param painter: The chart widget's painter.
 * @param rectFull: The QRect of the entire drawing area of the chart widget.
 * @param pointCircleCenter: the center of the dot that is being drawn for the mouseover.
 * @param strLabel: the label text that is being placed in the tooltip.
 * @return QRect with the coordinates that the tooltip should be drawn in.
 */
QRect PieChart::MouseOverTooltipRect(const QPainter& painter, const QRect& rectFull, const QPointF& pointCircleCenter, const QString& strLabel) const
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

void PieChart::SetFillBrush(const QBrush &brush)
{
    m_brushFill = brush;
    m_fChangesMade = true;
}

void PieChart::SetLineBrush(const QBrush &brush)
{
    m_brushLine = brush;
    m_fChangesMade = true;
}

void PieChart::SetLineWidth(int nWidth)
{
    m_lineWidth = nWidth;
    m_fChangesMade = true;
}

/**
 * Set the linechart to fill in the area between the line and the bottom of the chart.
 * Default is enabled.
 * @param fEnable
 */
void PieChart::EnableFill(bool fEnable)
{
    m_fEnableFill = fEnable;
}

void PieChart::SetChartSize(int nSize)
{
    m_size = nSize;
    m_fChangesMade = true;
}

void PieChart::SetStartingAngle(int nAngle)
{
    // Full Circle is 16*360 = 5760
    // Modulo then set to angle for use in code
    m_nStartingAngle = 16*(nAngle % 360);
    m_fChangesMade = true;
}

void PieChart::SetDonutSize(int nSize)
{
    if(nSize > m_size) {
        m_nDountSize = m_size;
    } else {
        m_nDountSize = nSize;
    }
    m_fChangesMade = true;
}

void PieChart::EnableDonut(bool fEnable)
{
    m_fDountHole = fEnable;
}

void PieChart::EnableOutline(bool fEnable)
{
    m_fDountHole = fEnable;
}

void PieChart::SetLabelType(std::string nType)
{
    m_labelType = LabelTypeFromString(nType);
}

void PieChart::SetLabelType(PieLabelType nType)
{
    m_labelType = nType;
}

void PieChart::SetXLabelPadding(double nPadding)
{
    m_xLabelPadding = nPadding;
}

void PieChart::SetYLabelPadding(double nPadding)
{
    m_yLabelPadding = nPadding;
}
/**
 * @brief PssChart::ChartArea : Get the area of the widget that is dedicated to the chart itself
 * @return
 */
QRect PieChart::ChartArea() const
{
    QRect rectFull = this->rect();
    QRect rectChart = rectFull;
    rectChart.setTop(rectFull.top() + HeightTopTitleArea());
    rectChart.setBottom(rectFull.bottom());
    rectChart.setLeft(rectFull.left());
    rectChart.setRight(rectFull.right());
    return rectChart;
}

}//namespace
