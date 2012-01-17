/**
  @file
  @author Stefan Frings
  @version $Id: httprequest.cpp 938 2010-12-05 14:29:58Z stefan $
*/

#include "httprequest.h"
#include <QList>
#include <QDir>
#include <QXmlStreamReader>
#include "httpcookie.h"

HttpRequest::HttpRequest(QSettings* settings) {
    status=waitForRequest;
    currentSize=0;
    expectedBodySize=0;
    maxSize=settings->value("maxRequestSize","16000").toInt();
    maxMultiPartSize=settings->value("maxMultiPartSize","1000000").toInt();

    // Convert relative path to absolute, based on the directory of the config file.
#ifdef Q_OS_WIN32
    if (QDir::isRelativePath(tempDir) && settings->format()!=QSettings::NativeFormat)
#else
        if (QDir::isRelativePath(tempDir))
#endif
        {
        QFileInfo configFile(settings->fileName());
        tempDir=QFileInfo(configFile.absolutePath(),tempDir).absoluteFilePath();
    }
}

/* Example:
    GET /path/to/file/index.html HTTP/1.0
    POST /path/to/file/index.html HTTP/1.1
*/

void HttpRequest::readRequest(QTcpSocket& socket)
{
    int toRead=maxSize-currentSize+1; // allow one byte more to be able to detect overflow
    QByteArray newData=socket.readLine(toRead).trimmed();
    currentSize+=newData.size();

    //Do nothing. This allows whitespace characters before HTTP header.
    if (newData.isEmpty())
        return;

    //Should contains at least 2 elements
    QList<QByteArray> list = newData.split(' ');
    if (list.count() < 2) {
        lastError = "Received broken HTTP request, invalid first line";
        qWarning("HttpRequest: received broken HTTP request, invalid first line");
        status=abort;
        return;
    }

    //Invalid HTTP method
    method = list.at(0).toUpper();
    if (method != "GET" && method != "POST") {
        lastError = "Invalid method name " + method + ". Expecting GET,POST";
        qWarning("Invalid method name %s. Expecting GET,POST", method.constData());
        status=abort;
        return;
    }

    //Path
    path=list.at(1);

    //HTTP 1.0 may ommit version number
    version = "HTTP/1.0";
    if (list.count() >= 3)
    {
        QByteArray versionField = list.at(2).toUpper();
        if (versionField.contains("HTTP/1.") && versionField.size() == 8)
            version = versionField;
    }

    status=waitForHeader;
}

/* Example:
    Host: 192.168.1.207
    Origin: http://192.168.1.207
    User-Agent: Mozilla/5.0 (iPad; U; CPU OS 4_3_3 like Mac OS X; en-us) AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 Mobile/8J3 Safari/6533.18.5
    Content-Type: application/x-www-form-urlencoded; charset=UTF-8
    Referer: http://192.168.1.207/html_web/
    Accept: * / *   <---- extra spacing added
    Accept-Language: en-us
    Accept-Encoding: gzip, deflate
    Content-Length: 30
    Connection: keep-alive
    cmd=RemoteControl&value=widget
*/
void HttpRequest::readHeader(QTcpSocket& socket)
{
    int toRead = maxSize-currentSize+1; // allow one byte more to be able to detect overflow
    QByteArray newLine = socket.readLine(toRead).trimmed();
    currentSize += newLine.size();
    int colonIndex=newLine.indexOf(':');

    if (colonIndex>0)
    {
        // Received a line with a colon - a header
        currentHeader = newLine.left(colonIndex);
        QByteArray value = newLine.mid(colonIndex+1).trimmed();
        headers.insert(currentHeader,value);
    }
    else if (!newLine.isEmpty())
    {
        // received another line - belongs to the previous header
        // Received additional line of previous header
        if (headers.contains(currentHeader))
        {
            headers.insert(currentHeader,headers.value(currentHeader)+" "+newLine);
        }
    }
    else
    {
        // received an empty line - end of headers reached
        // Empty line received, that means all headers have been received
        // Check for multipart/form-data
        QByteArray contentType = headers.value("Content-Type");
        if (contentType.toLower().contains("multipart/form-data"))
        {
            int posi=contentType.indexOf("boundary=");
            if (posi>=0) {
                boundary=contentType.mid(posi+9);
            }
        }

        QByteArray contentLength=getHeader("Content-Length");
        if (!contentLength.isEmpty()) {
            expectedBodySize=contentLength.toInt();
        }
        if (expectedBodySize==0) {
            status=complete;
        }
        else if (boundary.isEmpty() && expectedBodySize+currentSize>maxSize) {
            qWarning("HttpRequest: expected body is too large");
            status=abort;
        }
        else if (!boundary.isEmpty() && expectedBodySize>maxMultiPartSize) {
            qWarning("HttpRequest: expected multipart body is too large");
            status=abort;
        }
        else {
            status=waitForBody;
        }
    }
}

