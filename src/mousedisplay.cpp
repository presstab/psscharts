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

#include "mousedisplay.h"

namespace PssCharts {

void MouseDisplay::SetNull()
{
    m_pen = QPen();
    m_pen.setWidth(2);
    m_isEnabled = false;
    m_colorLabelBackground = Qt::lightGray;
}

void MouseDisplay::SetColor(const QColor &color)
{
    m_pen.setColor(color);
}

void MouseDisplay::AddDot(const QPointF &pos, const QColor &color)
{
    m_vDots.emplace_back(MouseDot(color, pos));
}

void MouseDisplay::SetLabelBackgroundColor(const QColor &color)
{
    m_colorLabelBackground = color;
}

void MouseDisplay::SetWidth(int nWidth)
{
    m_pen.setWidth(nWidth);
}

MouseDot MouseDisplay::GetDot(const uint32_t &nSeries)
{
    if (m_vDots.size() >= nSeries+1) {
        return m_vDots.at(nSeries);
    }

    return MouseDot();
}

} //namespace
