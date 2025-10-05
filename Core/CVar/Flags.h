//
// Created by IDKTHIS on 01.07.2025.
//

#ifndef FLAGS_H
#define FLAGS_H

enum CVarFlags {
    CVAR_NONE        = 0,
    CVAR_ARCHIVE     = 1 << 0,
    CVAR_READONLY    = 1 << 1,
    CVAR_CHEAT       = 1 << 2,
    CVAR_RUNTIME_ONLY= 1 << 3,
    CVAR_CONSOLE_EDIT= 1 << 4
};



#endif //FLAGS_H