void HttpRequest::readBody(QTcpSocket& socket) {
    Q_ASSERT(expectedBodySize!=0);
    if (boundary.isEmpty()) {
        // normal body, no multipart
#ifdef SUPERVERBOSE
        //qDebug("HttpRequest: receive body");
#endif
        int toRead=expectedBodySize-bodyData.size();
        QByteArray newData=socket.read(toRead);
        currentSize+=newData.size();
        bodyData.append(newData);
        if (bodyData.size()>=expectedBodySize) {
            status=complete;
        }
    }
    else {
        // multipart body, store into temp file
#ifdef SUPERVERBOSE
        //qDebug("HttpRequest: receiving multipart body");
#endif
        if (!tempFile.isOpen()) {
            tempFile.open();
        }
        // Transfer data in 64kb blocks
        int fileSize=tempFile.size();
        int toRead=expectedBodySize-fileSize;
        if (toRead>65536) {
            toRead=65536;
        }
        fileSize+=tempFile.write(socket.read(toRead));
        if (fileSize>=maxMultiPartSize) {
            qWarning("HttpRequest: received too many multipart bytes");
            status=abort;
        }
        else if (fileSize>=expectedBodySize) {
#ifdef SUPERVERBOSE
            //qDebug("HttpRequest: received whole multipart body");
#endif
            tempFile.flush();
            if (tempFile.error()) {
                qCritical("HttpRequest: Error writing temp file for multipart body");
            }
            parseMultiPartFile();
            tempFile.close();
            status=complete;
        }
    }
}

//Extract and decode POST/GET request parameters
void HttpRequest::decodeRequestParams()
{
    QByteArray rawParameters;
    QByteArray contentType = headers.value("Content-Type");
    contentType = contentType.toLower();
    //qDebug("NeTVServer: contentType: %s", contentType.constData());

    //bodyData should be already UTF-8 encoded here
    //Example: 测试 --> %E6%B5%8B%E8%AF%95

    //NOTE: Mobile Safari will send extra charset like this "application/x-www-form-urlencoded; charset=UTF-8"
    //Enable this when needed
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
        //qDebug("NeTVServer: body: %s", bodyData.constData());
    }
    else
    {
        //GET
        int questionMark=path.indexOf('?');
        if (questionMark>=0) {
            rawParameters=path.mid(questionMark+1);
            path=path.left(questionMark);
        }
    }

    // Split the parameters into pairs of value and name
    QList<QByteArray> list=rawParameters.split('&');
    foreach (QByteArray part, list)
    {
        int equalsChar=part.indexOf('=');
        if (equalsChar>=0)
        {
            QByteArray name=part.left(equalsChar).trimmed();
            QByteArray value=part.mid(equalsChar+1).trimmed();
            if (name.startsWith("dataxml_"))
                name = name.right(name.length()-8);
            parameters.insert(urlDecode(name),urlDecode(value));
        }
        else if (!part.isEmpty())
        {
            // Name without value
            if (part.startsWith("dataxml_"))
                part = part.right(part.length()-8);
            parameters.insert(urlDecode(part),"");
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
        }
    }
    currentTag = "";
    delete xml;
}

void HttpRequest::extractCookies() {
#ifdef SUPERVERBOSE
    //qDebug("HttpRequest: extract cookies");
#endif
    foreach(QByteArray cookieStr, headers.values("Cookie")) {
        QList<QByteArray> list=HttpCookie::splitCSV(cookieStr);
        foreach(QByteArray part, list) {
#ifdef SUPERVERBOSE
            //qDebug("HttpRequest: found cookie %s",part.data());
#endif                // Split the part into name and value
            QByteArray name;
            QByteArray value;
            int posi=part.indexOf('=');
            if (posi) {
                name=part.left(posi).trimmed();
                value=part.mid(posi+1).trimmed();
            }
            else {
                name=part.trimmed();
                value="";
            }
            cookies.insert(name,value);
        }
    }
    headers.remove("Cookie");
}

void HttpRequest::readFromSocket(QTcpSocket& socket) {
    Q_ASSERT(status!=complete);
    if (status==waitForRequest) {
        readRequest(socket);
    }
    else if (status==waitForHeader) {
        readHeader(socket);
    }
    else if (status==waitForBody) {
        readBody(socket);
    }
    if (currentSize>maxSize) {
        qWarning("HttpRequest: received too many bytes");
        status=abort;
    }
    if (status==complete) {
        // Extract and decode request parameters from url and body
        decodeRequestParams();
        // Extract and decode request parameters from XML style message
        decodeXMLParams();
        // Extract cookies from headers
        extractCookies();
    }
}


HttpRequest::RequestStatus HttpRequest::getStatus() const {
    return status;
}


