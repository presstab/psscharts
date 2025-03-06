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

#include <cctype>
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

QString TimeStampToString(const uint64_t &nTime, const int32_t& nOffset)
{
    QDateTime datetime;
    datetime.setSecsSinceEpoch(static_cast<int64_t>(nTime) + nOffset);
    const QDate& date = datetime.date();
    return QString("%1/%2/%3").arg(QString::number(date.month()))
            .arg(QString::number(date.day()))
            .arg(QString::number(date.year()));
}

QString TimeStampToString_Hours(const uint64_t &nTime, const uint32_t nRange, const int32_t& nOffset)
{
    QDateTime datetime;
    datetime.setSecsSinceEpoch(static_cast<int64_t>(nTime) + nOffset);


    const QTime& time = datetime.time();
    auto hour = time.hour();
    QString strAMPM = "AM";
    if (hour > 12) {
        hour -= 12;
        strAMPM = "PM";
    }

    if (nRange > 60*60*12) {
        //Show date if greater than 12 hours
        return QString("%1/%2 %3 %4").arg(QString::number(datetime.date().month()))
            .arg(QString::number(datetime.date().day()))
            .arg(QString::number(hour))
            .arg(strAMPM);
    } else if (nRange > 60*60) {
        //Don't show seconds if its over an hour length
        return QString("%1:%2 %3").arg(QString::number(hour))
            .arg(QString::number(time.minute()))
            .arg(strAMPM);
    }

    return QString("%1:%2:%3 %4").arg(QString::number(hour))
        .arg(QString::number(time.minute()))
        .arg(QString::number(time.second()))
        .arg(strAMPM);
}

int PrecisionHint(const double& n)
{
    if (n >10000)
        return  0;
    else if (n > 1000)
        return 1;
    else if (n > 100)
        return 2;
    else if (n > 10)
        return 3;
    else if (n > 1)
        return 4;
    else if (n > 0.1)
        return 5;
    else if (n > 0.01)
        return 5;
    else if (n > 0.001)
        return 6;

     return 8;
}

std::string convertToUpper(std::string const &strInput)
{
    std::string strOutput = "";
    for (std::string::size_type i = 0; i < strInput.length(); ++i)
        strOutput += std::toupper(static_cast<unsigned char>(strInput[i]));

    return strOutput;
}

} //namespace
