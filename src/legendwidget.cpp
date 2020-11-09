#include "legendwidget.h"
#include "ui_legendwidget.h"
#include <string.h>
#include <vector>
#include <functional>
#include <utility>
#include <iostream>
#include <QPushButton>
#include <QToolButton>

LegendWidget::LegendWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LegendWidget)
{
    ui->setupUi(this);
    m_strTitle = "Chart Legend";
    m_data.emplace_back(std::make_pair(QString("red"), QColor(Qt::red)));
    m_data.emplace_back(std::make_pair(QString("blue"), QColor(Qt::blue)));
    SetLegendData(m_data);
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
    ClearLayout();
    ui->labelTitle->setText(m_strTitle);
    QHBoxLayout *layout = new QHBoxLayout;
    for (auto pair: chartData) {
        QHBoxLayout *layout = new QHBoxLayout;
        QToolButton *button = new QToolButton;
        QLabel* label = new QLabel;
        label->setText(pair.first);
        QPalette pal;
        pal.setColor(QPalette::Button, pair.second);
        button->setDisabled(true);
        button->setAutoFillBackground(true);
        button->setPalette(pal);
        button->update();
        layout->addWidget(label);
        layout->addWidget(button);
        ui->formLayoutLabels->addRow(layout);
    }
    this->setLayout(layout);

}

void LegendWidget::ClearLayout()
{
    qDeleteAll(this->children());
    ui->setupUi(this);
}

void LegendWidget::SetTitle(QString title)
{
    m_strTitle = title;
}
