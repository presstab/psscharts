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

#ifndef AXISLABELSETTINGS_H
#define AXISLABELSETTINGS_H

#include <QFont>
#include <QSize>

namespace PssCharts {

enum class AxisLabelType;

class AxisLabelSettings
{
private:
    int m_nPrecision; //If label is a number, how many places after the decimal
    bool fAutoPrecision;

public:
    QFont font;
    int nDimension; //Width if Y labels, Height if X Labels
    bool fDynamicSizing; //Whether the dimension should be followed, or should go with a hint
    QSize sizeDynamicDimension;
    AxisLabelType labeltype;
    int32_t timeOffset; //If labeltype is timestamp, can offset time to be in local
    bool fEnabled; //Is the drawing of the axis labels enabled
    bool fPriceDisplay; //Display price instead of % on right side Y Label

    AxisLabelSettings()
    {
        SetNull();
    }

    void SetNull()
    {
        font.setPointSize(8);
        nDimension = 30;
        fDynamicSizing = true;
        sizeDynamicDimension = QSize(0,0);
        m_nPrecision = 2;
        fEnabled = false;
        fAutoPrecision = true;
    }

    void SetPrecision(int n, bool fDisableAutoPrecision)
    {
        m_nPrecision = n;
        if (fDisableAutoPrecision)
            fAutoPrecision = false;
    }

    int Precision() const
    {
        return m_nPrecision;
    }

    void SetAutoPrecision(bool fEnable)
    {
        fAutoPrecision = fEnable;
    }

    bool AutoPrecisionEnabled() const
    {
        return fAutoPrecision;
    }

};

}//namespace

#endif // AXISLABELSETTINGS_H