QByteArray HttpRequest::getLastError() const {
    return lastError;
}


QByteArray HttpRequest::getMethod() const {
    return method;
}


QByteArray HttpRequest::getPath() const {
    return urlDecode(path);
}


QByteArray HttpRequest::getVersion() const {
    return version;
}


QByteArray HttpRequest::getHeader(const QByteArray& name) const {
    return headers.value(name);
}

QList<QByteArray> HttpRequest::getHeaders(const QByteArray& name) const {
    return headers.values(name);
}

QMultiMap<QByteArray,QByteArray> HttpRequest::getHeaderMap() const {
    return headers;
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

QByteArray HttpRequest::urlDecode(const QByteArray source) {
    QByteArray buffer(source);
    buffer.replace('+',' ');
    int percentChar=buffer.indexOf('%');
    while (percentChar>=0) {
        bool ok;
        char byte=buffer.mid(percentChar+1,2).toInt(&ok,16);
        if (ok) {
            buffer.replace(percentChar,3,(char*)&byte,1);
        }
        percentChar=buffer.indexOf('%',percentChar+1);
    }
    return buffer;
}


void HttpRequest::parseMultiPartFile() {
    //qDebug("HttpRequest: parsing multipart temp file");
    tempFile.seek(0);
    bool finished=false;
    while (!tempFile.atEnd() && !finished && !tempFile.error()) {

#ifdef SUPERVERBOSE
        //qDebug("HttpRequest: reading multpart headers");
#endif
        QByteArray fieldName;
        QByteArray fileName;
        while (!tempFile.atEnd() && !finished && !tempFile.error()) {
            QByteArray line=tempFile.readLine(65536).trimmed();
            if (line.startsWith("Content-Disposition:")) {
                if (line.contains("form-data")) {
                    int start=line.indexOf(" name=\"");
                    int end=line.indexOf("\"",start+7);
                    if (start>=0 && end>=start) {
                        fieldName=line.mid(start+7,end-start-7);
                    }
                    start=line.indexOf(" filename=\"");
                    end=line.indexOf("\"",start+11);
                    if (start>=0 && end>=start) {
                        fileName=line.mid(start+11,end-start-11);
                    }
#ifdef SUPERVERBOSE
                    //qDebug("HttpRequest: multipart field=%s, filename=%s",fieldName.data(),fileName.data());
#endif
                }
                else {
                    //qDebug("HttpRequest: ignoring unsupported content part %s",line.data());
                }
            }
            else if (line.isEmpty()) {
                break;
            }
        }

#ifdef SUPERVERBOSE
        //qDebug("HttpRequest: reading multpart data");
#endif
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
                else if (!fileName.isEmpty() && !fieldName.isEmpty()) {
                    // last field was a file
#ifdef SUPERVERBOSE
                    //qDebug("HttpRequest: finishing writing to uploaded file");
#endif
                    uploadedFile->resize(uploadedFile->size()-2);
                    uploadedFile->flush();
                    uploadedFile->seek(0);
                    parameters.insert(fieldName,fileName);
                    //qDebug("HttpRequest: set parameter %s=%s",fieldName.data(),fileName.data());
                    uploadedFiles.insert(fieldName,uploadedFile);
                    //qDebug("HttpRequest: uploaded file size is %i",(int) uploadedFile->size());
                }
                if (line.contains(boundary+"--")) {
                    finished=true;
                }
                break;
            }
            else {
                if (fileName.isEmpty() && !fieldName.isEmpty()) {
                    // this is a form field.
                    currentSize+=line.size();
                    fieldValue.append(line);
                }
                else if (!fileName.isEmpty() && !fieldName.isEmpty()) {
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
    if (tempFile.error()) {
        qCritical("HttpRequest: cannot read temp file, %s",qPrintable(tempFile.errorString()));
    }
#ifdef SUPERVERBOSE
    //qDebug("HttpRequest: finished parsing multipart temp file");
#endif
}

HttpRequest::~HttpRequest() {
    foreach(QByteArray key, uploadedFiles.keys()) {
        QTemporaryFile* file=uploadedFiles.value(key);
        file->close();
        delete file;
    }
}

QTemporaryFile* HttpRequest::getUploadedFile(const QByteArray fieldName) {
    return uploadedFiles.value(fieldName);
}

QByteArray HttpRequest::getCookie(const QByteArray& name) const {
    return cookies.value(name);
}

QMap<QByteArray,QByteArray>& HttpRequest::getCookieMap() {
    return cookies;
}

QByteArray HttpRequest::XMLEscape(QByteArray inputString) const
{
    return inputString.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");     //.replace("'", "&apos;");
}

QByteArray HttpRequest::XMLUnescape(QByteArray inputString) const
{
    return inputString.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
}
