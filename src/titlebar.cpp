#include "titlebar.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QLabel>

TitleBar::TitleBar(QWidget *parent) : QFrame(parent)
{
    this->setObjectName("SideBarWidgetTitleBar");
    this->setFrameStyle(QFrame::StyledPanel);
    this->setContentsMargins(1,1,1,1);
    m_hlayoutTitleBar = new QHBoxLayout(this);
    m_hlayoutTitleBar->setContentsMargins(3,3,3,3);
    m_hlayoutTitleBar->setSpacing(0);
    m_title = new QLabel(this);
    m_title->setObjectName("TitleBar-TitleLabel");
    m_hlayoutTitleBar->addWidget(m_title);
    m_hlayoutTitleBar->addStretch();

    m_buttonClose = new QPushButton(this);
    m_hlayoutTitleBar->addWidget(m_buttonClose);
    QIcon icon(":/icons/x-circle.png");
    m_buttonClose->setIcon(icon);

    connect(m_buttonClose, &QPushButton::clicked, this, &TitleBar::CloseButtonClicked);
}

QPushButton *TitleBar::CloseButton() const
{
    return m_buttonClose;
}

void TitleBar::InsertWidget(int pos, QWidget *widget)
{
    m_hlayoutTitleBar->insertWidget(pos, widget);
}

void TitleBar::SetTitle(const QString &strTitle)
{
    m_title->setText(strTitle);
}

QString TitleBar::Title() const
{
    return m_title->text();
}

void TitleBar::CloseButtonClicked()
{
    emit CloseRequested();
}
