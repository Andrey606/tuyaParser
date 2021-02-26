#ifndef TUYAPARSER_H
#define TUYAPARSER_H

#include <QObject>
#include <QtCore>
#include <QString>
#include <QMap>
#include <QMetaEnum>

class TuyaParser
{
public:

    // ON_OFF -             0x0006 0x0000
    // TARGET_TEMP_VALUE -  0x0201 0x0012
    // TARGET_VOLT_VALUE -  0x0008 0x0011
    // CURRENT_TEMP_VALUE - 0x0201 0x0000
    // MODE_TEMP_CHANGE -   0x0201 0x001e

    struct _tuyaFormat{
        QString raw;
        uint8_t status;
        uint8_t transId;
        uint16_t dp;
        uint8_t fn;
        uint8_t size;
        QByteArray data;
    };
    struct _zclFormat{
        quint16 clusterId;
        quint16 attributeId;
        QByteArray data;
    };

    struct _tuya{quint16 dp; quint16 size; quint16 devide;};
    struct _zcl{quint16 cluster; quint16 attribute; quint16 size; quint16 devide;};
    struct _data {_tuya tuya; _zcl zcl;};

signals:

public slots:

public:
    explicit TuyaParser();

    void uploadConfig();
    _tuyaFormat parseTuyaData(QString payload);
    _zclFormat converteTuyaDataToZCL(_tuyaFormat data, QString deviceType);
    _tuyaFormat converteZclToTuyaData(quint16 cluster, quint16 attribute, QString payload, QString deviceType);
    QByteArray dataSizeConverter(_tuyaFormat data, quint8 convertSize, quint16 devide=0, bool revertDivide=false);
    void printZclFormat(_zclFormat data);
    void printTuyaFormat(_tuyaFormat data);

    QMap<QString, QList<_data>> table;
};

#endif // TUYAPARSER_H



