#undef _NDEBUG
#include <assert.h>

#include "main.h"

#if defined(_WIN32)

#include <vfw.h>
#include <SDL_syswm.h>

int PAL_PlayAVI(const char *szFilename) {
        SDL_SysWMinfo wm;
        HWND hw;
        DWORD len, starttime;
        Uint32 prevw, prevh;
        extern SDL_Surface *gpScreenReal;

        SDL_VERSION(&wm.version);
        SDL_GetWMInfo(&wm);

        hw = MCIWndCreate(wm.window, (HINSTANCE)GetModuleHandle(NULL),
                WS_CHILD | WS_VISIBLE | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR, NULL);

        if (hw == NULL) return -1;

        if (MCIWndOpen(hw, szFilename, 0) != 0) {
                MCIWndDestroy(hw);
                return -1;
        }

        SDL_FillRect(gpScreen, NULL, 0);
        VIDEO_UpdateScreen(NULL);

        MCIWndSetZoom(hw, 100);
        MCIWndPlay(hw);

        PAL_ClearKeyState();
        len = MCIWndGetLength(hw) * (1000 / 15);
        starttime = SDL_GetTicks();

        prevw = gpScreenReal->w;
        prevh = gpScreenReal->h;

        MoveWindow(hw, 0, 0, gpScreenReal->w, gpScreenReal->h, TRUE);

        while (SDL_GetTicks() - starttime < len) {
                if (g_InputState.dwKeyPress != 0) break;
                if (prevw != gpScreenReal->w || prevh != gpScreenReal->h) {
                        MoveWindow(hw, 0, 0, gpScreenReal->w, gpScreenReal->h, TRUE);
                        prevw = gpScreenReal->w;
                        prevh = gpScreenReal->h;
                }
                UTIL_Delay(500);
        }

        PAL_ClearKeyState();

        MCIWndStop(hw);
        MCIWndDestroy(hw);

        return 0;
}

#else

VOID
PAL_TrademarkScreen(
  VOID
)
/*++
 Purpose:

   Show the trademark screen.

 Parameters:

   None.

 Return value:

   None.

--*/
{
  PAL_SetPalette(3, FALSE);
  PAL_RNGPlay(6, 0, 1000, 25);
  UTIL_Delay(1000);
  PAL_FadeOut(1);
}

#define BITMAPNUM_SPLASH_UP         3
#define BITMAPNUM_SPLASH_DOWN       4
#define SPRITENUM_SPLASH_TITLE      0x47
#define SPRITENUM_SPLASH_CRANE      0x49
#define NUM_RIX_TITLE               0x5

