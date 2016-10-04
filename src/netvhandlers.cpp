#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QMimeDatabase>

#include <QXmlStreamReader>

#include "netvhandlers.h"
#include "netvserverapplication.h"
#include "nhttprequest.h"
#include "nhttpresponse.h"
#include "geturlfinisher.h"

#include "qhttpclient.hpp"
#include "qhttpclientresponse.hpp"

BEGIN_NETV_HANDLER(handleDefault, cmd, app, request, response)
{
    response->standardResponse(cmd, NETV_STATUS_UNIMPLEMENTED, "Unhandled command received");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetUrl, cmd, app, request, response)
{
    QString postData = request->parameter("post");
    QString urlString = request->parameter("url");

    if (urlString.isEmpty()) {
        response->standardResponse(cmd, NETV_STATUS_ERROR, QString("No \"url\" specified"));
        return 0;
    }

    QUrl url = QUrl::fromPercentEncoding(urlString.toUtf8());

    if (!url.isValid()) {
        qDebug() << "Invalid URL:" << urlString;
        response->standardResponse(cmd, NETV_STATUS_ERROR, QString("Invalid URL: %1").arg(url.errorString()));
        return 0;
    }

    if (url.host().isEmpty()) {
        response->standardResponse(cmd, NETV_STATUS_ERROR, QString("No host specified"));
        return 0;
    }

    if (url.scheme() == "")
        url.setScheme("http");

    if ((url.scheme() != "http") && (url.scheme() != "https")) {
        response->standardResponse(cmd, NETV_STATUS_ERROR, QString("Only http and https are supported"));
        return 0;
    }

    if (url.port() == -1) {
        if (url.scheme() == "https")
            url.setPort(443);
        else
            url.setPort(80);
    }

    qDebug() << "Getting URL:" << url << "Path:" << url.path() << "Host:" << url.host() << " Port:" << url.port() << " Scheme:" << url.scheme();

    QNetworkAccessManager *accessManager = new QNetworkAccessManager(response);
    GetUrlFinisher *urlFinisher = new GetUrlFinisher(cmd, response, response);
    QNetworkRequest request(url);

    QObject::connect(accessManager, SIGNAL(finished(QNetworkReply*)),
                     urlFinisher, SLOT(responseFinished(QNetworkReply*)));
    accessManager->get(request);

    // Return 1, to indicate to the caller that response->end() shouldn't be called.
    return 1;
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetJpeg, cmd, app, request, response)
{
    //QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString), xmlEscape);
    response->standardResponse(cmd, NETV_STATUS_SUCCESS, "get jpeg unimplemented");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetLocalFileContents, cmd, app, request, response)
{
    response->standardResponse(cmd, NETV_STATUS_ERROR, "command no longer implemented");
    return 0;
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetChannelInfo, cmd, app, request, response)
{
    QFileInfo targetInfo(QFileInfo(app->staticDocRoot().canonicalPath() + QDir::separator() + "widgets" + QDir::separator() + "channelinfo.xml"));

    if (!targetInfo.exists() || targetInfo.isDir()) {
        response->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
        response->standardResponse(cmd, NETV_STATUS_ERROR, "File not found");
        return 0;
    }

    QFile targetFile(targetInfo.canonicalFilePath());
    if (!targetFile.open(QIODevice::ReadOnly)) {
        response->setStatusCode(qhttp::ESTATUS_FORBIDDEN);
        response->standardResponse(cmd, NETV_STATUS_ERROR, QString("Unable to open file: %1").arg(targetFile.errorString()));
        return 0;
    }

    QMimeDatabase db;
    response->addHeader("Content-Type", db.mimeTypeForFile(targetInfo).name().toUtf8());

    response->standardResponseHeader(cmd, NETV_STATUS_SUCCESS);
    while (!targetFile.atEnd()) {
        QByteArray data = targetFile.read(2048);
        response->write(data);
    }
    targetFile.close();
    response->standardResponseFooter();

    return 0;
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleChromaKey, cmd, app, request, response)
{
    //Contruct arguments
    QVariant dataString(request->parameter("value"));
    QStringList argList;

    if (dataString == "on" || dataString == "true" || dataString == "yes")
        argList.append(QString("on"));
    else if (dataString == "off" || dataString == "false" || dataString == "no")
        argList.append(QString("off"));

    //Execute the script
    if (argList.size() <= 0)
    {
        response->standardResponse(cmd, NETV_STATUS_ERROR, "Invalid arguments");
    }
    else
    {
        //QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList, xmlEscape);
        QByteArray buffer;
        response->standardResponse(cmd, NETV_STATUS_SUCCESS, buffer.trimmed());
    }
    return 0;
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleEnableSsh, cmd, app, request, response)
{
    /*
    QString data
    if (dataString.length() < 1)
        dataString = "start-chumby";
    QByteArray buffer = this->Execute("/etc/init.d/sshd", QStringList(QString(dataString)), xmlEscape);

    response->write(QByteArray("<xml><status>") + BRIDGE_RETURN_STATUS_SUCCESS + "</status><cmd>" + cmdString + "</cmd><data><value>" + buffer.trimmed() + "</value></data></xml>");
    buffer = QByteArray();
    */
    response->standardResponse(cmd, NETV_STATUS_UNIMPLEMENTED, "ssh enable not yet implemented");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleSetDocRoot, cmd, app, request, response)
{
    QString current_docroot = app->setStaticDocRoot(request->parameter("data")).canonicalPath();

    if (current_docroot.length() < 1)
        response->standardResponse(cmd, NETV_STATUS_ERROR, "path not found");
    else
        response->standardResponse(cmd, NETV_STATUS_SUCCESS, current_docroot.trimmed());
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleInitialHello, cmd, app, request, response)
{
    response->write(QString("<xml>\n"
                           "    <status>%1</status>\n"
                           "    <cmd>%2</cmd>\n"
                           "    <data>\n"
                           "        <guid>%3</guid>\n"
                           "        <dcid>\n"
                           "            <vers>%4</vers>\n"
                           "            <rgin>%5</rgin>\n"
                           "            <skin>%6</skin>\n"
                           "            <part>%7</part>\n"
                           "            <camp>%8</camp>\n"
                           "        </dcid>\n"
                           "        <hwver>%9</hwver>\n"
                           "        <fwver>%10</fwver>\n"
                           "        <flashplugin>%11</flashplugin>\n"
                           "        <flashver>%12</flashver>\n"
                           "        <mac>%13</mac>\n"
                           "        <internet>%14</internet>"
                           "        <ip>%15</ip>\n"
                           "        <network>%16</network>\n"
                           "    </data>\n"
                           "</xml>\n")
                   .arg(NETV_STATUS_SUCCESS)
                   .arg(cmd)
                   .arg("d9fc84d2-6e12-4332-b5c9-c719dc3b7e84")
                   .arg(1)
                   .arg(1)
                   .arg(1)
                   .arg(1)
                   .arg(1)
                   .arg(1)
                   .arg(1)
                   .arg("false")
                   .arg(0)
                   .arg("11:22:33:44:55:66")
                   .arg("true")
                   .arg("192.168.1.10")
                   .arg("<connection if=\"net\" up=\"true\" broadcast=\"192.168.1.255\" netmask=\"255.255.255.0\" gateway=\"192.168.1.1\" nameserver1=\"8.8.8.8\"/>")
                   );
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetParam, cmd, app, request, response)
{
    response->standardResponse(cmd, NETV_STATUS_SUCCESS, app->setting(request->parameter("data")).toString().toHtmlEscaped());
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetLocalWidgets, cmd, app, request, response)
{
    QJsonArray widgets;

    foreach (QString filename, app->widgetRoot().entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString url = QString("/widgets/%1/").arg(filename);
        QJsonObject widget
        {
            {"top", "100px"},
            {"left", "50px"},
            {"width", "300px"},
            {"height", "200px"},
            {"name", filename},
            {"url", url}
        };
        widgets.append(widget);
    }
    QJsonDocument json(widgets);
    response->standardResponse(cmd, NETV_STATUS_SUCCESS, json.toJson());
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetLocalWidgetConfig, cmd, app, request, response)
{
    response->standardResponse(cmd, NETV_STATUS_SUCCESS, "{\"top\": \"100px\", \"left\": \"50px\", \"width\": \"300px\", \"height\": \"200px\"}");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleTickerEvent, cmd, app, request, response)
{
    QString postData = "<xml>";
    postData += request->parameter("data");
    postData += "</xml>";
    postData.replace("%2F", "/");
    qDebug() << "Attempting to decode" << postData;

    QXmlStreamReader xmlStream(postData);
    QHash<QString, QString> tickerParams;

    QString key, value;

    while (!xmlStream.atEnd()) {
        xmlStream.readNext();

        if (xmlStream.isStartElement()) {
            qDebug() << "New element:" << xmlStream.name();
            key = xmlStream.name().toString();
            value = "";
        }
        else if (xmlStream.isCharacters()) {
            qDebug() << "New value:" << xmlStream.text();
            value = xmlStream.text().toString();
        }
        else if (xmlStream.isEndElement()) {
            qDebug() << "End element:" << xmlStream.name();
            tickerParams.insert(key, value);
        }
    }

    if (xmlStream.hasError()) {
        qDebug() << "XML stream had an error";
        qDebug() << "XML error:" << xmlStream.errorString();
    }

    //Convert to a JavaScript command & forward to NeTVBrowser
    QByteArray javaScriptString = "fTickerEvents(\"" + tickerParams.value("message").toUtf8().toPercentEncoding()
                                                     + "\",\"" + tickerParams.value("title").toUtf8().toPercentEncoding()
                                                     + "\",\"" + tickerParams.value("image").toUtf8().toPercentEncoding()
                                                     + "\",\"" + tickerParams.value("type").toUtf8().toPercentEncoding()
                                                     + "\",\"" + tickerParams.value("level").toUtf8().toPercentEncoding()
                                                     + "\");";
    app->sendBroadcast(javaScriptString);
    response->standardResponse(cmd, NETV_STATUS_SUCCESS, javaScriptString);
}
END_NETV_HANDLER()
