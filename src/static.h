#/**
  @file
  @author Stefan Frings
  @version $Id: static.h 938 2010-12-05 14:29:58Z stefan $
*/

#ifndef STATIC_H
#define STATIC_H

#include <QString>
#include "templatecache.h"
#include "httpsessionstore.h"
#include "controller/staticfilecontroller.h"
#include "controller/scriptcontroller.h"
#include "controller/bridgecontroller.h"

#if defined Q_OS_UNIX && !defined Q_OS_MAC
    #include "controller/cursorcontroller.h"
#endif

/**
  This class contains some static resources that are used by the application.
*/

class Static
{
public:

    /**
      Search the main config file and return its full path.
      On the first call, the INI file gets searched. If not found,
      the application aborts with an error message.
      <p>
      The filename is the applications name plus the ending ".ini". It is searched
      in the following directories:

      - Same directory as the applications executable file
      - In ../etc relative to the applications executable file
      - In ../../etc relative to the applications executable file
      - In /etc/xdg/{organisation name} on the root drive
      - In /etc/opt on the root drive
      - In /etc on the root drive

     */
    static QString getConfigFileName();

    /**
      Gets the directory where the main config file is located.
      On the first call, the INI file gets searched. If not found,
      the application aborts with an error message.
      @see getConfigFileName()
     */
    static QString getConfigDir();

    /** Cache for template files */
    static TemplateLoader* templateLoader;

    /** Storage for session cookies */
    static HttpSessionStore* sessionStore;

    /** Controller for script files */
    static ScriptController* scriptController;

    /** Controller for hardware bridge */
    static BridgeController* bridgeController;

    /** Controller for input device */
#if defined Q_OS_UNIX && !defined Q_OS_MAC
    static CursorController* cursorController;
#endif

    /** Controller for static files */
    static StaticFileController* staticFileController;

private:

    /** Directory of the main config file */
    static QString configDir;

};

#endif // STATIC_H
