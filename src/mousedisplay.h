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

class MouseDisplay
{
private:
    bool m_isEnabled;
    QPen m_pen;
    QColor m_colorLabelBackground;
    QPointF m_posDot;

public:
    MouseDisplay()
    {
        SetNull();
    }

    void SetNull();
    void SetColor(const QColor& color);
    void SetDot(const QPointF& pos) { m_posDot = pos; }
    void SetLabelBackgroundColor(const QColor& color);
    void SetEnabled(bool fEnable) { m_isEnabled = fEnable; }
    void SetWidth(int nWidth);

    QPointF DotPos() const { return m_posDot; }
    bool IsEnabled() const { return m_isEnabled; }
    QPen Pen() const { return m_pen; }
    QColor LabelBackgroundColor() const { return m_colorLabelBackground; }
};

} //namespace
#endif // MOUSEDISPLAY_H
