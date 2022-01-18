#if !defined HEADER_H
#define HEADER_H
#define HAVE_STDINT_H
#include "api/moduleconfig.h"
#include <amxxmodule.h>

#define CONST const
#define INT int
#define CHAR char
#define _stricmp strcasecmp
#define VOID void
#define PINT int*

void OnAmxxAttach(void);
void OnAmxxDetach(void);
void StartFrame(void);
#endif