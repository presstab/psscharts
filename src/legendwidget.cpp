#include "legendwidget.h"
#include "ui_legendwidget.h"
#include <string.h>
#include <vector>
#include <functional>
#include <utility>
#include <iostream>
#include <QPushButton>

LegendWidget::LegendWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LegendWidget)
{
    ui->setupUi(this);
    m_data.emplace_back(std::make_pair(QString("red"), QColor(Qt::red)));
    m_data.emplace_back(std::make_pair(QString("blue"), QColor(Qt::blue)));
    SetLegendData(m_data);
}

LegendWidget::LegendWidget(std::vector<std::pair<QString, QColor>> labels, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LegendWidget)
{
    ui->setupUi(this);
    m_data = labels;
}

LegendWidget::~LegendWidget()
{
    delete ui;
}


void LegendWidget::SetLegendData(std::vector<std::pair<QString, QColor>> chartData)
{
    ClearLayout(ui->formLayout);
    for (auto pair: chartData) {
        QHBoxLayout *layout = new QHBoxLayout;
        QPushButton *button = new QPushButton;
        QLabel* label = new QLabel;
        label->setText(pair.first);
        QPalette pal;
        pal.setColor(QPalette::Button, pair.second);
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
        layout->addWidget(label);
        layout->addWidget(button);
        ui->formLayout->addRow(layout);
    }

}

void LegendWidget::ClearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            ClearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete  item->widget();
        }
        delete item;
    }
}
