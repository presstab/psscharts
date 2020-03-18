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

#include "stringutil.h"

#include <QDateTime>

#include <sstream>
#include <iomanip>

namespace PssCharts {

QString PrecisionToString(double d, int precision)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision);
    stream << d;
    return QString::fromStdString(stream.str());
}

QString TimeStampToString(const uint64_t &nTime)
{
    QDateTime datetime;
    datetime.setSecsSinceEpoch(nTime);
    QDate date = datetime.date();
    return QString("%1/%2/%3").arg(QString::number(date.month()))
            .arg(QString::number(date.day()))
            .arg(QString::number(date.year()));
}

int PrecisionHint(const double& n)
{
    if (n >100)
        return  0;
    else if (n > 10)
        return 1;
    else if (n > 1)
        return 2;
    else if (n > 0.1)
        return 3;
    else if (n > 0.01)
        return 4;
    else if (n > 0.001)
        return 5;
    else if (n > 0.0001)
        return 6;
    else if (n > 0.00001)
        return 7;

     return 8;
}

} //namespace
