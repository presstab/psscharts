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

#ifndef MOUSEDISPLAY_H
#define MOUSEDISPLAY_H

#include <QBrush>
#include <QColor>
#include <QPen>

namespace PssCharts {

class MouseDot
{
private:
    QColor m_color;
    QPointF m_posDot;

public:
    MouseDot()
    {
        m_color = Qt::black;
        m_posDot = QPointF();
    }

    MouseDot(const QColor& color, const QPointF& pos)
    {
        m_color = color;
        m_posDot = pos;
    }

    QColor Color() const { return m_color; }
    QPointF Pos() const { return m_posDot; }
};

class MouseDisplay
{
private:
    bool m_isEnabled;
    QPen m_pen;
    QColor m_colorLabelBackground;

    std::vector<MouseDot> m_vDots;

public:
    MouseDisplay()
    {
        SetNull();
    }

    void SetNull();
    void SetColor(const QColor& color);
    void AddDot(const QPointF& pos, const QColor& color);
    void SetLabelBackgroundColor(const QColor& color);
    void SetEnabled(bool fEnable) { m_isEnabled = fEnable; }
    void SetWidth(int nWidth);
    void ClearDots() { m_vDots.clear(); }

    //QPointF DotPos() const { return m_posDot; }
    MouseDot GetDot(const uint32_t& nSeries);
    std::vector<MouseDot> GetDots() const { return m_vDots; }
    bool IsEnabled() const { return m_isEnabled; }
    QPen Pen() const { return m_pen; }
    QColor LabelBackgroundColor() const { return m_colorLabelBackground; }
};

} //namespace
#endif // MOUSEDISPLAY_H
