/*
 * Copyright (C) 2013 Jolla Ltd.
 * Contact: John Brooks <john.brooks@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "declarativewindowattributes.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QGuiApplication>
#include <QQuickWindow>
#include <qpa/qplatformnativeinterface.h>
#else
#include <QGraphicsView>
#include <QX11Info>
#endif
#include <QTimer>
#include <QDebug>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

DeclarativeWindowAttributes::DeclarativeWindowAttributes(QDeclarativeItem *parent)
    : QDeclarativeItem(parent), m_stackingLayer(0), m_cannotMinimize(false)
{
}

void DeclarativeWindowAttributes::setStackingLayer(int layer)
{
    if (m_stackingLayer == layer)
        return;

    m_stackingLayer = layer;
    updateX11(false);
    emit stackingLayerChanged();
}

void DeclarativeWindowAttributes::setCannotMinimize(bool on)
{
    if (m_cannotMinimize == on)
        return;

    m_cannotMinimize = on;
    updateX11(false);
    emit cannotMinimizeChanged();
}

bool DeclarativeWindowAttributes::updateX11(bool delayed)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    if (qApp->platformName() != QLatin1String("xcb")) {
        qWarning() << "org.nemomobile.utilities: unsupported platform:" << qApp->platformName();
        return false;
    }
    QWindow *view = window();
#else
    QWidget *view = scene() ? scene()->views().value(0) : 0;
#endif

    if (!view) {
        if (delayed)
            qWarning() << "org.nemomobile.utilities: WindowAttributes has no window";
        else
            QTimer::singleShot(1, this, SLOT(updateX11()));
        return false;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    WId winId = window()->winId();
    QPlatformNativeInterface *iface = qApp->platformNativeInterface();
    Display *display = reinterpret_cast<Display *>(iface->nativeResourceForScreen("display", QGuiApplication::primaryScreen()));
#else
    if (!view->isWindow())
        view = view->window();

    WId winId = view->winId();
    Display *display = QX11Info::display();
#endif

    Atom stackingLayerAtom = XInternAtom(display, "_MEEGO_STACKING_LAYER", False);
    Atom cannotMinimizeAtom = XInternAtom(display, "_MEEGOTOUCH_CANNOT_MINIMIZE", False);

    if (stackingLayerAtom != None) {
        if (m_stackingLayer) {
            long l = m_stackingLayer;
            XChangeProperty(display, winId, stackingLayerAtom, XA_CARDINAL,
                            32, PropModeReplace, (unsigned char*)&l, 1);
        } else {
            XDeleteProperty(display, winId, stackingLayerAtom);
        }
    }

    // Set _MEEGOTOUCH_CANNOT_MINIMIZE
    if (cannotMinimizeAtom != None) {
        if (m_cannotMinimize) {
            long v = 1;
            XChangeProperty(display, winId, cannotMinimizeAtom, XA_CARDINAL,
                            32, PropModeReplace, (unsigned char*)&v, 1);
        } else {
            XDeleteProperty(display, winId, cannotMinimizeAtom);
        }
    }

    return true;
}

