#include "httprequest.h"
#include <QList>
#include <QDir>
#include <QXmlStreamReader>

HttpRequest::HttpRequest(FCGX_Request *request)
{
    this->request=request;
    status=waitForBody;
    currentBodySize=0;
    contentLength=0;
    contentType="";
    maxSize = 16000;
    maxMultiPartSize = 1000000;
    parameters.clear();
    bodyData.clear();
    path.clear();
    lastError.clear();

    // Requested URI or path
    QByteArray pathRaw = QByteArray( FCGX_GetParam("REQUEST_URI", request->envp) );
    char *uri = FCGX_GetParam("REQUEST_URI", request->envp) + strlen("/bridge");
    if (uri != NULL)
        this->path = QByteArray(uri);

    //Parse GET parameters
    decodeRequestParams();

    // Check for non-empty body, file upload type of requests
    readHeader();

    qDebug("bridge: (Content-Length: %d) %s", this->contentLength, pathRaw.constData());

    // Parse the entire HTTP request
    while (this->getStatus() != HttpRequest::complete && this->getStatus() != HttpRequest::abort)
    {
        if (status == complete)
            break;

        if (status == waitForBody)
            readBody();

        if (currentBodySize > maxSize)
        {
            qWarning("HttpRequest: received too many bytes");
            status=abort;
        }

        if (status == complete)
        {
            // Extract and decode request parameters from url and body
            decodeRequestParams();

            // Extract and decode request parameters from XML style message
            decodeXMLParams();
        }

        if (this->getStatus()==HttpRequest::waitForBody)
        {
            // Restart timer for read timeout, otherwise it would
            // expire during large file uploads.
            //int readTimeout=10000;
            //readTimer.start(readTimeout);
        }
    }
}

HttpRequest::~HttpRequest()
{
    foreach(QByteArray key, uploadedFiles.keys())
    {
        QTemporaryFile* file = uploadedFiles.value(key);
        if (file != NULL)
            file->close();
        delete file;
        file = NULL;
    }
    uploadedFiles.clear();

    path.clear();
    lastError.clear();
    contentLength = 0;
    contentType.clear();
    currentBodySize = 0;
    bodyData.clear();
    parameters.clear();
    this->request = NULL;
}







/* Request Environment already parsed by FastCGI:
FCGI_ROLE=RESPONDER
SERVER_SOFTWARE=lighttpd/1.4.26-devel-3f7f9af
SERVER_NAME=192.168.1.207
GATEWAY_INTERFACE=CGI/1.1
SERVER_PORT=81
SERVER_ADDR=192.168.1.207
REMOTE_PORT=62409
REMOTE_ADDR=192.168.1.30
CONTENT_LENGTH=36494
SCRIPT_NAME=/bridge
PATH_INFO=
SCRIPT_FILENAME=/usr/share/netvserver/docroot/bridge
DOCUMENT_ROOT=/usr/share/netvserver/docroot/
REQUEST_URI=/bridge
QUERY_STRING=
REQUEST_METHOD=POST
REDIRECT_STATUS=200
SERVER_PROTOCOL=HTTP/1.1
HTTP_HOST=192.168.1.207:81
HTTP_CONNECTION=keep-alive
HTTP_CONTENT_LENGTH=36494
HTTP_CACHE_CONTROL=max-age=0
HTTP_ORIGIN=http://192.168.1.207:81
HTTP_USER_AGENT=Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.75 Safari/535.7
CONTENT_TYPE=multipart/form-data; boundary=----WebKitFormBoundaryWIA16j1tOCg8NcRH
HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,* / *;q=0.8
HTTP_REFERER=http://192.168.1.207:81/html_test/
HTTP_ACCEPT_ENCODING=gzip,deflate,sdch
HTTP_ACCEPT_LANGUAGE=en-GB,en-US;q=0.8,en;q=0.6
HTTP_ACCEPT_CHARSET=ISO-8859-1,utf-8;q=0.7,*;q=0.3
HTTP/1.1 200 OK
*/
void HttpRequest::readHeader()
{
    char *contentLengthRaw = FCGX_GetParam("CONTENT_LENGTH", request->envp);
    if (contentLengthRaw != NULL)
        this->contentLength = (int)strtol(contentLengthRaw, NULL, 10);

    char *contentTypeRaw = FCGX_GetParam("CONTENT_TYPE", request->envp);
    if (contentTypeRaw != NULL)
        this->contentType = QByteArray(contentTypeRaw).toLower();

    // Check for multipart/form-data
    if (contentType.contains("multipart/form-data"))
    {
        int position = contentType.indexOf("boundary=");
        if (position >= 0)
            this->boundary = contentType.mid(position+9);
    }

    // Parsing status
    if (contentLength==0) {
        status=complete;
    }
    else if (boundary.isEmpty() && contentLength+currentBodySize>maxSize) {
        qWarning("HttpRequest: expected body is too large");
        status=abort;
    }
    else if (!boundary.isEmpty() && contentLength>maxMultiPartSize) {
        qWarning("HttpRequest: expected multipart body is too large");
        status=abort;
    }
    else {
        status=waitForBody;
    }
}

