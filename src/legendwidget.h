#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QWidget>

namespace Ui {
class LegendWidget;
}

class LegendWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LegendWidget(QWidget *parent = nullptr);
    LegendWidget(std::vector<std::pair<QString, QColor>> labels, QWidget *parent = nullptr);
    ~LegendWidget();
    void SetLegendData(std::vector<std::pair<QString, QColor>> chartData);
    void ClearLayout();
    void SetTitle(QString title);

private:
    Ui::LegendWidget *ui;
    std::vector<std::pair<QString, QColor>> m_data;
    QString m_strTitle;

};

#endif // LEGENDWIDGET_H
