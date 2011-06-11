#include "socketrequest.h"
#include <QXmlStreamReader>
#include <QString>

SocketRequest::SocketRequest(QByteArray data, QByteArray address)
{
    this->commandText = "no-no-no-no-command";
    this->containsError = false;
    this->address = address;

    //Parse the XML string
    this->ParseMessageXML( data );

    //Sanity check
    if (commandText == "" || commandText == "no-no-no-no-command")
        this->containsError = true;
}

SocketRequest::~SocketRequest()
{
    this->containsError = true;
    this->commandText = "";
    this->address = "";
    this->parameters.clear();
}

bool SocketRequest::hasError()
{
    return containsError;
}

QByteArray SocketRequest::getCommand()
{
    return this->commandText;
}

QByteArray SocketRequest::getAddress()
{
    return this->address;
}

QByteArray SocketRequest::getParameter(QByteArray paramName)
{
    return parameters.value(paramName);
}

QMap<QByteArray,QByteArray>& SocketRequest::getParameters()
{
    return parameters;
}

int SocketRequest::getParametersCount()
{
    return parameters.count();
}

void SocketRequest::ParseMessageXML(const char* data)
{
    QXmlStreamReader* xml = new QXmlStreamReader();
    xml->addData(data);

    //Input format is similar to this https://internal.chumby.com/wiki/index.php/JavaScript/HTML_-_Hardware_Bridge_protocol
    //Example: <cmd>PlayWidget</cmd><data><value>1234567890</value></data>

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
                commandText = xml->readElementText().toLatin1();

            else if (currentTag != "data")
                parameters.insert(currentTag.toLatin1(), xml->readElementText().toLatin1());
        }
    }
    currentTag = "";
    delete xml;
}
