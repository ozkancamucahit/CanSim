
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// types of ECU's
typedef enum type_ECU {KWP, UDS} ECU_t;

extern FILE* fp;

//////////////   Static variables

extern uint8_t gs_noOfLines;
extern ECU_t gs_ecu_type;
extern uint8_t gs_noOfResBytes;

//////////////  end Static variables


typedef struct message
{
    uint8_t ecu_id;  /**< id of ecu, used to asssume type of ECU */
    ECU_t   ecu_type;  /**< type is set according to id */
    uint8_t numberOfResBytes;  /**< for setting global static variable */
    uint8_t* reqBytes;  /**< array of request bytes*/
    uint8_t* resBytes; /**< array of response bytes */
}message_t;

uint8_t get_noOfLines( FILE* );

char ** make_2D (FILE* fp);

uint8_t isEmpty(const char*);

int isFoundReq ( uint8_t*, message_t*, int*);

uint8_t  getId( char*);

uint8_t* getReq ( char* line, uint8_t*);

uint8_t* getRes ( char* line, uint8_t*);

uint8_t hex2Int( char* hex);

uint8_t getNumberOfReqBytes ( char * );

void Int2Hex ( uint8_t*, size_t);

void print_ReqAndResponse( uint8_t*, uint8_t* );

void initMessage ( message_t*, uint8_t, uint8_t*, uint8_t*, uint8_t*);

void terminateLine(char * line);


#endif