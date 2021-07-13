
#ifndef REQUEST_AND_READ_H
#define REQUEST_AND_READ_H

#include "parser.h"

char * f_name; /**< get file name from main */

uint8_t* SendRequest( uint8_t, uint8_t*);

void ReadEcuParameters( uint8_t, ECU_t, uint8_t*);


#endif
