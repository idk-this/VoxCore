//
// Created by IDKTHIS on 22.09.2025.
//
#include "IWindow.h"

#include "Core/CVar/CVar.h"



DECLARE_CONVAR("w_title", "VoxCore base", "Game window title", CVAR_RUNTIME_ONLY);
DECLARE_CONVAR_MINMAX("w_size_width", 1920, 320, 15360, "Game window width", CVAR_ARCHIVE);
DECLARE_CONVAR_MINMAX("w_size_height", 1080, 240, 8640, "Game window height", CVAR_ARCHIVE);
