/***************************************************************************
 *   Copyright © 2010 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *   Copyright © 2004 Canonical                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "workerinstallprogress.h"

#include <QtCore/QStringBuilder>
#include <QtCore/QStringList>
#include <QDebug>

#include <apt-pkg/error.h>

#include <errno.h>
#include <sys/statvfs.h>
#include <sys/statfs.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <pty.h>

#include <iostream>
#include <stdlib.h>

#include "transaction.h"

using namespace std;

WorkerInstallProgress::WorkerInstallProgress(QObject* parent, int begin, int end)
        : QObject(parent)
        , m_trans(nullptr)
        , m_startCounting(false)
        , m_progressBegin(begin)
        , m_progressEnd(end)
{
    setenv("DEBIAN_FRONTEND", "passthrough", 1);
    setenv("DEBCONF_PIPE", "/tmp/qapt-sock", 1);
    setenv("APT_LISTBUGS_FRONTEND", "none", 1);
    setenv("APT_LISTCHANGES_FRONTEND", "debconf", 1);

    m_ansiRegex = QRegExp(QChar(27) + QLatin1String("\[[;?0-9]*[A-Za-z]"));
}

void WorkerInstallProgress::setTransaction(Transaction *trans)
{
    m_trans = trans;
    std::setlocale(LC_ALL, m_trans->locale().toAscii());
}

pkgPackageManager::OrderResult WorkerInstallProgress::start(pkgPackageManager *pm)
{
    m_trans->setStatus(QApt::CommittingStatus);
    pkgPackageManager::OrderResult res;

    res = pm->DoInstallPreFork();
    if (res == pkgPackageManager::Failed) {
        return res;
    }

    int readFromChildFD[2];

    //Initialize both pipes
    if (pipe(readFromChildFD) < 0) {
        return res;
    }

    int pty_master;
    m_child_id = forkpty(&pty_master, 0, 0, 0);

    if (m_child_id == -1) {
        return res;
    } else if (m_child_id == 0) {
        // close pipe we don't need
        close(readFromChildFD[0]);

        res = pm->DoInstallPostFork(readFromChildFD[1]);

        // dump errors into cerr (pass it to the parent process)
        _error->DumpErrors();

        close(readFromChildFD[1]);

        _exit(res);
    }

    // make it nonblocking
    fcntl(readFromChildFD[0], F_SETFL, O_NONBLOCK);
    fcntl(pty_master, F_SETFL, O_NONBLOCK);

    // Update the interface until the child dies
    int ret;
    char masterbuf[1024];
    QString dpkgLine;
    while (waitpid(m_child_id, &ret, WNOHANG) == 0) {
        // Read dpkg's raw output
        while(read(pty_master, masterbuf, sizeof(masterbuf)) > 0);

        // Format raw dpkg output, remove ansi escape sequences
        dpkgLine = QString(masterbuf);
        dpkgLine.remove(m_ansiRegex);
        //qDebug() << dpkgLine;

        // Update high-level status info
        updateInterface(readFromChildFD[0], pty_master);
    }

    close(readFromChildFD[0]);
    close(readFromChildFD[1]);
    close(pty_master);

    return res;
}

void WorkerInstallProgress::updateInterface(int fd, int writeFd)
{
    char buf[2];
    static char line[1024] = "";

    while (1) {
        int len = read(fd, buf, 1);

        // Status message didn't change
        if (len < 1) {
            break;
        }

        if (buf[0] == '\n') {
            const QStringList list = QString::fromUtf8(line).split(QLatin1Char(':'));
            const QString status = list.at(0);
            const QString package = list.at(1);
            QString percent = list.at(2);
            QString str = list.at(3);
            // If str legitimately had a ':' in it (such as a package version)
            // we need to retrieve the next string in the list.
            if (list.count() == 5) {
                str += QString(':' % list.at(4));
            }

            if (package.isEmpty() || status.isEmpty()) {
                continue;
            }

            if (status.contains(QLatin1String("pmerror"))) {
                // Append error string to existing error details
                m_trans->setErrorDetails(m_trans->errorDetails() % package % '\n' % str % "\n\n");
            } else if (status.contains(QLatin1String("pmconffile"))) {
                // From what I understand, the original file starts after the ' character ('\'') and
                // goes to a second ' character. The new conf file starts at the next ' and goes to
                // the next '.
                QStringList strList = str.split(QLatin1Char('\''));
                QString oldFile = strList.at(1);
                QString newFile = strList.at(2);

                QVariantMap args;
                args[QLatin1String("OldConfFile")] = oldFile;
                args[QLatin1String("NewConfFile")] = newFile;

                // FIXME: use transaction to query user, wait for resume
                bool replaceFile;

                if (replaceFile) {
                    ssize_t reply = write(writeFd, "Y\n", 2);
                    Q_UNUSED(reply);
                } else {
                    ssize_t reply = write(writeFd, "N\n", 2);
                    Q_UNUSED(reply);
                }
            } else {
                m_startCounting = true;
            }

            int percentage;
            int progress;
            if (percent.contains(QLatin1Char('.'))) {
                QStringList percentList = percent.split(QLatin1Char('.'));
                percentage = percentList.at(0).toInt();
            } else {
                percentage = percent.toInt();
            }

            progress = qRound(m_progressBegin + (percentage / 100) * (m_progressEnd - m_progressBegin));

            m_trans->setProgress(progress);
            m_trans->setStatusDetails(str);
            // clean-up
            line[0] = 0;
        } else {
            buf[1] = 0;
            strcat(line, buf);
        }
    }
    // 30 frames per second
    usleep(1000000/30);
}
