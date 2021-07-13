
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/**
 * @author M.Mücahit Özkanca
 * @brief  CAN bus prototype
 * 03/07/2021
 */

// types of ECU's
typedef enum type_ECU {KWP, UDS} ECU_t;

typedef struct message
{
    uint8_t ecu_id;  /**< id of ecu, used to asssume type of ECU */
    ECU_t   ecu_type;  /**< type is set according to id */
    uint8_t numberOfResBytes;  /**< for setting global static variable */
    uint8_t* reqBytes;  /**< array of request bytes*/
    uint8_t* resBytes; /**< array of response bytes */
}message_t;


FILE* fp = NULL;
char * f_name = NULL; /**< get file name from main */

//////////////   Static variables

static uint8_t gs_noOfLines;
static ECU_t gs_ecu_type;
static uint8_t gs_noOfResBytes;

//////////////  end Static variables


uint8_t* SendRequest( uint8_t, uint8_t*);

void ReadEcuParameters( uint8_t, ECU_t, uint8_t*);

uint8_t get_noOfLines( FILE* );

int isFoundReq ( uint8_t*, message_t*, int*);

char ** make_2D (FILE* fp);

uint8_t isEmpty(const char*);

uint8_t  getId( char*);

uint8_t* getReq ( char* line, uint8_t*);

uint8_t* getRes ( char* line, uint8_t*);

void initMessage ( message_t*, uint8_t, uint8_t*, uint8_t*, uint8_t*);

uint8_t hex2Int( char* hex);

uint8_t getNumberOfReqBytes ( char * );

void Int2Hex ( uint8_t*, size_t);

void print_ReqAndResponse( uint8_t*, uint8_t* );



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



uint8_t isEmpty(const char* line)
{
    return (!strcmp("\r\n", line) || (!strcmp("\n", line)));
}

/**
 * From line get number of Bytes required to store them
 * \return number of bytes to allocate
 */
uint8_t getNumberOfReqBytes ( char * line)
{
    char * pch;
    pch = strtok ( line," ");
    uint8_t number = 0;
    while (pch != NULL)
    {
        pch = strtok (NULL, " ");
        ++number;
    }

    return number-1;
}

/**
 * To avoid buffer problems and white spaces with scanf
 * 
 */
