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
#include <QScreen>

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

QStringList listChartFormats = {
    "Line",
    "Candlestick"
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_chart = new PssCharts::LineChart(this);
    QSize sizeScreen = QGuiApplication::screens()[0]->size();
    if (sizeScreen.height() > sizeScreen.width())
        ui->formLayout->addRow(m_chart);
    else
        ui->hlayoutMain->addWidget(m_chart, /*stretch*/1);

    setWindowTitle(QString("PssCharts %1").arg(m_chart->VersionString()));

    ui->comboboxChartFillColor->addItems(listQtColors);
    ui->comboboxChartFillColor->setCurrentIndex(8); //cyan
    ui->checkboxFillChart->setChecked(true);

    ui->comboboxBgColor->addItems(listQtColors);
    ui->comboboxBgColor->setCurrentIndex(1); //white
    ui->checkboxFillBackground->setChecked(true);

    ui->comboboxLineColor->addItems(listQtColors);
    ui->comboboxLineColor->setCurrentIndex(0); //black

    ui->comboBoxChartType->addItems(listChartFormats);
    ui->comboBoxChartType->setCurrentIndex(1); //line

    //Chart Title
    ui->lineeditChartTitle->setText("PssCharts");
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
    ui->spinboxYLabelPrecision->setValue(2);

    m_chart->SetYLabelType(PssCharts::AxisLabelType::AX_NUMBER);
    m_chart->SetXLabelType(PssCharts::AxisLabelType::AX_TIMESTAMP);

    ui->spinboxGridlines->setValue(5);
    ui->spinboxLineWidth->setValue(3);

    //Crosshairs
    ui->checkboxCrosshairs->setChecked(true);
    ui->spinboxCrosshairWidth->setValue(2);
    ui->comboboxCrosshairColor->addItems(listQtColors);
    ui->comboboxCrosshairColor->setCurrentIndex(0); //black

    //Generate some data points to fill the chart
    std::map<uint32_t, PssCharts::Candle> mapCandlePoints;
    for (auto i = 0; i < 100; i = i+5) {
        mapCandlePoints.emplace(i*(60*60*24), PssCharts::Candle(.32, .43, 0.1, .17));
        mapCandlePoints.emplace((i+1)*(60*60*24), PssCharts::Candle(.34, .45, 0.17, .21));
        mapCandlePoints.emplace((i+2)*(60*60*24), PssCharts::Candle(.215, .35, 0.21, .32));
        mapCandlePoints.emplace((i+3)*(60*60*24), PssCharts::Candle(.25, .39, 0.13, .33));
        mapCandlePoints.emplace((i+4)*(60*60*24), PssCharts::Candle(.27, .41, 0.11, .23));
    }
    m_chart->SetCandleDataPoints(mapCandlePoints);

    //Generate some data points to fill the chart
    std::map<uint32_t, double> mapPoints;
//    double nLastPoint = 0;
//    for (auto i = 0; i < 100; i++) {
//        double y = QRandomGenerator::global()->generateDouble();
//        if (nLastPoint > 0) {
//            double nPercentChange = (y - nLastPoint) / nLastPoint;
//            if (nPercentChange > 0.3)
//                y = nLastPoint*1.3;
//            if (nPercentChange < -0.3)
//                y = nLastPoint*0.7;
//        }
//        mapPoints.emplace(i*(60*60*24), y);
//        nLastPoint = y;
//    }
    mapPoints.emplace(0*(60*60*24), .284);
    mapPoints.emplace(33*(60*60*24), .1);
    mapPoints.emplace(50*(60*60*24), .33);
    mapPoints.emplace(75*(60*60*24), .215);
    mapPoints.emplace(100*(60*60*24), .45);
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
    connect(ui->checkboxCrosshairs, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->comboboxChartFillColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxBgColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxLineColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxCrosshairColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxChartType, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);

    connect(ui->spinboxGridlines, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYLabelPrecision, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCrosshairWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RedrawChart()
{
    //Chart Type
    m_chart->SetChartType(ui->comboBoxChartType->currentText());

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

    //Axis Labels
    m_chart->SetLabelPrecision(ui->spinboxYLabelPrecision->value());
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

    //Line Color
    m_chart->SetLineWidth(ui->spinboxLineWidth->value());
    QColor colorLine = static_cast<Qt::GlobalColor>(ui->comboboxLineColor->currentIndex()+2);
    m_chart->SetLineBrush(colorLine);

    //Mouse Display
    m_chart->EnableMouseDisplay(ui->checkboxCrosshairs->isChecked());
    PssCharts::MouseDisplay* display = m_chart->GetMouseDisplay();
    display->SetWidth(ui->spinboxCrosshairWidth->value());
    QColor colorCrosshair = static_cast<Qt::GlobalColor>(ui->comboboxCrosshairColor->currentIndex()+2);
    display->SetColor(colorCrosshair);

    m_chart->repaint();
}
