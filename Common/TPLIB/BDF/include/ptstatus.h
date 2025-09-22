//
//  $Id$
//
//  Copyright (C) 2009 BitDefender S.R.L.
//  Author(s): Sandor LUKACS (slukacs@bitdefender.com)
//

#ifndef _PTSTATUS_H_
#define _PTSTATUS_H_

#ifdef KERNEL_MODE
    #ifndef PETRU_BUILD_WIN7
    #include "no_sal2.h"
    #else
    #include <sal.h>
    #endif
#else
    #ifdef USER_MODE
        #if _MSC_VER <= 1600
        #include "ptnosal.h"
        #else
        #include <sal.h>
        #endif
    #endif
#endif

//
// basic status type definition
//
typedef _Return_type_success_(return >= 0) long PTSTATUS;

//
// description from NTSTATUS.H
//
// values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
// where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

//
// PETRU specific bits and values
//
#define STATUS_CUSTOM                       0x20000000
#define STATUS_FACILITY_PETRU               0x800

// AVC:     0x8E1
// NAPOCA   0x8E2
// RAW_NTFS 0x801
// RAW_REG  0x802
// IGNIS firewall : 0x803

#define STATUS_FACILITY_PETRU_WIN32         0x8F0

// the following bit combinations MUST be present in every PETRU specific status value
#define STATUS_PETRU_SUCCESS_BITS           0x28000000
#define STATUS_PETRU_INFORMATIONAL_BITS     0x68000000
#define STATUS_PETRU_WARNING_BITS           0xA8000000
#define STATUS_PETRU_ERROR_BITS             0xE8000000


//
// status evaluation macros
//
#define PTSUCCESS(Status)                        (((PTSTATUS)(Status)) >= 0)
#define PT_SUCCESS(Status)                       (((PTSTATUS)(Status)) >= 0)


//
// PETRU specific status codes (both user and kernel mode)
//

//
// success - passes PT_SUCCESS
//
#define STATUS_WAIT_OBJECT_0                    ((PTSTATUS)0x00000000L)

//
// informational - passes PT_SUCCESS
//
#define STATUS_NOT_INITIALIZED_HINT             ((PTSTATUS)0x68000001L)
#define STATUS_AREADY_INITIALIZED_HINT          ((PTSTATUS)0x68000002L)
#define STATUS_REF_COUNT_REACHED_ZERO           ((PTSTATUS)0x68000003L)
#define STATUS_EVENT_ALREADY_SIGNALED           ((PTSTATUS)0x68000004L)
#define STATUS_EVENT_ALREADY_NONSIGNALED        ((PTSTATUS)0x68000005L)
#define STATUS_KEY_PAIR_ALREADY_EXISTS_HINT     ((PTSTATUS)0x68000006L)
#define STATUS_DUPLICATES_FOUND                 ((PTSTATUS)0x68000007L)
#define STATUS_EQUAL                            ((PTSTATUS)0x68000008L)
#define STATUS_NOT_EQUAL                        ((PTSTATUS)0x68000009L)
#define STATUS_GREATER                          ((PTSTATUS)0x6800000AL)
#define STATUS_LESS                             ((PTSTATUS)0x6800000BL)
#define STATUS_MORE_TOKENS_AVAILABLE            ((PTSTATUS)0x6800000CL)
#define STATUS_FOUND                            ((PTSTATUS)0x6800000DL)
#define STATUS_SUBSTRING_IS_LONGER              ((PTSTATUS)0x6800000EL)
#define STATUS_TIMEOUT_SIGNALED                 ((PTSTATUS)0x6800000FL)
#define STATUS_CLOSING_SIGNALED                 ((PTSTATUS)0x68000010L)
#define STATUS_UNICODE_BUFFER_TRUNCATED         ((PTSTATUS)0x68000011L)
#define STATUS_HUGE_BUFFER_TRUNCATED            ((PTSTATUS)0x68000012L)
#define STATUS_ITEM_JUST_REMOVED                ((PTSTATUS)0x68000013L)
#define STATUS_THREAD_EXITED_ON_ERROR           ((PTSTATUS)0x68000014L)
#define STATUS_REMOVE_ITEM_FROM_LIST            ((PTSTATUS)0x68000015L)

#define STATUS_ITEM_WAS_INSERTED                ((PTSTATUS)0x68000016L)
#define STATUS_ITEM_WAS_FOUND                   ((PTSTATUS)0x68000017L)
#define STATUS_KEY_INSERTED                     ((PTSTATUS)0x68000018L)

#define STATUS_ANSI_BUFFER_TRUNCATED            ((PTSTATUS)0x68000019L)
#define STATUS_WIM_BACKED_FILE                  ((PTSTATUS)0x6800001AL)


//
// warning - fails PT_SUCCESS
//
///...

//
// error - fails PT_SUCCESS
//

