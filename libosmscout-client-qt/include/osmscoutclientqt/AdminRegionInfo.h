#ifndef OSMSCOUT_CLIENT_QT_ADMINREGIONINFO_H
#define OSMSCOUT_CLIENT_QT_ADMINREGIONINFO_H
/*
 OSMScout - a Qt backend for libosmscout and libosmscout-map
 Copyright (C) 2017 Lukas Karas

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <QObject>
#include <QString>

#include <memory>

#include <osmscoutclientqt/ClientQtImportExport.h>

#include <osmscout/location/Location.h>

namespace osmscout {

/**
 * \ingroup QtAPI
 */
class OSMSCOUT_CLIENT_QT_API AdminRegionInfo
{
public:
  QString database;
  osmscout::AdminRegionRef adminRegion;
  QString type; //!< adminRegion->object's type
  int adminLevel{-1};

public:
  std::string name() const
  {
    return adminRegion->name;
  }

  QString qStringName() const
  {
    return QString::fromStdString(adminRegion->name);
  }

  std::string altName() const
  {
    return adminRegion->altName;
  }

  QString qStringAltName() const
  {
    return QString::fromStdString(adminRegion->altName);
  }
};

typedef std::shared_ptr<AdminRegionInfo> AdminRegionInfoRef;

}

#endif //OSMSCOUT_CLIENT_QT_ADMINREGIONINFO_H