VOID
PAL_SplashScreen(
  VOID
)
/*++
 Purpose:

   Show the splash screen.

 Parameters:

   None.

 Return value:

   None.

--*/
{
  SDL_Color     *palette = PAL_GetPalette(1, FALSE);
  SDL_Color      rgCurrentPalette[256];
  SDL_Surface   *lpBitmapDown, *lpBitmapUp;
  SDL_Rect       srcrect, dstrect;
  LPSPRITE       lpSpriteCrane;
  LPBITMAPRLE    lpBitmapTitle;
  LPBYTE         buf, buf2;
  int            cranepos[9][3], i, iImgPos = 200, iCraneFrame = 0, iTitleHeight;
  DWORD          dwTime, dwBeginTime;
  BOOL           fUseCD = TRUE;

  if (palette == NULL)
  {
     fprintf(stderr, "ERROR: PAL_SplashScreen(): palette == NULL\n");
     return;
  }

  //
  // Allocate all the needed memory at once for simplification
  //
  buf = (LPBYTE)UTIL_calloc(1, 320 * 200 * 2);
  buf2 = (LPBYTE)(buf + 320 * 200);
  lpSpriteCrane = (LPSPRITE)buf2 + 32000;

  //
  // Create the surfaces
  //
  lpBitmapDown = SDL_CreateRGBSurface(gpScreen->flags, 320, 200, 8,
     gpScreen->format->Rmask, gpScreen->format->Gmask, gpScreen->format->Bmask,
     gpScreen->format->Amask);
  lpBitmapUp = SDL_CreateRGBSurface(gpScreen->flags, 320, 200, 8,
     gpScreen->format->Rmask, gpScreen->format->Gmask, gpScreen->format->Bmask,
     gpScreen->format->Amask);

  //
  // Read the bitmaps
  //
  PAL_MKFReadChunk(buf, 320 * 200, BITMAPNUM_SPLASH_UP, gpGlobals->f.fpFBP);
  DecodeYJ2(buf, buf2, 320 * 200);
  PAL_FBPBlitToSurface(buf2, lpBitmapUp);
  PAL_MKFReadChunk(buf, 320 * 200, BITMAPNUM_SPLASH_DOWN, gpGlobals->f.fpFBP);
  DecodeYJ2(buf, buf2, 320 * 200);
  PAL_FBPBlitToSurface(buf2, lpBitmapDown);
  PAL_MKFReadChunk(buf, 32000, SPRITENUM_SPLASH_TITLE, gpGlobals->f.fpMGO);
  DecodeYJ2(buf, buf2, 32000);
  lpBitmapTitle = (LPBITMAPRLE)PAL_SpriteGetFrame(buf2, 0);
  PAL_MKFReadChunk(buf, 32000, SPRITENUM_SPLASH_CRANE, gpGlobals->f.fpMGO);
  DecodeYJ2(buf, lpSpriteCrane, 32000);

  iTitleHeight = PAL_RLEGetHeight(lpBitmapTitle);
  lpBitmapTitle[2] = 0;
  lpBitmapTitle[3] = 0; // HACKHACK

  //
  // Generate the positions of the cranes
  //
  for (i = 0; i < 9; i++)
  {
     cranepos[i][0] = RandomLong(300, 600);
     cranepos[i][1] = RandomLong(0, 80);
     cranepos[i][2] = RandomLong(0, 8);
  }

  //
  // Play the title music
  //
  if (!SOUND_PlayCDA(7))
  {
     fUseCD = FALSE;
     RIX_Play(NUM_RIX_TITLE, TRUE, 2);
  }

  //
  // Clear all of the events and key states
  //
  PAL_ProcessEvent();
  PAL_ClearKeyState();

  dwBeginTime = SDL_GetTicks();

  srcrect.x = 0;
  srcrect.w = 320;
  dstrect.x = 0;
  dstrect.w = 320;

  while (TRUE)
  {
     PAL_ProcessEvent();
     dwTime = SDL_GetTicks() - dwBeginTime;

     //
     // Set the palette
     //
     if (dwTime < 15000)
     {
        for (i = 0; i < 256; i++)
        {
           rgCurrentPalette[i].r = (BYTE)(palette[i].r * ((float)dwTime / 15000));
           rgCurrentPalette[i].g = (BYTE)(palette[i].g * ((float)dwTime / 15000));
           rgCurrentPalette[i].b = (BYTE)(palette[i].b * ((float)dwTime / 15000));
        }
     }

     VIDEO_SetPalette(rgCurrentPalette);
     SDL_SetPalette(lpBitmapDown, SDL_LOGPAL | SDL_PHYSPAL, rgCurrentPalette, 0, 256);
     SDL_SetPalette(lpBitmapUp, SDL_LOGPAL | SDL_PHYSPAL, rgCurrentPalette, 0, 256);

     //
     // Draw the screen
     //
     if (iImgPos > 1)
     {
        iImgPos--;
     }

     //
     // The upper part...
     //
     srcrect.y = iImgPos;
     srcrect.h = 200 - iImgPos;

     dstrect.y = 0;
     dstrect.h = srcrect.h;

     SDL_BlitSurface(lpBitmapUp, &srcrect, gpScreen, &dstrect);

     //
     // The lower part...
     //
     srcrect.y = 0;
     srcrect.h = iImgPos;

     dstrect.y = 200 - iImgPos;
     dstrect.h = srcrect.h;

     SDL_BlitSurface(lpBitmapDown, &srcrect, gpScreen, &dstrect);

     //
     // Draw the cranes...
     //
     for (i = 0; i < 9; i++)
     {
        LPCBITMAPRLE lpFrame = PAL_SpriteGetFrame(lpSpriteCrane,
           cranepos[i][2] = (cranepos[i][2] + (iCraneFrame & 1)) % 8);
        cranepos[i][1] += ((iImgPos > 1) && (iImgPos & 1)) ? 1 : 0;
        PAL_RLEBlitToSurface(lpFrame, gpScreen,
           PAL_XY(cranepos[i][0], cranepos[i][1]));
        cranepos[i][0]--;
     }
     iCraneFrame++;

     //
     // Draw the title...
     //
     if (PAL_RLEGetHeight(lpBitmapTitle) < iTitleHeight)
     {
        //
        // HACKHACK
        //
        WORD w = lpBitmapTitle[2] | (lpBitmapTitle[3] << 8);
        w++;
        lpBitmapTitle[2] = (w & 0xFF);
        lpBitmapTitle[3] = (w >> 8);
     }

     PAL_RLEBlitToSurface(lpBitmapTitle, gpScreen, PAL_XY(255, 10));

     VIDEO_UpdateScreen(NULL);

     //
     // Check for keypress...
     //
     if (g_InputState.dwKeyPress & (kKeyMenu | kKeySearch))
     {
        //
        // User has pressed a key...
        //
        lpBitmapTitle[2] = iTitleHeight & 0xFF;
        lpBitmapTitle[3] = iTitleHeight >> 8; // HACKHACK

        PAL_RLEBlitToSurface(lpBitmapTitle, gpScreen, PAL_XY(255, 10));

        VIDEO_UpdateScreen(NULL);

        if (dwTime < 15000)
        {
           //
           // If the picture has not completed fading in, complete the rest
           //
           while (dwTime < 15000)
           {
              for (i = 0; i < 256; i++)
              {
                 rgCurrentPalette[i].r = (BYTE)(palette[i].r * ((float)dwTime / 15000));
                 rgCurrentPalette[i].g = (BYTE)(palette[i].g * ((float)dwTime / 15000));
                 rgCurrentPalette[i].b = (BYTE)(palette[i].b * ((float)dwTime / 15000));
              }
              VIDEO_SetPalette(rgCurrentPalette);
              UTIL_Delay(8);
              dwTime += 250;
           }
           UTIL_Delay(500);
        }

        //
        // Quit the splash screen
        //
        break;
     }

     //
     // Delay a while...
     //
     PAL_ProcessEvent();
     while (SDL_GetTicks() - dwBeginTime < dwTime + 85)
     {
        SDL_Delay(1);
        PAL_ProcessEvent();
     }
  }

  SDL_FreeSurface(lpBitmapDown);
  SDL_FreeSurface(lpBitmapUp);
  free(buf);

  if (!fUseCD)
  {
     RIX_Play(0, FALSE, 1);
  }

  PAL_FadeOut(1);
}

