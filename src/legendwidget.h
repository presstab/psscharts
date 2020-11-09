#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QWidget>
#include <QDockWidget>
#include "titlebar.h"

namespace Ui {
class LegendWidget;
}

class LegendWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit LegendWidget(QWidget *parent = nullptr);
    LegendWidget(std::vector<std::pair<QString, QColor>> labels, QWidget *parent = nullptr);
    ~LegendWidget();
    void SetLegendData(std::vector<std::pair<QString, QColor>> chartData);
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
    TitleBar* m_widgetTitleBar;

private slots:
    void TitleBarRequestClose();

public:
    QString Title() const;

signals:
    void RequestClose();
};

#endif // LEGENDWIDGET_H
