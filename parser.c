


#include "parser.h"

uint8_t gs_noOfLines;
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

uint8_t isEmpty(const char* line)
{
    return (!strcmp("\r\n", line) || (!strcmp("\n", line)));
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
