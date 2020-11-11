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
#include <QDebug>

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
    QPoint pointCenter = rectChart.center();
    QPoint pointCircle(static_cast<int>(point.x() - pointCenter.x()), static_cast<int>(pointCenter.y() - point.y()));

    // Angle
    uint32_t pointAngle = static_cast<uint32_t>(std::atan2(pointCircle.y(), pointCircle.x()) * 180 / pi * 16);
    pairValues.first = pointAngle;

    // Distance
    double pointDistance = static_cast<double>(std::sqrt(pointCircle.x() * pointCircle.x() + pointCircle.y() * pointCircle.y()));
    pairValues.second = pointDistance;

    return pairValues;
}

QPointF PieChart::ConvertToPlotPoint(const std::pair<uint32_t, double>& pair)
{
    QRect rectChart = ChartArea();
    QPoint pointCenter = rectChart.center();
    return QPoint(static_cast<int>(pointCenter.x() + (pair.second * std::sin(pair.first))), static_cast<int>(pointCenter.y() + (pair.second * std::cos(pair.second))));
}

void PieChart::AddDataPoint(const std::string& label, const double& value)
{
    m_mapPoints.emplace(label, value);
    m_mapColors.emplace(label, QColor(std::rand()%256, std::rand()%256, std::rand()%256));
    ProcessChangedData();
}

