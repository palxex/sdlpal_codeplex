//
// Copyright (c) 2009, Wei Mingzhi <whistler@openoffice.org>.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "main.h"

// Screen buffer
SDL_Surface              *gpScreen           = NULL;

// Backup screen buffer
SDL_Surface              *gpScreenBak        = NULL;

// The real screen surface
SDL_Surface              *gpScreenReal       = NULL;

static SDL_Surface       *gpScreenBuf        = NULL;

#if (defined (__SYMBIAN32__) && !defined (__S60_5X__)) || defined (PSP)
   static BOOL bScaleScreen = FALSE;
#else
   static BOOL bScaleScreen = TRUE;
#endif

// Initial screen size
static WORD               g_wInitialWidth    = 640;
static WORD               g_wInitialHeight   = 400;

// Shake times and level
static WORD               g_wShakeTime       = 0;
static WORD               g_wShakeLevel      = 0;

INT
VIDEO_Init(
   WORD             wScreenWidth,
   WORD             wScreenHeight,
   BOOL             fFullScreen
)
/*++
  Purpose:

    Initialze the video subsystem.

  Parameters:

    [IN]  wScreenWidth - width of the screen.

    [IN]  wScreenHeight - height of the screen.

    [IN]  fFullScreen - TRUE to use full screen mode, FALSE to use windowed mode.

  Return value:

    0 = success, -1 = fail to create the screen surface,
    -2 = fail to create screen buffer.

--*/
{
   g_wInitialWidth = wScreenWidth;
   g_wInitialHeight = wScreenHeight;

   //
   // Create the screen surface.
   //
   gpScreenReal = SDL_SetVideoMode(wScreenWidth, wScreenHeight, 24,
      SDL_HWSURFACE | SDL_RESIZABLE | (fFullScreen ? SDL_FULLSCREEN : 0));

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to 640x480 software mode.
      //
      gpScreenReal = SDL_SetVideoMode(640, 480, 24,
         SDL_SWSURFACE | (fFullScreen ? SDL_FULLSCREEN : 0));
   }

   //
   // Still fail?
   //
   if (gpScreenReal == NULL)
   {
      return -1;
   }

   //
   // Create the screen buffer and the backup screen buffer.
   //
   gpScreen = SDL_CreateRGBSurface(gpScreenReal->flags & ~SDL_HWSURFACE, 320, 200, 8,
      gpScreenReal->format->Rmask, gpScreenReal->format->Gmask,
      gpScreenReal->format->Bmask, gpScreenReal->format->Amask);

   gpScreenBak = SDL_CreateRGBSurface(gpScreenReal->flags & ~SDL_HWSURFACE, 320, 200, 8,
      gpScreenReal->format->Rmask, gpScreenReal->format->Gmask,
      gpScreenReal->format->Bmask, gpScreenReal->format->Amask);

   gpScreenBuf = SDL_CreateRGBSurface(gpScreenReal->flags & ~SDL_HWSURFACE, 320, 200, 24,
      gpScreenReal->format->Rmask, gpScreenReal->format->Gmask,
      gpScreenReal->format->Bmask, gpScreenReal->format->Amask);

   //
   // Failed?
   //
   if (gpScreen == NULL || gpScreenBak == NULL || gpScreenBuf == NULL)
   {
      if (gpScreen != NULL)
      {
         SDL_FreeSurface(gpScreen);
      }

      if (gpScreenBak != NULL)
      {
         SDL_FreeSurface(gpScreenBak);
      }

      if (gpScreenBuf != NULL)
      {
         SDL_FreeSurface(gpScreenBuf);
      }

      SDL_FreeSurface(gpScreenReal);
      return -2;
   }

   if (fFullScreen)
   {
      SDL_ShowCursor(FALSE);
   }

   PAL_SetPalette(0, FALSE);
   return 0;
}

