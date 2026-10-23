#include "Model.h"

#include <QSettings>

void persist(const Model &model){
    QSettings settings( "settings.ini", QSettings::IniFormat );

    settings.beginGroup( "Config" );
    settings.setValue( "Port", model.serialPort() );
    settings.setValue( "Speed", static_cast<int>( model.serialPortSpeed() ) );
    settings.setValue( "RecentSaveDir", model.recentSaveDir() );
    settings.endGroup();

    settings.beginGroup( "Info" );
    settings.setValue( "Pilot", model.pilot() );
    settings.setValue( "Glider", model.glider() );
    settings.setValue( "GliderSerial", model.gliderId() );
    settings.setValue( "Site", model.site() );
    settings.setValue( "Contest", model.contest() );
    settings.endGroup();
}

void load(Model &model){
    QSettings settings( "settings.ini", QSettings::IniFormat );

    settings.beginGroup( "Config" );
#if defined(Q_OS_WIN)
    const QString defaultPort = "COM1";
#else
    const QString defaultPort = "/dev/cu.usbserial";
#endif
    model.serialPort = settings.value( "Port", defaultPort ).toString();
    model.serialPortSpeed = static_cast<Speed>( settings.value( "Speed", 57600 ).toInt() );
    model.recentSaveDir = settings.value( "RecentSaveDir", "" ).toString();
    settings.endGroup();

    settings.beginGroup( "Info" );
    model.pilot = settings.value( "Pilot", "" ).toString();
    model.glider = settings.value( "Glider", "" ).toString();
    model.gliderId = settings.value( "GliderSerial", "" ).toString();
    model.site = settings.value( "Site", "" ).toString();
    model.contest = settings.value( "Contest", "" ).toString();
    settings.endGroup();
}
