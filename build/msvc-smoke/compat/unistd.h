/* MSVC smoke-test shim: minimal unistd.h replacement */
#ifndef COMPAT_UNISTD_H
#define COMPAT_UNISTD_H
#include <io.h>
#include <direct.h>
#include <process.h>
#ifndef F_OK
#define F_OK 0
#define W_OK 2
#define R_OK 4
#endif
#endif