static WORD g_wCurEffectSprite = 0;

VOID
PAL_EndingSetEffectSprite(
  WORD         wSpriteNum
)
/*++
 Purpose:

   Set the effect sprite of the ending.

 Parameters:

   [IN]  wSpriteNum - the number of the sprite.

 Return value:

   None.

--*/
{
  g_wCurEffectSprite = wSpriteNum;
}

VOID
PAL_ShowFBP(
  WORD         wChunkNum,
  WORD         wFade
)
/*++
 Purpose:

   Draw an FBP picture to the screen.

 Parameters:

   [IN]  wChunkNum - number of chunk in fbp.mkf file.

   [IN]  wFade - fading speed of showing the picture.

 Return value:

   None.

--*/
{
  PAL_LARGE BYTE            buf[320 * 200];
  PAL_LARGE BYTE            bufSprite[320 * 200];
  const int                 rgIndex[6] = {0, 3, 1, 5, 2, 4};
  SDL_Surface              *p;
  int                       i, j, k;
  BYTE                      a, b;

  if (PAL_MKFDecompressChunk(buf, 320 * 200, wChunkNum, gpGlobals->f.fpFBP) <= 0)
  {
     memset(buf, 0, sizeof(buf));
  }

  if (g_wCurEffectSprite != 0)
  {
     PAL_MKFDecompressChunk(bufSprite, 320 * 200, g_wCurEffectSprite, gpGlobals->f.fpMGO);
  }

  if (wFade)
  {
     wFade++;
     wFade *= 10;

     p = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
        gpScreen->format->Rmask, gpScreen->format->Gmask,
        gpScreen->format->Bmask, gpScreen->format->Amask);

     SDL_SetPalette(p, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);

     PAL_FBPBlitToSurface(buf, p);
     VIDEO_BackupScreen();

     for (i = 0; i < 16; i++)
     {
        for (j = 0; j < 6; j++)
        {
           //
           // Blend the pixels in the 2 buffers, and put the result into the
           // backup buffer
           //
           for (k = rgIndex[j]; k < gpScreen->pitch * gpScreen->h; k += 6)
           {
              a = ((LPBYTE)(p->pixels))[k];
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

           SDL_BlitSurface(gpScreenBak, NULL, gpScreen, NULL);

           if (g_wCurEffectSprite != 0)
           {
              int f = SDL_GetTicks() / 150;
              PAL_RLEBlitToSurface(PAL_SpriteGetFrame(bufSprite, f % PAL_SpriteGetNumFrames(bufSprite)),
                 gpScreen, PAL_XY(0, 0));
           }

           VIDEO_UpdateScreen(NULL);
           UTIL_Delay(wFade);
        }
     }

     SDL_FreeSurface(p);
  }

  //
  // HACKHACK: to make the ending show correctly
  //
  if (wChunkNum != 68)
  {
     PAL_FBPBlitToSurface(buf, gpScreen);
  }

  VIDEO_UpdateScreen(NULL);
}

