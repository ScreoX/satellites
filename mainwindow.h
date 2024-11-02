#ifndef SATELLITIES_MAINWINDOW_H
#define SATELLITIES_MAINWINDOW_H

#include <QMainWindow>

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
    QString findStatistics(QList<Satellite>& satellites);

    Ui::MainWindow *ui;
    QNetworkAccessManager* networkManager;
};

#endif
