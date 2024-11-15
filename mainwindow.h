#ifndef SATELLITIES_MAINWINDOW_H
#define SATELLITIES_MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

#include "tle_parser.h"

namespace Ui { class MainWindow; }

class QNetworkAccessManager;

class MainWindow : public QMainWindow
{
public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:

    void clickOnFileButton();
    void clickOnUrlButton();
    void clickOnSaveButton();
    void networkReplyFinished(QNetworkReply* reply);
    static QString getStatistics(QList<Satellite>& satellites);

    Ui::MainWindow *ui;
    QNetworkAccessManager* networkManager;

    QString m_statistics;
};

#endif
