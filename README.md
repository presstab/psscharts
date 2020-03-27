# PssCharts
Simple and easy to use charts and tool. Made using [Qt Widgets](https://doc.qt.io/qt-5/qtwidgets-index.html). Released under MIT License, meaning our charts are **free to use for both personal and commercial software**.

To see a chart in action, try out our example chart tool. The example charts can be built using chart_examples.pro.
![LineChart](chartexamples/linechart_example.gif)

To use our charts in your project, add the files from the `src/` directory to your project.

### Quick example:
```
    PssCharts::LineChart* chart = new PssCharts::LineChart();
    chart->resize(400, 400);
    chart->SetTopTitle("Line Chart");
    std::map<uint32_t, double> mapPoints = {
        {1585337651, 3.0928},
        {1585348000, 4.3948},
        {1585368000, 5.09},
        {1585398000, 4.988}
    };
    chart->SetDataPoints(mapPoints);
    chart->show();
```
