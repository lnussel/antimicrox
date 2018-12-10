#ifndef GAMECONTROLLERSETXML_H
#define GAMECONTROLLERSETXML_H

#include "xml/setjoystickxml.h"
#include <SDL2/SDL_gamecontroller.h>

#include <QObject>
#include <QHash>
#include <QList>
#include <QPointer>

class GameControllerSet;
class SetJoystick;
class JoyDPadXml;
class JoyControlStickXml;

class QXmlStreamReader;


class GameControllerSetXml : public SetJoystickXml
{

public:
    GameControllerSetXml(GameControllerSet* gameContrSet, SetJoystick* setJoystick, QObject* parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml); // GameControllerSetXml
    virtual void readJoystickConfig(QXmlStreamReader *xml,
                            QHash<int, SDL_GameControllerButton> &buttons,
                            QHash<int, SDL_GameControllerAxis> &axes,
                            QList<SDL_GameControllerButtonBind> &hatButtons);

private:
    GameControllerSet* m_gameContrSet;
    QPointer<JoyDPadXml> dpadXml;
    QPointer<JoyControlStickXml> joyContrStickXml;

    void getElemFromXml(QString elemName, QXmlStreamReader *xml); // GameControllerSetXml class
    void readConfDpad(QXmlStreamReader *xml, QList<SDL_GameControllerButtonBind> &hatButtons, bool vdpadExists, bool dpadExists); // GameControllerSetXml class


};

#endif // GAMECONTROLLERSETXML_H
