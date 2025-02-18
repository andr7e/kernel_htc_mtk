 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 * 
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef _TL_SEC_TCI_H_
#define _TL_SEC_TCI_H_

typedef uint32_t tciCommandId_t;
typedef uint32_t tciResponseId_t;
typedef uint32_t tciReturnCode_t;

#define RSP_ID_MASK (1U << 31)
#define RSP_ID(cmdId) (((uint32_t)(cmdId)) | RSP_ID_MASK)
#define IS_CMD(cmdId) ((((uint32_t)(cmdId)) & RSP_ID_MASK) == 0)
#define IS_RSP(cmdId) ((((uint32_t)(cmdId)) & RSP_ID_MASK) == RSP_ID_MASK)

#define RET_OK              0            
#define RET_ERR_UNKNOWN_CMD 1            
#define INVALIDE_VIRTUAL_ADDR 2

typedef struct{
    tciCommandId_t commandId; 
} tciCommandHeader_t;

typedef struct{
    tciResponseId_t     responseId; 
    tciReturnCode_t     returnCode; 
} tciResponseHeader_t;

#endif 
