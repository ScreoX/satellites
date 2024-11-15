#include "tle_parser.h"

#include <QRegularExpression>
#include <QDate>

bool TLEParser::parse(const QStringList &lines, QList<Satellite> &satellites, QString &errorMessage) {

    if (lines.isEmpty()) {
        errorMessage = "Входные данные пусты.";
        return false;
    }

    bool hasName = !lines[0].startsWith("1") && !lines[0].startsWith("2");
    int linesPerSatellite = hasName ? 3 : 2;

    if (lines.size() % linesPerSatellite != 0) {
        errorMessage = QString("Некорректное количество строк в данных TLE. Ожидалось кратное %1 строкам на спутник.").arg(linesPerSatellite);
        return false;
    }

    QRegularExpression regexLine1(
            R"(^1\s+(\d{5})([A-Z])\s+(\d{2})(\d{3})([A-Z]{0,3})\s+(\d{5}\.\d{8})\s+([\d\.\+\-]+)\s+([\d\.\+\-]+)\s+([\d\.\+\-]+)\s+(\d)\s+(\d{4})$)"
    );

    QRegularExpression regexLine2(
            R"(^2\s+(\d{5})\s+(\d{2}\.\d{4})\s+(\d{2,3}\.\d{4})\s+(\d{7})\s+(\d{2,3}\.\d{4})\s+(\d{1,3}\.\d{4})\s+(\d{1,3}\.\d{8})\s+(\d{2})$)"
    );

    for (int i = 0; i < lines.size(); ) {
        Satellite satellite;

        if (hasName) {
            satellite.name = lines[i].trimmed();
            i++;
        }

        if (i + 1 >= lines.size()) {
            errorMessage = "Некорректный формат данных: недостаточно строк для парсинга TLE.";
            return false;
        }

        const QString& line1 = lines[i];
        const QString& line2 = lines[i + 1];
        i += 2;

        if (!line1.startsWith("1") || !line2.startsWith("2")) {
            errorMessage = QString("Некорректный формат строк TLE для спутника.");
            return false;
        }

        QRegularExpressionMatch matchLine1 = regexLine1.match(line1);
        if (!matchLine1.hasMatch()) {
            errorMessage = "Не удалось распарсить первую строку TLE.";
            return false;
        }

        QString NORAD_ID = matchLine1.captured(1);
        QString launchYear = matchLine1.captured(3);
        QString launchNumber = matchLine1.captured(4);
        QString launchPiece = matchLine1.captured(5);
        QString epoch = matchLine1.captured(6);

        bool conversionSuccess;
        satellite.NORAD = NORAD_ID.toInt(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге NORAD ID для спутника: %1").arg(NORAD_ID);
            return false;
        }

        satellite.launchYear = launchYear;
        satellite.launchNumber = launchNumber;
        satellite.launchPiece = launchPiece;

        QRegularExpressionMatch matchLine2 = regexLine2.match(line2);
        if (!matchLine2.hasMatch()) {
            errorMessage = QString("Не удалось распарсить вторую строку TLE для спутника NORAD: %1").arg(satellite.NORAD);
            return false;
        }

        QString NORAD_ID_Line2 = matchLine2.captured(1);
        QString inclinationStr = matchLine2.captured(2);
        QString raanStr = matchLine2.captured(3);
        QString eccentricityStr = matchLine2.captured(4);
        QString argPerigeeStr = matchLine2.captured(5);
        QString meanAnomalyStr = matchLine2.captured(6);
        QString meanMotionStr = matchLine2.captured(7);

        if (NORAD_ID != NORAD_ID_Line2) {
            errorMessage = QString("Несоответствие NORAD ID между Line 1 и Line 2 для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        satellite.inclination = inclinationStr.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге наклонения орбиты для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        satellite.raan = raanStr.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге правого восхождения восходящего узла для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        if (eccentricityStr.length() != 7) {
            errorMessage = QString("Некорректный формат эксцентриситета для спутника: %1").arg(satellite.NORAD);
            return false;
        }
        QString formattedEccentricity = eccentricityStr;
        formattedEccentricity.insert(1, '.');
        satellite.eccentricity = formattedEccentricity.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге эксцентриситета для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        satellite.argPerigee = argPerigeeStr.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге аргумента перигея для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        satellite.meanAnomaly = meanAnomalyStr.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге средней аномалии для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        satellite.meanMotion = meanMotionStr.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге среднего движения для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        QString epochDateStr = epoch;
        if (epochDateStr.length() < 5) {
            errorMessage = QString("Некорректный формат даты эпохи для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        int year = epochDateStr.left(2).toInt(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге года эпохи для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        int dayOfYear = epochDateStr.mid(2, 3).toInt(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = QString("Ошибка при парсинге дня года для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        int fullYear = (year < 57) ? year + 2000 : year + 1900;

        QDate date = QDate::fromJulianDay(QDate(fullYear, 1, 1).toJulianDay() + dayOfYear - 1);
        if (!date.isValid()) {
            errorMessage = QString("Неверная дата эпохи для спутника: %1").arg(satellite.NORAD);
            return false;
        }

        satellite.date = date;

        satellites.append(satellite);
    }

    return true;
}