VOID
VIDEO_Shutdown(
   VOID
)
/*++
  Purpose:

    Shutdown the video subsystem.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   if (gpScreen != NULL)
   {
      SDL_FreeSurface(gpScreen);
   }
   gpScreen = NULL;

   if (gpScreenBak != NULL)
   {
      SDL_FreeSurface(gpScreenBak);
   }
   gpScreenBak = NULL;

   if (gpScreenBuf != NULL)
   {
      SDL_FreeSurface(gpScreenBuf);
   }
   gpScreenBuf = NULL;

   if (gpScreenReal != NULL)
   {
      SDL_FreeSurface(gpScreenReal);
   }
   gpScreenReal = NULL;
}

VOID
VIDEO_UpdateScreen(
   const SDL_Rect  *lpRect
)
/*++
  Purpose:

    Update the screen area specified by lpRect.

  Parameters:

    [IN]  lpRect - Screen area to update.

  Return value:

    None.

--*/
{
   SDL_Rect        srcrect, dstrect, rect;
   short           offset = 240 - 200;
   short           screenRealHeight = gpScreenReal->h;
   short           screenRealY = 0;

   //
   // Lock surface if needed
   //
   if (SDL_MUSTLOCK(gpScreenReal))
   {
      if (SDL_LockSurface(gpScreenReal) < 0)
         return;
   }

   if (!bScaleScreen)
   {
      screenRealHeight -= offset;
      screenRealY = offset / 2;
   }

   if (lpRect != NULL)
   {
      rect = *lpRect;
	  
      if (rect.x + rect.w > 320) rect.w = 320 - rect.x;
      if (rect.y + rect.h > 200) rect.h = 200 - rect.y;

	  if ((Sint16)rect.w <= 0 || (Sint16)rect.h <= 0) return;

      dstrect.x = (SHORT)((INT)rect.x * gpScreenReal->w / gpScreen->w);
      dstrect.y = (SHORT)((INT)(screenRealY + rect.y) * screenRealHeight / gpScreen->h);
      dstrect.w = (WORD)((DWORD)rect.w * gpScreenReal->w / gpScreen->w);
      dstrect.h = (WORD)((DWORD)rect.h * screenRealHeight / gpScreen->h);

      SDL_BlitSurface(gpScreen, &rect, gpScreenBuf, &rect);
      SDL_SoftStretch(gpScreenBuf, &rect, gpScreenReal, &dstrect);

      if (SDL_MUSTLOCK(gpScreenReal))
      {
         SDL_UnlockSurface(gpScreenReal);
      }

      SDL_UpdateRect(gpScreenReal, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
   }
   else if (g_wShakeTime != 0)
   {
      //
      // Shake the screen
      //
      srcrect.x = 0;
      srcrect.y = 0;
      srcrect.w = 320;
      srcrect.h = 200 - g_wShakeLevel;

      dstrect.x = 0;
      dstrect.y = screenRealY;
      dstrect.w = 320 * gpScreenReal->w / gpScreen->w;
      dstrect.h = (200 - g_wShakeLevel) * screenRealHeight / gpScreen->h;

      if (g_wShakeTime & 1)
      {
         srcrect.y = g_wShakeLevel;
      }
      else
      {
         dstrect.y = (screenRealY + g_wShakeLevel) * screenRealHeight / gpScreen->h;
      }

      SDL_BlitSurface(gpScreen, NULL, gpScreenBuf, NULL);
      SDL_SoftStretch(gpScreenBuf, &srcrect, gpScreenReal, &dstrect);

      if (g_wShakeTime & 1)
      {
         dstrect.y = (screenRealY + screenRealHeight - g_wShakeLevel) * screenRealHeight / gpScreen->h;
      }
      else
      {
         dstrect.y = screenRealY;
      }

      dstrect.h = g_wShakeLevel * screenRealHeight / gpScreen->h;

      SDL_FillRect(gpScreenReal, &dstrect, 0);

      if (SDL_MUSTLOCK(gpScreenReal))
      {
         SDL_UnlockSurface(gpScreenReal);
      }

      SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);

      g_wShakeTime--;
   }
   else
   {
      dstrect.x = 0;
      dstrect.y = screenRealY;
      dstrect.w = gpScreenReal->w;
      dstrect.h = screenRealHeight;

      SDL_BlitSurface(gpScreen, NULL, gpScreenBuf, NULL);
      SDL_SoftStretch(gpScreenBuf, NULL, gpScreenReal, &dstrect);

      if (SDL_MUSTLOCK(gpScreenReal))
      {
         SDL_UnlockSurface(gpScreenReal);
      }

      SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
   }
}

