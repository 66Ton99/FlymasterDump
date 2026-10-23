#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "CheckDeviceTask.h"
#include "DumpTask.h"
#include "FlightListTask.h"

#include <serialPortCom/Request.h>
#include <serialPortCom/SerialPort.h>

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>


MainWindow::MainWindow( QWidget* parent )
    : QMainWindow( parent ), ui( new Ui::MainWindow ), taskProgress_( processor_, this ) {
    ui->setupUi( this );

    ui->baudComboBox->addItem( "9600", static_cast< unsigned int >( Speed::BAUD9600 ) );
    ui->baudComboBox->addItem( "57600", static_cast< unsigned int >( Speed::BAUD57600 ) );
    ui->baudComboBox->addItem( "115200", static_cast< unsigned int >( Speed::BAUD115200 ) );
    ui->baudComboBox->setCurrentIndex( 2 );
    connectProperty( model_.serialPortSpeed, this, [&]() {
        auto idx = ui->baudComboBox->findData( static_cast< unsigned int >( model_.serialPortSpeed() ) );
        ui->baudComboBox->setCurrentIndex( idx );
    } );
    connect( ui->baudComboBox, &QComboBox::currentTextChanged, this, [&]() {
        model_.serialPortSpeed = static_cast< Speed >( ui->baudComboBox->currentData().toInt() );
    } );

    load( model_ );

    ui->comComboBox->setEditable( true );
    refreshSerialPorts();

    connectProperty( model_.serialPort, this, [&]() {
        auto idx = ui->comComboBox->findData( model_.serialPort() );
        if( idx != -1 ) {
            ui->comComboBox->setCurrentIndex( idx );
        } else {
            ui->comComboBox->setEditText( model_.serialPort() );
        }
    } );
    connect( ui->comComboBox, &QComboBox::currentTextChanged, this, [&]( const QString& text ) {
        auto data = ui->comComboBox->currentData();
        if( data.isValid() && !data.toString().isEmpty()
            && text == ui->comComboBox->itemText( ui->comComboBox->currentIndex() ) ) {
            model_.serialPort = data.toString();
        } else {
            model_.serialPort = text;
        }
    } );

    sync( model_.contest, ui->contestEdit );
    sync( model_.glider, ui->gliderEdit );
    sync( model_.gliderId, ui->gliderIdEdit );
    sync( model_.pilot, ui->pilotEdit );
    sync( model_.site, ui->siteEdit );

    connect( &processor_, &TaskProcessor::finishedTask, this, &MainWindow::onTaskFinished );
    ui->statusbar->showMessage( tr( "Ready" ) );
}

MainWindow::~MainWindow() {
    persist( model_ );
    delete ui;
}


void MainWindow::refreshSerialPorts() {
    QString current = model_.serialPort();
    ui->comComboBox->blockSignals( true );
    ui->comComboBox->clear();

#if defined( Q_OS_WIN )
    for( int i = 1; i <= 10; ++i ) {
        QString portName = QString( "COM%1" ).arg( i );
        ui->comComboBox->addItem( portName, portName );
    }
#else
    QDir devDir( "/dev" );
    // Спочатку шукаємо справжні USB-Serial адаптери (FTDI, CP210x тощо)
    QStringList usbFilters;
    usbFilters << "cu.usbserial*" << "cu.SLAB*" << "cu.usbmodem*" << "ttyUSB*" << "ttyACM*";
    QFileInfoList usbEntries = devDir.entryInfoList( usbFilters, QDir::System );
    for( const auto& entry : usbEntries ) {
        ui->comComboBox->addItem( entry.fileName(), entry.absoluteFilePath() );
    }

    // Додаємо інші cu.* порти нижче
    QStringList otherFilters;
    otherFilters << "cu.*";
    QFileInfoList otherEntries = devDir.entryInfoList( otherFilters, QDir::System );
    for( const auto& entry : otherEntries ) {
        if( ui->comComboBox->findData( entry.absoluteFilePath() ) == -1 ) {
            ui->comComboBox->addItem( entry.fileName(), entry.absoluteFilePath() );
        }
    }

    // Якщо збережений порт не існує або порожній, а у нас є реальний USB-Serial порт:
    if( ( !QFile::exists( current ) || current.isEmpty() ) && !usbEntries.isEmpty() ) {
        current = usbEntries.first().absoluteFilePath();
        model_.serialPort = current;
    }
#endif

    // Відновлюємо вибраний або автоматично вибираємо перший знайдений USB порт
    int idx = ui->comComboBox->findData( current );
    if( idx != -1 ) {
        ui->comComboBox->setCurrentIndex( idx );
    } else if( !current.isEmpty() ) {
        ui->comComboBox->setEditText( current );
    } else if( ui->comComboBox->count() > 0 ) {
        ui->comComboBox->setCurrentIndex( 0 );
        model_.serialPort = ui->comComboBox->currentData().toString();
    }
    ui->comComboBox->blockSignals( false );
}

