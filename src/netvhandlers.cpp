#include "netvhandlers.h"
#include "netvserverapplication.h"
#include "nhttprequest.h"
#include "nhttpresponse.h"

BEGIN_NETV_HANDLER(handleDefault, cmd, app, request, response)
{
    response.standardResponse(cmd, NETV_STATUS_UNIMPLEMENTED, "Unhandled command received");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetUrl, cmd, app, request, response)
{
    Q_UNUSED(app);
    QString urlString = request.parameter("url");
    QString postString = request.parameter("post");
    QStringList argsList;

    if (urlString.length() > 0)
        argsList.append(QString(urlString));

    if (postString.length() > 0)
        argsList.append(QString(postString));

    if (argsList.length() < 1) {
        response.standardResponse(cmd, NETV_STATUS_ERROR, "no arguments received");
    }
    else {
        //QByteArray buffer = this->Execute(docroot + "/scripts/geturl.sh", argsList, xmlEscape);
        response.standardResponse(cmd, NETV_STATUS_SUCCESS, "script execution disabled");
    }
    return 0;
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleGetJpeg, cmd, app, request, response)
{
    //QByteArray buffer = this->Execute(docroot + "/scripts/tmp_download.sh", QStringList(dataString), xmlEscape);
    response.standardResponse(cmd, NETV_STATUS_SUCCESS, "get jpeg unimplemented");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleChromaKey, cmd, app, request, response)
{
    //Contruct arguments
    QVariant dataString(request.parameter("value"));
    QStringList argList;

    if (dataString == "on" || dataString == "true" || dataString == "yes")
        argList.append(QString("on"));
    else if (dataString == "off" || dataString == "false" || dataString == "no")
        argList.append(QString("off"));

    //Execute the script
    if (argList.size() <= 0)
    {
        response.standardResponse(cmd, NETV_STATUS_ERROR, "Invalid arguments");
    }
    else
    {
        //QByteArray buffer = this->Execute(docroot + "/scripts/chromakey.sh", argList, xmlEscape);
        QByteArray buffer;
        response.standardResponse(cmd, NETV_STATUS_SUCCESS, buffer.trimmed());
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
    response.standardResponse(cmd, NETV_STATUS_UNIMPLEMENTED, "ssh enable not yet implemented");
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleSetDocRoot, cmd, app, request, response)
{
    QString current_docroot = app.setStaticDocRoot(request.parameter("data")).canonicalPath();

    /*
    if (xmlEscape)
        current_docroot = XMLEscape(current_docroot);
    */

    if (current_docroot.length() < 1)
        response.standardResponse(cmd, NETV_STATUS_ERROR, "path not found");
    else
        response.standardResponse(cmd, NETV_STATUS_SUCCESS, current_docroot.trimmed());
}
END_NETV_HANDLER()

BEGIN_NETV_HANDLER(handleInitialHello, cmd, app, request, response)
{
    response.write(QString("<xml>\n"
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
    response.standardResponse(cmd, NETV_STATUS_SUCCESS, app.setting(request.parameter("data")).toString().toHtmlEscaped());
}

END_NETV_HANDLER()
