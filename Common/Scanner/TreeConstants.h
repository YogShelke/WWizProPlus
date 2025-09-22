#pragma once
#define MAX_FILE_PATH 260
#include <map>

using namespace std;

//Forward declare the node
struct _Node;

//Our map
typedef std::map <char, _Node*> SearchMap;

//================================ Node Declaration ================================
typedef struct _Node
{
	char			aChar;			//Our character
	bool			bFinal;			//Do we have a word here
	SearchMap		aMap;			//Our next nodes
	_Node*			pFailureNode;	//Where we go incase of failure
	unsigned short	usDepth;		//Depth of this level
} Node;

/*=========================================================================*/
/*							ENUMS										   */
/*=========================================================================*/

typedef enum ___SCAN_TYPE
{
	FULLSCAN,
	CUSTOMSCAN,
	QUICKSCAN,
	USBSCAN,
	USBDETECT,
	ANTIROOTKITSCAN,
	ACTIVESCAN,
	INDEXING,
	RESERVED,
	RESERVED1,
	RESERVED2,
	BOOTSCANNER,
	EMAILSCAN,
	SYSFILESCAN
}SCANTYPE;
/*=========================================================================*/
