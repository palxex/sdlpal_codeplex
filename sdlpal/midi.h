//
// Copyright (c) 2011, Wei Mingzhi <whistler@openoffice.org>.
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

#ifndef PAL_MIDI_H
#define PAL_MIDI_H

#include "common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "native_midi/native_midi.h"

VOID
MIDI_Play(
   INT       iNumRIX,
   BOOL      fLoop
);

VOID
MIDI_CheckLoop(
   VOID
);

#ifdef __cplusplus
}
#endif

#endif
