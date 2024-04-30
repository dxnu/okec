///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include <okec/utils/sys.h>
#ifdef __linux__
    #include <sys/ioctl.h>
    #include <unistd.h>
#elif _WIN32
#else
#endif

namespace okec
{

auto get_winsize() -> winsize_t {
#ifdef __linux__
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return winsize_t { .row = w.ws_row, .col = w.ws_col, .x = w.ws_xpixel, .y = w.ws_ypixel };
#elif _WIN32
    return winsize_t { .row = 0, .col = 0, .x = 0, .y = 0 };
#else
    return winsize_t { .row = 0, .col = 0, .x = 0, .y = 0 };
#endif // sys
}


} // namespace okec