#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QByteArray>
#include <QMap>
#include <QMultiMap>
#include <QSettings>
#include <QTemporaryFile>
#include <QUuid>
#include <fcgiapp.h>

class HttpRequest
{
    Q_DISABLE_COPY(HttpRequest)
    friend class HttpSessionStore;
public:

    /** Values for getStatus() */
    enum RequestStatus {waitForBody, complete, abort};

    /**
      Constructor.
      @param settings Configuration settings
    */
    HttpRequest(FCGX_Request *request);

    /**
      Destructor.
    */
    virtual ~HttpRequest();

    /** Get the last error message (e.g. "invalid header") */
    QByteArray getLastError() const;

    /**
      Get the parsing status of this request
      @see RequestStatus
    */
    RequestStatus getStatus() const;

    /** Get the method of the HTTP request  (e.g. "GET") */
    QByteArray getMethod() const;

    /** Get the decoded path of the HTPP request (e.g. "/index.html") */
    QByteArray getPath() const;

    /** Get the version of the HTPP request (e.g. "HTTP/1.1") */
    QByteArray getVersion() const;

    /**
      Get the value of a HTTP request header.
      @param name Name of the header
      @return If the header occurs multiple times, only the last
      one is returned.
    */
    QByteArray getHeader(const QByteArray& name) const;

    /**
      Get the value of a HTTP request parameter.
      @param name Name of the parameter
      @return If the parameter occurs multiple times, only the last
      one is returned.
    */
    QByteArray getParameter(const QByteArray& name) const;

    /**
      Remove a HTTP request parameter off the parameters map.
      @param name Name of the parameter
    */
    void removeParameter(const QByteArray& name);

    /**
      Get the values of a HTTP request parameter.
      @param name Name of the parameter
    */
    QList<QByteArray> getParameters(const QByteArray& name) const;

    /** Get all HTTP request parameters */
    QMultiMap<QByteArray,QByteArray> getParameterMap() const;

    /** Get all HTTP request parameters as a XML string */
    QByteArray getParameterMapXML() const;

    /** Get the HTTP request body  */
    QByteArray getBody() const;

    /**
      Decode an URL parameter.
      E.g. replace "%23" by '#' and replace '+' by ' '
      @param source The url encoded string
    */
    static QByteArray urlDecode(const QByteArray source);

    /**
      Get an uploaded file. The file is already open. It will
      be closed and deleted by the destructor of this HttpRequest
      object (after processing the request).
      <p>
      For uploaded files, the method getParameters() returns
      the original fileName as provided by the calling web browser.
    */
    QTemporaryFile* getUploadedFile(const QByteArray fieldName);

private:

    /** FastCGI request for writing output */
    FCGX_Request* request;

    /** Request path (in raw encoded format) */
    QByteArray path;

    /** Last error message */
    QByteArray lastError;

    /** HTTP Header: Content-Type */
    QByteArray contentType;

    /** Parameters of the request */
    QMultiMap<QByteArray,QByteArray> parameters;

    /** Uploaded files of the request, key is the field name. */
    QMap<QByteArray,QTemporaryFile*> uploadedFiles;

    /** Storage for raw body data */
    QByteArray bodyData;

    /**
      Status of this request.
      @see RequestStatus
    */
    RequestStatus status;

    /** Maximum size of requests in bytes. */
    int maxSize;

    /** Maximum allowed size of multipart forms in bytes. */
    int maxMultiPartSize;

    /** Directory for temp files */
    QString tempDir;

    /** Current received body data size */
    int currentBodySize;

    /** Expected size of body */
    int contentLength;

    /** Boundary of multipart/form-data body. Empty if there is no such header */
    QByteArray boundary;

    /** Temp file, that is used to store the multipart/form-data body */
    QTemporaryFile tempFile;

    /** Parset the multipart body, that has been stored in the temp file. */
    void parseMultiPartFile();

    /** Sub-procedure of readFromSocket(), read header lines. */
    void readHeader();

    /** Sub-procedure of readFromSocket(), read the request body. */
    void readBody();

    /** Sub-procedure of readFromSocket(), extract and decode request parameters. */
    void decodeRequestParams();

    /** Sub-procedure of decodeXMLParams(), extract and decode request parameters in XML format. */
    void decodeXMLParams();

    /** Escape special XML characters */
    QByteArray XMLEscape(QByteArray inputString) const;

    /** Unescape special XML characters */
    QByteArray XMLUnescape(QByteArray inputString) const;
};

#endif // HTTPREQUEST_H
