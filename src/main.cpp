// This file is part of the SpeedCrunch project
// Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2007-2018 @heldercorreia
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "gui/application.h"
#include "gui/mainwindow.h"

#include <QCoreApplication>
#include <QApplication>
#include <QWindow>

int main(int argc, char* argv[])
{
    Application application(argc, argv);

    if (application.isRunning()) {
        application.sendRaiseRequest();
        return 0;
    }

    QCoreApplication::setApplicationName("SpeedCrunch");
    QCoreApplication::setOrganizationDomain("speedcrunch.org");

    MainWindow window;
    window.show();

    application.connect(&application, SIGNAL(lastWindowClosed()), &application, SLOT(quit()));

    application.connect(&application, &Application::raiseRequested, &window, [&] {
        printf("RAISE!!!\n");
        window.setWindowState( (window.windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        window.raise();
        window.activateWindow();
        for (QWindow* appWindow : QGuiApplication::allWindows()) {
            appWindow->show();
            appWindow->raise();
            appWindow->requestActivate();
        }
    });

    return application.exec();
}