VOID
VIDEO_SetPalette(
   SDL_Color        rgPalette[256]
)
/*++
  Purpose:

    Set the palette of the screen.

  Parameters:

    [IN]  rgPalette - array of 256 colors.

  Return value:

    None.

--*/
{
   WORD wShakeTimeBak = g_wShakeTime;

   SDL_SetPalette(gpScreen, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);
   SDL_SetPalette(gpScreenBak, SDL_LOGPAL | SDL_PHYSPAL, rgPalette, 0, 256);

   g_wShakeTime = 0;
   VIDEO_UpdateScreen(NULL);
   g_wShakeTime = wShakeTimeBak;
}

VOID
VIDEO_Resize(
   INT             w,
   INT             h
)
/*++
  Purpose:

    This function is called when user resized the window.

  Parameters:

    [IN]  w - width of the window after resizing.

    [IN]  h - height of the window after resizing.

  Return value:

    None.

--*/
{
   DWORD                    flags;

   //
   // Create the screen surface.
   //
   flags = gpScreenReal->flags;

   SDL_FreeSurface(gpScreenReal);
   gpScreenReal = SDL_SetVideoMode(w, h, 24, flags);

   if (gpScreenReal == NULL)
   {
      //
      // Fall back to 640x480 software windowed mode.
      //
      gpScreenReal = SDL_SetVideoMode(640, 480, 24, SDL_SWSURFACE);
   }

   VIDEO_UpdateScreen(NULL);
}

SDL_Color *
VIDEO_GetPalette(
   VOID
)
/*++
  Purpose:

    Get the current palette of the screen.

  Parameters:

    None.

  Return value:

    Pointer to the current palette.

--*/
{
   return gpScreen->format->palette->colors;
}

VOID
VIDEO_ToggleFullscreen(
   VOID
)
/*++
  Purpose:

    Toggle fullscreen mode.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   DWORD                    flags;

   //
   // Get the flags of the original screen surface
   //
   flags = gpScreenReal->flags;

   if (flags & SDL_FULLSCREEN)
   {
      //
      // Already in fullscreen mode. Remove the fullscreen flag.
      //
      flags &= ~SDL_FULLSCREEN;
      flags |= SDL_RESIZABLE;
      SDL_ShowCursor(TRUE);
   }
   else
   {
      //
      // Not in fullscreen mode. Set the fullscreen flag.
      //
      flags |= SDL_FULLSCREEN;
      SDL_ShowCursor(FALSE);
   }

   //
   // Free the original screen surface
   //
   SDL_FreeSurface(gpScreenReal);

   //
   // ... and create a new one
   //
   if (g_wInitialWidth == 640 && g_wInitialHeight == 400 && (flags & SDL_FULLSCREEN))
   {
      gpScreenReal = SDL_SetVideoMode(640, 480, 24, flags);
   }
   else if (g_wInitialWidth == 640 && g_wInitialHeight == 480 && !(flags & SDL_FULLSCREEN))
   {
      gpScreenReal = SDL_SetVideoMode(640, 400, 24, flags);
   }
   else
   {
      gpScreenReal = SDL_SetVideoMode(g_wInitialWidth, g_wInitialHeight, 24, flags);
   }

   //
   // Update the screen
   //
   VIDEO_UpdateScreen(NULL);
}

VOID
VIDEO_SaveScreenshot(
   VOID
)
/*++
  Purpose:

    Save the screenshot of current screen to a BMP file.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   int      iNumBMP = 0;
   FILE    *fp;

   //
   // Find a usable BMP filename.
   //
   for (iNumBMP = 0; iNumBMP <= 9999; iNumBMP++)
   {
      fp = fopen(va("%sscrn%.4d.bmp", PAL_PREFIX, iNumBMP), "rb");
      if (fp == NULL)
      {
         break;
      }
      fclose(fp);
   }

   if (iNumBMP > 9999)
   {
      return;
   }

   //
   // Save the screenshot.
   //
   SDL_SaveBMP(gpScreenReal, va("%sscrn%.4d.bmp", PAL_PREFIX, iNumBMP));
}

VOID
VIDEO_BackupScreen(
   VOID
)
/*++
  Purpose:

    Backup the screen buffer.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   SDL_BlitSurface(gpScreen, NULL, gpScreenBak, NULL);
}

VOID
VIDEO_RestoreScreen(
   VOID
)
/*++
  Purpose:

    Restore the screen buffer which has been saved with VIDEO_BackupScreen().

  Parameters:

    None.

  Return value:

    None.

--*/
{
   SDL_BlitSurface(gpScreenBak, NULL, gpScreen, NULL);
}

