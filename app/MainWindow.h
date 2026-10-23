#pragma once

#include "QtCallback.h"
#include "Model.h"
#include "TaskProcessor.h"
#include "TaskProgressDialog.h"

#include <serialPortCom/SerialPort.h>
#include <serialPortCom/ascii/FlightMetaInfo.h>

#include <QListWidgetItem>
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>

#include <map>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( QWidget* parent = 0 );
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_checkButton_clicked();
    void on_refreshButton_clicked();
    void on_saveButton_clicked();

private:
    Ui::MainWindow* ui;
    Model model_;
    TaskProcessor processor_;
    TaskProgressDialog taskProgress_;

    std::map< QListWidgetItem*, FlightMetaInfo > itemInfo_;

    void refreshSerialPorts();
    void sync( Property<QString>& p, QLineEdit* edit );
    void onTaskFinished( TaskPtr task );
};