uint8_t hex2Int(char *hex) {
    uint8_t val = 0;
    while (*hex) {
        uint8_t byte = *hex++; 
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}


uint8_t getId( char* line)
{
    char hex[2];
    char id[3];
    int num = 0;

    if ( line == NULL ) return 0;

    int res = sscanf(line, "%s %s", id, hex); /*< works for 2 variables*/

    if ( res != 2 ) return 0;

    sscanf(hex, "%x", &num);
    return num;
}


/**
 * Get REQ bytes from line. Parse and set number of bytes.
 * 
 * \param [in] line From 2-D array
 * \param [in] noOfBytes to set global static variable
 */
uint8_t* getReq ( char* line, uint8_t* noOfReqBytes)
{

    if ( line == NULL ) return 0;
    
    uint8_t tempLen = strlen( line);
    char * temp = (char*)calloc( tempLen, sizeof(char));
    assert(temp);

    strcpy ( temp, line);
    
    #ifdef __linux__
    temp[ tempLen -1] = '\0';
    #elif   __WIN32
    temp[ tempLen -2] = '\0';

    #endif

    uint8_t numberOfBytes = getNumberOfReqBytes( line);
    *noOfReqBytes = numberOfBytes;

    uint8_t* req = (uint8_t*)calloc( numberOfBytes, sizeof(uint8_t));
    assert(req);
    char * pch;
    pch = strtok ( temp," ");

    for (size_t i = 0; i < numberOfBytes; ++i)
    {
        pch = strtok (NULL, " ");

        req[i] = hex2Int( pch );
    }

    return req;   

}

uint8_t* getRes ( char* line, uint8_t* noOfResBytes)
{
    return getReq ( line, noOfResBytes);
}


/**
 *  Get number of nonEmpty lines.
 *  Used to get number of lines and compute number of messages.
 *  Then rewinds file cursor to beginnig of fp
 *  \param [in] fp opened FILE
 */
uint8_t get_noOfLines( FILE* fp )
{
    size_t len = 0;
    size_t read;
    char * ch= NULL;
    gs_noOfLines = 0;
    while ((read = getline(&ch, &len, fp)) != -1) {

        if (isEmpty(ch)) continue;
       // printf("Retrieved line of length %zu :\n", read);
        //printf("%s", ch);
        ++gs_noOfLines;
    }

    rewind(fp);

    return gs_noOfLines;
}

/**
 * Make lines 2d array to parse and store
 * each row is not fixed size;
 * \param [in] fp open file poiter. rewinds before retuns
 * \returns 2-D array of chars
 */
char ** make_2D (FILE* fp)
{

    char ** messages2D;
    uint8_t lines = get_noOfLines(fp);

    messages2D = (char**)calloc(lines, sizeof(char*));
    assert(messages2D);

    size_t len = 0;
    size_t read;
    char * ch= NULL;
    uint8_t row = 0;

    while ((read = getline(&ch, &len, fp)) != -1) {

        if (isEmpty(ch)) continue;
        if ( (messages2D[row++] = (char*)calloc(read, sizeof(char))) == NULL)
        {
            fprintf(stderr, "Out Of Memory\n");
            exit(EXIT_FAILURE);
        }
    }
    rewind(fp);

    row = 0;

    while ((read = getline(&ch, &len, fp)) != -1) {

        if (isEmpty(ch)) continue;

        //messages2D[row++] = ch;
        strcpy(messages2D[row++], ch);
    }


    return messages2D;
}


/**
 * init each message with attributes and set number of bytes.
 * \param [in] msg_t array of messages;
 */
void initMessage ( message_t* msg_t, uint8_t ecu_id_t, uint8_t* req_t, uint8_t* res_t, uint8_t* noOfResBytes)
{
    msg_t ->ecu_id   = ecu_id_t;
    msg_t->reqBytes  = req_t;
    msg_t ->resBytes = res_t;
    msg_t->numberOfResBytes = *noOfResBytes;
    if ( ecu_id_t == 1 ) msg_t->ecu_type = UDS;
    else msg_t->ecu_type = KWP;
}

/**
 * convert values in buffer to hex
 */
void Int2Hex ( uint8_t* searchReq, size_t req_length)
{
    char buff[3];

    for (size_t i = 0; i < req_length; ++i)
    {
        sprintf(buff, "%d", searchReq[i]);
        searchReq[i] = hex2Int( buff);
    }
}

void print_ReqAndResponse( uint8_t* req, uint8_t* res)
{
    printf("\nREQ: ");
    while ( * req)
    {
        printf("%X ", *req++);
    }
    printf("\n");

    printf("RES: ");

    while ( * res)
    {
        printf("%X ", *res++);
    }
    printf("\n");

}

/**
 * \brief compare request to same type of requests
 * \param [in] searchReq request to search
 * \param [in] messages array of messages to search in
 * \param [in] idx store result message index
 * \returns 0 if nothing found. else number of mat
 */
int isFoundReq ( uint8_t* searchReq , message_t* messages, int* idx)
{
    uint8_t noOfMessages = gs_noOfLines /3;

    size_t req_length;
    uint8_t isFound;

    switch (gs_ecu_type)
    {
        case KWP:
        {
            req_length = 2U;
            Int2Hex( searchReq, req_length);

            for (size_t i = 0; i < noOfMessages; ++i)
            {

                if ( messages[i].ecu_type != KWP ) continue;
                isFound = 0;

                for (size_t j = 0; j < req_length; ++j)
                {
                    if( searchReq[j] == messages[i].reqBytes[j] )
                        ++isFound;
                }

                if ( isFound == req_length )
                {
                    *idx = i;
                     return isFound;
                }
                else isFound=0;

            }
            
            break;
        }
        case UDS:
        {
            req_length = 3U;
            Int2Hex( searchReq, req_length);

            for (size_t i = 0; i < noOfMessages; ++i)
            {

                if ( messages[i].ecu_type != UDS ) continue;
                isFound = 0;

                for (size_t j = 0; j < req_length; ++j)
                {
                    if( searchReq[j] == messages[i].reqBytes[j] )
                        ++isFound;
                }

                if ( isFound == req_length )
                {
                    *idx = i;
                    return isFound;
                }
                else isFound=0;;
            }

            break;

        }
        default:
            isFound =0;
        break;
    }

    
    return isFound;

}


/**
 * @brief Parse file and make request
 * @param [in] id id of ECU
 * @param [in] RequestBytes requested bytes
 * @returns if found returns response else  returns 7F #firstByteofReq 31
 */
uint8_t* SendRequest( uint8_t id_ecu, uint8_t* RequestBytes)
{
    fp = fopen( f_name, "rb");

    if ( fp == NULL)
    { 
        fprintf(stderr,"Can't find file %s\n", f_name);
        exit(EXIT_FAILURE);
    }

    static uint8_t negResponse[4]={0};
    negResponse[0] = 0x7FU;
    negResponse[2] = 0x31U;

    char ** messages2D = make_2D( fp);

    size_t line_idx = 0, msg_idx=0;

    uint8_t noOfMessages = gs_noOfLines /3;
    int foundMessage_idx =-1;

    message_t messages [ noOfMessages];

    uint8_t noOfResBytes=0;
        do {
                uint8_t id   = getId  ( messages2D[line_idx++]);
                uint8_t* req = getReq ( messages2D[line_idx++], &noOfResBytes);
                uint8_t* res = getRes ( messages2D[line_idx++], &noOfResBytes);
           
                initMessage( &messages[msg_idx++], id, req, res, &noOfResBytes);
        }while (line_idx < gs_noOfLines);
            
    
    if ( !isFoundReq( RequestBytes, messages, &foundMessage_idx) )
    {
        //not found
        gs_noOfResBytes = 3;
        negResponse[1] = RequestBytes[0];
        return negResponse;
    }

    //printf("Found at %d.th idx", foundMessage_idx);
    else 
    {
        gs_noOfResBytes = messages[foundMessage_idx].numberOfResBytes;
        return messages[foundMessage_idx].resBytes;
    }
}


/**
 * \brief Get list of data ids and send request to same type of ECU
 * \param [in] ecu_id id of ECU
 * \param [in] ecu_type type of ECU
 * \param [in] did_list list of data ids
 * 
 */
void ReadEcuParameters( uint8_t ecu_id, ECU_t ecu_type, uint8_t* did_list)
{
    uint8_t * temp = NULL;
    assert(did_list);

    switch (ecu_type)
    {
        case KWP:
        {
            gs_ecu_type = KWP;
            uint8_t kwp_length = 1;
            uint8_t* reqBytes = (uint8_t*)calloc( kwp_length +2, sizeof(int));
            assert(reqBytes);
            reqBytes [0] = 21U;
            reqBytes [1] = did_list[0];
            // reqbytes[2] = '0' terminating

            temp = SendRequest( ecu_id, reqBytes);
            temp[gs_noOfResBytes] = '\0'; // terminate
            print_ReqAndResponse( reqBytes, temp);
            break;
        }
        case UDS:
        {
            gs_ecu_type = UDS;
            uint8_t uds_length =2;
            uint8_t* reqBytes = (uint8_t*)calloc( uds_length +2, sizeof(int));
            assert(reqBytes);
            reqBytes [0] = 22U;
            reqBytes [1] = did_list[0];
            reqBytes[2] = did_list[1];

            temp = SendRequest( ecu_id, reqBytes);
            temp[gs_noOfResBytes] = '\0';
            print_ReqAndResponse( reqBytes, temp);
            break;
        }
    default:
        fprintf(stderr, "Invalid Ecu Type\n");
        exit(EXIT_FAILURE);
        break;
    }

}