VOID
VIDEO_ShakeScreen(
   WORD           wShakeTime,
   WORD           wShakeLevel
)
/*++
  Purpose:

    Set the screen shake time and level.

  Parameters:

    [IN]  wShakeTime - how many times should we shake the screen.

    [IN]  wShakeLevel - level of shaking.

  Return value:

    None.

--*/
{
   g_wShakeTime = wShakeTime;
   g_wShakeLevel = wShakeLevel;
}

VOID
VIDEO_SwitchScreen(
   WORD           wSpeed
)
/*++
  Purpose:

    Switch the screen from the backup screen buffer to the current screen buffer.
    NOTE: This will destroy the backup buffer.

  Parameters:

    [IN]  wSpeed - speed of fading (the larger value, the slower).

  Return value:

    None.

--*/
{
   int               i, j;
   const int         rgIndex[6] = {0, 3, 1, 5, 2, 4};
   SDL_Rect          dstrect;

   short             offset = 240 - 200;
   short             screenRealHeight = gpScreenReal->h;
   short             screenRealY = 0;

   if (!bScaleScreen)
   {
      screenRealHeight -= offset;
      screenRealY = offset / 2;
   }

   wSpeed++;
   wSpeed *= 10;

   for (i = 0; i < 6; i++)
   {
      for (j = rgIndex[i]; j < gpScreen->pitch * gpScreen->h; j += 6)
      {
         ((LPBYTE)(gpScreenBak->pixels))[j] = ((LPBYTE)(gpScreen->pixels))[j];
      }

      //
      // Draw the backup buffer to the screen
      //
      dstrect.x = 0;
      dstrect.y = screenRealY;
      dstrect.w = gpScreenReal->w;
      dstrect.h = screenRealHeight;

      SDL_BlitSurface(gpScreenBak, NULL, gpScreenBuf, NULL);
      SDL_SoftStretch(gpScreenBuf, NULL, gpScreenReal, &dstrect);
      SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);

      UTIL_Delay(wSpeed);
   }
}

