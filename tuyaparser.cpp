#include "tuyaparser.h"

TuyaParser::TuyaParser()
{
    uploadConfig();
}

void TuyaParser::uploadConfig()
{
    QFile jsonFile(QString("../configs/")+QString("map.json"));
    jsonFile.open(QFile::ReadOnly);
    QJsonObject rootObject = QJsonDocument().fromJson(jsonFile.readAll()).object();

    foreach (const QJsonValue & v, rootObject.value("table").toArray())
    {
        QString deviceType = v.toObject().value("deviceType").toString();

        _data data;
        data.tuya.dp = v.toObject().value("tuya").toObject().value("dp").toString().right(4).toLocal8Bit().toULongLong(nullptr,16);
        data.tuya.size = v.toObject().value("tuya").toObject().value("size").toInt();

        data.zcl.cluster = v.toObject().value("zcl").toObject().value("cluster").toString().right(4).toLocal8Bit().toULongLong(nullptr,16);
        data.zcl.attribute = v.toObject().value("zcl").toObject().value("attribute").toString().right(4).toLocal8Bit().toULongLong(nullptr,16);
        data.zcl.size = v.toObject().value("zcl").toObject().value("size").toInt();
        data.zcl.devide = v.toObject().value("zcl").toObject().value("devide").toInt();


        table[deviceType].push_back(data);
    }
}

TuyaParser::_tuyaFormat TuyaParser::parseTuyaData(QString payload)
{
    _tuyaFormat tuyaData;

    tuyaData.raw = payload;

    // 1 bytes = status
    // 1 bytes = transId
    // 2 byte  = dp
    // 1 bytes = fn
    // octetStr:
    // 1 bytes = size
    // ...     = data

    /*qDebug() << "========= parseTuyaAttributes ==========";
    qDebug() << "Raw data:   " + payload;*/

    payload.remove(' ');

    uint8_t     status =    static_cast<uint8_t>(payload.left(2).toLocal8Bit().leftJustified(2,'0').toUShort(nullptr,16));
    uint8_t     transId =   static_cast<uint8_t>(payload.remove(0,2).left(2).toLocal8Bit().leftJustified(2,'0').toUShort(nullptr,16));
    uint16_t    dp =        static_cast<uint16_t>(payload.remove(0,2).left(4).toLocal8Bit().leftJustified(4,'0').toUShort(nullptr,16));
    uint8_t     fn =        static_cast<uint8_t>(payload.remove(0,4).left(2).toLocal8Bit().leftJustified(2,'0').toUShort(nullptr,16));
    uint8_t     size =      static_cast<uint8_t>(payload.remove(0,2).left(2).toLocal8Bit().leftJustified(2,'0').toUShort(nullptr,16));

    /*qDebug() << "header:     ";
    qDebug() << "-status:    " << hex << status;
    qDebug() << "-transId:   " << hex << transId;
    qDebug() << "-dp:        " << hex << dp;
    qDebug() << "-fn:        " << hex << fn;
    qDebug() << "payload:    ";
    qDebug() << "-size (dec):" << dec << size;*/
    QString data = payload.remove(0,2).left(size*2);
    /*qDebug() << "-data:      " << data;
    qDebug() << "========================================";*/

    tuyaData.dp = dp;
    tuyaData.fn = fn;
    tuyaData.size = size;
    tuyaData.status = status;
    tuyaData.transId = transId;
    tuyaData.data = data.toLocal8Bit();

    return tuyaData;
}

TuyaParser::_tuyaFormat TuyaParser::converteZclToTuyaData(quint16 cluster, quint16 attribute, QString payload, QString deviceType)
{
    _tuyaFormat tuyaData;

    if(table[deviceType].isEmpty())
    {
        qt_assert("no info about this device type", __FILE__, 228);
    }

    foreach(_data d, table[deviceType])
    {
        if(cluster == d.zcl.cluster && attribute == d.zcl.attribute)
        {
            tuyaData.status =   0x00;
            tuyaData.transId =  0x0f;
            tuyaData.dp =       d.tuya.dp;
            tuyaData.fn =       0x00;

            if(d.tuya.size != 0)
                tuyaData.size = d.tuya.size;
            else
                tuyaData.size = payload.size();

            tuyaData.data =     payload.toLocal8Bit();

            tuyaData.data = dataSizeConverter(tuyaData, d.tuya.size, d.zcl.devide, true);
            return tuyaData;
        }
    }

    qt_assert("no info about this cluster", __FILE__, 228);
}

TuyaParser::_zclFormat TuyaParser::converteTuyaDataToZCL(_tuyaFormat data, QString deviceType)
{
    _zclFormat zclData;

    if(table[deviceType].isEmpty())
    {
        qt_assert("no info about this device type", __FILE__, 228);
    }

    foreach(_data d, table[deviceType])
    {
        if(data.dp == d.tuya.dp)
        {
            zclData.clusterId = d.zcl.cluster;
            zclData.attributeId = d.zcl.attribute;
            zclData.data = dataSizeConverter(data, d.zcl.size, d.zcl.devide, false);
            return zclData;
        }
    }

    qt_assert("no info about this descriptor", __FILE__, 228);
}

QByteArray TuyaParser::dataSizeConverter(_tuyaFormat data, quint8 convertSize, quint16 devide, bool revertDivide)
{
    int tmpData;
    QByteArray tmpByteArr;

    if(devide)
    {
        if(revertDivide)
        {
            tmpData = data.data.toInt(nullptr, 16)*devide;
            tmpByteArr = QString::number(tmpData, 16).toLocal8Bit();
        }
        else
        {
            tmpData = data.data.toInt(nullptr, 16)/devide;
            tmpByteArr = QString::number(tmpData, 16).toLocal8Bit();
        }
    }
    else
    {
        tmpByteArr = data.data;
    }

    if(convertSize != 0)
    {
        tmpData = data.data.toInt(nullptr, 16)*devide;
        tmpByteArr = QString::number(tmpData, 16).rightJustified(convertSize*2, '0').toLocal8Bit();
    }
    else
    {
        if(!data.raw.isEmpty())
            tmpByteArr = data.raw.remove(' ').mid(8).toUtf8();
    }

    return tmpByteArr;
}

void TuyaParser::printZclFormat(_zclFormat data)
{
    QString msg;

    msg += "clusterId: " + QString("0x") + QString::number(data.clusterId, 16).rightJustified(4,'0');
    msg += ", attributeId: " + QString("0x") +  QString::number(data.attributeId, 16).rightJustified(4,'0');
    msg += ", payload: {" + data.data + "}";

    qInfo() << msg;
}


void TuyaParser::printTuyaFormat(_tuyaFormat data)
{
    QString msg;

    msg += "clusterId: 0xEF00";
    msg += ", payload: {011100";
    msg += QString::number(data.status, 16).rightJustified(2,'0');
    msg += QString::number(data.transId, 16).rightJustified(2,'0');
    msg += QString::number(data.dp, 16).rightJustified(4,'0');
    msg += QString::number(data.fn, 16).rightJustified(2,'0');
    msg += QString::number(data.size, 16).rightJustified(2,'0');
    msg += data.data + "}";

    qInfo() << msg;
}