void HttpRequest::readBody()
{
    // normal body, no multipart
    if (boundary.isEmpty())
    {
        int toRead=contentLength - bodyData.length();
        char buffer[toRead+1];
        int numRead = FCGX_GetStr(buffer, toRead, request->in);
        QByteArray newData = QByteArray(buffer, numRead);

        currentBodySize += newData.length();
        bodyData.append(newData);
        if (bodyData.size() >= contentLength)
            status=complete;

        return;
    }

    // receive multipart body (file upload) & store into temp file
    if (!tempFile.isOpen())
        tempFile.open();

    // Transfer data in 64kb blocks
    int fileSize=tempFile.size();
    int toRead=contentLength-fileSize;
    if (toRead>65536)
        toRead=65536;

    char buffer2[toRead];
    int rx = FCGX_GetStr(buffer2, toRead, request->in);

    fileSize += tempFile.write(buffer2, rx);
    if (fileSize>=maxMultiPartSize)
    {
        qWarning("HttpRequest: received too many multipart bytes");
        status=abort;
    }
    else if (fileSize>=contentLength)
    {
        //qDebug("HttpRequest: received whole multipart body");
        tempFile.flush();
        if (tempFile.error())
            qCritical("HttpRequest: Error writing temp file for multipart body");

        parseMultiPartFile();
        tempFile.close();
        status=complete;
    }
}

//Extract and decode POST/GET request parameters
void HttpRequest::decodeRequestParams()
{
    QByteArray rawParameters;

    //bodyData should be already UTF-8 encoded here
    //Example: 测试 --> %E6%B5%8B%E8%AF%95

    //NOTE: Mobile Safari will send extra charset like this "application/x-www-form-urlencoded; charset=UTF-8"
    //Uncomment this when we want to deal with charset
    /*
    int semicolonIndex = contentType.indexOf(';');
    QByteArray charset;
    if (semicolonIndex > 0 && contentType.contains("charset"))
        charset = contentType.mid(semicolonIndex+1).trimmed();
    */

    if (contentType.contains("application/x-www-form-urlencoded"))
    {
        //POST
        rawParameters = bodyData;
    }
    else
    {
        //GET
        QByteArray pathRaw = QByteArray( FCGX_GetParam("REQUEST_URI", request->envp) );
        int questionMark = pathRaw.indexOf('?');
        if (questionMark >= 0)
            rawParameters = pathRaw.mid(questionMark+1);
    }

    if (rawParameters.length() <= 0)
        return;

    // Split the parameters into pairs of value   name
    QList<QByteArray> list = rawParameters.split('&');
    foreach (QByteArray part, list)
    {
        int equalsChar = part.indexOf('=');
        if (equalsChar >= 0)
        {
            QByteArray name=part.left(equalsChar).trimmed();
            QByteArray value=part.mid(equalsChar+1).trimmed();
            if (name.startsWith("dataxml_"))
                name = name.right(name.length()-8);
            name = urlDecode(name);
            value = urlDecode(value);
            parameters.insert(name, value);

            qDebug("GET/POST: %s = %s", name.constData(), value.constData());
        }
        else if (!part.isEmpty())
        {
            // Name without value
            if (part.startsWith("dataxml_"))
                part = part.right(part.length()-8);
            part = urlDecode(part);
            parameters.insert(part, "");

            qDebug("GET/POST: %s = %s", part.constData(), "");
        }
    }
}

