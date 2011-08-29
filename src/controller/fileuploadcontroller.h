#ifndef FILEUPLOADCONTROLLER_H
#define FILEUPLOADCONTROLLER_H

#include "httprequesthandler.h"

/**
  This controller displays a HTML form for file upload and recieved the file.
*/

class FileUploadController : public HttpRequestHandler
{

public:

    /** Constructor */
    FileUploadController();

    /** Generates the response */
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // FILEUPLOADCONTROLLER_H