void MainWindow::on_refreshButton_clicked() {
    refreshSerialPorts();
}

void MainWindow::on_checkButton_clicked() {
    QString text = ui->comComboBox->currentText().trimmed();
    auto data = ui->comComboBox->currentData();
    if( data.isValid() && !data.toString().isEmpty()
        && text == ui->comComboBox->itemText( ui->comComboBox->currentIndex() ) ) {
        model_.serialPort = data.toString();
    } else if( !text.isEmpty() ) {
        model_.serialPort = text;
    }

    ui->statusbar->showMessage( tr( "Checking device on %1..." ).arg( model_.serialPort() ) );
    TaskPtr task = std::make_shared< CheckDeviceTask >( model_ );
    processor_.schedule( task );
}

void MainWindow::on_connectButton_clicked() {
    QString text = ui->comComboBox->currentText().trimmed();
    auto data = ui->comComboBox->currentData();
    if( data.isValid() && !data.toString().isEmpty()
        && text == ui->comComboBox->itemText( ui->comComboBox->currentIndex() ) ) {
        model_.serialPort = data.toString();
    } else if( !text.isEmpty() ) {
        model_.serialPort = text;
    }

    TaskPtr flt = std::make_shared< FlightListTask >( model_ );
    processor_.schedule( flt );
}

void MainWindow::on_saveButton_clicked() {
    auto dir = QFileDialog::getExistingDirectory( this, "Folder to save IGC files", model_.recentSaveDir() );
    if( dir.isEmpty() ) {
        return;
    }
    model_.recentSaveDir = dir;
    for( int i = 0; i < ui->listWidget->count(); ++i ) {
        auto item = ui->listWidget->item( i );
        if( item->checkState() == Qt::Checked ) {
            auto task = std::make_shared< DumpTask >( dir, itemInfo_.at( item ), model_ );
            processor_.schedule( task );
        }
    }
}

void MainWindow::sync( Property< QString >& p, QLineEdit* edit ) {
    edit->setText( p() );
    connect( &p, &Property< QString >::propertyChanged, edit, [&, edit]() {
        if( edit->text() != p() ) {
            edit->setText( p() );
        }
    } );
    connect( edit, &QLineEdit::textChanged, this, [&]( const QString& text ) { p = text; } );
}

