#pragma once

#include <stddef.h>     /* size_t */
#include <stdio.h>
#include "vcmpq_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

void VCMPQ_CreateQuery( vcmpq_request pRequest, unsigned char* szBuf );


#ifdef __cplusplus
}
#endif
