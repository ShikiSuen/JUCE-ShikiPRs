/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../text/juce_String.h"
#include "juce_SystemStats.h"
#include "juce_Random.h"
#include "juce_Time.h"
#include "juce_Atomic.h"
#include "../threads/juce_Thread.h"
#include "../text/juce_LocalisedStrings.h"
#include "juce_PlatformUtilities.h"
void juce_initialiseStrings();


//==============================================================================
const String SystemStats::getJUCEVersion() throw()
{
    return "JUCE v" + String (JUCE_MAJOR_VERSION) + "." + String (JUCE_MINOR_VERSION);
}


//==============================================================================
static bool juceInitialisedNonGUI = false;

void JUCE_PUBLIC_FUNCTION initialiseJuce_NonGUI()
{
    if (! juceInitialisedNonGUI)
    {
#if JUCE_MAC
        const ScopedAutoReleasePool pool;
#endif

#ifdef JUCE_DEBUG
        // Some simple test code to keep an eye on things and make sure these functions
        // work ok on all platforms. Let me know if any of these assertions fail!
        int n = 1;
        atomicIncrement (n);
        jassert (atomicIncrementAndReturn (n) == 3);
        atomicDecrement (n);
        jassert (atomicDecrementAndReturn (n) == 1);

        jassert (swapByteOrder ((uint32) 0x11223344) == 0x44332211);

        // quick test to make sure the run-time lib doesn't crash on freeing a null-pointer.
        SystemStats* nullPointer = 0;
        juce_free (nullPointer);
        delete[] nullPointer;
        delete nullPointer;
#endif
        // Now the real initialisation..

        juceInitialisedNonGUI = true;

        DBG (SystemStats::getJUCEVersion());
        Random::getSystemRandom().setSeedRandomly(); // (calling this more than once improves its randomness)
        juce_initialiseStrings();
        SystemStats::initialiseStats();
        Random::getSystemRandom().setSeedRandomly(); // (calling this more than once improves its randomness)
    }
}

#if JUCE_WIN32
 // This is imported from the sockets code..
 typedef int (__stdcall juce_CloseWin32SocketLibCall) (void);
 extern juce_CloseWin32SocketLibCall* juce_CloseWin32SocketLib;
#endif

#if JUCE_DEBUG
  extern void juce_CheckForDanglingStreams();
#endif

void JUCE_PUBLIC_FUNCTION shutdownJuce_NonGUI()
{
    if (juceInitialisedNonGUI)
    {
#if JUCE_MAC
        const ScopedAutoReleasePool pool;
#endif

#if JUCE_WIN32
        // need to shut down sockets if they were used..
        if (juce_CloseWin32SocketLib != 0)
            (*juce_CloseWin32SocketLib)();
#endif

        LocalisedStrings::setCurrentMappings (0);
        Thread::stopAllThreads (3000);

#if JUCE_DEBUG
        juce_CheckForDanglingStreams();
#endif

        juceInitialisedNonGUI = false;
    }
}

//==============================================================================
#ifdef JUCE_DLL

void* juce_Malloc (const int size)
{
    return malloc (size);
}

void* juce_Calloc (const int size)
{
    return calloc (1, size);
}

void* juce_Realloc (void* const block, const int size)
{
    return realloc (block, size);
}

void juce_Free (void* const block)
{
    free (block);
}

#if defined (JUCE_DEBUG) && JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS

void* juce_DebugMalloc (const int size, const char* file, const int line)
{
    return _malloc_dbg  (size, _NORMAL_BLOCK, file, line);
}

void* juce_DebugCalloc (const int size, const char* file, const int line)
{
    return _calloc_dbg  (1, size, _NORMAL_BLOCK, file, line);
}

void* juce_DebugRealloc (void* const block, const int size, const char* file, const int line)
{
    return _realloc_dbg  (block, size, _NORMAL_BLOCK, file, line);
}

void juce_DebugFree (void* const block)
{
    _free_dbg (block, _NORMAL_BLOCK);
}

#endif
#endif


END_JUCE_NAMESPACE
