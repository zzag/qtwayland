// Copyright (C) 2017 ITAGE Corporation, author: <yusuke.binsaki@itage.co.jp>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qwaylandivisurface_p.h"
#include "qwaylandivishellintegration.h"

#include <QtWaylandClient/private/qwaylandabstractdecoration_p.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/qwaylandscreen_p.h>
#include <QtWaylandClient/private/qwaylandextendedsurface_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandIviSurface::QWaylandIviSurface(QWaylandIviShellIntegration *shell, uint32_t surfaceId, QWaylandWindow *window)
    : QWaylandShellSurface(window)
    , m_shell(shell)
    , m_window(window)
    , m_surfaceId(surfaceId)
{
}

QWaylandIviSurface::~QWaylandIviSurface()
{
    destroy();
}

bool QWaylandIviSurface::isCreated() const
{
    return QtWayland::ivi_surface::isInitialized();
}

bool QWaylandIviSurface::create()
{
    QtWayland::ivi_surface::init(m_shell->iviApplication()->surface_create(m_surfaceId, m_window->wlSurface()));
    if (m_shell->iviController()) {
        QtWayland::ivi_controller_surface::init(m_shell->iviController()->surface_create(m_surfaceId));
    }

    if (m_window->window()->type() == Qt::Popup) {
        QPoint transientPos = m_window->geometry().topLeft(); // this is absolute
        QWaylandWindow *parent = m_window->transientParent();
        if (parent && parent->decoration()) {
            transientPos -= parent->geometry().topLeft();
            transientPos.setX(transientPos.x() + parent->decoration()->margins().left());
            transientPos.setY(transientPos.y() + parent->decoration()->margins().top());
        }
        QSize size = m_window->windowGeometry().size();
        QtWayland::ivi_controller_surface::set_destination_rectangle(transientPos.x(),
                                                                     transientPos.y(),
                                                                     size.width(),
                                                                     size.height());
    }

    createExtendedSurface(m_window);

    return true;
}

void QWaylandIviSurface::destroy()
{
    if (QtWayland::ivi_surface::object())
        QtWayland::ivi_surface::destroy();
    if (QtWayland::ivi_controller_surface::object())
        QtWayland::ivi_controller_surface::destroy(0);

    delete m_extendedWindow;
    m_extendedWindow = nullptr;
}

void QWaylandIviSurface::applyConfigure()
{
    m_window->resizeFromApplyConfigure(m_pendingSize);
}

void QWaylandIviSurface::createExtendedSurface(QWaylandWindow *window)
{
    if (window->display()->windowExtension())
        m_extendedWindow = new QWaylandExtendedSurface(window);
}

void QWaylandIviSurface::ivi_surface_configure(int32_t width, int32_t height)
{
    m_pendingSize = {width, height};
    m_window->applyConfigureWhenPossible();
}

void QWaylandIviSurface::ivi_controller_surface_visibility(int32_t visibility)
{
    this->m_window->window()->setVisible(visibility != 0);
}

}

QT_END_NAMESPACE
