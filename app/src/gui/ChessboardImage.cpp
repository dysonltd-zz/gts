/*
 * Copyright (C) 2007-2013 Dyson Technology Ltd, all rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ChessboardImage.h"

namespace ChessboardImage
{
    const QImage CreateImage(const int rows,
                              const int cols,
                              const int squareSizePxls)
    {
        QImage img(cols*squareSizePxls, rows*squareSizePxls, QImage::Format_Mono);

        for (int r = 0; r < rows; ++r)
        {
            bool black = r % 2;

            for (int c = 0; c < cols; ++c)
            {
                uint colour = black ? 0 : 1;
                black = !black;

                for (int imgX = c*squareSizePxls; imgX < (c+1)*squareSizePxls; ++imgX)
                {
                    for (int imgY = r*squareSizePxls; imgY < (r+1)*squareSizePxls; ++imgY)
                    {
                        img.setPixel(imgX, imgY, colour);
                    }
                }
            }
        }

        return img;
    }
}
