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
    m_data.emplace_back(std::make_pair(QString("red"), QColor(Qt::red)));
    m_data.emplace_back(std::make_pair(QString("blue"), QColor(Qt::blue)));
    m_data.emplace_back(std::make_pair(QString("green"), QColor(Qt::green)));
}

LegendWidget::LegendWidget(std::vector<std::pair<QString, QColor>> labels, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LegendWidget)
{
    ui->setupUi(this);
    LegendWidget();
    m_data = labels;
}

LegendWidget::~LegendWidget()
{
    delete ui;
}

void LegendWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QRect rectChart = LabelArea();
    ui->labelTitle->setText(m_strTitle);
    QFont font = painter.font();
    font.setPointSize(m_nLabelSize);
    painter.setFont(font);
    for(int i = 0; i < static_cast<int>(m_data.size()); i++) {
        QRect textRect((15 + 2*(m_nLabelSize+3)), (2*i*(m_nLabelSize+3))+HeightTopTitleArea()+5, rectChart.right()-rectChart.left(), 2*m_nLabelSize);
        QRect colorRect(15,(2*i*(m_nLabelSize+3))+HeightTopTitleArea()+5, 2*m_nLabelSize, 2*m_nLabelSize);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_data[i].first);
        painter.fillRect(colorRect, m_data[i].second);
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
