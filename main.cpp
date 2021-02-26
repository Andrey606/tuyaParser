#include <QCoreApplication>
#include <QCommandLineParser>

#include "tuyaparser.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Tuya parser");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption commandOption(QStringList() << "c" << "command",
            QCoreApplication::translate("main", "tuyaToZcl, zclToTuya."),
            QCoreApplication::translate("main", "command"));
    QCommandLineOption clusterOption(QStringList() << "k" << "cluster",
            QCoreApplication::translate("main", "clusterId."),
            QCoreApplication::translate("main", "uint16t"));
    QCommandLineOption attributeOption(QStringList() << "a" << "attribute",
            QCoreApplication::translate("main", "attributeId."),
            QCoreApplication::translate("main", "uint16t"));
    QCommandLineOption deviceTypeOption(QStringList() << "t" << "deviceType",
            QCoreApplication::translate("main", "deviceType."),
            QCoreApplication::translate("main", "device type"));
    QCommandLineOption payloadOption(QStringList() << "p" << "payload",
            QCoreApplication::translate("main", "'payload'."),
            QCoreApplication::translate("main", "byte array"));

    parser.addOption(commandOption);
    parser.addOption(clusterOption);
    parser.addOption(attributeOption);
    parser.addOption(deviceTypeOption);
    parser.addOption(payloadOption);

    parser.process(app);

    if(parser.values(commandOption).contains("tuyaToZcl"))
    {
        quint16 cluster;
        QByteArray payload;
        QString deviceType;

        if(!parser.values(clusterOption).isEmpty() &&
                !parser.values(deviceTypeOption).isEmpty() &&
                !parser.values(payloadOption).isEmpty())
        {
            cluster = parser.values(clusterOption)[0].toInt(nullptr, 16);
            payload = parser.values(payloadOption)[0].toUtf8();
            deviceType = parser.values(deviceTypeOption)[0].toUtf8();

            if(cluster == 0xef00)
            {
                TuyaParser tuyaParser;
                tuyaParser.printZclFormat(tuyaParser.converteTuyaDataToZCL(tuyaParser.parseTuyaData(payload), deviceType));

                exit(0);
            }
            else
            {
                qt_assert("support just 0xef00 cluster", __FILE__, 228);
            }
        }
        else
        {
            qt_assert("enter all attributes!", __FILE__, 228);
        }
    }
    else if(parser.values(commandOption).contains("zclToTuya"))
    {
        quint16 cluster, attribute;
        QByteArray payload;
        QString deviceType;

        cluster = parser.values(clusterOption)[0].toInt(nullptr, 16);
        attribute = parser.values(attributeOption)[0].toInt(nullptr, 16);
        payload = parser.values(payloadOption)[0].toUtf8();
        deviceType = parser.values(deviceTypeOption)[0].toUtf8();

        TuyaParser tuyaParser;
        tuyaParser.printTuyaFormat(tuyaParser.converteZclToTuyaData(cluster, attribute, payload, deviceType));

        exit(0);
    }
    else
    {
        qt_assert("not supoorted command", __FILE__, 228);
    }

    return a.exec();
}
