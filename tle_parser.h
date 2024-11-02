#ifndef SATELLITIES_TLE_PARSER_H
#define SATELLITIES_TLE_PARSER_H

#include <QString>
#include <QList>
#include <QDate>

struct Satellite {
    QString name;
    QString satelliteNumber;
    QString launchYear;
    QString launchNumber;
    QString launchPiece;
    QDate date;
    double inclination;
    double raan;
    double eccentricity;
    double argPerigee;
    double meanAnomaly;
    double meanMotion;
};

class TLEParser {
public:

    bool parse(const QStringList& lines, QList<Satellite>& satellites, QString& errorMsg);

};

#endif
