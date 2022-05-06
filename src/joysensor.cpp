/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _USE_MATH_DEFINES

#include "joysensor.h"
#include "inputdevice.h"
#include "joybuttontypes/joysensorbutton.h"
#include "xml/joybuttonxml.h"

#include <cmath>

JoySensor::JoySensor(JoySensorType type, int originset, SetJoystick *parent_set, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_pending_event(false)
    , m_originset(originset)
    , m_parent_set(parent_set)
{
}

JoySensor::~JoySensor() {}

/**
 * @brief Main sensor mapping function.
 *  When activated, it generates a "moved" QT event which updates various parts of the UI.
 *  XXX: will do more in the future
 */
void JoySensor::joyEvent(float *values, bool ignoresets)
{
    m_current_value[0] = values[0];
    m_current_value[1] = values[1];
    m_current_value[2] = values[2];

    emit moved(m_current_value[0], m_current_value[1], m_current_value[2]);
}

/**
 * @brief Queues next movement event from InputDaemon
 */
void JoySensor::queuePendingEvent(float *values, bool ignoresets)
{
    m_pending_event = true;
    m_pending_value[0] = values[0];
    m_pending_value[1] = values[1];
    m_pending_value[2] = values[2];
    m_pending_ignore_sets = ignoresets;
}

/**
 * @brief Activates previously queued movement event
 *  This is called by InputDevice.
 */
void JoySensor::activatePendingEvent()
{
    if (!m_pending_event)
        return;

    joyEvent(m_pending_value, m_pending_ignore_sets);

    clearPendingEvent();
}

/**
 * @brief Checks if an event is queued
 * @returns True if an event is queued, false otherwise.
 */
bool JoySensor::hasPendingEvent() const { return m_pending_event; }

/**
 * @brief Clears a previously queued event
 */
void JoySensor::clearPendingEvent()
{
    m_pending_event = false;
    m_pending_ignore_sets = false;
}

/**
 * @brief Check if any direction is mapped to a keyboard or mouse event
 * @returns True if a mapping exists, false otherwise
 */
bool JoySensor::hasSlotsAssigned() const
{
    for (const auto &button : m_buttons)
    {
        if (button != nullptr)
        {
            if (button->getAssignedSlots()->count() > 0)
                return true;
        }
    }
    return false;
}

/**
 * @brief Get the name of this sensor
 * @returns Sensor name
 */
QString JoySensor::getPartialName(bool forceFullFormat, bool displayNames) const
{
    QString label = QString();

    if (!m_sensor_name.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            label.append(sensorTypeName()).append(" ");
        }

        label.append(m_sensor_name);
    } else
    {
        label.append(sensorTypeName()).append(" ");
    }

    return label;
}

/**
 * @brief Returns the sensor name
 */
QString JoySensor::getSensorName() const { return m_sensor_name; }

/**
 * @brief Returns the sensor type
 */
JoySensorType JoySensor::getType() const { return m_type; }

/**
 * @brief Returns the current sensor direction
 */
JoySensorDirection JoySensor::getCurrentDirection() const { return m_current_direction; }

/**
 * @brief Get the assigned dead zone value
 * @return Assigned dead zone value in degree or degree/s
 */
double JoySensor::getDeadZone() const { return radToDeg(m_dead_zone); }

/**
 * @brief Get the assigned diagonal range value
 * @return Assigned diagonal range in degree or degree/s
 */
double JoySensor::getDiagonalRange() const { return radToDeg(m_diagonal_range); }

/**
 * @brief Get the assigned max zone value
 * @return Assigned max zone value in degree or degree/s
 */
double JoySensor::getMaxZone() const { return radToDeg(m_max_zone); }

/**
 * @brief Checks if the sensor vector is currently in the dead zone
 * @returns True if it is in the dead zone, false otherwise
 */
bool JoySensor::inDeadZone(float *values) const { return calculateDistance(values[0], values[1], values[2]) < m_dead_zone; }

/**
 * @brief Get current radial distance of the sensor past the assigned
 *   dead zone.
 * @return Distance between 0 and max zone in radiants.
 */
double JoySensor::getDistanceFromDeadZone() const
{
    return getDistanceFromDeadZone(m_current_value[0], m_current_value[1], m_current_value[2]);
}

