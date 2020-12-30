#include "legendwidget.h"
#include "ui_legendwidget.h"
#include <string.h>
#include <vector>
#include <functional>
#include <utility>
#include <iostream>
#include <QPainter>

LegendWidget::LegendWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LegendWidget)
{
    ui->setupUi(this);
    m_strTitle = "Chart Legend";
    m_topTitleHeight = -1;
    m_nLabelSize = 11;
}

LegendWidget::~LegendWidget()
{
    delete ui;
}

void LegendWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    ui->labelTitle->setText(m_strTitle);
    QFont font = painter.font();
    font.setPointSize(m_nLabelSize);
    painter.setFont(font);
    int maxWidth = ui->labelTitle->fontMetrics().horizontalAdvance(m_strTitle) + 30;

    QFontMetrics labelMetric = painter.fontMetrics();
    bool fTallWidget =  this->height() >= this->width();

    // Draw labels based on the size of the widget
    if(fTallWidget) {
        for(int i = 0; i < static_cast<int>(m_data.size()); i++) {
            QRect colorRect(15, 2*i*labelMetric.height()+HeightTopTitleArea()+10, labelMetric.height(), labelMetric.height());
            painter.fillRect(colorRect, m_data[i].second);
            QRect textRect(25 + labelMetric.height(), 2*i*labelMetric.height()+HeightTopTitleArea()+10, labelMetric.horizontalAdvance(m_data[i].first), labelMetric.height());
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_data[i].first);
            if(labelMetric.horizontalAdvance(m_data[i].first) + labelMetric.height() + 30 > maxWidth) {
                maxWidth = labelMetric.horizontalAdvance(m_data[i].first) + labelMetric.height() + 30;
            }
        }
        this->setMinimumHeight(2*static_cast<int>(m_data.size())*labelMetric.height()+HeightTopTitleArea()+10);
        this->setMinimumWidth(maxWidth);
    } else {
        int labelWidth = 0;
        int row = 0;
        for(int i = 0; i < static_cast<int>(m_data.size()); i++) {
            QRect colorRect(i*labelMetric.height() + labelWidth, 2*row*labelMetric.height()+HeightTopTitleArea()+10, labelMetric.height(), labelMetric.height());
            painter.fillRect(colorRect, m_data[i].second);
            QRect textRect(labelMetric.height() + 10 + i*labelMetric.height() + labelWidth, 2*row*labelMetric.height()+HeightTopTitleArea()+10, labelMetric.horizontalAdvance(m_data[i].first), labelMetric.height());
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_data[i].first);
            labelWidth += labelMetric.height() + 10 + labelMetric.horizontalAdvance(m_data[i].first);
            if (labelWidth > this->width()) {
                labelWidth = 0;
                row++;
                if (labelWidth > this->minimumWidth()) {
                    this->setMinimumWidth(labelWidth);
                }
            }
        }
        this->setMinimumHeight((2*(++row)*labelMetric.height())+HeightTopTitleArea()+10);
    }
}

void LegendWidget::SetTitle(QString title)
{
    m_strTitle = title;
}

void LegendWidget::SetLegendData(std::vector<std::pair<QString, QColor>> chartData)
{
    m_data = chartData;
}

/**
 * @brief LegendWidget::LabelArea : Get the area of the widget that is dedicated to the labels
 * @return
 */
QRect LegendWidget::LabelArea() const
{
    QRect rectFull = this->rect();
    QRect rectChart = rectFull;
    rectChart.setTop(rectFull.top() + HeightTopTitleArea());
    return rectChart;
}

int LegendWidget::HeightTopTitleArea() const
{
    if (m_topTitleHeight != -1)
        return m_topTitleHeight;

    if (m_strTitle.isEmpty())
        return 0;

    return 30;
}

void LegendWidget::SetTopTitleHeight(int height)
{
    m_topTitleHeight = height;
}
