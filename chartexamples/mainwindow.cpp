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
#include "src/chart.h"
#include "src/linechart.h"
#include "src/candlestickchart.h"

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

    m_lineChart = new PssCharts::LineChart(this);
    m_candleChart = new PssCharts::CandlestickChart(this);
    QSize sizeScreen = QGuiApplication::screens()[0]->size();
    if (sizeScreen.height() > sizeScreen.width()) {
        ui->formLayout->addRow(m_lineChart);
        ui->formLayout->addRow(m_candleChart);
    } else {
        ui->hlayoutMain->addWidget(m_lineChart, /*stretch*/1);
        ui->hlayoutMain->addWidget(m_candleChart, /*stretch*/1);
    }
    m_chartType = PssCharts::ChartType::CANDLESTICK;

    setWindowTitle(QString("PssCharts %1").arg(m_lineChart->VersionString()));

    //Chart Formats
    ui->comboBoxChartType->addItems(listChartFormats);
    ui->comboBoxChartType->setCurrentIndex(1); //candlestick

    ui->comboboxChartFillColor->addItems(listQtColors);
    ui->comboboxChartFillColor->setCurrentIndex(8); //cyan
    ui->checkboxFillChart->setChecked(true);

    ui->comboboxBgColor->addItems(listQtColors);
    ui->comboboxBgColor->setCurrentIndex(1); //white
    ui->checkboxFillBackground->setChecked(true);

    ui->comboboxLineColor->addItems(listQtColors);
    ui->comboboxLineColor->setCurrentIndex(0); //black

    // Candlestick Colors
    ui->checkBoxCandleFill->setChecked(true);
    ui->comboBoxUpCandleFillColor->addItems(listQtColors);
    ui->comboBoxUpCandleFillColor->setCurrentIndex(6); //green
    ui->comboBoxDownCandleFillColor->addItems(listQtColors);
    ui->comboBoxDownCandleFillColor->setCurrentIndex(5); //red

    ui->checkBoxBorderColor->setChecked(true);
    ui->comboBoxUpBorderColor->addItems(listQtColors);
    ui->comboBoxUpBorderColor->setCurrentIndex(14); //dark cyan
    ui->comboBoxDownBorderColor->addItems(listQtColors);
    ui->comboBoxDownBorderColor->setCurrentIndex(16); //dark yellow

    ui->checkBoxWickColor->setChecked(true);
    ui->comboBoxUpWickColor->addItems(listQtColors);
    ui->comboBoxUpWickColor->setCurrentIndex(12); //dark green
    ui->comboBoxDownWickColor->addItems(listQtColors);
    ui->comboBoxDownWickColor->setCurrentIndex(11); //dark red

    ui->checkboxCandleDash->setChecked(false);
    ui->comboBoxUpCandleDashColor->addItems(listQtColors);
    ui->comboBoxUpCandleDashColor->setCurrentIndex(12); //dark green
    ui->comboBoxDownCandleDashColor->addItems(listQtColors);
    ui->comboBoxDownCandleDashColor->setCurrentIndex(11); //dark red

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

    // OHLC Display
    ui->spinboxOHLCFontSize->setValue(8);
    ui->checkBoxOHLCDisplay->setCheckState(Qt::Checked);

    //Axis gridlines
    ui->checkboxDrawXAxisLine->setChecked(true);
    ui->checkBoxDrawYAxisLine->setChecked(true);
    ui->spinboxYLabelPrecision->setValue(2);

    m_lineChart->SetYLabelType(PssCharts::AxisLabelType::AX_NUMBER);
    m_lineChart->SetXLabelType(PssCharts::AxisLabelType::AX_TIMESTAMP);
    m_candleChart->SetYLabelType(PssCharts::AxisLabelType::AX_NUMBER);
    m_candleChart->SetXLabelType(PssCharts::AxisLabelType::AX_TIMESTAMP);

    ui->spinboxGridlines->setValue(5);
    ui->spinboxLineWidth->setValue(3);

    ui->spinboxCandleWidth->setMinimum(5);
    ui->spinboxCandleWidth->setMaximum(25);
    ui->spinboxCandleWidth->setValue(10);
    ui->spinboxCandleLineWidth->setMinimum(1);
    ui->spinboxCandleLineWidth->setMaximum(20);
    ui->spinboxCandleLineWidth->setValue(2);
    m_candleChart->SetCandleWidth(10, 5, 25);

    //Crosshairs
    ui->checkboxCrosshairs->setChecked(true);
    ui->spinboxCrosshairWidth->setValue(2);
    ui->comboboxCrosshairColor->addItems(listQtColors);
    ui->comboboxCrosshairColor->setCurrentIndex(0); //black

    //Generate some data points to fill the chart
    std::map<uint32_t, double> mapPoints;
    double nLastPoint = 0;
    for (auto i = 0; i < 365; i++) {
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
    m_lineChart->SetDataPoints(mapPoints);
    m_lineChart->setMinimumSize(QSize(600,400));
    m_lineChart->hide();
    m_candleChart->SetDataPoints(mapPoints, 7*60*60*24);
    m_candleChart->setMinimumSize(QSize(600,400));
    m_candleChart->show();
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
    connect(ui->checkBoxWickColor, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkBoxBorderColor, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkBoxCandleFill, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxOHLCBold, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkBoxOHLCDisplay, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxCandleDash, &QCheckBox::clicked, this, &MainWindow::RedrawChart);

    connect(ui->comboBoxChartType, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxChartFillColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxBgColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxLineColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxCrosshairColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxUpCandleFillColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxDownCandleFillColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxUpWickColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxDownWickColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxUpBorderColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxDownBorderColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxUpCandleDashColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboBoxDownCandleDashColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);

    connect(ui->spinboxGridlines, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYLabelPrecision, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCrosshairWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCandleWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCandleLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxOHLCFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));

    connect(m_candleChart, &PssCharts::CandlestickChart::candleWidthChanged, this, &MainWindow::ChangeCandleWidth);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RedrawChart()
{
    m_chartType = PssCharts::ChartTypeFromString(ui->comboBoxChartType->currentText().toStdString());
    switch (m_chartType) {
        case PssCharts::ChartType::LINE:{
            m_candleChart->setVisible(false);
            m_lineChart->setVisible(true);

            //Chart Title
            m_lineChart->SetTopTitle(ui->lineeditChartTitle->text());
            QFont fontChartTitle;
            fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
            fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
            m_lineChart->SetTopTitleFont(fontChartTitle);

            //Y Title
            m_lineChart->SetYTitle(ui->lineeditYTitle->text());
            QFont fontYTitle;
            fontYTitle.setPointSize(ui->spinboxYTitleFontSize->value());
            fontYTitle.setBold(ui->checkboxYTitleBold->isChecked());
            m_lineChart->SetYTitleFont(fontYTitle);

            //Axis Labels
            m_lineChart->SetLabelPrecision(ui->spinboxYLabelPrecision->value());
            m_lineChart->SetAxisLabelsOnOff(ui->checkboxDrawXTitle->checkState() == Qt::Checked, ui->checkboxDrawYTitle->checkState() == Qt::Checked);

            //Axis gridlines
            m_lineChart->SetAxisOnOff(ui->checkboxDrawXAxisLine->checkState() == Qt::Checked, ui->checkBoxDrawYAxisLine->checkState() == Qt::Checked);
            m_lineChart->SetAxisSectionCount(ui->spinboxGridlines->value());
            m_lineChart->SetAxisLabelsBrush(QBrush(Qt::black));

            m_lineChart->EnableFill(ui->checkboxFillChart->checkState() == Qt::Checked);
            QColor colorFill = static_cast<Qt::GlobalColor>(ui->comboboxChartFillColor->currentIndex()+2);
            m_lineChart->SetFillBrush(QBrush(colorFill));

            QColor colorBackground = palette().window().color();
            if (ui->checkboxFillBackground->checkState() == Qt::Checked)
                colorBackground = static_cast<Qt::GlobalColor>(ui->comboboxBgColor->currentIndex()+2);
            m_lineChart->SetBackgroundBrush(QBrush(colorBackground));

            //Line Color
            m_lineChart->SetLineWidth(ui->spinboxLineWidth->value());
            QColor colorLine = static_cast<Qt::GlobalColor>(ui->comboboxLineColor->currentIndex()+2);
            m_lineChart->SetLineBrush(colorLine);

            //Mouse Display
            m_lineChart->EnableMouseDisplay(ui->checkboxCrosshairs->isChecked());
            PssCharts::MouseDisplay* display = m_lineChart->GetMouseDisplay();
            display->SetWidth(ui->spinboxCrosshairWidth->value());
            QColor colorCrosshair = static_cast<Qt::GlobalColor>(ui->comboboxCrosshairColor->currentIndex()+2);
            display->SetColor(colorCrosshair);

            m_lineChart->repaint();
            break;
        }
        case PssCharts::ChartType::CANDLESTICK: {
            m_lineChart->setVisible(false);
            m_candleChart->setVisible(true);

            //Chart Title
            m_candleChart->SetTopTitle(ui->lineeditChartTitle->text());
            QFont fontChartTitle;
            fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
            fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
            m_candleChart->SetTopTitleFont(fontChartTitle);

            //Y Title
            m_candleChart->SetYTitle(ui->lineeditYTitle->text());
            QFont fontYTitle;
            fontYTitle.setPointSize(ui->spinboxYTitleFontSize->value());
            fontYTitle.setBold(ui->checkboxYTitleBold->isChecked());
            m_candleChart->SetYTitleFont(fontYTitle);

            //Axis Labels
            m_candleChart->SetLabelPrecision(ui->spinboxYLabelPrecision->value());
            m_candleChart->SetAxisLabelsOnOff(ui->checkboxDrawXTitle->checkState() == Qt::Checked, ui->checkboxDrawYTitle->checkState() == Qt::Checked);

            //Axis gridlines
            m_candleChart->SetAxisOnOff(ui->checkboxDrawXAxisLine->checkState() == Qt::Checked, ui->checkBoxDrawYAxisLine->checkState() == Qt::Checked);
            m_candleChart->SetAxisSectionCount(ui->spinboxGridlines->value());
            m_candleChart->SetAxisLabelsBrush(QBrush(Qt::black));

            QColor UpCandleColor = static_cast<Qt::GlobalColor>(ui->comboBoxUpCandleFillColor->currentIndex()+2);
            QColor DownCandleColor = static_cast<Qt::GlobalColor>(ui->comboBoxDownCandleFillColor->currentIndex()+2);
            m_candleChart->SetCandleBodyColor(UpCandleColor, DownCandleColor);
            QColor UpBorderColor = static_cast<Qt::GlobalColor>(ui->comboBoxUpBorderColor->currentIndex()+2);
            QColor DownBorderColor = static_cast<Qt::GlobalColor>(ui->comboBoxDownBorderColor->currentIndex()+2);
            m_candleChart->SetCandleLineColor(UpBorderColor, DownBorderColor);
            QColor UpWickColor = static_cast<Qt::GlobalColor>(ui->comboBoxUpWickColor->currentIndex()+2);
            QColor DownWickColor = static_cast<Qt::GlobalColor>(ui->comboBoxDownWickColor->currentIndex()+2);
            m_candleChart->SetTailColor(UpWickColor, DownWickColor);
            QColor UpDashColor = static_cast<Qt::GlobalColor>(ui->comboBoxUpCandleDashColor->currentIndex()+2);
            QColor DownDashColor = static_cast<Qt::GlobalColor>(ui->comboBoxDownCandleDashColor->currentIndex()+2);
            m_candleChart->SetDashColor(UpDashColor, DownDashColor);

            m_candleChart->SetCandleLineWidth(ui->spinboxCandleLineWidth->value());
            m_candleChart->SetCandleWidth(ui->spinboxCandleWidth->value());
            m_candleChart->EnableCandleFill(ui->checkBoxCandleFill->checkState() == Qt::Checked);
            m_candleChart->EnableWick(ui->checkBoxWickColor->checkState() == Qt::Checked);
            m_candleChart->EnableCandleBorder(ui->checkBoxBorderColor->checkState() == Qt::Checked);
            m_candleChart->EnableCandleDash(ui->checkboxCandleDash->checkState() == Qt::Checked);

            //OHLC Display
            m_candleChart->EnableOHLCDisplay(ui->checkBoxOHLCDisplay->checkState() == Qt::Checked);
            QFont fontOHLC;
            fontOHLC.setPointSize(ui->spinboxOHLCFontSize->value());
            fontOHLC.setBold(ui->checkboxOHLCBold->isChecked());
            m_candleChart->SetOLHCFont(fontOHLC);

            QColor colorBackground = palette().window().color();
            if (ui->checkboxFillBackground->checkState() == Qt::Checked)
                colorBackground = static_cast<Qt::GlobalColor>(ui->comboboxBgColor->currentIndex()+2);
            m_candleChart->SetBackgroundBrush(QBrush(colorBackground));

            //Mouse Display
            m_candleChart->EnableMouseDisplay(ui->checkboxCrosshairs->isChecked());
            PssCharts::MouseDisplay* display = m_candleChart->GetMouseDisplay();
            display->SetWidth(ui->spinboxCrosshairWidth->value());
            QColor colorCrosshair = static_cast<Qt::GlobalColor>(ui->comboboxCrosshairColor->currentIndex()+2);
            display->SetColor(colorCrosshair);

            m_candleChart->repaint();
            break;
        }
        default:
            break;
    }
}

void MainWindow::ChangeCandleWidth(int dChange) {
    ui->spinboxCandleWidth->setValue(ui->spinboxCandleWidth->value() + dChange);
}