// special value for WIN32 error codes
#define STATUS_WIN32_ERROR                      ((PTSTATUS)0xE8F00000L) // + Win32 error from GetLastError()

#define STATUS_NOT_INITIALIZED                  ((PTSTATUS)0xE8000001L)
#define STATUS_AREADY_INITIALIZED               ((PTSTATUS)0xE8000002L)
#define STATUS_INVALID_REFERENCE_COUNT          ((PTSTATUS)0xE8000003L)
#define STATUS_CANT_GET_KERNEL32_HMODULE        ((PTSTATUS)0xE8000004L)
#define STATUS_CANT_GET_NTDLL_HMODULE           ((PTSTATUS)0xE8000005L)
#define STATUS_REQUIRED_FUNCTION_NOT_FOUND      ((PTSTATUS)0xE8000007L)
#define STATUS_API_CALL_FAILED                  ((PTSTATUS)0xE8000008L)
#define STATUS_INVALID_OBJECT_TYPE              ((PTSTATUS)0xE8000009L)
#define STATUS_CANT_FREE_STATIC_OBJECT          ((PTSTATUS)0xE800000AL)
#define STATUS_OPERATION_NOT_SUPPORTED          ((PTSTATUS)0xE800000BL)
#define STATUS_INVALID_LOCK_TYPE                ((PTSTATUS)0xE800000CL)
#define STATUS_UNEXPECTED_WAIT_RESULT           ((PTSTATUS)0xE800000DL)
#define STATUS_COULD_NOT_CREATE_EVENT           ((PTSTATUS)0xE800000EL)
#define STATUS_OBJECT_IN_USE                    ((PTSTATUS)0xE800000FL)

#define STATUS_DEBUG_ZONE_CORRUPTED             ((PTSTATUS)0xE8000010L)
#define STATUS_OBJECT_CORRUPTED                 ((PTSTATUS)0xE8000011L)
#define STATUS_ELEM_NOT_IN_LIST                 ((PTSTATUS)0xE8000012L)
#define STATUS_KEY_ALREADY_EXISTS               ((PTSTATUS)0xE8000013L)
#define STATUS_TOO_MANY_ITEMS_IN_LIST           ((PTSTATUS)0xE8000014L)
#define STATUS_MULTIPLE_EQUIVALENT_KEYS         ((PTSTATUS)0xE8000015L)
#define STATUS_INVALID_ITEMS_COUNTER            ((PTSTATUS)0xE8000016L)
#define STATUS_INVALID_HASH_FUNC_TYPE           ((PTSTATUS)0xE8000017L)
#define STATUS_LIST_IS_FULL                     ((PTSTATUS)0xE8000018L)
#define STATUS_ARRAY_NOT_BIG_ENOUGH             ((PTSTATUS)0xE8000019L)
#define STATUS_UNICODE_STRING_NOT_BIG_ENOUGH    ((PTSTATUS)0xE800001AL)
#define STATUS_UNICODE_SIZE_EXCEEDED            ((PTSTATUS)0xE800001BL)
#define STATUS_HUGE_STRING_NOT_BIG_ENOUGH       ((PTSTATUS)0xE800001CL)
#define STATUS_HUGE_SIZE_EXCEEDED               ((PTSTATUS)0xE800001DL)
#define STATUS_MANDATORY_ARG_NOT_FOUND          ((PTSTATUS)0xE800001EL)
#define STATUS_ERROR_STRING_PROCESSING          ((PTSTATUS)0xE800001FL)
#define STATUS_KEY_NOT_FOUND                    ((PTSTATUS)0xE8000020L)
#define STATUS_NODE_WITHOUT_TREE                ((PTSTATUS)0xE8000021L)
#define STATUS_NODES_FROM_DIFFERENT_TREES       ((PTSTATUS)0xE8000022L)
#define STATUS_INVALID_TREE_NODES               ((PTSTATUS)0xE8000023L)
#define STATUS_INVALID_TREE_NODE_COUNT          ((PTSTATUS)0xE8000024L)
#define STATUS_CHILD_WITH_INVALID_PARENT_LINK   ((PTSTATUS)0xE8000025L)
#define STATUS_INVALID_LEFT_CHILD_KEY_VALUE     ((PTSTATUS)0xE8000026L)
#define STATUS_INVALID_RIGHT_CHILD_KEY_VALUE    ((PTSTATUS)0xE8000027L)
#define STATUS_INVALID_SUBTREE_HEIGHTS          ((PTSTATUS)0xE8000028L)
#define STATUS_INVALID_NODE_BALANCE             ((PTSTATUS)0xE8000029L)
#define STATUS_STRING_TOO_BIG                   ((PTSTATUS)0xE800002AL)
#define STATUS_INVALID_PORT_TYPE                ((PTSTATUS)0xE800002BL)

