/*
    Copyright 2016-2017 StapleButter

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include "libui/ui.h"

#include "../types.h"
#include "../version.h"


uiWindow* MainWindow;
uiArea* MainDrawArea;

SDL_Thread* EmuThread;
int EmuRunning;

u32 derpo[256*384];
uiDrawBitmap* test = NULL;


int EmuThreadFunc(void* burp)
{
    // init shit.

    for (int i = 0; i < 256*384; i++)
    {
        if (i >= 256*192)
        {
            if (i&1) derpo[i] = 0xFF0000FF;
            else     derpo[i] = 0xFF00FF00;
        }
        else
        {
            if (i&1) derpo[i] = 0xFFFF0000;
            else     derpo[i] = 0xFFFFFF00;
        }
    }

    while (EmuRunning != 0)
    {
        if (EmuRunning == 1)
        {
            // emulate
            printf("dfdssdf\n");
        }
        else
        {
            // paused

            uiAreaQueueRedrawAll(MainDrawArea);
            SDL_Delay(50);
        }
    }

    return 44203;
}


void OnAreaDraw(uiAreaHandler* handler, uiArea* area, uiAreaDrawParams* params)
{
    if (!test) test = uiDrawNewBitmap(params->Context, 256, 384);

    uiRect dorp = {0, 0, 256, 384};

    uiDrawBitmapUpdate(test, derpo);
    uiDrawBitmapDraw(params->Context, test, &dorp, &dorp);
    //printf("draw\n");
}

void OnAreaMouseEvent(uiAreaHandler* handler, uiArea* area, uiAreaMouseEvent* evt)
{
    //
}

void OnAreaMouseCrossed(uiAreaHandler* handler, uiArea* area, int left)
{
    //
}

void OnAreaDragBroken(uiAreaHandler* handler, uiArea* area)
{
    //
}

int OnAreaKeyEvent(uiAreaHandler* handler, uiArea* area, uiAreaKeyEvent* evt)
{
    printf("key event: %04X %02X\n", evt->ExtKey, evt->Key);
    uiAreaQueueRedrawAll(MainDrawArea);
    return 1;
}


int OnCloseWindow(uiWindow* window, void* blarg)
{
    uiQuit();
    return 1;
}

void OnOpenFile(uiMenuItem* item, uiWindow* window, void* blarg)
{
    char* file = uiOpenFile(window, "DS ROM (*.nds)|*.nds;*.srl|Any file|*.*", NULL);
    if (!file) return;

    printf("file opened: %s\n", file);
}


int main(int argc, char** argv)
{
    srand(time(NULL));

    // http://stackoverflow.com/questions/14543333/joystick-wont-work-using-sdl
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL shat itself :(\n");
        return 1;
    }

    uiInitOptions ui_opt;
    memset(&ui_opt, 0, sizeof(uiInitOptions));
    const char* ui_err = uiInit(&ui_opt);
    if (ui_err != NULL)
    {
        printf("libui shat itself :( %s\n", ui_err);
        uiFreeInitError(ui_err);
        return 1;
    }

    uiMenu* menu;
    uiMenuItem* menuitem;

    menu = uiNewMenu("File");
    menuitem = uiMenuAppendItem(menu, "Open...");
    uiMenuItemOnClicked(menuitem, OnOpenFile, NULL);
    uiMenuAppendSeparator(menu);
    uiMenuAppendItem(menu, "Quit");

    uiWindow* win;
    win = uiNewWindow("melonDS " MELONDS_VERSION, 256, 384, 1);
    uiWindowOnClosing(win, OnCloseWindow, NULL);

    uiAreaHandler areahandler;

    areahandler.Draw = OnAreaDraw;
    areahandler.MouseEvent = OnAreaMouseEvent;
    areahandler.MouseCrossed = OnAreaMouseCrossed;
    areahandler.DragBroken = OnAreaDragBroken;
    areahandler.KeyEvent = OnAreaKeyEvent;

    MainDrawArea = uiNewArea(&areahandler);
    uiWindowSetChild(win, uiControl(MainDrawArea));
    //uiWindowSetChild(win, uiControl(uiNewButton("become a girl")));

    EmuRunning = 2;
    EmuThread = SDL_CreateThread(EmuThreadFunc, "melonDS magic", NULL);

    uiControlShow(uiControl(win));
    uiMain();

    EmuRunning = 0;
    SDL_WaitThread(EmuThread, NULL);

    uiUninit();
    SDL_Quit();
    return 0;
}

#ifdef __WIN32__

#include <windows.h>

int CALLBACK WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmdline, int cmdshow)
{
    char cmdargs[16][256];
    int arg = 0;
    int j = 0;
    bool inquote = false;
    int len = strlen(cmdline);
    for (int i = 0; i < len; i++)
    {
        char c = cmdline[i];
        if (c == '\0') break;
        if (c == '"') inquote = !inquote;
        if (!inquote && c==' ')
        {
            if (j > 255) j = 255;
            if (arg < 16) cmdargs[arg][j] = '\0';
            arg++;
            j = 0;
        }
        else
        {
            if (arg < 16 && j < 255) cmdargs[arg][j] = c;
            j++;
        }
    }
    if (j > 255) j = 255;
    if (arg < 16) cmdargs[arg][j] = '\0';

    return main(arg, (char**)cmdargs);
}

#endif
