/***************************************************************************
 *   Copyright © 2011 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *   Copyright © 2014 Harald Sitter <apachelogger@kubuntu.org>             *
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

#include "debthumbnailer.h"

#include <QDir>
#include <QStringBuilder>
#include <QImage>
#include <QPainter>
#include <QTemporaryDir>

#include <QIcon>
#include <QDebug>

#include <QApt/DebFile>
#include <KPluginFactory>

DebThumbnailer::~DebThumbnailer()
{
}

static bool generateThumbnail(const QString &path, const int width, const int height, QImage &img)
{
    const QApt::DebFile debFile(path);

    if (!debFile.isValid()) {
        qDebug() << Q_FUNC_INFO << "debfile not valid";
        return false;
    }

    QStringList iconsList = debFile.iconList();

    // Drop everything but pngs and xpms.
    // ::iconList is based on ::fileList which contrary to what the name suggests
    // does a full content list including parent directories.
    // To get sensible results we therefore need to discard everything we cannot
    // identify as supported.
    // TODO: should debfile ever get more sensible this should be changed to
    //       exclude unsupported formats (svg) rather than include supported ones.
    for (auto it = iconsList.begin(); it != iconsList.end(); ++it) {
        if (!(*it).endsWith(QStringLiteral(".png")) && !(*it).endsWith(QStringLiteral(".xpm"))) {
            iconsList.erase(it);
        }
    }

    iconsList.sort();

    if (iconsList.isEmpty()) {
        return false;
    }

    QString iconPath = iconsList.last();

    QTemporaryDir tempDir = QTemporaryDir();
    tempDir.setAutoRemove(true);

    QString destPath = tempDir.path();

    if (!debFile.extractFileFromArchive(iconPath, destPath)) {
        return false;
    }

    QPixmap mimeIcon = QIcon::fromTheme("application-x-deb").pixmap(width, height);
    QPixmap appOverlay = QPixmap(destPath % iconPath).scaledToWidth(width/2);

    QPainter painter(&mimeIcon);
    for (int y = 0; y < appOverlay.height(); y += appOverlay.height()) {
        painter.drawPixmap( 0, y, appOverlay );
    }

    img = mimeIcon.toImage();

    return true;
}

#if KIO_VERSION_MAJOR >= 6
KIO::ThumbnailResult DebThumbnailer::create(const KIO::ThumbnailRequest &request) {
    const auto url = request.url();
    const QString path = url.toLocalFile();
    const QSize targetSize = request.targetSize();
    const int width = targetSize.width();
    const int height = targetSize.height();
    QImage img{};

    if (generateThumbnail(path, width, height, img)) {
        return KIO::ThumbnailResult::pass(img);
    } else {
        return KIO::ThumbnailResult::fail();
    }
}
#else
bool DebThumbnailer::create(const QString &path, int width, int height, QImage &img) {
    return generateThumbnail(path, width, height, img);
}
#endif

K_PLUGIN_CLASS_WITH_JSON(DebThumbnailer, "debthumbnailer.json")

#include "debthumbnailer.moc"
#include "moc_debthumbnailer.cpp"