void HttpRequest::decodeXMLParams()
{
    //No extra param
    QByteArray dataXmlString = this->getParameter("data");
    if (dataXmlString.length() <= 3)
        return;

    //Strip the XML tag if it contains only a single value
    bool singleValueArg = dataXmlString.startsWith("<value>") && dataXmlString.endsWith("</value>");
    if (singleValueArg)
    {
        QByteArray dataString = dataXmlString.mid(7, dataXmlString.length() - 15).trimmed();
        parameters.insert("value", urlDecode(dataString));
        return;
    }

    QXmlStreamReader* xml = new QXmlStreamReader();
    xml->addData(QByteArray("<xml>") + dataXmlString + QByteArray("</xml>"));

    //Input format is documented here
    //https://internal.chumby.com/wiki/index.php/JavaScript/HTML_-_Hardware_Bridge_protocol
    //Example: <xml><cmd>PlayWidget</cmd><data><value>1234567890</value></data></xml>

    QString currentTag;
    bool isFirstElement = true;

    while (!xml->atEnd())
    {
        xml->readNext();

        if (xml->isStartElement() && isFirstElement)
        {
            isFirstElement = false;
        }
        else if (xml->isStartElement())
        {
            currentTag = xml->name().toString().trimmed();

            if (currentTag == "cmd")
                parameters.insert(QByteArray("cmd"), urlDecode(xml->readElementText().toLatin1()));
            else if (currentTag != "data")
                parameters.insert(urlDecode(currentTag.toLatin1()), urlDecode(xml->readElementText().toLatin1()));

            qDebug("GET/POST (XML-format): %s = %s", qPrintable(currentTag), urlDecode(xml->readElementText().toLatin1()).constData() );
        }
    }
    currentTag = "";
    delete xml;
}




HttpRequest::RequestStatus HttpRequest::getStatus() const {
    return status;
}

QByteArray HttpRequest::getLastError() const {
    return lastError;
}

QByteArray HttpRequest::getPath() const {
    return urlDecode(path);
}

QByteArray HttpRequest::getHeader(const QByteArray& name) const
{
    //Read directly from FastCGI environment variable
    char *valueRaw = FCGX_GetParam(name.toUpper().constData(), this->request->envp);
    if (valueRaw != NULL)
        return QByteArray(valueRaw);
    return QByteArray();
}

QByteArray HttpRequest::getParameter(const QByteArray& name) const {
    return parameters.value(name);
}

void HttpRequest::removeParameter(const QByteArray& name) {
    parameters.remove(name);
}

QList<QByteArray> HttpRequest::getParameters(const QByteArray& name) const {
    return parameters.values(name);
}

QMultiMap<QByteArray,QByteArray> HttpRequest::getParameterMap() const {
    return parameters;
}

QByteArray HttpRequest::getParameterMapXML() const {
    QMapIterator<QByteArray, QByteArray> i(getParameterMap());
    QByteArray xmlString;
    while (i.hasNext())
    {
        i.next();
        xmlString += "<" + i.key() + ">" + XMLEscape(i.value()) + "</" + i.key() + ">";
    }
    return QByteArray("<xml>") + xmlString + QByteArray("</xml>");
}

QByteArray HttpRequest::getBody() const {
    return bodyData;
}