void PieChart::RemoveDataPoint(const std::string &label)
{
    m_mapPoints.erase(label);
    m_mapColors.erase(label);
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
    double nFilled = 0;
    int nHighlightStartAngle = 0;
    int nHighlightSpan = 0;
    bool fDrawHighlight = false;
    for(auto pair: m_mapData) {
        // Draw Pie Slice
        painter.setPen(penLine);
        std::pair<uint32_t, double> mouseData = ConvertFromPlotPoint(lposMouse);
        int mouseAngle = static_cast<int>(mouseData.first);
        mouseAngle = (mouseAngle + 5760) % 5760; // prevents negative angles in calculation
        int nSliceStartingAngle = static_cast<int>(m_nStartingAngle + nFilled + 5760) % 5760; // angle for start of slice
        int nSliceSpan = static_cast<int>(pair.first * m_nRatio + 5760) % 5760; // angle from start of slice to end of slice
        int nSliceEndingAngle = static_cast<int>(nSliceStartingAngle + nSliceSpan + 5760) % 5760; // angle for end of slice
        // Check if end angle is set properly after modulo
        if (nSliceStartingAngle > nSliceEndingAngle) {
            nSliceEndingAngle += 5760;
        }
        int mouseDistance = static_cast<int>(mouseData.second);
        bool fHighlightText = false;
        if (m_fEnableFill) {
            painter.setBrush(m_mapColors[pair.second]);
        }
        // Highlight data
        if (mouseAngle > nSliceStartingAngle && mouseAngle <= nSliceEndingAngle && mouseDistance < m_size) {
            fDrawHighlight = true;
            fHighlightText = true;
            nHighlightStartAngle = nSliceStartingAngle;
            nHighlightSpan = nSliceSpan;
        }
        painter.drawPie(rectPie, nSliceStartingAngle, nSliceSpan);
        nFilled += nSliceSpan;

        // Draw Pie Label
        QRect rectText;
        double nSliceMidPoint = (90 + ((nSliceStartingAngle + nSliceSpan/2) / 16)) * pi / 180;
        QPoint pointText(pointCenter.x() + (m_xLabelPadding * m_size * std::sin(nSliceMidPoint)), pointCenter.y() + (m_yLabelPadding * m_size * std::cos(nSliceMidPoint)));
        rectText.setHeight(1000);
        rectText.setWidth(1000);
        rectText.moveCenter(pointText);
        QString strLabel;
        painter.setPen(m_colorYTitle);
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
        if (fHighlightText) {
            QFont font = painter.font();
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(rectText, Qt::AlignCenter, strLabel);
        } else {
            QFont font = painter.font();
            font.setBold(false);
            painter.setFont(font);
            painter.drawText(rectText, Qt::AlignCenter, strLabel);
        }
    }

    // Draw Highlight
    if (fDrawHighlight && m_fEnableHighlight) {
        if(m_fEnableHighlightOutline) {
            penLine.setColor(m_colorHighlightOutline);
        }
        painter.setPen(penLine);
        painter.setBrush(m_colorHighlight);
        painter.drawPie(rectPie, nHighlightStartAngle, nHighlightSpan);
    }

    // Draw Donut Hole
    if(m_fDountHole) {
        QRect rectHole;
        QPoint pointCenter = rectChart.center();
        rectHole.setBottom(pointCenter.y() - m_nDountSize);
        rectHole.setTop(pointCenter.y() + m_nDountSize);
        rectHole.setRight(pointCenter.x() - m_nDountSize);
        rectHole.setLeft(pointCenter.x() + m_nDountSize);

        // Draw Highlight
        if(m_fEnableHighlightOutline && m_fEnableHighlight) {
            // Draw small circle to get covered by outline
            penLine.setColor(m_brushBackground.color());
            penLine.setWidth(1);
            painter.setPen(penLine);
            painter.setBrush(m_brushBackground);
            painter.drawEllipse(rectHole);
            // Draw arcs to correspond with highlight
            penLine.setWidth(m_lineWidth + 1);
            penLine.setColor(m_brushLine.color());
            painter.setPen(penLine);
            painter.drawArc(rectHole, nHighlightStartAngle + nHighlightSpan, 5760 - nHighlightSpan);
            penLine.setColor(m_colorHighlightOutline);
            painter.setPen(penLine);
            painter.drawArc(rectHole, nHighlightStartAngle, nHighlightSpan);
        } else {
            penLine.setColor(m_brushLine.color());
            penLine.setWidth(m_lineWidth);
            painter.setPen(penLine);
            painter.setBrush(m_brushBackground);
            painter.drawEllipse(rectHole);
        }
    }

    //Draw top title
    if (!m_strTopTitle.isEmpty()) {
        painter.save();
        painter.setPen(m_colorTopTitle);
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

void PieChart::EnableFill(bool fEnable)
{
    m_fEnableFill = fEnable;
}

/**
 * @brief PieChart::SetChartSize set size of the full pie chart
 * @param nSize size in pixels
 */
void PieChart::SetChartSize(int nSize)
{
    m_size = nSize;
    m_fChangesMade = true;
}

/**
 * @brief PieChart::SetStartingAngle set the initial angle of when the chart is first made
 * @param nAngle
 */
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
    m_fChangesMade = true;
}

void PieChart::EnableOutline(bool fEnable)
{
    m_fDountHole = fEnable;
    m_fChangesMade = true;
}

void PieChart::SetLabelType(std::string nType)
{
    m_labelType = LabelTypeFromString(nType);
    m_fChangesMade = true;
}

void PieChart::SetLabelType(PieLabelType nType)
{
    m_labelType = nType;
    m_fChangesMade = true;
}

void PieChart::SetXLabelPadding(double nPadding)
{
    m_xLabelPadding = nPadding;
    m_fChangesMade = true;
}

void PieChart::SetYLabelPadding(double nPadding)
{
    m_yLabelPadding = nPadding;
    m_fChangesMade = true;
}

void PieChart::SetColor(std::string label, QColor qColor)
{
    m_mapColors.at(label) = qColor;
    m_fChangesMade = true;
}

/**
 * @brief PieChart::GetColor gets the color of a specific label
 * @param label
 * @return
 */
QColor PieChart::GetColor(std::string label)
{
    return m_mapColors.at(label);
}

std::map<std::string, QColor> PieChart::GetColorMap() {
    return m_mapColors;
}

/**
 * @brief PieChart::ChartLabels Gives a list of the labels for the chart
 * @return Alphabetized list of the labels
 */
QStringList PieChart::ChartLabels() {
    QStringList listLabels;
    for (auto pair: m_mapPoints) {
        listLabels.append(QString::fromStdString(pair.first));
    }
    return listLabels;
}

void PieChart::EnableHighlight(bool fEnable)
{
    m_fEnableHighlight = fEnable;
    m_fChangesMade = true;
}

void PieChart::SetHighlight(QColor color)
{
    m_colorHighlight = color;
    m_fChangesMade = true;
}

void PieChart::EnableHighlightOutline(bool fEnable)
{
    m_fEnableHighlightOutline = fEnable;
    m_fChangesMade = true;
}

void PieChart::SetHighlightOutline(QColor color)
{
    m_colorHighlightOutline = color;
    m_fChangesMade = true;
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
