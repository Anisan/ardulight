/*
 * grab_winapi.cpp
 *
 *  Created on: 9.03.2011
 *      Author: Mike Shatohin (brunql)
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Thanks Amblone project for WinAPI grab example: http://amblone.com/
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define WINVER 0x0500 /* Windows2000 for MonitorFromWindow(..) func */
#include <windows.h>
#include <cmath>

#include "grab_api.h"

namespace GrabWinAPI
{

// Position of monitor, initialize in captureScreen() using in getColor()
MONITORINFO monitorInfo;

// Size of screen in pixels, initialize in captureScreen() using in getColor()
unsigned screenWidth;
unsigned screenHeight;

// Captured screen buffer, contains actual RGB data in reversed order
BYTE * pbPixelsBuff = NULL;
unsigned pixelsBuffSize;
unsigned bytesPerPixel;

HWND hWndForFindMonitor = NULL;
bool updateScreenAndAllocateMemory = true;

HDC hScreenDC;
HDC hMemDC;
HBITMAP hBitmap;

 QPixmap grabScreen(WId id,int x, int y,int w,int h)
 {
     if( updateScreenAndAllocateMemory ){
         // Find the monitor, what contains firstLedWidget
         HMONITOR hMonitor = MonitorFromWindow( id, MONITOR_DEFAULTTONEAREST );
         ZeroMemory( &monitorInfo, sizeof(MONITORINFO) );
         monitorInfo.cbSize = sizeof(MONITORINFO);
         // Get position and resolution of the monitor
         GetMonitorInfo( hMonitor, &monitorInfo );
         screenWidth  = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
         screenHeight = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
         // CreateDC for multiple monitors
         hScreenDC = CreateDC( TEXT("DISPLAY"), NULL, NULL, NULL );
         // Create a bitmap compatible with the screen DC
         hBitmap = CreateCompatibleBitmap( hScreenDC, screenWidth, screenHeight );
         // Create a memory DC compatible to screen DC
         hMemDC = CreateCompatibleDC( hScreenDC );
         // Select new bitmap into memory DC
         SelectObject( hMemDC, hBitmap );
     }
     // Copy screen
     BitBlt( hMemDC, 0, 0, screenWidth, screenHeight, hScreenDC,
            monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, SRCCOPY );
     if( updateScreenAndAllocateMemory ){
         BITMAP * bmp = new BITMAP;
         // Now get the actual Bitmap
         GetObject( hBitmap, sizeof(BITMAP), bmp );
         // Calculate the size the buffer needs to be
         unsigned pixelsBuffSizeNew = bmp->bmWidthBytes * bmp->bmHeight;
         if(pixelsBuffSize != pixelsBuffSizeNew){
             pixelsBuffSize = pixelsBuffSizeNew;
             // ReAllocate memory for new buffer size
             if( pbPixelsBuff ) delete pbPixelsBuff;
             // Allocate
             pbPixelsBuff = new BYTE[ pixelsBuffSize ];
         }
         // The amount of bytes per pixel is the amount of bits divided by 8
         bytesPerPixel = bmp->bmBitsPixel / 8;
         if( bytesPerPixel != 4 ){
         //    qDebug() << "Not 32-bit mode is not supported!" << bytesPerPixel;
         }
         DeleteObject( bmp );
         updateScreenAndAllocateMemory = false;
     }
     // Get the actual RGB data and put it into pbPixelsBuff
     GetBitmapBits( hBitmap, pixelsBuffSize, pbPixelsBuff );
     return QPixmap::fromWinHBITMAP(hBitmap);//, QPixmap::PremultipliedAlpha);
 }
}
