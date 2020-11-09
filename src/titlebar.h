#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QFrame>
#include <QWidget>
#include <QLabel>

class QHBoxLayout;
class QPushButton;
class QString;

class TitleBar : public QFrame
{
    Q_OBJECT
private:
    QHBoxLayout* m_hlayoutTitleBar;
    QPushButton* m_buttonClose;
    QLabel* m_title;

public:
    explicit TitleBar(QWidget* parent);

    QPushButton* CloseButton() const;
    void InsertWidget(int pos, QWidget* widget);
    void SetTitle(const QString& strTitle);
    QString Title() const;

private slots:
    void CloseButtonClicked();

signals:
    void CloseRequested();
};

#endif // TITLEBAR_H
