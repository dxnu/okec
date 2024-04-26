#ifndef OKEC_SYS_H_
#define OKEC_SYS_H_


namespace okec {

struct winsize_t {
    unsigned short int row;
    unsigned short int col;
    unsigned short int x;
    unsigned short int y;
};

auto get_winsize() -> winsize_t;


} // namespace okec

#endif // OKEC_SYS_H_