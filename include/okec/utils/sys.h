///////////////////////////////////////////////////////////////////////////////
//   __  __ _  ____  ___ 
//  /  \(  / )(  __)/ __) OKEC(a.k.a. EdgeSim++)
// (  O ))  (  ) _)( (__  version 1.0.1
//  \__/(__\_)(____)\___) https://github.com/dxnu/okec
// 
// Copyright (C) 2023-2024 Gaoxing Li
// Licenced under Apache-2.0 license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#ifndef OKEC_SYS_H_
#define OKEC_SYS_H_


namespace okec {

struct winsize_t {
    unsigned short int row;
    unsigned short int col;
    // unsigned short int x;
    // unsigned short int y;
};

auto get_winsize() -> winsize_t;


} // namespace okec

#endif // OKEC_SYS_H_