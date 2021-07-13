

#include "parser.h"
#include "requestAndRead.h"

/**
 * @author M.Mücahit Özkanca
 * @brief  CAN bus prototype
 * 03/07/2021
 */


int main ( int argc,  char** argv)
{
    f_name = argv[1];
    assert(f_name);

    uint8_t dIdlist1[] = {12, 34}; /** <simulation file*/
    uint8_t dIdlist2[] = {11};

    uint8_t dIdlist3[] = {12, 35};
    uint8_t dIdlist4[] = {12};     /** < simulation file*/

    uint8_t dIdlist5[] = {21, 34};  /*< Random values use simulation2.txt for input*/
    uint8_t dIdlist6[] = {24};
    
    uint8_t dIdlist7[] = {17, 71};
    uint8_t dIdlist8[] = {17};

    uint8_t dIdlist9[] = {18, 81};
    uint8_t dIdlist10[] = {71};

    uint8_t dIdlist11[] = {14, 53};
    uint8_t dIdlist12[] = {23};
    
    ReadEcuParameters( 1, UDS, dIdlist1); // change name for different input.  1-> UDS
    ReadEcuParameters( 2, KWP, dIdlist12); // change name for different input. 2-> KWP
    ReadEcuParameters( 2, KWP, dIdlist8); 
    ReadEcuParameters( 1, UDS, dIdlist9); 
    ReadEcuParameters( 1, UDS, dIdlist11); 
    ReadEcuParameters( 2, KWP, dIdlist6); 



    return 0;
} 
