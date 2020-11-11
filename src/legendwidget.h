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
    ~LegendWidget();
    void SetLegendData(std::vector<std::pair<QString, QColor>> chartData);
    void SetLegendData(std::map<std::string, QColor> chartData);
    void SetTitle(QString title);

private:
    void paintEvent(QPaintEvent *event) override;
    Ui::LegendWidget *ui;
    std::vector<std::pair<QString, QColor>> m_data;
    QString m_strTitle;
    QRect LabelArea() const;
    int m_topTitleHeight;
    int HeightTopTitleArea() const;
    void SetTopTitleHeight(int height);
    int m_nLabelSize;
};

#endif // LEGENDWIDGET_H
