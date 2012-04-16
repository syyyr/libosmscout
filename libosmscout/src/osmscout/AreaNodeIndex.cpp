/*
  This source is part of the libosmscout library
  Copyright (C) 2011  Tim Teulings

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

#include <osmscout/AreaNodeIndex.h>

#include <iostream>

#include <osmscout/private/Math.h>

namespace osmscout {

  AreaNodeIndex::TypeData::TypeData()
  : indexLevel(0),
    indexOffset(0),
    cellXStart(0),
    cellXEnd(0),
    cellYStart(0),
    cellYEnd(0),
    cellXCount(0),
    cellYCount(0),
    cellWidth(0.0),
    cellHeight(0.0),
    minLon(0.0),
    maxLon(0.0),
    minLat(0.0),
    maxLat(0.0)
  {
  }

  AreaNodeIndex::AreaNodeIndex()
  : filepart("areanode.idx")
  {
    // no code
  }

  bool AreaNodeIndex::Load(const std::string& path)
  {
    datafilename=path+"/"+filepart;

    if (!scanner.Open(datafilename)) {
      std::cerr << "Cannot open file '" << datafilename << "'" << std::endl;
      return false;
    }

    uint32_t indexEntries;

    scanner.Read(indexEntries);

    for (size_t i=0; i<indexEntries; i++) {
      TypeId type;

      scanner.ReadNumber(type);

      if (type>=nodeTypeData.size()) {
        nodeTypeData.resize(type+1);
      }

      scanner.Read(nodeTypeData[type].indexOffset);

      scanner.ReadNumber(nodeTypeData[type].indexLevel);

      scanner.ReadNumber(nodeTypeData[type].cellXStart);
      scanner.ReadNumber(nodeTypeData[type].cellXEnd);
      scanner.ReadNumber(nodeTypeData[type].cellYStart);
      scanner.ReadNumber(nodeTypeData[type].cellYEnd);

      nodeTypeData[type].cellXCount=nodeTypeData[type].cellXEnd-nodeTypeData[type].cellXStart+1;
      nodeTypeData[type].cellYCount=nodeTypeData[type].cellYEnd-nodeTypeData[type].cellYStart+1;

      nodeTypeData[type].cellWidth=360.0/pow(2.0,(int)nodeTypeData[type].indexLevel);
      nodeTypeData[type].cellHeight=180.0/pow(2.0,(int)nodeTypeData[type].indexLevel);

      nodeTypeData[type].minLon=nodeTypeData[type].cellXStart*nodeTypeData[type].cellWidth-180.0;
      nodeTypeData[type].maxLon=(nodeTypeData[type].cellXEnd+1)*nodeTypeData[type].cellWidth-180.0;
      nodeTypeData[type].minLat=nodeTypeData[type].cellYStart*nodeTypeData[type].cellHeight-90.0;
      nodeTypeData[type].maxLat=(nodeTypeData[type].cellYEnd+1)*nodeTypeData[type].cellHeight-90.0;
    }

    return !scanner.HasError() && scanner.Close();
  }

  bool AreaNodeIndex::GetOffsets(const TypeData& typeData,
                                 double minlon,
                                 double minlat,
                                 double maxlon,
                                 double maxlat,
                                 size_t maxNodeCount,
                                 std::vector<FileOffset>& offsets,
                                 size_t currentSize,
                                 bool& sizeExceeded) const
  {
    std::set<FileOffset> newOffsets;

    if (typeData.indexOffset==0) {
      // No data for this type available
      return true;
    }

    if (maxlon<typeData.minLon ||
        minlon>=typeData.maxLon ||
        maxlat<typeData.minLat ||
        minlat>=typeData.maxLat) {
      // No data available in given bounding box
      return true;
    }

    uint32_t minxc=(uint32_t)floor((minlon+180.0)/typeData.cellWidth);
    uint32_t maxxc=(uint32_t)floor((maxlon+180.0)/typeData.cellWidth);

    uint32_t minyc=(uint32_t)floor((minlat+90.0)/typeData.cellHeight);
    uint32_t maxyc=(uint32_t)floor((maxlat+90.0)/typeData.cellHeight);

    minxc=std::max(minxc,typeData.cellXStart);
    maxxc=std::min(maxxc,typeData.cellXEnd);

    minyc=std::max(minyc,typeData.cellYStart);
    maxyc=std::min(maxyc,typeData.cellYEnd);

    std::vector<FileOffset> cellDataOffsets;

    cellDataOffsets.reserve(maxxc-minxc+1);

    // For each row
    for (size_t y=minyc; y<=maxyc; y++) {
      FileOffset cellIndexOffset=typeData.indexOffset+
                                 ((y-typeData.cellYStart)*typeData.cellXCount+
                                  minxc-typeData.cellXStart)*sizeof(FileOffset);

      cellDataOffsets.clear();

      if (!scanner.SetPos(cellIndexOffset)) {
        std::cerr << "Cannot go to type cell index position " << cellIndexOffset << std::endl;
        return false;
      }

      // For each column in row
      for (size_t x=minxc; x<=maxxc; x++) {
        FileOffset cellDataOffset;

        if (!scanner.Read(cellDataOffset)) {
          std::cerr << "Cannot read cell data position" << std::endl;
          return false;
        }

        if (cellDataOffset==0) {
          continue;
        }

        cellDataOffsets.push_back(cellDataOffset);
      }

      if (cellDataOffsets.empty()) {
        continue;
      }

      FileOffset offset=cellDataOffsets.front();

      assert(offset>cellIndexOffset);

      if (!scanner.SetPos(offset)) {
        std::cerr << "Cannot go to cell data position " << offset << std::endl;
        return false;
      }

      // For each data cell in row found
      for (size_t i=0; i<cellDataOffsets.size(); i++) {
        uint32_t   dataCount;
        FileOffset lastOffset=0;


        if (!scanner.ReadNumber(dataCount)) {
          std::cerr << "Cannot read cell data count" << std::endl;
          return false;
        }

        if (currentSize+newOffsets.size()+dataCount>maxNodeCount) {
          //std::cout << currentSize<< "+" << newOffsets.size() << "+" << dataCount << ">" << maxWayCount << std::endl;
          sizeExceeded=true;
          return true;
        }

        for (size_t d=0; d<dataCount; d++) {
          FileOffset objectOffset;

          scanner.ReadNumber(objectOffset);

          objectOffset+=lastOffset;

          newOffsets.insert(objectOffset);

          lastOffset=objectOffset;
        }
      }
    }

    for (std::set<FileOffset>::const_iterator offset=newOffsets.begin();
         offset!=newOffsets.end();
         ++offset) {
      offsets.push_back(*offset);
    }

    return true;
  }

  bool AreaNodeIndex::GetOffsets(double minlon,
                                 double minlat,
                                 double maxlon,
                                 double maxlat,
                                 const TypeSet& nodeTypes,
                                 size_t maxNodeCount,
                                 std::vector<FileOffset>& nodeOffsets) const
  {
    if (!scanner.IsOpen()) {
      if (!scanner.Open(datafilename)) {
        std::cerr << "Error while opening " << datafilename << " for reading!" << std::endl;
        return false;
      }
    }

    bool sizeExceeded=false;

    for (size_t i=0; i<nodeTypeData.size(); i++) {
      if (nodeTypes.IsTypeSet(i)) {
        if (!GetOffsets(nodeTypeData[i],
                        minlon,
                        minlat,
                        maxlon,
                        maxlat,
                        maxNodeCount,
                        nodeOffsets,
                        nodeOffsets.size(),
                        sizeExceeded)) {
          return false;
        }

        if (sizeExceeded) {
          break;
        }
      }
    }

    return true;
  }

  void AreaNodeIndex::DumpStatistics()
  {
  }
}

