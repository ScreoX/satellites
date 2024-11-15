#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>
#include <QDate>
#include <QRegularExpression>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
        , networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    connect(ui->loadUrlButton, &QPushButton::clicked, this, &MainWindow::clickOnUrlButton);
    connect(ui->selectFileButton, &QPushButton::clicked, this, &MainWindow::clickOnFileButton);
    connect(ui->saveStatsButton, &QPushButton::clicked, this, &MainWindow::clickOnSaveButton);

    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::networkReplyFinished);
}


MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getStatistics(QList<Satellite>& satellites) {
    QStringList statisticsList;

    statisticsList << QString("Общее количество спутников: %1").arg(satellites.size());

    if (!satellites.isEmpty()) {
        QDate oldestDate = satellites.first().date;
        for (const auto& satellite : satellites) {
            oldestDate = std::min(satellite.date, oldestDate);
        }
        statisticsList << QString("Дата самых старых данных: %1").arg(oldestDate.toString("dd.MM.yyyy"));
    }

    QMap<int, int> launchesByYear;

    for (const auto& sat : satellites) {
        bool conversionSuccess;
        int year = sat.launchYear.toInt(&conversionSuccess);

        if (conversionSuccess) {
            if (year < 57) {
                year += 2000;
            } else {
                year += 1900;
            }

            if (!launchesByYear.contains(year)) {
                launchesByYear.insert(year, 0);
            }

            launchesByYear[year]++;
        }
    }

    statisticsList << "Количество запущенных спутников по годам:";

    for (auto it = launchesByYear.begin(); it != launchesByYear.end(); ++it) {
        statisticsList << QString("%1: %2").arg(it.key()).arg(it.value());
    }

    QMap<int, int> inclinationDistribution;
    for (const auto& satellite : satellites) {
        int inclinationDegree = qRound(satellite.inclination);

        if (!inclinationDistribution.contains(inclinationDegree)) {
            inclinationDistribution.insert(inclinationDegree, 0);
        }

        inclinationDistribution[inclinationDegree]++;
    }

    statisticsList << "Количество спутников по наклонению орбиты:";

    for (auto it = inclinationDistribution.begin(); it != inclinationDistribution.end(); ++it) {
        statisticsList << QString("%1°: %2").arg(it.key()).arg(it.value());
    }

    return statisticsList.join("\n");
}


void MainWindow::clickOnFileButton()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select file", "", "Text Files (*.txt);");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error with file opening", file.errorString());
        return;
    }

    QTextStream inputData(&file);
    QStringList linesData;

    while (!inputData.atEnd()) {
        QString line = inputData.readLine();
        if (!line.trimmed().isEmpty()) {
            linesData.append(line);
        }
    }

    file.close();

    TLEParser parser;
    QList<Satellite> satellites;
    QString errorMessage;

    if (!parser.parse(linesData, satellites, errorMessage)) {
        QMessageBox::critical(this, "Error with parsing TLE", errorMessage);
        return;
    }

    m_statistics = getStatistics(satellites);

    ui->statsTextEdit->setPlainText(m_statistics);
    ui->statusLabel->setText("Status: Success");
}

void MainWindow::clickOnUrlButton()
{
    QString inputUrl = ui->urlLineEdit->text().trimmed();
    if (inputUrl.isEmpty()) {
        QMessageBox::warning(this, "URL", "Enter URL");
        return;
    }

    QUrl qUrl(inputUrl);
    if (!qUrl.isValid()) {
        QMessageBox::critical(this, "Incorrect URL", "Enter correct URL.");
        return;
    }

    QNetworkRequest request(qUrl);
    networkManager->get(request);
}

void MainWindow::networkReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this, "Error with loading URL", reply->errorString());
        ui->statusLabel->setText("Status: Error with loading URL");
        reply->deleteLater();
        return;
    }

    QByteArray inputData = reply->readAll();

    TLEParser parser;
    QList<Satellite> satellites;
    QString errorMessage;

    if (!parser.parse(QString(inputData).split(QRegularExpression("\\r?\\n"), Qt::SkipEmptyParts), satellites, errorMessage)) {
        QMessageBox::critical(this, "Error with parsing TLE", errorMessage);
        ui->statusLabel->setText("Status: error with parsing");
        reply->deleteLater();
        return;
    }

    m_statistics = getStatistics(satellites);

    ui->statsTextEdit->setPlainText(m_statistics);
    ui->statusLabel->setText("Status: Success");
    reply->deleteLater();
}

void MainWindow::clickOnSaveButton()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить статистику", "", "Text Files (*.txt);");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error with saving file", file.errorString());
        return;
    }

    QTextStream out(&file);
    out << m_statistics;
    file.close();

    QMessageBox::information(this, "Saving statistics", "Success of saving statistics");
}
