// Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDIVIINTEGRATION_H
#define QWAYLANDIVIINTEGRATION_H

#include <QtCore/qmutex.h>

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>
#include "qwayland-ivi-application.h"
#include "qwayland-ivi-controller.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandDisplay;

class Q_WAYLANDCLIENT_EXPORT QWaylandIviShellIntegration : public QWaylandShellIntegration
{
public:
    QWaylandIviShellIntegration();

    bool initialize(QWaylandDisplay *display) override;
    QWaylandShellSurface *createShellSurface(QWaylandWindow *window) override;

    QtWayland::ivi_application *iviApplication() const { return m_iviApplication.get(); }
    QtWayland::ivi_controller *iviController() const { return m_iviController.get(); }

private:
    uint32_t getNextUniqueSurfaceId();

private:
    QScopedPointer<QtWayland::ivi_application> m_iviApplication;
    QScopedPointer<QtWayland::ivi_controller> m_iviController;
    uint32_t m_lastSurfaceId = 0;
    uint32_t m_surfaceNumber = 0;
    bool m_useEnvSurfaceId = false;
    QRecursiveMutex m_mutex;
};

}

QT_END_NAMESPACE

#endif //  QWAYLANDIVIINTEGRATION_H
