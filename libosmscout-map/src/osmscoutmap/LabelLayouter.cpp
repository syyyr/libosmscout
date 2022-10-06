/*
  This source is part of the libosmscout-map library
  Copyright (C) 2016  Tim Teulings

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

#include <osmscoutmap/LabelLayouter.h>

namespace osmscout {
  ContourLabelPositioner::Position ContourLabelPositioner::calculatePositions(const Projection& /*projection*/,
                                                                              const MapParameter& /*parameter*/,
                                                                              const PathLabelData& labelData,
                                                                              double pathLength,
                                                                              double labelWidth) const
  {
    double minimalSpace=2*labelData.contourLabelOffset;
    size_t workingLabelCount  =0;
    size_t labelCountIncrement=0;

    while (true) {
      size_t newLabelCount;

      if (workingLabelCount==0) {
        newLabelCount      =1;
        labelCountIncrement=1;
      }
      else {
        labelCountIncrement*=2;
        newLabelCount=workingLabelCount+labelCountIncrement;
      }

      double length=minimalSpace+
                    double(newLabelCount-1)*labelData.contourLabelSpace+
                    double(newLabelCount)*labelWidth;

      if (length>pathLength) {
        // labels + spaces exceed the length of the path
        break;
      }

      workingLabelCount=newLabelCount;
    }

    size_t countLabels=workingLabelCount;
    double offset=labelData.contourLabelOffset;
    double labelSpace=0.0;

    if (countLabels==0) {
      if (labelWidth>pathLength) {
        return {0,0.0,0.0};
      }

      countLabels=1;
    }

    if (countLabels==1) {
      // If we have one label, we center it
      offset=(pathLength-labelWidth)/2;
      assert(offset>=0.0);
    }
    else {
      // else we increase the labelSpace
      labelSpace=(pathLength-minimalSpace-double(countLabels)*labelWidth)/double(countLabels-1);
      assert(labelSpace>=labelData.contourLabelSpace);
    }

    assert((countLabels % 2)!=0);

    return {countLabels, offset, labelSpace};
  }


  OSMSCOUT_MAP_API void Mask::prepare(const IntRectangle &rect)
  {
    // clear
    for (int c = std::max(0, cellFrom); c <= std::min((int) d.size() - 1, cellTo); c++) {
      d[c] = 0;
    }

    // setup
    cellFrom = rect.x / 64;
    uint16_t cellFromBit = rect.x < 0 ? 0 : rect.x % 64;
    int to = (rect.x + rect.width);
    if (to < 0 || cellFrom >= (int)d.size())
      return; // mask is outside viewport, keep it blank
    cellTo = to / 64;
    uint16_t cellToBit = (rect.x + rect.width) % 64;
    if (cellToBit == 0){
      cellTo --;
    }

    rowFrom = rect.y;
    rowTo = rect.y + rect.height;

    constexpr uint64_t mask = ~0;
    for (int c = std::max(0, cellFrom); c <= std::min((int) d.size() - 1, cellTo); c++) {
      d[c] = mask;
    }
    if (cellFrom >= 0 && cellFrom < size() && cellFromBit > 0) {
      d[cellFrom] = mask << cellFromBit;
    }
    if (cellTo >= 0 && cellTo < size() && cellToBit > 0) {
      d[cellTo] = d[cellTo] & (mask >> (64 - cellToBit));
    }
  }
}