QByteArray HttpRequest::urlDecode(const QByteArray source)
{
    QByteArray buffer(source);
    buffer.replace('+',' ');
    int percentChar=buffer.indexOf('%');
    while (percentChar>=0) {
        bool ok;
        char byte=buffer.mid(percentChar+1,2).toInt(&ok,16);
        if (ok)
            buffer.replace(percentChar,3,(char*)&byte,1);
        percentChar=buffer.indexOf('%',percentChar+1);
    }

    return buffer;
}

void HttpRequest::parseMultiPartFile()
{
    tempFile.seek(0);
    bool finished=false;
    while (!tempFile.atEnd() && !finished && !tempFile.error())
    {
        QByteArray fieldName;
        QByteArray fileName;
        while (!tempFile.atEnd() && !finished && !tempFile.error())
        {
            QByteArray line=tempFile.readLine(65536).trimmed();
            if (line.startsWith("Content-Disposition:"))
            {
                if (line.contains("form-data"))
                {
                    int start=line.indexOf(" name=\"");
                    int end=line.indexOf("\"",start+7);
                    if (start>=0 && end>=start)
                        fieldName=line.mid(start+7,end-start-7);
                    start=line.indexOf(" filename=\"");
                    end=line.indexOf("\"",start+11);
                    if (start>=0 && end>=start)
                        fileName=line.mid(start+11,end-start-11);
                }
                else
                {
                    //qDebug("HttpRequest: ignoring unsupported content part %s",line.data());
                }
            }
            else if (line.isEmpty()) {
                break;
            }
        }

        QTemporaryFile* uploadedFile=0;
        QByteArray fieldValue;
        while (!tempFile.atEnd() && !finished && !tempFile.error()) {
            QByteArray line=tempFile.readLine(65536);
            if (line.startsWith("--"+boundary)) {
                // Boundary found. Until now we have collected 2 bytes too much,
                // so remove them from the last result
                if (fileName.isEmpty() && !fieldName.isEmpty()) {
                    // last field was a form field
                    fieldValue.remove(fieldValue.size()-2,2);
                    parameters.insert(fieldName,fieldValue);
                    //qDebug("HttpRequest: set parameter %s=%s",fieldName.data(),fieldValue.data());
                }
                else if (!fileName.isEmpty() && !fieldName.isEmpty())
                {
                    // last field was a file
                    uploadedFile->resize(uploadedFile->size()-2);
                    uploadedFile->flush();
                    uploadedFile->seek(0);
                    parameters.insert(fieldName,fileName);
                    //qDebug("HttpRequest: set parameter %s=%s",fieldName.data(),fileName.data());
                    uploadedFiles.insert(fieldName,uploadedFile);
                    //qDebug("HttpRequest: uploaded file size is %i",(int) uploadedFile->size());
                }

                if (line.contains(boundary+"--"))
                    finished=true;
                break;
            }
            else
            {
                if (fileName.isEmpty() && !fieldName.isEmpty())
                {
                    // this is a form field.
                    currentBodySize+=line.size();
                    fieldValue.append(line);
                }
                else if (!fileName.isEmpty() && !fieldName.isEmpty())
                {
                    // this is a file
                    if (!uploadedFile) {
                        uploadedFile=new QTemporaryFile();
                        uploadedFile->open();
                    }
                    uploadedFile->write(line);
                    if (uploadedFile->error()) {
                        qCritical("HttpRequest: error writing temp file, %s",qPrintable(uploadedFile->errorString()));
                    }
                }
            }
        }
    }

    if (tempFile.error())
        qCritical("HttpRequest: cannot read temp file, %s",qPrintable(tempFile.errorString()));
}

QTemporaryFile* HttpRequest::getUploadedFile(const QByteArray fieldName) {
    return uploadedFiles.value(fieldName);
}

QByteArray HttpRequest::XMLEscape(QByteArray inputString) const
{
    return inputString.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");     //.replace("'", "&apos;");
}

QByteArray HttpRequest::XMLUnescape(QByteArray inputString) const
{
    return inputString.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
}
