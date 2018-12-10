#ifndef JOYCONTROLSTICKXML_H
#define JOYCONTROLSTICKXML_H

#include "joycontrolstickdirectionstype.h"
#include <QObject>

class JoyControlStick;
class QXmlStreamReader;
class QXmlStreamWriter;

class JoyControlStickXml : public QObject, public JoyStickDirectionsType
{
    Q_OBJECT
public:
    explicit JoyControlStickXml(JoyControlStick* stick, QObject *parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml); // JoyControlStickXml class
    virtual void writeConfig(QXmlStreamWriter *xml); // JoyControlStickXml class

signals:

public slots:

private:
    JoyControlStick* m_stick;
};

#endif // JOYCONTROLSTICKXML_H
