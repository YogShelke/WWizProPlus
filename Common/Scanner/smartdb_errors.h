


#define SSERR_FILE_OPEN_ERROR						-1
#define SSERR_INVALID_PARAMETERS					-2
#define	SSERR_FILE_MAPPING_ERROR					-3
#define SSERR_NTFS_STREAM							-4
#define SSERR_FILE_TOO_SMALL						-5


//NTFS Specific

#define	SSERR_FILE_NOT_ON_NTFS_VOLUME				-6
#define	SSERR_GET_FILE_INFORMATION					-7

#define	SSERR_NTFS_FILE_NOT_FOUND_IN_CACHE			-8
#define SSERR_NO_FREE_NODE							-9
#define	SSERR_MAX_HASH_LIST_SIZE_REACHED			-10

//Read drive journal error
#define SSERR_JOURNAL_INACTIVE						-11
#define SSERR_JOURNAL_EXPIRED						-12
#define SSERR_JOURNAL_UNKNOWN_ERROR					-13

//Initialize errors
#define SSERR_VOLUME_MANAGE_ERROR					-14

//perform journal tests
#define SSERR_JOURNAL_RST_TIMEOUT					-15	//reached timeout while resetting journal
#define	SSERR_JOURNAL_RST_UNKNOWN_ERROR				-16	//encountered other error than 'delete in progress' when trying to create journal

//multiple key return code
#define SSERR_NO_KEY_FOUND							-17 //record exists but no matching key can be found


#define SSERR_TRANSACTION_ALREADY_ADDED				-21
#define SSERR_PARENT_TRANSACTION_ALREADY_ADDED		-22
#define SSERR_TRANSACTION_NOT_FOUND					-23
#define SSERR_TOO_MANY_CONCURRENT_TRANSACTION		-24
#define SSERR_DIR_NO_MORE_EMPTY_SLOTS				-25
#define	SSERR_NTFS_DIR_NOT_FOUND_IN_CACHE			-26
#define SSERR_NTFS_FILE_NOT_SET_BY_AV				-27
#define SSERR_NTFS_FILE_TOO_MANY_PARENTS			-28
#define SSERR_NTFS_DELAYED_START					-29
#define SSERR_INVALID_TRANSACTION_PARAMETER			-30
#define SSERR_IO_EXCEPT_ERROR						-31

#define SSERR_EXTRA_DATA_INIT_FAILED				-51
#define SSERR_TOO_MANY_DIR_TO_INVALIDATE			-52

#define SSERR_ENTRY_NOT_FOUND						-53
#define SSERR_CRASH_ERROR							-54

#define SSERR_QID_COULD_NOT_BE_COMPUTED				-55
//MESSAGES CAN OVERLAP

//Search file messages
#define SSMSG_FILE_IN_CACHE_NOT_NEEDING_RESCAN		1
#define	SSMSG_FILE_IN_CACHE_BUT_NEEDING_RESCAN		0

//Search Internal messages
#define SSMSG_FILE_IN_CACHE							0

//Add file messages
#define SSMSG_NEW_FILE_ADDED						2
#define	SSMSG_OLD_FILE_UPDATED						1


//delete messages
#define SSMSG_FILE_DELETED							0


//Read drive journal messages
#define	SSMSG_READ_JOURNAL_TIMEOUT					2
#define SSMSG_READ_ALL_JOURNAL_ENTRIES				1

//Read drive journal action error messages
#define	SSMSG_DRIVE_JOURNAL_USABLE					1
#define SSMSG_DRIVE_JOURNAL_UNUSABLE				0

//Perform journal test messages
#define SSMSG_JOURNAL_USABLE						1




//Extra data errors
#define SSERR_EXTRA_DATA_ADD_OR_UPDATE_MD5					-101
#define SSERR_EXTRA_DATA_FILE_NOT_FOUND				-102
#define SSERR_EXTRA_DATA_ADD_OR_UPDATE_ENTRY		-103
#define SSERR_EXTRA_DATA_RESOURCES					-104
#define SSERR_EXTRA_DATA_OPEN_DATABASE				-105
#define SSERR_EXTRA_DATA_DELETE_FILE				-106
#define SSERR_EXTRA_DATA_BAD_MD5					-107
#define SSERR_EXTRA_MD5_NOT_FOUND					-1
#define SSERR_EXTRA_DATA_ADD_MD5					-7


#define RET_EXTRA_DATA_MD5_NOT_FOUND				-1
#define RET_EXTRA_DATA_WRITE_ERROR					-2
#define RET_EXTRA_DATA_READ_ERROR					-2
#define RET_EXTRA_DATA_SUCCES						1
#define RET_EXTRA_DATA_INVALID_DATABASE_HANDLE		-3
#define RET_EXTRA_DATABASE_ALREADY_INITIALISED		-1
#define RET_EXTRA_DATABASE_UNINITIALISED_OFFSET		-1
#define RET_EXTRA_DATABASE_OPEN_ERROR				-1
#define RET_EXTRA_DATA_GET_FILE_INFORMATION_ERROR	-3