/**
 * @brief Get radial distance of the sensor past the assigned dead zone
 *   based on the passed X, Y and Z axes values associated with the sensor.
 * @param X axis value
 * @param Y axis value
 * @param Z axis value
 * @return Distance between 0 and max zone in radiants.
 */
double JoySensor::getDistanceFromDeadZone(double x, double y, double z) const
{
    double distance = calculateDistance(x, y, z);
    return qBound(0.0, distance - m_dead_zone, m_max_zone);
}

/**
 * @brief Get the vector length of the sensor
 * @return Vector length
 */
double JoySensor::calculateDistance() const
{
    return calculateDistance(m_current_value[0], m_current_value[1], m_current_value[2]);
}

/**
 * @brief Get the vector length of the sensor based on the passed
 *  X, Y and Z axes values associated with the sensor.
 * @return Vector length
 */
double JoySensor::calculateDistance(double x, double y, double z) const { return sqrt(x * x + y * y + z * z); }

/**
 * @brief Calculate the pitch angle (in degrees) corresponding to the current
 *   position of controller.
 * @return Pitch (in degrees)
 */
double JoySensor::calculatePitch() const
{
    return calculatePitch(m_current_value[0], m_current_value[1], m_current_value[2]);
}

/**
 * @brief Calculate the pitch angle (in degrees) corresponding to the current
 *   passed X, Y and Z axes values associated with the sensor.
 *   position of controller.
 *   See https://www.nxp.com/files-static/sensors/doc/app_note/AN3461.pdf
 *   for a description of the used algorithm.
 * @param X axis value
 * @param Y axis value
 * @param Z axis value
 * @return Pitch (in degrees)
 */
double JoySensor::calculatePitch(double x, double y, double z) const
{
    double rad = calculateDistance(x, y, z);
    double pitch = -atan2(z / rad, y / rad) - M_PI / 2;
    if (pitch < -M_PI)
        pitch += 2 * M_PI;
    return pitch;
}

/**
 * @brief Calculate the roll angle (in degrees) corresponding to the current
 *   position of controller.
 * @return Roll (in degrees)
 */
double JoySensor::calculateRoll() const { return calculateRoll(m_current_value[0], m_current_value[1], m_current_value[2]); }

/**
 * @brief Calculate the roll angle (in degrees) corresponding to the current
 *   passed X, Y and Z axes values associated with the sensor.
 *   position of controller.
 *   See https://www.nxp.com/files-static/sensors/doc/app_note/AN3461.pdf
 *   for a description of the used algorithm.
 * @param X axis value
 * @param Y axis value
 * @param Z axis value
 * @return Roll (in degrees)
 */
double JoySensor::calculateRoll(double x, double y, double z) const
{
    double rad = calculateDistance(x, y, z);

    double xp, yp, zp;
    xp = x / rad;
    yp = y / rad;
    zp = z / rad;
    double roll = atan2(sqrt(yp * yp + zp * zp), -xp) - M_PI / 2;
    if (roll < -M_PI)
        roll += 2 * M_PI;
    return roll;
}

double JoySensor::calculateDirectionalDistance(JoySensorDirection direction) const { return 0; }

/**
 * @brief Utility function which converts a given value from radians to degree.
 */
double JoySensor::radToDeg(double value) { return value * 180 / M_PI; }

/**
 * @brief Utility function which converts a given value from degree to radians.
 */
double JoySensor::degToRad(double value) { return value * M_PI / 180; }

/**
 * @brief Returns a QHash which maps the SensorDirection to
 *  the corresponding JoySensorButton.
 */
QHash<JoySensorDirection, JoySensorButton *> *JoySensor::getButtons() { return &m_buttons; }

bool JoySensor::isDefault() const { return false; }

/**
 * @brief Sets the name of this sensor
 * @param[in] tempName New sensor name
 */
void JoySensor::setSensorName(QString tempName)
{
    if ((tempName.length() <= 20) && (tempName != m_sensor_name))
    {
        m_sensor_name = tempName;
        emit sensorNameChanged();
    }
}

void JoySensor::establishPropertyUpdatedConnection()
{
    connect(this, &JoySensor::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

/**
 * @brief Get pointer to the set that a sensor belongs to.
 * @return Pointer to the set that a sensor belongs to.
 */
SetJoystick *JoySensor::getParentSet() const { return m_parent_set; }