VOID
PAL_ScrollFBP(
  WORD         wChunkNum,
  WORD         wScrollSpeed,
  BOOL         fScrollDown
)
/*++
 Purpose:

   Scroll up an FBP picture to the screen.

 Parameters:

   [IN]  wChunkNum - number of chunk in fbp.mkf file.

   [IN]  wScrollSpeed - scrolling speed of showing the picture.

   [IN]  fScrollDown - TRUE if scroll down, FALSE if scroll up.

 Return value:

   None.

--*/
{
  SDL_Surface          *p;
  PAL_LARGE BYTE        buf[320 * 200];
  PAL_LARGE BYTE        bufSprite[320 * 200];
  int                   i, l;
  SDL_Rect              rect, dstrect;

  if (PAL_MKFDecompressChunk(buf, 320 * 200, wChunkNum, gpGlobals->f.fpFBP) <= 0)
  {
     return;
  }

  if (g_wCurEffectSprite != 0)
  {
     PAL_MKFDecompressChunk(bufSprite, 320 * 200, g_wCurEffectSprite, gpGlobals->f.fpMGO);
  }

  p = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
     gpScreen->format->Rmask, gpScreen->format->Gmask,
     gpScreen->format->Bmask, gpScreen->format->Amask);

  if (p == NULL)
  {
     return;
  }

  SDL_SetPalette(p, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);

  VIDEO_BackupScreen();
  PAL_FBPBlitToSurface(buf, p);

  if (wScrollSpeed == 0)
  {
     wScrollSpeed = 1;
  }

  rect.x = 0;
  rect.w = 320;
  dstrect.x = 0;
  dstrect.w = 320;

  for (l = 0; l < 220; l++)
  {
     i = l;
     if (i > 200)
     {
        i = 200;
     }

     if (fScrollDown)
     {
        rect.y = 0;
        dstrect.y = i;
        rect.h = 200 - i;
        dstrect.h = 200 - i;
     }
     else
     {
        rect.y = i;
        dstrect.y = 0;
        rect.h = 200 - i;
        dstrect.h = 200 - i;
     }

     SDL_BlitSurface(gpScreenBak, &rect, gpScreen, &dstrect);

     if (fScrollDown)
     {
        rect.y = 200 - i;
        dstrect.y = 0;
        rect.h = i;
        dstrect.h = i;
     }
     else
     {
        rect.y = 0;
        dstrect.y = 200 - i;
        rect.h = i;
        dstrect.h = i;
     }

     SDL_BlitSurface(p, &rect, gpScreen, &dstrect);

     PAL_ApplyWave(gpScreen);

     if (g_wCurEffectSprite != 0)
     {
        int f = SDL_GetTicks() / 150;
        PAL_RLEBlitToSurface(PAL_SpriteGetFrame(bufSprite, f % PAL_SpriteGetNumFrames(bufSprite)),
           gpScreen, PAL_XY(0, 0));
     }

     VIDEO_UpdateScreen(NULL);

     if (gpGlobals->fNeedToFadeIn)
     {
        PAL_FadeIn(gpGlobals->wNumPalette, gpGlobals->fNightPalette, 1);
        gpGlobals->fNeedToFadeIn = FALSE;
     }

     UTIL_Delay(800 / wScrollSpeed);
  }

  SDL_BlitSurface(p, NULL, gpScreen, NULL);
  SDL_FreeSurface(p);
  VIDEO_UpdateScreen(NULL);
}

