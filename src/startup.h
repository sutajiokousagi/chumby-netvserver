/**
  @file
  @author Stefan Frings
  @version $Id: startup.h 883 2010-10-14 18:51:33Z stefan $
*/

#ifndef STARTUP_H
#define STARTUP_H

#include <QApplication>
#include "qtservice.h"

/**
  Helper class to install and run the application
*/
class Startup : public QApplication
{
public:

    /** Constructor */
    Startup(int argc, char *argv[]);

protected:

    /** Start the service */
    void start();
};

#endif // STARTUP_H
