
#include "requestAndRead.h"


ECU_t gs_ecu_type;
//uint8_t gs_noOfLines;
uint8_t gs_noOfResBytes;


/**
 * @brief Parse file and make request
 * @param [in] id id of ECU
 * @param [in] RequestBytes requested bytes
 * @returns if found returns response else  returns 7F #firstByteofReq 31
 */
uint8_t* SendRequest( uint8_t id_ecu, uint8_t* RequestBytes)
{

    FILE* fp = fopen( f_name, "rb");
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


