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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "src/linechart.h"

#include <QRandomGenerator>

QStringList listQtColors = {
    "black",
    "white",
    "darkGray",
    "gray",
    "lightGray",
    "red",
    "green",
    "blue",
    "cyan",
    "magenta",
    "yellow",
    "darkRed",
    "darkGreen",
    "darkBlue",
    "darkCyan",
    "darkMagenta",
    "darkYellow",
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_chart = new LineChart(this);
    ui->hlayoutMain->addWidget(m_chart, /*stretch*/1);

    ui->comboboxChartFillColor->addItems(listQtColors);
    ui->comboboxChartFillColor->setCurrentIndex(8); //cyan
    ui->checkboxFillChart->setChecked(true);

    ui->comboboxBgColor->addItems(listQtColors);
    ui->comboboxBgColor->setCurrentIndex(1); //white
    ui->checkboxFillBackground->setChecked(true);

    ui->comboboxLineColor->addItems(listQtColors);
    ui->comboboxLineColor->setCurrentIndex(0); //black

    //Chart Title
    ui->lineeditChartTitle->setText("Line Chart");
    ui->spinboxTitleFontSize->setValue(14);
    ui->checkboxChartTitleBold->setCheckState(Qt::Checked);

    // Y Title
    ui->lineeditYTitle->setText("Price");
    ui->spinboxYTitleFontSize->setValue(12);
    ui->checkboxYTitleBold->setCheckState(Qt::Unchecked);

    ui->checkboxDrawXTitle->setChecked(true);
    ui->checkboxDrawYTitle->setChecked(true);

    //Axis gridlines
    ui->checkboxDrawXAxisLine->setChecked(true);
    ui->checkBoxDrawYAxisLine->setChecked(true);

    m_chart->SetYLabelFont(QFont());
    m_chart->SetYLabelType(AxisLabelType::AX_NUMBER);
    m_chart->SetXLabelType(AxisLabelType::AX_TIMESTAMP);

    ui->spinboxGridlines->setValue(5);
    ui->spinboxLineWidth->setValue(3);

    //Generate some data points to fill the chart
    std::map<uint32_t, double> mapPoints;
    double nLastPoint = 0;
    for (auto i = 0; i < 100; i++) {
        double y = QRandomGenerator::global()->generateDouble();
        if (nLastPoint > 0) {
            double nPercentChange = (y - nLastPoint) / nLastPoint;
            if (nPercentChange > 0.3)
                y = nLastPoint*1.3;
            if (nPercentChange < -0.3)
                y = nLastPoint*0.7;
        }
        mapPoints.emplace(i*(60*60*24), y);
        nLastPoint = y;
    }
    m_chart->SetDataPoints(mapPoints);
    m_chart->setMinimumSize(QSize(600,400));
    m_chart->show();
    RedrawChart();

    connect(ui->lineeditChartTitle, &QLineEdit::textChanged, this, &MainWindow::RedrawChart);
    connect(ui->lineeditYTitle, &QLineEdit::textChanged, this, &MainWindow::RedrawChart);
    connect(ui->checkboxDrawXTitle, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxDrawYTitle, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxDrawXAxisLine, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkBoxDrawYAxisLine, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxFillChart, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxFillBackground, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxChartTitleBold, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxYTitleBold, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->comboboxChartFillColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxBgColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxLineColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);

    connect(ui->spinboxGridlines, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RedrawChart()
{
    //Chart Title
    m_chart->SetTopTitle(ui->lineeditChartTitle->text());
    QFont fontChartTitle;
    fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
    fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
    m_chart->SetTopTitleFont(fontChartTitle);

    //Y Title
    m_chart->SetYTitle(ui->lineeditYTitle->text());
    QFont fontYTitle;
    fontYTitle.setPointSize(ui->spinboxYTitleFontSize->value());
    fontYTitle.setBold(ui->checkboxYTitleBold->isChecked());
    m_chart->SetYTitleFont(fontYTitle);

    m_chart->SetAxisLabelsOnOff(ui->checkboxDrawXTitle->checkState() == Qt::Checked, ui->checkboxDrawYTitle->checkState() == Qt::Checked);

    //Axis gridlines
    m_chart->SetAxisOnOff(ui->checkboxDrawXAxisLine->checkState() == Qt::Checked, ui->checkBoxDrawYAxisLine->checkState() == Qt::Checked);
    m_chart->SetAxisSectionCount(ui->spinboxGridlines->value());
    m_chart->SetAxisLabelsBrush(QBrush(Qt::black));

    m_chart->EnableFill(ui->checkboxFillChart->checkState() == Qt::Checked);
    QColor colorFill = static_cast<Qt::GlobalColor>(ui->comboboxChartFillColor->currentIndex()+2);
    m_chart->SetFillBrush(QBrush(colorFill));

    QColor colorBackground = palette().window().color();
    if (ui->checkboxFillBackground->checkState() == Qt::Checked)
        colorBackground = static_cast<Qt::GlobalColor>(ui->comboboxBgColor->currentIndex()+2);
    m_chart->SetBackgroundBrush(QBrush(colorBackground));

    m_chart->SetLineWidth(ui->spinboxLineWidth->value());
    QColor colorLine = static_cast<Qt::GlobalColor>(ui->comboboxLineColor->currentIndex()+2);
    m_chart->SetLineBrush(colorLine);

    m_chart->repaint();
}
