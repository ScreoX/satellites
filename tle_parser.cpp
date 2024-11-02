#include "tle_parser.h"

#include <QRegularExpression>


bool TLEParser::parse(const QStringList &lines, QList<Satellite> &satellites, QString &errorMessage) {

    if (lines.size() % 3 != 0) {
        errorMessage = "Incorrect number of lines TLE";
        return false;
    }

    for (int i = 0; i < lines.size(); i += 3) {
        Satellite satellite;
        satellite.name = lines[i].trimmed();

        const QString& line1 = lines[i + 1];
        const QString& line2 = lines[i + 2];

        if (!line1.startsWith("1") || !line2.startsWith("2")) {
            errorMessage = QString("Invalid TLE string format for satellite: %1").arg(satellite.name);
            return false;
        }

        QStringList parts1 = line1.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        if (parts1.size() < 7) {
            errorMessage = QString("Not enough data in the first TLE line for the satellite: %1").arg(satellite.name);
            return false;
        }

        satellite.satelliteNumber = parts1[1];
        QString launchInfo = parts1[2];

        if (launchInfo.length() < 5) {
            errorMessage = QString("Incorrect format of ID: %1").arg(satellite.name);
            return false;
        }

        satellite.launchYear = launchInfo.left(2);
        satellite.launchNumber = launchInfo.mid(2, 3);
        satellite.launchPiece = launchInfo.mid(5, 1);

        QString epochDate = parts1[3];

        bool conversionSuccess;

        int year = epochDate.left(2).toInt(&conversionSuccess);

        if (!conversionSuccess) {
            errorMessage = "Error parsing year";
            return false;
        }

        int dayOfYear = epochDate.mid(2, 3).toInt(&conversionSuccess);

        if (!conversionSuccess) {
            errorMessage = "Error parsing day";
            return false;
        }

        int fullYear = year;

        if (fullYear < 57) {
            fullYear += 2000;
        } else {
            fullYear += 1900;
        }

        QDate date = QDate::fromJulianDay(QDate(fullYear, 1, 1).toJulianDay() + dayOfYear - 1);

        if (!date.isValid()) {
            errorMessage = "Invalid epoch date.";
            return false;
        }

        satellite.date = date;

        QStringList parts2 = line2.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts2.size() < 7) {
            errorMessage = QString("Not enough data in the second TLE line for the satellite: %1").arg(satellite.name);
            return false;
        }

        satellite.inclination = parts2[2].toDouble(&conversionSuccess);

        if (!conversionSuccess) {
            errorMessage = "Error parsing orbital inclination.";
            return false;
        }

        satellite.raan = parts2[3].toDouble(&conversionSuccess);

        if (!conversionSuccess) {
            errorMessage = "Error parsing uplink longitude.";
            return false;
        }

        QString eccentricity = parts2[4];
        eccentricity.insert(1, '.');

        satellite.eccentricity = eccentricity.toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = "Error parsing eccentricity.";
            return false;
        }

        satellite.argPerigee = parts2[5].toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = "Error parsing perigee argument.";
            return false;
        }

        satellite.meanAnomaly = parts2[6].toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = "Error parsing average anomaly.";
            return false;
        }

        satellite.meanMotion = parts2[7].toDouble(&conversionSuccess);
        if (!conversionSuccess) {
            errorMessage = "Error parsing average motion.";
            return false;
        }

        satellites.append(satellite);
    }

    return true;
}