#define STATUS_LIST_IS_ALREADY_EMPTY            ((PTSTATUS)0xE800002CL)
#define STATUS_LIST_IS_EMPTY                    ((PTSTATUS)0xE800002CL)

#define STATUS_VERSION_INFO_NOT_FOUND           ((PTSTATUS)0xE800002DL)
#define STATUS_BUFFER_OUTSIDE_ALLOCATED_MEM     ((PTSTATUS)0xE800002EL)
#define STATUS_BUFFER_CORRUPTED                 ((PTSTATUS)0xE800002FL)
//OUTSIDE ALLOCATED, CORRUPTED

#define STATUS_QUEUE_IS_EMPTY                   ((PTSTATUS)0xE8000030L)

#define STATUS_OPERATION_FAILED                 ((PTSTATUS)0xE8000100L)
#define STATUS_CONFIRMATION_FAILED              ((PTSTATUS)0xE8000101L)
#define STATUS_INVALID_NAME                     ((PTSTATUS)0xE8000102L)
#define STATUS_TERMINATION_STARTED              ((PTSTATUS)0xE8000103L)
#define STATUS_MESSAGE_REPLY_UNEXPECTED         ((PTSTATUS)0xE8000104L)
#define STATUS_INVALID_REQUEST_TYPE             ((PTSTATUS)0xE8000105L)
#define STATUS_SERVER_FULL                      ((PTSTATUS)0xE8000106L)

#define STATUS_UNSUPPORTED                      ((PTSTATUS)0xE8000107L)
#define STATUS_MESSAGE_NOT_DELIVERED            ((PTSTATUS)0xE8000108L)
#define STATUS_EXCEPTION_ENCOUNTERED            ((PTSTATUS)0xE8000109L)
#define STATUS_CLIENT_DISCONNECTED              ((PTSTATUS)0xE800010AL)
#define STATUS_SERVER_STOPPED                   ((PTSTATUS)0xE800010BL)

#define STATUS_ITEM_CHANGED_QUEUE               ((PTSTATUS)0xE8000200L)
#define STATUS_ITEM_DOES_NOT_FIT                ((PTSTATUS)0xE8000201L)
#define STATUS_CORRUPTED_WORK_ITEM              ((PTSTATUS)0xE8000202L)

#define STATUS_PE_FORWARD_EXPORT                ((PTSTATUS)0xE8000300L)

#define STATUS_INVALID_PORT                     ((PTSTATUS)0xE8000400L)
#define STATUS_INVALID_PORT_REPLY_FROM_CALLBACK ((PTSTATUS)0xE8000401L)
#define STATUS_INVALID_IOCTL_CODE               ((PTSTATUS)0xE8000402L)
#define STATUS_PORT_CLIENT_NOT_FOUND            ((PTSTATUS)0xE8000403L)
#define STATUS_INVALID_PORT_BUFFER              ((PTSTATUS)0xE8000404L)
#define STATUS_INVALID_PORT_MESSAGE_TYPE        ((PTSTATUS)0xE8000405L)
#define STATUS_REGISTRY_KEY_ACCESS_DENIED_WIN   ((PTSTATUS)0xE8000406L)

#define STATUS_IRQL_NOT_PASSIVE_LEVEL           ((PTSTATUS)0xE8000407L)

#define STATUS_CANT_OPEN_CERTIFICATE_STORE      ((PTSTATUS)0xE8000408L)
#define STATUS_CANT_RETRIEVE_CERT_FROM_STORE    ((PTSTATUS)0xE8000409L)
#define STATUS_FILE_ENCRYPTED_BY_SYSTEM_USER    ((PTSTATUS)0xE800040AL)
#define STATUS_FILE_ENCRYPTED_BY_DIFFERENT_USER ((PTSTATUS)0xE800040BL)
#define STATUS_BLOCKED_IMAGE_COUNT_NOT_ZERO     ((PTSTATUS)0xE800040CL)

#define STATUS_AR_COMMANDS_SIZE_EXCEEDED        ((PTSTATUS)0xE800040DL)

#define STATUS_TOO_MANY_RELEASES                ((PTSTATUS)0xE800040EL)
#define STATUS_NOT_ACQUIRED                     ((PTSTATUS)0xE800040FL)
#define STATUS_TOO_MANY_WAITERS                 ((PTSTATUS)0xE8000410L)
#define STATUS_TOO_MANY_ACQUIRES                ((PTSTATUS)0xE8000411L)

#define STATUS_IRQL_TOO_HIGH                    ((PTSTATUS)0xE8000412L)

#define STATUS_OBJECT_LOCKED                    ((PTSTATUS)0xE8000413L)
#define STATUS_VI_FIELDS_ARE_MISSING            ((PTSTATUS)0xE8000414L)

//
// prototypes
//
char*
PtStatusToString(
    PTSTATUS Status
    );

#endif // _PTSTATUS_H_