VOID
PAL_EndingAnimation(
  VOID
)
/*++
 Purpose:

   Show the ending animation.

 Parameters:

   None.

 Return value:

   None.

--*/
{
  LPBYTE            buf;
  LPBYTE            bufGirl;
  SDL_Surface      *pUpper;
  SDL_Surface      *pLower;
  SDL_Rect          srcrect, dstrect;

  int               yPosGirl = 180;
  int               i;

  buf = (LPBYTE)UTIL_calloc(1, 64000);
  bufGirl = (LPBYTE)UTIL_calloc(1, 6000);

  pUpper = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
     gpScreen->format->Rmask, gpScreen->format->Gmask,
     gpScreen->format->Bmask, gpScreen->format->Amask);

  pLower = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
     gpScreen->format->Rmask, gpScreen->format->Gmask,
     gpScreen->format->Bmask, gpScreen->format->Amask);

  SDL_SetPalette(pUpper, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);
  SDL_SetPalette(pLower, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);

  PAL_MKFDecompressChunk(buf, 64000, 69, gpGlobals->f.fpFBP);
  PAL_FBPBlitToSurface(buf, pUpper);

  PAL_MKFDecompressChunk(buf, 64000, 70, gpGlobals->f.fpFBP);
  PAL_FBPBlitToSurface(buf, pLower);

  PAL_MKFDecompressChunk(buf, 64000, 571, gpGlobals->f.fpMGO);
  PAL_MKFDecompressChunk(bufGirl, 6000, 572, gpGlobals->f.fpMGO);

  srcrect.x = 0;
  dstrect.x = 0;
  srcrect.w = 320;
  dstrect.w = 320;

  gpGlobals->wScreenWave = 2;

  for (i = 0; i < 400; i++)
  {
     //
     // Draw the background
     //
     srcrect.y = 0;
     srcrect.h = 200 - i / 2;

     dstrect.y = i / 2;
     dstrect.h = 200 - i / 2;

     SDL_BlitSurface(pLower, &srcrect, gpScreen, &dstrect);

     srcrect.y = 200 - i / 2;
     srcrect.h = i / 2;

     dstrect.y = 0;
     dstrect.h = i / 2;

     SDL_BlitSurface(pUpper, &srcrect, gpScreen, &dstrect);

     PAL_ApplyWave(gpScreen);

     //
     // Draw the beast
     //
     PAL_RLEBlitToSurface(PAL_SpriteGetFrame(buf, 0), gpScreen, PAL_XY(0, -400 + i));
     PAL_RLEBlitToSurface(buf + 0x8444, gpScreen, PAL_XY(0, -200 + i));

     //
     // Draw the girl
     //
     yPosGirl -= i & 1;
     if (yPosGirl < 80)
     {
        yPosGirl = 80;
     }

     PAL_RLEBlitToSurface(PAL_SpriteGetFrame(bufGirl, (SDL_GetTicks() / 50) % 4),
        gpScreen, PAL_XY(220, yPosGirl));

     //
     // Update the screen
     //
     VIDEO_UpdateScreen(NULL);
     if (gpGlobals->fNeedToFadeIn)
     {
        PAL_FadeIn(gpGlobals->wNumPalette, gpGlobals->fNightPalette, 1);
        gpGlobals->fNeedToFadeIn = FALSE;
     }

     UTIL_Delay(50);
  }

  gpGlobals->wScreenWave = 0;

  SDL_FreeSurface(pUpper);
  SDL_FreeSurface(pLower);

  free(buf);
  free(bufGirl);
}

