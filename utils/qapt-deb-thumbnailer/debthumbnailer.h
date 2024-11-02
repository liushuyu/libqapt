/***************************************************************************
 *   Copyright © 2011 Jonathan Thomas <echidnaman@kubuntu.org>             *
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

#ifndef DEBTHUMBNAILER_H
#define DEBTHUMBNAILER_H

#include <QStringList>

#include <kio_version.h>
#if KIO_VERSION_MAJOR >= 6
#include <kio/thumbnailcreator.h>
using ThumbCreator = KIO::ThumbnailCreator;
#else
#include <kio/thumbcreator.h>
#endif

namespace QApt {
    class DebFile;
}

class DebThumbnailer : public ThumbCreator
{
    Q_OBJECT
public:
    virtual ~DebThumbnailer() override;

#if KIO_VERSION_MAJOR >= 6
    DebThumbnailer(QObject *parent, const QVariantList &args) : ThumbCreator(parent, args) {}
    virtual KIO::ThumbnailResult create(const KIO::ThumbnailRequest &request) override;
#else
    DebThumbnailer() = default;
    virtual bool create(const QString &path, int w, int h, QImage &img);
    virtual Flags flags() const override {
        return None;
    }
#endif
};

#endif