VOID
VIDEO_FadeScreen(
   WORD           wSpeed
)
/*++
  Purpose:

    Fade from the backup screen buffer to the current screen buffer.
    NOTE: This will destroy the backup buffer.

  Parameters:

    [IN]  wSpeed - speed of fading (the larger value, the slower).

  Return value:

    None.

--*/
{
   int               i, j, k;
   DWORD             time;
   BYTE              a, b;
   const int         rgIndex[6] = {0, 3, 1, 5, 2, 4};
   SDL_Rect          dstrect;
   short             offset = 240 - 200;
   short             screenRealHeight = gpScreenReal->h;
   short             screenRealY = 0;

   //
   // Lock surface if needed
   //
   if (SDL_MUSTLOCK(gpScreenReal))
   {
      if (SDL_LockSurface(gpScreenReal) < 0)
         return;
   }

   if (!bScaleScreen)
   {
      screenRealHeight -= offset;
      screenRealY = offset / 2;
   }

   time = SDL_GetTicks();

   wSpeed++;
   wSpeed *= 10;

   for (i = 0; i < 12; i++)
   {
      for (j = 0; j < 6; j++)
      {
         PAL_ProcessEvent();
         while (SDL_GetTicks() <= time)
         {
            PAL_ProcessEvent();
            SDL_Delay(5);
         }
         time = SDL_GetTicks() + wSpeed;

         //
         // Blend the pixels in the 2 buffers, and put the result into the
         // backup buffer
         //
         for (k = rgIndex[j]; k < gpScreen->pitch * gpScreen->h; k += 6)
         {
            a = ((LPBYTE)(gpScreen->pixels))[k];
            b = ((LPBYTE)(gpScreenBak->pixels))[k];

            if (i > 0)
            {
               if ((a & 0x0F) > (b & 0x0F))
               {
                  b++;
               }
               else if ((a & 0x0F) < (b & 0x0F))
               {
                  b--;
               }
            }

            ((LPBYTE)(gpScreenBak->pixels))[k] = ((a & 0xF0) | (b & 0x0F));
         }

         //
         // Draw the backup buffer to the screen
         //
         if (g_wShakeTime != 0)
         {
            //
            // Shake the screen
            //
            SDL_Rect srcrect, dstrect;

            srcrect.x = 0;
            srcrect.y = 0;
            srcrect.w = 320;
            srcrect.h = 200 - g_wShakeLevel;

            dstrect.x = 0;
            dstrect.y = screenRealY;
            dstrect.w = 320 * gpScreenReal->w / gpScreen->w;
            dstrect.h = (200 - g_wShakeLevel) * screenRealHeight / gpScreen->h;

            if (g_wShakeTime & 1)
            {
               srcrect.y = g_wShakeLevel;
            }
            else
            {
               dstrect.y = (screenRealY + g_wShakeLevel) * screenRealHeight / gpScreen->h;
            }

            SDL_BlitSurface(gpScreenBak, &srcrect, gpScreenBuf, &srcrect);
            SDL_SoftStretch(gpScreenBuf, &srcrect, gpScreenReal, &dstrect);

            if (g_wShakeTime & 1)
            {
               dstrect.y = (screenRealY + screenRealHeight - g_wShakeLevel) * screenRealHeight / gpScreen->h;
            }
            else
            {
               dstrect.y = screenRealY;
            }

            dstrect.h = g_wShakeLevel * screenRealHeight / gpScreen->h;

            SDL_FillRect(gpScreenReal, &dstrect, 0);
            SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
            g_wShakeTime--;
         }
         else
         {
            dstrect.x = 0;
            dstrect.y = screenRealY;
            dstrect.w = gpScreenReal->w;
            dstrect.h = screenRealHeight;

            SDL_BlitSurface(gpScreenBak, NULL, gpScreenBuf, NULL);
            SDL_SoftStretch(gpScreenBuf, NULL, gpScreenReal, &dstrect);
            SDL_UpdateRect(gpScreenReal, 0, 0, gpScreenReal->w, gpScreenReal->h);
         }
      }
   }

   if (SDL_MUSTLOCK(gpScreenReal))
   {
      SDL_UnlockSurface(gpScreenReal);
   }

   //
   // Draw the result buffer to the screen as the final step
   //
   VIDEO_UpdateScreen(NULL);
}
