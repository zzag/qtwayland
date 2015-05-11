/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QSGTexture>
#include <QOpenGLTexture>
#include <QQuickWindow>
#include <QDebug>

#include "qwaylandquicksurface.h"
#include "qwaylandquickcompositor.h"
#include "qwaylandoutput.h"
#include "qwaylandquickview.h"
#include <QtCompositor/qwaylandbufferref.h>
#include <QtCompositor/private/qwaylandsurface_p.h>

#include <QtCompositor/private/qwayland-server-surface-extension.h>
#include <QtCompositor/private/qwlextendedsurface_p.h>

QT_BEGIN_NAMESPACE

class QWaylandQuickSurfacePrivate : public QWaylandSurfacePrivate
{
public:
    QWaylandQuickSurfacePrivate(wl_client *client, quint32 id, int version, QWaylandQuickCompositor *c, QWaylandQuickSurface *surf)
        : QWaylandSurfacePrivate(client, id, version, c, surf)
        , compositor(c)
        , useTextureAlpha(true)
        , clientRenderingEnabled(true)
    {
    }

    ~QWaylandQuickSurfacePrivate()
    {
    }

    void surface_commit(Resource *resource) Q_DECL_OVERRIDE
    {
        QWaylandSurfacePrivate::surface_commit(resource);

        Q_FOREACH (QWaylandSurfaceView *view, views) {
            if (view->output())
                view->output()->update();
        }
    }

    QWaylandQuickCompositor *compositor;
    bool useTextureAlpha;
    bool clientRenderingEnabled;
};

QWaylandQuickSurface::QWaylandQuickSurface(wl_client *client, quint32 id, int version, QWaylandQuickCompositor *compositor)
                    : QWaylandSurface(new QWaylandQuickSurfacePrivate(client, id, version, compositor, this))
{
    connect(this, &QWaylandSurface::primaryOutputChanged, this, &QWaylandQuickSurface::primaryOutputWindowChanged);

    connect(this, &QWaylandSurface::windowPropertyChanged, d->windowPropertyMap, &QQmlPropertyMap::insert);
    connect(d->windowPropertyMap, &QQmlPropertyMap::valueChanged, this, &QWaylandSurface::setWindowProperty);
    connect(this, &QWaylandSurface::shellViewCreated, this, &QWaylandQuickSurface::shellViewCreated);
}

QWaylandQuickSurface::~QWaylandQuickSurface()
{

}

bool QWaylandQuickSurface::useTextureAlpha() const
{
    Q_D(const QWaylandQuickSurface);
    return d->useTextureAlpha;
}

void QWaylandQuickSurface::setUseTextureAlpha(bool useTextureAlpha)
{
    Q_D(QWaylandQuickSurface);
    if (d->useTextureAlpha != useTextureAlpha) {
        d->useTextureAlpha = useTextureAlpha;
        emit useTextureAlphaChanged();
        emit configure(handle()->currentBufferRef().hasBuffer());
    }
}

QWindow *QWaylandQuickSurface::primaryOutputWindow() const
{
    return primaryOutput() ? primaryOutput()->window() : Q_NULLPTR;
}

bool QWaylandQuickSurface::event(QEvent *e)
{
    if (e->type() == static_cast<QEvent::Type>(QWaylandSurfaceLeaveEvent::WaylandSurfaceLeave)) {
        QWaylandSurfaceLeaveEvent *event = static_cast<QWaylandSurfaceLeaveEvent *>(e);

        if (event->output()) {
            QQuickWindow *oldWindow = static_cast<QQuickWindow *>(event->output()->window());
            disconnect(oldWindow, &QQuickWindow::beforeSynchronizing,
                       this, &QWaylandQuickSurface::updateTexture);
            disconnect(oldWindow, &QQuickWindow::sceneGraphInvalidated,
                       this, &QWaylandQuickSurface::invalidateTexture);
            disconnect(oldWindow, &QQuickWindow::sceneGraphAboutToStop,
                       this, &QWaylandQuickSurface::invalidateTexture);
        }

        return true;
    }

    if (e->type() == static_cast<QEvent::Type>(QWaylandSurfaceEnterEvent::WaylandSurfaceEnter)) {
        QWaylandSurfaceEnterEvent *event = static_cast<QWaylandSurfaceEnterEvent *>(e);

        if (event->output()) {
            QQuickWindow *window = static_cast<QQuickWindow *>(event->output()->window());
            connect(window, &QQuickWindow::beforeSynchronizing,
                    this, &QWaylandQuickSurface::updateTexture,
                    Qt::DirectConnection);
            connect(window, &QQuickWindow::sceneGraphInvalidated,
                    this, &QWaylandQuickSurface::invalidateTexture,
                    Qt::DirectConnection);
            connect(window, &QQuickWindow::sceneGraphAboutToStop,
                    this, &QWaylandQuickSurface::invalidateTexture,
                    Qt::DirectConnection);
        }

        return true;
    }

    return QObject::event(e);
}

bool QWaylandQuickSurface::clientRenderingEnabled() const
{
    Q_D(const QWaylandQuickSurface);
    return d->clientRenderingEnabled;
}

void QWaylandQuickSurface::setClientRenderingEnabled(bool enabled)
{
    Q_D(QWaylandQuickSurface);
    if (d->clientRenderingEnabled != enabled) {
        d->clientRenderingEnabled = enabled;

        if (QtWayland::ExtendedSurface *extSurface = QtWayland::ExtendedSurface::findIn(this))
            extSurface->setVisibility(enabled ? QWindow::AutomaticVisibility : QWindow::Hidden);

        emit clientRenderingEnabledChanged();
    }
}

QT_END_NAMESPACE
