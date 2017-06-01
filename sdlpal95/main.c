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
#include "getopt.h"

#ifdef NDS
#include "fat.h"
#endif

#define BITMAPNUM_SPLASH_UP         0x26
#define BITMAPNUM_SPLASH_DOWN       0x27
#define SPRITENUM_SPLASH_TITLE      0x47
#define SPRITENUM_SPLASH_CRANE      0x49
#define NUM_RIX_TITLE               0x5

static VOID
PAL_Init(
   WORD             wScreenWidth,
   WORD             wScreenHeight,
   BOOL             fFullScreen
)
/*++
  Purpose:

    Initialize everything needed by the game.

  Parameters:

    [IN]  wScreenWidth - width of the screen.

    [IN]  wScreenHeight - height of the screen.

    [IN]  fFullScreen - TRUE to use full screen mode, FALSE to use windowed mode.

  Return value:

    None.

--*/
{
   int           e;

#ifdef NDS
   fatInitDefault();
#endif

   //
   // Initialize defaults, video and audio
   //
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_CDROM | SDL_INIT_NOPARACHUTE | SDL_INIT_JOYSTICK) == -1)
   {
#if defined (_WIN32) && SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
      //
      // Try the WINDIB driver if DirectX failed.
      //
      putenv("SDL_VIDEODRIVER=windib");
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE | SDL_INIT_JOYSTICK) == -1)
      {
         TerminateOnError("Could not initialize SDL: %s.\n", SDL_GetError());
      }
#else
      TerminateOnError("Could not initialize SDL: %s.\n", SDL_GetError());
#endif
   }

   //
   // Initialize subsystems.
   //
   e = VIDEO_Init(wScreenWidth, wScreenHeight, fFullScreen);
   if (e != 0)
   {
      TerminateOnError("Could not initialize Video: %d.\n", e);
   }

   SDL_WM_SetCaption("Loading...", NULL);

   e = PAL_InitGlobals();
   if (e != 0)
   {
      TerminateOnError("Could not initialize global data: %d.\n", e);
   }

   e = PAL_InitFont();
   if (e != 0)
   {
      TerminateOnError("Could not load fonts: %d.\n", e);
   }

   e = PAL_InitUI();
   if (e != 0)
   {
      TerminateOnError("Could not initialize UI subsystem: %d.\n", e);
   }

   e = PAL_InitText();
   if (e != 0)
   {
      TerminateOnError("Could not initialize text subsystem: %d.\n", e);
   }

   PAL_InitInput();
   PAL_InitResources();
   SOUND_OpenAudio();

#ifdef _DEBUG
   SDL_WM_SetCaption("Pal WIN95 (Debug Build)", NULL);
#else
   SDL_WM_SetCaption("Pal WIN95", NULL);
#endif
}

VOID
PAL_Shutdown(
   VOID
)
/*++
  Purpose:

    Free everything needed by the game.

  Parameters:

    None.

  Return value:

    None.

--*/
{
   SOUND_CloseAudio();
   PAL_FreeFont();
   PAL_FreeResources();
   PAL_FreeGlobals();
   PAL_FreeUI();
   PAL_FreeText();
   PAL_ShutdownInput();
   VIDEO_Shutdown();

   UTIL_CloseLog();

   SDL_Quit();
}

int
main(
   int      argc,
   char    *argv[]
)
/*++
  Purpose:

    Program entry.

  Parameters:

    argc - Number of arguments.

    argv - Array of arguments.

  Return value:

    Integer value.

--*/
{
   WORD          wScreenWidth = 0, wScreenHeight = 0;
   int           c;
   BOOL          fFullScreen = FALSE;

   UTIL_OpenLog();

#ifdef _WIN32
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION <= 2
   putenv("SDL_VIDEODRIVER=directx");
#else
   putenv("SDL_VIDEODRIVER=win32");
#endif
#endif

#ifndef __SYMBIAN32__
   //
   // Parse parameters.
   //
   while ((c = getopt(argc, argv, "w:h:fj")) != -1)
   {
      switch (c)
      {
      case 'w':
         //
         // Set the width of the screen
         //
         wScreenWidth = atoi(optarg);
         if (wScreenHeight == 0)
         {
            wScreenHeight = wScreenWidth * 200 / 320;
         }
         break;

      case 'h':
         //
         // Set the height of the screen
         //
         wScreenHeight = atoi(optarg);
         if (wScreenWidth == 0)
         {
            wScreenWidth = wScreenHeight * 320 / 200;
         }
         break;

      case 'f':
         //
         // Fullscreen Mode
         //
         fFullScreen = TRUE;
         break;

      case 'j':
         //
         // Disable joystick
         //
         g_fUseJoystick = FALSE;
         break;
      }
   }
#endif

   //
   // Default resolution is 640x400 (windowed) or 640x480 (fullscreen).
   //
   if (wScreenWidth == 0)
   {
#ifdef __SYMBIAN32__
#ifdef __S60_5X__
      wScreenWidth = 640;
      wScreenHeight = 360;
#else
      wScreenWidth = 320;
      wScreenHeight = 240;
#endif
#else
      wScreenWidth = 640;
      wScreenHeight = fFullScreen ? 480 : 400;
#endif
   }

   //
   // Initialize everything
   //
   PAL_Init(wScreenWidth, wScreenHeight, fFullScreen);

   //
   // Show the trademark screen and splash screen
   //
   PAL_PlayAVI(PAL_PREFIX "1.avi");
   PAL_PlayAVI(PAL_PREFIX "2.avi");

   //
   // Run the main game routine
   //
   PAL_GameMain();

   //
   // Should not really reach here...
   //
   assert(FALSE);
   return 255;
}
