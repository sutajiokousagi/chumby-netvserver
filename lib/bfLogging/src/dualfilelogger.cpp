/**
  @file
  @author Stefan Frings
  @version $Id: dualfilelogger.cpp 902 2010-10-19 09:01:12Z stefan $
*/

#include "dualfilelogger.h"


DualFileLogger::DualFileLogger(QSettings* firstSettings, QSettings* secondSettings, const int refreshInterval, QObject* parent)
    :Logger(parent)
{
     firstLogger=new FileLogger(firstSettings, refreshInterval, this);
     secondLogger=new FileLogger(secondSettings, refreshInterval, this);
}


void DualFileLogger::log(const QtMsgType type, const QString& message) {
    firstLogger->log(type, message);
    secondLogger->log(type, message);
}