void MainWindow::onTaskFinished( TaskPtr task ) {
    DumpTask* d = dynamic_cast< DumpTask* >( task.get() );
    if( d != nullptr ) {
        QListWidgetItem* item = 0;
        for( const auto& p : itemInfo_ ) {
            if( p.second.index == d->info().index ) {
                item = p.first;
            }
        }
        if( item == nullptr ) {
            return;
        }
        if( task->callback().count( Severity::Critical ) == 0 && !task->callback().isCancelled() ) {
            item->setIcon( style()->standardIcon( QStyle::SP_DialogApplyButton ) );
            item->setToolTip( "Successfully stored IGC file" );
            item->setCheckState( Qt::Unchecked );
            item->setFlags( item->flags() ^ Qt::ItemIsUserCheckable ^ Qt::ItemIsEnabled );
        } else {
            item->setIcon( style()->standardIcon( QStyle::SP_MessageBoxCritical ) );
            item->setToolTip( task->callback().messages( Severity::Critical ).join( "\n" ) );
        }
    }

    FlightListTask* flt = dynamic_cast< FlightListTask* >( task.get() );
    if( flt != nullptr ) {
        if( task->callback().count( Severity::Critical ) == 0 ) {
            ui->listWidget->clear();
            for( size_t i = 0; i < flt->infos().size(); ++i ) {
                QListWidgetItem* item = new QListWidgetItem( flt->list()[i], ui->listWidget );
                item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
                item->setCheckState( Qt::Unchecked );
                itemInfo_.insert( std::make_pair( item, flt->infos()[i] ) );
            }
            ui->saveButton->setEnabled( true );
            ui->statusbar->showMessage( tr( "Loaded %1 flights from device" ).arg( flt->infos().size() ) );
        } else {
            ui->statusbar->showMessage( tr( "Connection failed on %1" ).arg( model_.serialPort() ) );
            QStringList errors = task->callback().messages( Severity::Critical );
            QString errorMsg = errors.isEmpty() ? tr( "No response from device." ) : errors.join( "\n" );
            QMessageBox::critical( this,
                                   "Connection Error",
                                   tr( "Unable to connect to %1 with %2 baud.\n\nError: %3\n\nPlease check:\n1. "
                                       "Flymaster is connected and POWERED ON\n2. USB cable supports data transfer\n3. "
                                       "Correct serial port is selected" )
                                       .arg( model_.serialPort() )
                                       .arg( static_cast< unsigned int >( model_.serialPortSpeed() ) )
                                       .arg( errorMsg ) );
        }
    }

    CheckDeviceTask* cdt = dynamic_cast< CheckDeviceTask* >( task.get() );
    if( cdt != nullptr ) {
        if( task->callback().count( Severity::Critical ) == 0 ) {
            const auto& info = cdt->deviceInfo();
            QString modelName = info.model.empty() ? QString( "Flymaster" ) : QString::fromStdString( info.model );
            QString sn = QString::fromStdString( info.serialNumber );
            QString fw = QString::fromStdString( info.firmware );
            QString pr = QString::fromStdString( info.pressure );

            ui->statusbar->showMessage( tr( "Connected: %1 (SN: %2, FW: %3)" ).arg( modelName ).arg( sn ).arg( fw ) );

            QString msg = tr( "<h3>Flymaster Device Verified!</h3>"
                              "<p><b>Model:</b> %1<br>"
                              "<b>Serial Number:</b> %2<br>"
                              "<b>Firmware:</b> %3<br>"
                              "<b>Atmospheric Pressure:</b> %4 hPa<br>"
                              "<b>Port:</b> %5<br>"
                              "<b>Speed:</b> %6 baud</p>"
                              "<p style='color:green;'><b>Status: Connected and responding normally.</b></p>" )
                              .arg( modelName )
                              .arg( sn.isEmpty() ? "N/A" : sn )
                              .arg( fw.isEmpty() ? "N/A" : fw )
                              .arg( pr.isEmpty() ? "N/A" : pr )
                              .arg( model_.serialPort() )
                              .arg( static_cast< unsigned int >( model_.serialPortSpeed() ) );

            QMessageBox::information( this, tr( "Device Info" ), msg );
        } else {
            ui->statusbar->showMessage( tr( "Device check failed on %1" ).arg( model_.serialPort() ) );
            QStringList errors = task->callback().messages( Severity::Critical );
            QString errorMsg = errors.isEmpty() ? tr( "No response from device." ) : errors.join( "\n" );
            QMessageBox::critical( this,
                                   tr( "Device Check Failed" ),
                                   tr( "Unable to communicate with Flymaster on %1 with %2 baud.\n\nError: "
                                       "%3\n\nPlease check:\n1. Flymaster is connected and POWERED ON\n2. USB cable "
                                       "supports data transfer\n3. Correct serial port is selected" )
                                       .arg( model_.serialPort() )
                                       .arg( static_cast< unsigned int >( model_.serialPortSpeed() ) )
                                       .arg( errorMsg ) );
        }
    }
}
