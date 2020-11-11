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
#include "src/piechart.h"
#include "src/barchart.h"

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
    "Bar",
    "Candlestick",
    "Line",
    "Pie"
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_lineChart = new PssCharts::LineChart(this);
    m_candleChart = new PssCharts::CandlestickChart(this);
    m_barChart = new PssCharts::BarChart(this);
    m_pieChart = new PssCharts::PieChart(this);
    m_legend = new LegendWidget(this);
    QSize sizeScreen = QGuiApplication::screens()[0]->size();
    if (sizeScreen.height() > sizeScreen.width()) {
        ui->formLayout->addRow(m_lineChart);
        ui->formLayout->addRow(m_candleChart);
        ui->formLayout->addRow(m_barChart);
        ui->formLayout->addRow(m_pieChart);
        ui->formLayout->addRow(m_legend);
    } else {
        ui->hlayoutMain->addWidget(m_lineChart, /*stretch*/1);
        ui->hlayoutMain->addWidget(m_candleChart, /*stretch*/1);
        ui->hlayoutMain->addWidget(m_barChart, /*stretch*/1);
        ui->hlayoutMain->addWidget(m_pieChart, /*stretch*/1);
        ui->hlayoutMain->addWidget(m_legend, /*stretch*/1);
    }
    m_chartType = PssCharts::ChartType::PIE;

    setWindowTitle(QString("PssCharts %1").arg(m_lineChart->VersionString()));

    //Chart Formats
    ui->comboBoxChartType->addItems(listChartFormats);
    ui->tabWidget->setCurrentIndex(0); // bar
    ui->comboBoxChartType->setCurrentIndex(0); // bar

    ui->comboboxChartFillColor->addItems(listQtColors);
    ui->comboboxChartFillColor->setCurrentIndex(8); //cyan
    ui->checkboxFillChart->setChecked(true);

    ui->comboboxBgColor->addItems(listQtColors);
    ui->comboboxBgColor->setCurrentIndex(1); //white
    ui->checkboxFillBackground->setChecked(true);

    ui->comboboxLineColor->addItems(listQtColors);
    ui->comboboxLineColor->setCurrentIndex(0); //black

    ui->comboboxChartTitleColor->addItems(listQtColors);
    ui->comboboxChartTitleColor->setCurrentIndex(0); //black

    ui->comboboxYTitleColor->addItems(listQtColors);
    ui->comboboxYTitleColor->setCurrentIndex(0); //black

    // Bar Colors
    ui->comboboxBarColor->addItems(listQtColors);
    ui->comboboxBarColor->setCurrentIndex(8); //cyan
    ui->checkboxBarColor->setChecked(true);

    ui->comboboxBarLineColor->addItems(listQtColors);
    ui->comboboxBarLineColor->setCurrentIndex(0); //black
    ui->checkboxBarLine->setChecked(true);

    ui->comboboxHighlightBar->addItems(listQtColors);
    ui->comboboxHighlightBar->setCurrentIndex(9); //magenta
    ui->checkboxHighlightBar->setChecked(true);

    ui->comboboxHighlightLine->addItems(listQtColors);
    ui->comboboxHighlightLine->setCurrentIndex(1); //white
    ui->checkboxHighlightLine->setChecked(true);


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

    // Candle Volume Bar
    ui->checkboxCandleVolumeBar->setChecked(true);
    ui->comboboxCandleVolumeBar->addItems(listQtColors);
    ui->comboboxCandleVolumeBar->setCurrentIndex(7); //blue

    // Line Volume Bar
    ui->checkboxLineVolumeBar->setChecked(true);
    ui->spinboxLineVolumeBar->setValue(3);

    // Pie Chart
    ui->spinboxPieSize->setMinimum(0);
    ui->spinboxPieSize->setMaximum(500);
    ui->spinboxPieSize->setValue(250);
    ui->spinboxPieDonut->setMinimum(0);
    ui->spinboxPieDonut->setMaximum(500);
    ui->spinboxPieDonut->setValue(100);
    ui->spinboxPieAngle->setMinimum(0);
    ui->spinboxPieAngle->setMaximum(360);
    ui->spinboxPieAngle->setValue(90);
    ui->checkboxPieDonut->setChecked(false);
    ui->comboboxPieOutline->addItems(listQtColors);
    ui->comboboxPieOutline->setCurrentIndex(0); //black
    ui->spinboxPieOutline->setValue(5);
    ui->doublespinboxPieLabelX->setSingleStep(0.05);
    ui->doublespinboxPieLabelY->setSingleStep(0.05);
    ui->doublespinboxPieLabelX->setValue(1.3);
    ui->doublespinboxPieLabelY->setValue(1.1);
    ui->checkboxPieHighlight->setChecked(true);
    ui->comboboxPieHighlight->addItems(listQtColors);
    ui->comboboxPieHighlight->setCurrentIndex(9); //magenta
    ui->checkboxPieHighlightOutline->setChecked(true);
    ui->comboboxPieHighlightOutline->addItems(listQtColors);
    ui->comboboxPieHighlightOutline->setCurrentIndex(1); //white

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
    m_barChart->SetYLabelType(PssCharts::AxisLabelType::AX_NUMBER);
    m_barChart->SetXLabelType(PssCharts::AxisLabelType::AX_TIMESTAMP);

    ui->spinboxGridlines->setValue(5);
    ui->spinboxLineWidth->setValue(3);

    //Bar Width
    ui->spinboxBarWidth->setMinimum(1);
    ui->spinboxBarWidth->setMaximum(25);
    ui->spinboxBarWidth->setValue(10);
    ui->spinboxBarLineWidth->setMinimum(1);
    ui->spinboxBarLineWidth->setMaximum(20);
    ui->spinboxBarLineWidth->setValue(2);
    m_barChart->SetBarWidth(10, 1, 25);

    //Candle Width
    ui->spinboxCandleWidth->setMinimum(5);
    ui->spinboxCandleWidth->setMaximum(99);
    ui->spinboxCandleWidth->setValue(10);
    ui->spinboxCandleLineWidth->setMinimum(1);
    ui->spinboxCandleLineWidth->setMaximum(20);
    ui->spinboxCandleLineWidth->setValue(2);
    m_candleChart->SetCandleWidth(10, 5, 99);

    //Crosshairs
    ui->checkboxCrosshairs->setChecked(true);
    ui->spinboxCrosshairWidth->setValue(2);
    ui->comboboxCrosshairColor->addItems(listQtColors);
    ui->comboboxCrosshairColor->setCurrentIndex(0); //black

    //Generate some data points to fill the chart
    std::map<uint32_t, double> mapPoints;
    std::map<uint32_t, double> mapPoints2;

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
        double y2 = y *0.9;
        if (i % 3)
            y2 *= 1.35;
        else if (i % 2)
            y2 *= 0.75;
        mapPoints2.emplace(i*(60*60*24), y2);

        nLastPoint = y;
    }
    m_lineChart->SetDataPoints(mapPoints, 0);
    m_lineChart->SetDataPoints(mapPoints2, 1);
    ui->comboboxSeriesLineColor->addItem("0");
    ui->comboboxSeriesLineColor->addItem("1");
    m_lineChart->SetVolumePoints(mapPoints, 0);
    m_lineChart->SetVolumePoints(mapPoints2, 1);
    m_lineChart->setMinimumSize(QSize(600,400));
    m_candleChart->SetDataPoints(mapPoints, mapPoints2, 5*60*60*24);
    m_candleChart->setMinimumSize(QSize(600,400));
    m_barChart->SetDataPoints(mapPoints);
    m_barChart->setMinimumSize(QSize(600,400));

    m_pieChart->AddDataPoint("USA", 4932.10437982);
    m_pieChart->AddDataPoint("Italy", 1235.7598342);
    m_pieChart->AddDataPoint("Belgium", 2556.91745);
    m_pieChart->AddDataPoint("China", 9876.0942323);
    m_pieChart->AddDataPoint("Brazil", 1235.7598342);
    m_pieChart->AddDataPoint("India", 6235.671273894);
    m_pieChart->AddDataPoint("Egypt", 943.9102548);
    ui->comboboxPieSlice->addItems(m_pieChart->ChartLabels());
    QColor sliceColor = m_pieChart->GetColor(ui->comboboxPieSlice->currentText().toStdString());
    ui->spinboxPieBlue->setValue(sliceColor.blue());
    ui->spinboxPieGreen->setValue(sliceColor.green());
    ui->spinboxPieRed->setValue(sliceColor.red());
    m_pieChart->setMinimumSize(QSize(600,400));
    RedrawChart();

    connect(ui->tabWidget, &QTabWidget::currentChanged, ui->comboBoxChartType, &QComboBox::setCurrentIndex);
    connect(ui->comboBoxChartType, SIGNAL(currentIndexChanged(int)), ui->tabWidget, SLOT(setCurrentIndex(int)));

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
    connect(ui->checkboxCandleVolumeBar, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxLineVolumeBar, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxBarColor, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxBarLine, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxHighlightBar, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxHighlightLine, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxPieDonut, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxPieHighlight, &QCheckBox::clicked, this, &MainWindow::RedrawChart);
    connect(ui->checkboxPieHighlightOutline, &QCheckBox::clicked, this, &MainWindow::RedrawChart);

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
    connect(ui->comboboxCandleVolumeBar, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxBarColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxBarLineColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxHighlightBar, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxHighlightLine, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxPieOutline, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxPieLabel, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxPieSlice, &QComboBox::currentTextChanged, this, &MainWindow::PieColorChanged);
    connect(ui->comboboxPieHighlight, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxPieHighlightOutline, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxChartTitleColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);
    connect(ui->comboboxYTitleColor, &QComboBox::currentTextChanged, this, &MainWindow::RedrawChart);

    connect(ui->spinboxGridlines, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYTitleFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxYLabelPrecision, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCrosshairWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCandleWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxCandleLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxOHLCFontSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxBarWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxBarLineWidth, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxLineVolumeBar, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieSize, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieAngle, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieDonut, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieOutline, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieRed, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieGreen, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->spinboxPieBlue, SIGNAL(valueChanged(int)), this, SLOT(RedrawChart()));
    connect(ui->doublespinboxPieLabelX, SIGNAL(valueChanged(double)), this, SLOT(RedrawChart()));
    connect(ui->doublespinboxPieLabelY, SIGNAL(valueChanged(double)), this, SLOT(RedrawChart()));

    connect(m_candleChart, &PssCharts::CandlestickChart::candleWidthChanged, this, &MainWindow::ChangeCandleWidth);
    connect(m_barChart, &PssCharts::BarChart::barWidthChanged, this, &MainWindow::ChangeBarWidth);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::RedrawChart()
{
    m_chartType = PssCharts::ChartTypeFromString(ui->comboBoxChartType->currentText().toStdString());

    if (m_chartType == PssCharts::ChartType::PIE) {
        ui->labelDrawAxisLabels->setVisible(false);
        ui->labelDrawAxisLine->setVisible(false);
        ui->labelYTitle->setVisible(false);
        ui->labelGridlineCount->setVisible(false);
        ui->spinboxGridlines->setVisible(false);
        ui->checkboxYTitleBold->setVisible(false);
        ui->lineeditYTitle->setVisible(false);
        ui->spinboxYTitleFontSize->setVisible(false);
        ui->checkBoxDrawYAxisLine->setVisible(false);
        ui->checkboxDrawXAxisLine->setVisible(false);
        ui->checkboxDrawXTitle->setVisible(false);
        ui->checkboxDrawYTitle->setVisible(false);
    } else {
        ui->labelDrawAxisLabels->setVisible(true);
        ui->labelDrawAxisLine->setVisible(true);
        ui->labelYTitle->setVisible(true);
        ui->labelGridlineCount->setVisible(true);
        ui->spinboxGridlines->setVisible(true);
        ui->checkboxYTitleBold->setVisible(true);
        ui->lineeditYTitle->setVisible(true);
        ui->spinboxYTitleFontSize->setVisible(true);
        ui->checkBoxDrawYAxisLine->setVisible(true);
        ui->checkboxDrawXAxisLine->setVisible(true);
        ui->checkboxDrawXTitle->setVisible(true);
        ui->checkboxDrawYTitle->setVisible(true);
    }

    switch (m_chartType) {
        case PssCharts::ChartType::LINE:{
            m_barChart->setVisible(false);
            m_candleChart->setVisible(false);
            m_pieChart->setVisible(false);
            m_lineChart->setVisible(true);

            //Chart Title
            m_lineChart->SetTopTitle(ui->lineeditChartTitle->text());
            QFont fontChartTitle;
            fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
            fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
            m_lineChart->SetTopTitleFont(fontChartTitle);
            m_lineChart->SetTopTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxChartTitleColor->currentIndex()+2));

            //Y Title
            m_lineChart->SetYTitle(ui->lineeditYTitle->text());
            QFont fontYTitle;
            fontYTitle.setPointSize(ui->spinboxYTitleFontSize->value());
            fontYTitle.setBold(ui->checkboxYTitleBold->isChecked());
            m_lineChart->SetYTitleFont(fontYTitle);
            m_lineChart->SetYTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxYTitleColor->currentIndex()+2));

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
            uint32_t nSeries = ui->comboboxSeriesLineColor->currentText().toUInt();
            m_lineChart->SetLineBrush(nSeries, colorLine);

            //Volume Bar
            m_lineChart->SetVolumeBarWidth(ui->spinboxLineVolumeBar->value());
            m_lineChart->EnableVolumeBar(ui->checkboxLineVolumeBar->checkState() == Qt::Checked);

            //Mouse Display
            m_lineChart->EnableMouseDisplay(ui->checkboxCrosshairs->isChecked());
            PssCharts::MouseDisplay* display = m_lineChart->GetMouseDisplay();
            display->SetWidth(ui->spinboxCrosshairWidth->value());
            QColor colorCrosshair = static_cast<Qt::GlobalColor>(ui->comboboxCrosshairColor->currentIndex()+2);
            display->SetColor(colorCrosshair);

            m_lineChart->repaint();
            m_legend->SetLegendData(m_lineChart->GetLegendData());
            m_legend->repaint();
            break;
        }
        case PssCharts::ChartType::CANDLESTICK: {
            m_lineChart->setVisible(false);
            m_barChart->setVisible(false);
            m_pieChart->setVisible(false);
            m_candleChart->setVisible(true);

            //Chart Title
            m_candleChart->SetTopTitle(ui->lineeditChartTitle->text());
            QFont fontChartTitle;
            fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
            fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
            m_candleChart->SetTopTitleFont(fontChartTitle);
            m_candleChart->SetTopTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxChartTitleColor->currentIndex()+2));

            //Y Title
            m_candleChart->SetYTitle(ui->lineeditYTitle->text());
            QFont fontYTitle;
            fontYTitle.setPointSize(ui->spinboxYTitleFontSize->value());
            fontYTitle.setBold(ui->checkboxYTitleBold->isChecked());
            m_candleChart->SetYTitleFont(fontYTitle);
            m_candleChart->SetYTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxYTitleColor->currentIndex()+2));

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
            m_candleChart->SetVolumeColor(static_cast<Qt::GlobalColor>(ui->comboboxCandleVolumeBar->currentIndex()+2));

            m_candleChart->SetCandleLineWidth(ui->spinboxCandleLineWidth->value());
            m_candleChart->SetCandleWidth(ui->spinboxCandleWidth->value());
            m_candleChart->EnableCandleFill(ui->checkBoxCandleFill->checkState() == Qt::Checked);
            m_candleChart->EnableWick(ui->checkBoxWickColor->checkState() == Qt::Checked);
            m_candleChart->EnableCandleBorder(ui->checkBoxBorderColor->checkState() == Qt::Checked);
            m_candleChart->EnableCandleDash(ui->checkboxCandleDash->checkState() == Qt::Checked);
            m_candleChart->EnableVolumeBar(ui->checkboxCandleVolumeBar->checkState() == Qt::Checked);

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
            m_legend->SetLegendData(m_candleChart->GetLegendData());
            m_legend->repaint();
            break;
        }
        case PssCharts::ChartType::BAR:
        {
            m_lineChart->setVisible(false);
            m_candleChart->setVisible(false);
            m_pieChart->setVisible(false);
            m_barChart->setVisible(true);

            //Chart Title
            m_barChart->SetTopTitle(ui->lineeditChartTitle->text());
            QFont fontChartTitle;
            fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
            fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
            m_barChart->SetTopTitleFont(fontChartTitle);
            m_barChart->SetTopTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxChartTitleColor->currentIndex()+2));

            //Y Title
            m_barChart->SetYTitle(ui->lineeditYTitle->text());
            QFont fontYTitle;
            fontYTitle.setPointSize(ui->spinboxYTitleFontSize->value());
            fontYTitle.setBold(ui->checkboxYTitleBold->isChecked());
            m_barChart->SetYTitleFont(fontYTitle);
            m_barChart->SetYTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxYTitleColor->currentIndex()+2));

            //Axis Labels
            m_barChart->SetLabelPrecision(ui->spinboxYLabelPrecision->value());
            m_barChart->SetAxisLabelsOnOff(ui->checkboxDrawXTitle->checkState() == Qt::Checked, ui->checkboxDrawYTitle->checkState() == Qt::Checked);

            //Axis gridlines
            m_barChart->SetAxisOnOff(ui->checkboxDrawXAxisLine->checkState() == Qt::Checked, ui->checkBoxDrawYAxisLine->checkState() == Qt::Checked);
            m_barChart->SetAxisSectionCount(ui->spinboxGridlines->value());
            m_barChart->SetAxisLabelsBrush(QBrush(Qt::black));

            QColor colorBackground = palette().window().color();
            if (ui->checkboxFillBackground->checkState() == Qt::Checked)
                colorBackground = static_cast<Qt::GlobalColor>(ui->comboboxBgColor->currentIndex()+2);
            m_barChart->SetBackgroundBrush(QBrush(colorBackground));

            //Bar Color
            m_barChart->SetLineWidth(ui->spinboxBarLineWidth->value());
            QColor colorLine = static_cast<Qt::GlobalColor>(ui->comboboxBarLineColor->currentIndex()+2);
            m_barChart->SetLineBrush(colorLine);
            QColor colorLineHighlight = static_cast<Qt::GlobalColor>(ui->comboboxHighlightLine->currentIndex()+2);
            m_barChart->SetLineHighlightBrush(colorLineHighlight);
            m_barChart->EnableFill(ui->checkboxBarColor->checkState() == Qt::Checked);
            QColor colorFill = static_cast<Qt::GlobalColor>(ui->comboboxBarColor->currentIndex()+2);
            m_barChart->SetBarColor(colorFill);
            QColor colorFillHighlight = static_cast<Qt::GlobalColor>(ui->comboboxHighlightBar->currentIndex()+2);
            m_barChart->SetBarHighlightColor(colorFillHighlight);

            m_barChart->SetBarWidth(ui->spinboxBarWidth->value());
            m_barChart->SetLineWidth(ui->spinboxBarLineWidth->value());
            m_barChart->EnableFill(ui->checkboxBarColor->checkState() == Qt::Checked);
            m_barChart->EnableBorder(ui->checkboxBarLine->checkState() == Qt::Checked);
            m_barChart->EnableHighlight(ui->checkboxHighlightBar->checkState() == Qt::Checked);
            m_barChart->EnableHighlightBorder(ui->checkboxHighlightLine->checkState() == Qt::Checked);

            //Mouse Display
            m_barChart->EnableMouseDisplay(ui->checkboxCrosshairs->isChecked());
            PssCharts::MouseDisplay* display = m_barChart->GetMouseDisplay();
            display->SetWidth(ui->spinboxCrosshairWidth->value());
            QColor colorCrosshair = static_cast<Qt::GlobalColor>(ui->comboboxCrosshairColor->currentIndex()+2);
            display->SetColor(colorCrosshair);

            m_barChart->repaint();
            m_legend->SetLegendData(m_barChart->GetLegendData());
            m_legend->repaint();
            break;
        }
        case PssCharts::ChartType::PIE:{
            m_lineChart->setVisible(false);
            m_candleChart->setVisible(false);
            m_barChart->setVisible(false);
            m_pieChart->setVisible(true);

            //Chart Title
            m_pieChart->SetTopTitle(ui->lineeditChartTitle->text());
            QFont fontChartTitle;
            fontChartTitle.setPointSize(ui->spinboxTitleFontSize->value());
            fontChartTitle.setBold(ui->checkboxChartTitleBold->isChecked());
            m_pieChart->SetTopTitleFont(fontChartTitle);
            m_pieChart->SetLabelPrecision(ui->spinboxYLabelPrecision->value());
            m_pieChart->SetTopTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxChartTitleColor->currentIndex()+2));
            m_pieChart->SetYTitleColor(static_cast<Qt::GlobalColor>(ui->comboboxYTitleColor->currentIndex()+2));

            QColor colorBackground = palette().window().color();
            if (ui->checkboxFillBackground->checkState() == Qt::Checked)
                colorBackground = static_cast<Qt::GlobalColor>(ui->comboboxBgColor->currentIndex()+2);
            m_pieChart->SetBackgroundBrush(QBrush(colorBackground));

            //Line Color
            m_pieChart->SetLineWidth(ui->spinboxPieOutline->value());
            QColor colorLine = static_cast<Qt::GlobalColor>(ui->comboboxPieOutline->currentIndex()+2);
            m_pieChart->SetLineBrush(colorLine);

            // Pie Chart
            m_pieChart->SetChartSize(ui->spinboxPieSize->value());
            m_pieChart->SetStartingAngle(ui->spinboxPieAngle->value());
            m_pieChart->SetDonutSize(ui->spinboxPieDonut->value());
            m_pieChart->EnableDonut(ui->checkboxPieDonut->checkState() == Qt::Checked);
            m_pieChart->SetLabelType(ui->comboboxPieLabel->currentText().toStdString());
            m_pieChart->EnableFill(ui->checkboxFillChart->checkState() == Qt::Checked);
            m_pieChart->SetXLabelPadding(ui->doublespinboxPieLabelX->value());
            m_pieChart->SetYLabelPadding(ui->doublespinboxPieLabelY->value());
            m_pieChart->SetColor(ui->comboboxPieSlice->currentText().toStdString(),
                                 QColor(ui->spinboxPieRed->value(), ui->spinboxPieGreen->value(), ui->spinboxPieBlue->value()));
            //m_pieChart->SetColor(ui->comboboxPieSlice->currentText().toStdString(),
                                 //static_cast<Qt::GlobalColor>(ui->comboboxPieSliceColor->currentIndex()+2));
            m_pieChart->EnableHighlight(ui->checkboxPieHighlight->checkState() == Qt::Checked);
            m_pieChart->SetHighlight(static_cast<Qt::GlobalColor>(ui->comboboxPieHighlight->currentIndex() + 2));
            m_pieChart->EnableHighlightOutline(ui->checkboxPieHighlightOutline->checkState() == Qt::Checked);
            m_pieChart->SetHighlightOutline(static_cast<Qt::GlobalColor>(ui->comboboxPieHighlightOutline->currentIndex() + 2));


            //Mouse Display
            m_pieChart->EnableMouseDisplay(ui->checkboxCrosshairs->isChecked());
            PssCharts::MouseDisplay* display = m_pieChart->GetMouseDisplay();
            display->SetWidth(ui->spinboxCrosshairWidth->value());
            QColor colorCrosshair = static_cast<Qt::GlobalColor>(ui->comboboxCrosshairColor->currentIndex()+2);
            display->SetColor(colorCrosshair);

            QColor sliceColor = m_pieChart->GetColor(ui->comboboxPieSlice->currentText().toStdString());
            ui->spinboxPieBlue->setValue(sliceColor.blue());
            ui->spinboxPieGreen->setValue(sliceColor.green());
            ui->spinboxPieRed->setValue(sliceColor.red());
            m_pieChart->repaint();
            m_legend->SetLegendData(m_pieChart->GetColorMap());
            m_legend->repaint();
            break;
        }

        default:
            break;
    }
}

void MainWindow::ChangeCandleWidth(int dChange) {
    ui->spinboxCandleWidth->setValue(ui->spinboxCandleWidth->value() + dChange);
}

void MainWindow::ChangeBarWidth(int dChange) {
    ui->spinboxBarWidth->setValue(ui->spinboxBarWidth->value() + dChange);
}

void MainWindow::PieColorChanged(const QString& text) {
    QColor sliceColor = m_pieChart->GetColor(text.toStdString());
    ui->spinboxPieBlue->setValue(sliceColor.blue());
    ui->spinboxPieGreen->setValue(sliceColor.green());
    ui->spinboxPieRed->setValue(sliceColor.red());
    RedrawChart();
}

