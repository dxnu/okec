///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/utils/sys.h>
#ifdef __linux__
    #include <sys/ioctl.h>
    #include <unistd.h>
#elif _WIN32
    #include <windows.h>
#else
#endif

namespace okec
{

auto get_winsize() -> winsize_t {
#ifdef __linux__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return winsize_t { .row = w.ws_row, .col = w.ws_col };
#elif _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int col = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return winsize_t { .row = row, .col = col };
#else
    return winsize_t { .row = 0, .col = 0 };
#endif // sys
}


} // namespace okec