#ifndef NHTTPRESPONSE_H
#define NHTTPRESPONSE_H

#include <QObject>
#include "qhttpfwd.hpp"

class NHttpResponse : public QObject
{
    Q_OBJECT
public:
    NHttpResponse(qhttp::server::QHttpResponse *response, QObject *parent = 0);
    ~NHttpResponse();

    /** set the response HTTP status code. @sa TStatusCode.
      * default value is ESTATUS_BAD_REQUEST.
      * @sa write()
      */
   void            setStatusCode(qhttp::TStatusCode code);

public: // QHttpAbstractOutput methods:
   /** @see QHttpAbstractOutput::setVersion(). */
   void            setVersion(const QString& versionString);

   /** @see QHttpAbstractOutput::addHeader(). */
   void            addHeader(const QByteArray& field, const QByteArray& value);

   /** @see QHttpAbstractOutput::headers(). */
   qhttp::THeaderHash&    headers();

   /** @see QHttpAbstractOutput::write(). */
   void            write(const QByteArray &data);

   /** @see QHttpAbstractOutput::write(). */
   void            write(const QString &data);

   /** @see QHttpAbstractOutput::end(). */
   void            end(const QByteArray &data = QByteArray());

   void standardResponse(const QString &command, int result, QString data = QString());

public:
   /** returns the parent QHttpConnection object. */
   qhttp::server::QHttpConnection* connection() const;

private:
    qhttp::server::QHttpResponse *response;
};

#endif // NHTTPRESPONSE_H