VOID
PAL_EndingScreen(
  VOID
)
{
   RIX_Play(0x1a, TRUE, 0);
   PAL_RNGPlay(gpGlobals->iCurPlayingRNG, 110, 150, 7);
   PAL_RNGPlay(gpGlobals->iCurPlayingRNG, 151, 999, 9);

   PAL_FadeOut(2);

   RIX_Play(0x19, TRUE, 0);

   PAL_ShowFBP(75, 0);
   PAL_FadeIn(5, FALSE, 1);
   PAL_ScrollFBP(74, 0xf, TRUE);

   PAL_FadeOut(1);

   SDL_FillRect(gpScreen, NULL, 0);
   PAL_SetPalette(4, FALSE);
   PAL_EndingAnimation();

   gpGlobals->wNumPalette = 4;
   RIX_Play(0, FALSE, 1);
   PAL_ColorFade(7, 15, FALSE);

   if (!SOUND_PlayCDA(2))
   {
      RIX_Play(0x11, TRUE, 0);
   }

   SDL_FillRect(gpScreen, NULL, 0);
   PAL_SetPalette(0, FALSE);
   PAL_RNGPlay(0xb, 0, 999, 7);

   PAL_FadeOut(2);

   SDL_FillRect(gpScreen, NULL, 0);
   PAL_SetPalette(8, FALSE);
   PAL_RNGPlay(10, 0, 999, 6);

   PAL_EndingSetEffectSprite(0);
   PAL_ShowFBP(77, 10);

   VIDEO_BackupScreen();

   PAL_EndingSetEffectSprite(0x27b);
   PAL_ShowFBP(76, 7);

   PAL_SetPalette(5, FALSE);
   PAL_ShowFBP(73, 7);
   PAL_ScrollFBP(72, 0xf, TRUE);

   PAL_ShowFBP(71, 7);
   PAL_ShowFBP(68, 7);

   PAL_EndingSetEffectSprite(0);
   PAL_ShowFBP(68, 6);

   PAL_WaitForKey(0);
   RIX_Play(0, FALSE, 0.5);
   UTIL_Delay(500);

   if (!SOUND_PlayCDA(13))
   {
      RIX_Play(9, TRUE, 0);
   }

   PAL_ScrollFBP(67, 0xf, TRUE);
   PAL_ScrollFBP(66, 0xf, TRUE);
   PAL_ScrollFBP(65, 0xf, TRUE);
   PAL_ScrollFBP(64, 0xf, TRUE);
   PAL_ScrollFBP(63, 0xf, TRUE);
   PAL_ScrollFBP(62, 0xf, TRUE);
   PAL_ScrollFBP(61, 0xf, TRUE);
   PAL_ScrollFBP(60, 0xf, TRUE);
   PAL_ScrollFBP(59, 0xf, TRUE);

   RIX_Play(0, FALSE, 3);
   PAL_FadeOut(3);
}

int PAL_PlayAVI(const char *szFilename) {
    // just use the opening/ending animations from DOS version
    if (strcmp(szFilename + strlen(szFilename) - 5, "1.avi") == 0)
        PAL_TrademarkScreen();
    else if (strcmp(szFilename + strlen(szFilename) - 5, "2.avi") == 0)
        PAL_SplashScreen();
    else if (strcmp(szFilename + strlen(szFilename) - 5, "6.avi") == 0)
        PAL_EndingScreen();

    return 0;
}

#endif
