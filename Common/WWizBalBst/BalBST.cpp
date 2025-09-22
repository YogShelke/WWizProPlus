/************************************************************************************************
*  Program Name		: BalBST.h
*  Description		: class declaration for 1 level binary tree of buffer -> structure
*  Author Name		: Ram Shelke
*  Date Of Creation	: 13 Jul 2019
*  Version No		: 3.1.0.0
*************************************************************************************************/

#include "stdafx.h"
#include "BalBST.h"

#define SWAP(x,y)		((x) !=(y)?((x)^=(y)^=(x)^=(y)):0)

unsigned long CRC32Table[]=
{
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076Db419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,
	0xE0D5E91E,0x97D2D988,0x09B64b2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,
	0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385b7,0x136C9856,0x646BA8b0,0xFD62F97A,0x8A65C9EC,0x14015b4F,0x63066CD9,
	0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4b69105E,0xD56041E4,0xA2677172,0x3b03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
	0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5b75,0xDCD60DCF,0xABD13D59,0x26D930AC,0x51DE003A,
	0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3b423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xb60CD9B2,0xB10BE924,
	0x2F6F7C87,0x58684b11,0xb1611DAB,0xB6662D3D,0x76Db4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
	0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635b01,
	0x6B6B51F4,0x1b6b6162,0x856530D8,0xF262004E,0x6b0695ED,0x1B01A57B,0x8208F4b1,0xF50Fb457,0x65B0D9b6,0x12B7E950,
	0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44b65,0x4DB26158,0x3AB551CE,0xA3Bb0074,0xD4BB30E2,
	0x4ADFA541,0x3DD895D7,0xA4D1b46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,
	0xAA0A4b5F,0xDD0D7CC9,0x5005713C,0x270241AA,0xBE0B1010,0xC90b2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
	0x5EDEF90E,0x29D9C998,0xB0D09822,0xb7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5b3B,0xb0BA6CAD,0xEDB88320,0x9ABFB3B6,
	0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73Db1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
	0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,0x6906b2FE,0xF762575D,0x806567CB,
	0x196b3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
	0xD6D6A3E8,0xA1D1937E,0x38D8b2b4,0x4FDFF252,0xD1BB67F1,0xA6Bb5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,
	0x36034AF6,0x41047A60,0xDF60EFb3,0xA867DF55,0x316E8EEF,0x4669BE79,0xCB61B38C,0xBb66831A,0x256FD2A0,0x5268E236,
	0xCb0b7795,0xBB0B4703,0x220216B9,0x5505262F,0xb5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xb2D7FFA7,0xB5D0CF31,
	0x2CD99E8B,0x5BDEAE1D,0x9B64b2B0,0xEb63F226,0x756AA39C,0x026D930A,0x9b0906A9,0xEB0E363F,0x72076785,0x05005713,
	0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
	0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,
	0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3b2,0xA7672661,0xD06016F7,
	0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9Eb5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAb28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xb4614AB8,
	0x5D681B02,0x2A6F2B94,0xB40BBE37,0xb30C8EA1,0x5A05DF1B,0x2D02EF8D
};

ULONG64 CRC64Table[] = {
	0x0000000000000000UL, 0x42f0e1eba9ea3693UL, 0x85e1c3d753d46d26UL, 0xc711223cfa3e5bb5UL, 0x493366450e42ecdfUL,
	0x0bc387aea7a8da4cUL, 0xccd2a5925d9681f9UL, 0x8e224479f47cb76aUL, 0x9266cc8a1c85d9beUL, 0xd0962d61b56fef2dUL,
	0x17870f5d4f51b498UL, 0x5577eeb6e6bb820bUL, 0xdb55aacf12c73561UL, 0x99a54b24bb2d03f2UL, 0x5eb4691841135847UL,
	0x1c4488f3e8f96ed4UL, 0x663d78ff90e185efUL, 0x24cd9914390bb37cUL, 0xe3dcbb28c335e8c9UL, 0xa12c5ac36adfde5aUL,
	0x2f0e1eba9ea36930UL, 0x6dfeff5137495fa3UL, 0xaaefdd6dcd770416UL, 0xe81f3c86649d3285UL, 0xf45bb4758c645c51UL,
	0xb6ab559e258e6ac2UL, 0x71ba77a2dfb03177UL, 0x334a9649765a07e4UL, 0xbd68d2308226b08eUL, 0xff9833db2bcc861dUL,
	0x388911e7d1f2dda8UL, 0x7a79f00c7818eb3bUL, 0xcc7af1ff21c30bdeUL, 0x8e8a101488293d4dUL, 0x499b3228721766f8UL,
	0x0b6bd3c3dbfd506bUL, 0x854997ba2f81e701UL, 0xc7b97651866bd192UL, 0x00a8546d7c558a27UL, 0x4258b586d5bfbcb4UL,
	0x5e1c3d753d46d260UL, 0x1cecdc9e94ace4f3UL, 0xdbfdfea26e92bf46UL, 0x990d1f49c77889d5UL, 0x172f5b3033043ebfUL,
	0x55dfbadb9aee082cUL, 0x92ce98e760d05399UL, 0xd03e790cc93a650aUL, 0xaa478900b1228e31UL, 0xe8b768eb18c8b8a2UL,
	0x2fa64ad7e2f6e317UL, 0x6d56ab3c4b1cd584UL, 0xe374ef45bf6062eeUL, 0xa1840eae168a547dUL, 0x66952c92ecb40fc8UL,
	0x2465cd79455e395bUL, 0x3821458aada7578fUL, 0x7ad1a461044d611cUL, 0xbdc0865dfe733aa9UL, 0xff3067b657990c3aUL,
	0x711223cfa3e5bb50UL, 0x33e2c2240a0f8dc3UL, 0xf4f3e018f031d676UL, 0xb60301f359dbe0e5UL, 0xda050215ea6c212fUL,
	0x98f5e3fe438617bcUL, 0x5fe4c1c2b9b84c09UL, 0x1d14202910527a9aUL, 0x93366450e42ecdf0UL, 0xd1c685bb4dc4fb63UL,
	0x16d7a787b7faa0d6UL, 0x5427466c1e109645UL, 0x4863ce9ff6e9f891UL, 0x0a932f745f03ce02UL, 0xcd820d48a53d95b7UL,
	0x8f72eca30cd7a324UL, 0x0150a8daf8ab144eUL, 0x43a04931514122ddUL, 0x84b16b0dab7f7968UL, 0xc6418ae602954ffbUL,
	0xbc387aea7a8da4c0UL, 0xfec89b01d3679253UL, 0x39d9b93d2959c9e6UL, 0x7b2958d680b3ff75UL, 0xf50b1caf74cf481fUL,
	0xb7fbfd44dd257e8cUL, 0x70eadf78271b2539UL, 0x321a3e938ef113aaUL, 0x2e5eb66066087d7eUL, 0x6cae578bcfe24bedUL,
	0xabbf75b735dc1058UL, 0xe94f945c9c3626cbUL, 0x676dd025684a91a1UL, 0x259d31cec1a0a732UL, 0xe28c13f23b9efc87UL,
	0xa07cf2199274ca14UL, 0x167ff3eacbaf2af1UL, 0x548f120162451c62UL, 0x939e303d987b47d7UL, 0xd16ed1d631917144UL,
	0x5f4c95afc5edc62eUL, 0x1dbc74446c07f0bdUL, 0xdaad56789639ab08UL, 0x985db7933fd39d9bUL, 0x84193f60d72af34fUL,
	0xc6e9de8b7ec0c5dcUL, 0x01f8fcb784fe9e69UL, 0x43081d5c2d14a8faUL, 0xcd2a5925d9681f90UL, 0x8fdab8ce70822903UL,
	0x48cb9af28abc72b6UL, 0x0a3b7b1923564425UL, 0x70428b155b4eaf1eUL, 0x32b26afef2a4998dUL, 0xf5a348c2089ac238UL,
	0xb753a929a170f4abUL, 0x3971ed50550c43c1UL, 0x7b810cbbfce67552UL, 0xbc902e8706d82ee7UL, 0xfe60cf6caf321874UL,
	0xe224479f47cb76a0UL, 0xa0d4a674ee214033UL, 0x67c58448141f1b86UL, 0x253565a3bdf52d15UL, 0xab1721da49899a7fUL,
	0xe9e7c031e063acecUL, 0x2ef6e20d1a5df759UL, 0x6c0603e6b3b7c1caUL, 0xf6fae5c07d3274cdUL, 0xb40a042bd4d8425eUL,
	0x731b26172ee619ebUL, 0x31ebc7fc870c2f78UL, 0xbfc9838573709812UL, 0xfd39626eda9aae81UL, 0x3a28405220a4f534UL,
	0x78d8a1b9894ec3a7UL, 0x649c294a61b7ad73UL, 0x266cc8a1c85d9be0UL, 0xe17dea9d3263c055UL, 0xa38d0b769b89f6c6UL,
	0x2daf4f0f6ff541acUL, 0x6f5faee4c61f773fUL, 0xa84e8cd83c212c8aUL, 0xeabe6d3395cb1a19UL, 0x90c79d3fedd3f122UL,
	0xd2377cd44439c7b1UL, 0x15265ee8be079c04UL, 0x57d6bf0317edaa97UL, 0xd9f4fb7ae3911dfdUL, 0x9b041a914a7b2b6eUL,
	0x5c1538adb04570dbUL, 0x1ee5d94619af4648UL, 0x02a151b5f156289cUL, 0x4051b05e58bc1e0fUL, 0x87409262a28245baUL,
	0xc5b073890b687329UL, 0x4b9237f0ff14c443UL, 0x0962d61b56fef2d0UL, 0xce73f427acc0a965UL, 0x8c8315cc052a9ff6UL,
	0x3a80143f5cf17f13UL, 0x7870f5d4f51b4980UL, 0xbf61d7e80f251235UL, 0xfd913603a6cf24a6UL, 0x73b3727a52b393ccUL,
	0x31439391fb59a55fUL, 0xf652b1ad0167feeaUL, 0xb4a25046a88dc879UL, 0xa8e6d8b54074a6adUL, 0xea16395ee99e903eUL,
	0x2d071b6213a0cb8bUL, 0x6ff7fa89ba4afd18UL, 0xe1d5bef04e364a72UL, 0xa3255f1be7dc7ce1UL, 0x64347d271de22754UL,
	0x26c49cccb40811c7UL, 0x5cbd6cc0cc10fafcUL, 0x1e4d8d2b65facc6fUL, 0xd95caf179fc497daUL, 0x9bac4efc362ea149UL,
	0x158e0a85c2521623UL, 0x577eeb6e6bb820b0UL, 0x906fc95291867b05UL, 0xd29f28b9386c4d96UL, 0xcedba04ad0952342UL,
	0x8c2b41a1797f15d1UL, 0x4b3a639d83414e64UL, 0x09ca82762aab78f7UL, 0x87e8c60fded7cf9dUL, 0xc51827e4773df90eUL,
	0x020905d88d03a2bbUL, 0x40f9e43324e99428UL, 0x2cffe7d5975e55e2UL, 0x6e0f063e3eb46371UL, 0xa91e2402c48a38c4UL,
	0xebeec5e96d600e57UL, 0x65cc8190991cb93dUL, 0x273c607b30f68faeUL, 0xe02d4247cac8d41bUL, 0xa2dda3ac6322e288UL,
	0xbe992b5f8bdb8c5cUL, 0xfc69cab42231bacfUL, 0x3b78e888d80fe17aUL, 0x7988096371e5d7e9UL, 0xf7aa4d1a85996083UL,
	0xb55aacf12c735610UL, 0x724b8ecdd64d0da5UL, 0x30bb6f267fa73b36UL, 0x4ac29f2a07bfd00dUL, 0x08327ec1ae55e69eUL,
	0xcf235cfd546bbd2bUL, 0x8dd3bd16fd818bb8UL, 0x03f1f96f09fd3cd2UL, 0x41011884a0170a41UL, 0x86103ab85a2951f4UL,
	0xc4e0db53f3c36767UL, 0xd8a453a01b3a09b3UL, 0x9a54b24bb2d03f20UL, 0x5d45907748ee6495UL, 0x1fb5719ce1045206UL,
	0x919735e51578e56cUL, 0xd367d40ebc92d3ffUL, 0x1476f63246ac884aUL, 0x568617d9ef46bed9UL, 0xe085162ab69d5e3cUL,
	0xa275f7c11f7768afUL, 0x6564d5fde549331aUL, 0x279434164ca30589UL, 0xa9b6706fb8dfb2e3UL, 0xeb46918411358470UL,
	0x2c57b3b8eb0bdfc5UL, 0x6ea7525342e1e956UL, 0x72e3daa0aa188782UL, 0x30133b4b03f2b111UL, 0xf7021977f9cceaa4UL,
	0xb5f2f89c5026dc37UL, 0x3bd0bce5a45a6b5dUL, 0x79205d0e0db05dceUL, 0xbe317f32f78e067bUL, 0xfcc19ed95e6430e8UL,
	0x86b86ed5267cdbd3UL, 0xc4488f3e8f96ed40UL, 0x0359ad0275a8b6f5UL, 0x41a94ce9dc428066UL, 0xcf8b0890283e370cUL,
	0x8d7be97b81d4019fUL, 0x4a6acb477bea5a2aUL, 0x089a2aacd2006cb9UL, 0x14dea25f3af9026dUL, 0x562e43b4931334feUL,
	0x913f6188692d6f4bUL, 0xd3cf8063c0c759d8UL, 0x5dedc41a34bbeeb2UL, 0x1f1d25f19d51d821UL, 0xd80c07cd676f8394UL,
	0x9afce626ce85b507UL
};

/***************************************************************************************************
Function       : CBalBST
Description    : constructor
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
CBalBST::CBalBST(bool bIsEmbedded): m_bIsEmbedded(bIsEmbedded)
{
	DestroyData();
}

/***************************************************************************************************
Function       : ~CBalBST
Description    : destructor
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
CBalBST::~CBalBST()
{
}

/***************************************************************************************************
Function       : DestroyData
Description    : reset the class object to nulls
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CBalBST::DestroyData()
{
	m_pRoot = m_pTemp = m_pLastSearchResult = m_pLinearTail = NULL;
	m_bLoadedFromFile = m_bTreeBalanced = m_bIsModified = false;
	m_pBuffer = NULL;
	m_nBufferSize = 0;
	m_dwCount = 0;
}

/***************************************************************************************************
Function       : GetNode
Description    : get one node with intialised data
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
NODE* CBalBST::GetNode(ULONG64 dwKey, ULONG64 dwData)
{
	NODE* pTemp = (NODE*)Allocate(sizeof NODE);
	if(NULL == pTemp)
	{
		return (pTemp);
	}

	pTemp->dwHold = 0;
	pTemp->dwData = dwData;
	pTemp->dwKey = dwKey;
	pTemp->pLeft = pTemp->pParent = pTemp->pRight = NULL;
	return (pTemp);
}

/***************************************************************************************************
Function       : AddNode
Description    : add a node to tree
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::AddNode(ULONG64 dwKey, ULONG64 dwData)
{
	bool bSuccess = true;

	m_bTreeBalanced = false;

	if(NULL == m_pRoot)
	{
		m_pRoot = GetNode(dwKey, dwData);
		if(NULL == m_pRoot)
		{
			bSuccess = false;
		}
	}
	else
	{
		COMPARE_RESULT CompResult = EQUAL;
		m_pTemp = m_pRoot;

		while(m_pTemp)
		{
			CompResult = Compare(m_pTemp->dwKey, dwKey);

			if(CompResult == LARGE)
			{
				if(NULL == m_pTemp->pLeft)
				{
					m_pTemp->pLeft = GetNode(dwKey, dwData);
					if(NULL == m_pTemp->pLeft)
					{
						bSuccess = false;
					}
					else
					{
						m_pTemp->pLeft->pParent = m_pTemp;
					}

					break;
				}
				else
				{
					m_pTemp = m_pTemp->pLeft;
				}
			}
			else if(CompResult == SMALL)
			{
				if(NULL == m_pTemp->pRight)
				{
					m_pTemp->pRight = GetNode(dwKey, dwData);
					if(NULL == m_pTemp->pRight)
					{
						bSuccess = false;
					}
					else
					{
						m_pTemp->pRight->pParent = m_pTemp;
					}

					break;
				}
				else
				{
					m_pTemp = m_pTemp->pRight;
				}
			}
			else
			{
				bSuccess = false;
				break;
			}
		}
	}

	if(bSuccess)
	{
		m_dwCount++;
		m_bIsModified = true;
	}

	return (bSuccess);
}

/***************************************************************************************************
Function       : AddNodeAscOrder
Description    : add a node in ascending order
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::AddNodeAscOrder(ULONG64 dwKey, ULONG64 dwData)
{
	bool bSuccess = true;

	m_bTreeBalanced = false;

	if(NULL == m_pRoot)
	{
		m_pRoot = GetNode(dwKey, dwData);
		if(NULL == m_pRoot)
		{
			bSuccess = false;
		}
		else
		{
			m_pLinearTail = m_pRoot;
		}
	}
	else
	{
		m_pLinearTail->pRight = GetNode(dwKey, dwData);
		if(NULL == m_pLinearTail->pRight)
		{
			bSuccess = false;
		}
		else
		{
			m_pLinearTail->pRight->pParent = m_pLinearTail;
			m_pLinearTail = m_pLinearTail->pRight;
		}
	}

	if(bSuccess)
	{
		m_dwCount++;
		m_bIsModified = true;
	}

	return (bSuccess);
}

/***************************************************************************************************
Function       : DeleteNode
Description    : delete a node and restructure the tree
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::DeleteNode(ULONG64 dwKey)
{
	NODE PseudoRoot ={0};
	PNODE pNodeToReplaceWith = 0;
	PNODE pNodeToDelete = 0;
	ULONG64 dwData = 0;

	m_bTreeBalanced = false;
	if((NULL == m_pLastSearchResult) ||(m_pLastSearchResult->dwKey != dwKey))
	{
		if(!FindNode(dwKey, dwData))
		{
			return (false);
		}
	}

	pNodeToDelete = m_pLastSearchResult;

	PseudoRoot.pRight = m_pRoot;
	m_pRoot->pParent = &PseudoRoot;

	if(pNodeToDelete->pLeft && pNodeToDelete->pRight)
	{
		pNodeToReplaceWith = pNodeToDelete->pLeft;
		while(pNodeToReplaceWith->pRight)
		{
			pNodeToReplaceWith = pNodeToReplaceWith->pRight;
		}

		if(pNodeToReplaceWith->pLeft)
		{
			pNodeToReplaceWith->pLeft->pParent = pNodeToReplaceWith->pParent;
		}

		if(pNodeToReplaceWith->pParent->pLeft == pNodeToReplaceWith)
		{
			pNodeToReplaceWith->pParent->pLeft = pNodeToReplaceWith->pLeft;
		}
		else
		{
			pNodeToReplaceWith->pParent->pRight = pNodeToReplaceWith->pLeft;
		}

		pNodeToReplaceWith->pLeft = pNodeToDelete->pLeft;
		pNodeToReplaceWith->pRight = pNodeToDelete->pRight;
		if(pNodeToReplaceWith->pLeft)
		{
			pNodeToReplaceWith->pLeft->pParent = pNodeToReplaceWith;
		}

		if(pNodeToReplaceWith->pRight)
		{
			pNodeToReplaceWith->pRight->pParent = pNodeToReplaceWith;
		}

		pNodeToReplaceWith->pParent = pNodeToDelete->pParent;

		if(pNodeToDelete->pParent->pLeft == pNodeToDelete)
		{
			pNodeToDelete->pParent->pLeft = pNodeToReplaceWith;
		}
		else
		{
			pNodeToDelete->pParent->pRight = pNodeToReplaceWith;
		}
	}
	else if(pNodeToDelete->pRight)
	{
		pNodeToDelete->pRight->pParent = pNodeToDelete->pParent;
		if(pNodeToDelete->pParent->pLeft == pNodeToDelete)
		{
			pNodeToDelete->pParent->pLeft = pNodeToDelete->pRight;
		}
		else
		{
			pNodeToDelete->pParent->pRight = pNodeToDelete->pRight;
		}
	}
	else if(pNodeToDelete->pLeft)
	{
		pNodeToDelete->pLeft->pParent = pNodeToDelete->pParent;
		if(pNodeToDelete->pParent->pLeft == pNodeToDelete)
		{
			pNodeToDelete->pParent->pLeft = pNodeToDelete->pLeft;
		}
		else
		{
			pNodeToDelete->pParent->pRight = pNodeToDelete->pLeft;
		}
	}
	else
	{
		if(pNodeToDelete->pParent->pLeft == pNodeToDelete)
		{
			pNodeToDelete->pParent->pLeft = NULL;
		}
		else
		{
			pNodeToDelete->pParent->pRight = NULL;
		}
	}

	m_pRoot = PseudoRoot.pRight;
	if(m_pRoot)
	{
		m_pRoot->pParent = NULL;
	}

	FreeData(pNodeToDelete->dwData);
	FreeKey(pNodeToDelete->dwKey);

	if(((LPBYTE)pNodeToDelete < m_pBuffer) ||((LPBYTE)pNodeToDelete >= m_pBuffer + m_nBufferSize))
	{
		Release((LPVOID&)pNodeToDelete);
	}

	if(m_dwCount)
	{
		m_dwCount--;
	}

	m_bIsModified = true;
	m_pLastSearchResult = NULL;
	return (true);
}

/***************************************************************************************************
Function       : RemoveAll
Description    : this frees all the memory and sets the tree object to null
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::RemoveAll()
{
	NODE* pHold = NULL;

	if(m_bIsEmbedded)
	{
		DestroyData();
		return (true);
	}

	m_bTreeBalanced = false;
	m_pTemp = m_pRoot;

	while(m_pTemp)
	{
		if(m_pTemp->pLeft)
		{
			m_pTemp = m_pTemp->pLeft;
		}
		else if(m_pTemp->pRight)
		{
			m_pTemp = m_pTemp->pRight;
		}
		else
		{
			if(m_pTemp->pParent)
			{
				if(m_pTemp == m_pTemp->pParent->pLeft)
				{
					m_pTemp->pParent->pLeft = NULL;
				}
				else
				{
					m_pTemp->pParent->pRight = NULL;
				}
			}

			FreeKey(m_pTemp->dwKey);
			FreeData(m_pTemp->dwData);
			pHold = m_pTemp->pParent;

			if(((LPBYTE)m_pTemp < m_pBuffer) ||((LPBYTE)m_pTemp >= m_pBuffer + m_nBufferSize))
			{
				Release((LPVOID &)m_pTemp);
			}

			m_pTemp = pHold;
		}
	}

	if(m_bLoadedFromFile)
	{
		Release((LPVOID&)m_pBuffer);
	}

	DestroyData();
	m_bIsModified = true;
	return (true);
}

/***************************************************************************************************
Function       : GetCount
Description    : traverse and count the nodes in tree
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
DWORD CBalBST::GetCount()
{
	if(m_dwCount)return m_dwCount;

	for(m_pTemp = m_pRoot; m_pTemp != NULL;)
	{
		m_dwCount++;

		if(m_pTemp->pLeft)
		{
			m_pTemp = m_pTemp->pLeft;
		}
		else if(m_pTemp->pRight)
		{
			m_pTemp = m_pTemp->pRight;
		}
		else
		{
			while(m_pTemp)
			{
				if(NULL == m_pTemp->pParent)
				{
					m_pTemp = NULL;
				}
				else if(m_pTemp == m_pTemp->pParent->pRight)
				{
					m_pTemp = m_pTemp->pParent;
				}
				else if(m_pTemp->pParent->pRight)
				{
					m_pTemp = m_pTemp->pParent->pRight;
					break;
				}
				else
				{
					m_pTemp = m_pTemp->pParent;
				}
			}
		}
	}

	return (m_dwCount);
}

/***************************************************************************************************
Function       : GetFirst
Description    : get the root pointer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPVOID CBalBST::GetFirst()
{
	return (m_pRoot);
}

/***************************************************************************************************
Function       : GetNext
Description    : get next preorder node, used in tree traversal
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPVOID CBalBST::GetNext(LPVOID pPrev)
{
	NODE * pNode = (NODE*)pPrev;

	if(pNode->pLeft)
	{
		pNode = pNode->pLeft;
	}
	else if(pNode->pRight)
	{
		pNode = pNode->pRight;
	}
	else
	{
		while(pNode)
		{
			if(NULL == pNode->pParent)
			{
				pNode = NULL;
			}
			else if(pNode == pNode->pParent->pRight)
			{
				pNode = pNode->pParent;
			}
			else if(pNode->pParent->pRight)
			{
				pNode = pNode->pParent->pRight;
				break;
			}
			else
			{
				pNode = pNode->pParent;
			}
		}
	}

	return (pNode);
}

/***************************************************************************************************
Function       : FullSize
Description    : calculate the size to compress when converting to tree
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
int CBalBST::FullSize(int size)
{
	int Rtn = 1;

	while(Rtn <= size)
	{
		Rtn = Rtn + Rtn + 1;
	}

	return Rtn / 2;
}

/***************************************************************************************************
Function       : Compress
Description    : rotate and balance the tree
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CBalBST::Compress(NODE* pRoot, int count)
{
	NODE* scanner = pRoot;

	for(int j = 0; j < count; j++)
	{
		NODE* child = scanner->pRight;
		scanner->pRight = child->pRight;
		scanner = scanner->pRight;
		child->pRight = scanner->pLeft;
		scanner->pLeft = child;
	}
}

/***************************************************************************************************
Function       : ConvertVineToTree
Description    : make the tree of vine, used in balancing
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CBalBST::ConvertVineToTree(NODE* pRoot, int size)
{
	int full_count = FullSize(size);
	Compress(pRoot, size - full_count);
	for(size = full_count; size > 1; size /= 2)
	{
		Compress(pRoot, size / 2);
	}
}

/***************************************************************************************************
Function       : ConvertTreeToVine
Description    : make the vine of tree, used in balancing
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CBalBST::ConvertTreeToVine(NODE* pRoot, int &size)
{
	NODE* vineTail = 0;
	NODE* remainder = 0;
	NODE* tempPtr = 0;

	vineTail = pRoot;
	remainder = vineTail->pRight;
	size = 0;

	while(remainder != NULL)
	{
		if(remainder->pLeft == NULL)
		{
			vineTail = remainder;
			remainder = remainder->pRight;
			size++;
		}
		else
		{
			tempPtr = remainder->pLeft;
			remainder->pLeft = tempPtr->pRight;
			tempPtr->pRight = remainder;
			remainder = tempPtr;
			vineTail->pRight = tempPtr;
		}
	}
}

/***************************************************************************************************
Function       : AdjustParents
Description    : adjust the parents to point to correct childs after balancing
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CBalBST::AdjustParents(NODE * pRoot)
{
	m_pTemp = pRoot;
	while(m_pTemp)
	{
		if(m_pTemp->pLeft)
		{
			m_pTemp->pLeft->pParent = m_pTemp;
			m_pTemp = m_pTemp->pLeft;
		}
		else if(m_pTemp->pRight)
		{
			m_pTemp->pRight->pParent = m_pTemp;
			m_pTemp = m_pTemp->pRight;
		}
		else
		{
			while(m_pTemp)
			{
				if(NULL == m_pTemp->pParent)
				{
					m_pTemp = NULL;
				}
				else if(m_pTemp == m_pTemp->pParent->pRight)
				{
					m_pTemp = m_pTemp->pParent;
				}
				else if(m_pTemp->pParent->pRight)
				{
					m_pTemp->pParent->pRight->pParent = m_pTemp->pParent;
					m_pTemp = m_pTemp->pParent->pRight;
					break;
				}
				else
				{
					m_pTemp = m_pTemp->pParent;
				}
			}
		}
	}
}

/***************************************************************************************************
Function       : Balance
Description    : balance the tree
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::Balance()
{
	int iCount = 0;
	NODE Pseudo_Root ={0, 0, 0, 0, m_pRoot, 0};

	ConvertTreeToVine(&Pseudo_Root, iCount);
	ConvertVineToTree(&Pseudo_Root, iCount);
	m_pRoot = Pseudo_Root.pRight;

	if(m_pRoot)
	{
		m_pRoot->pParent = NULL;
	}

	AdjustParents(m_pRoot);
	m_bIsModified = true;
	m_bTreeBalanced = true;
	return (true);
}

/***************************************************************************************************
Function       : FindNode
Description    : search for a node in tree by key
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::FindNode(ULONG64 dwKey, ULONG64& dwData)
{
	bool bFound = false;
	COMPARE_RESULT CompResult = EQUAL;

	m_pLastSearchResult = NULL;
	m_pTemp = m_pRoot;
	while(m_pTemp)
	{
		CompResult = Compare(dwKey, m_pTemp->dwKey);
		if(SMALL == CompResult)
		{
			m_pTemp = m_pTemp->pLeft;
		}
		else if(LARGE == CompResult)
		{
			m_pTemp = m_pTemp->pRight;
		}
		else
		{
			m_pLastSearchResult = m_pTemp;
			dwData = m_pTemp->dwData;
			bFound = true;
			break;
		}
	}

	return (bFound);
}

/***************************************************************************************************
Function       : GetDataPtr
Description    : get pointer to internal tree root
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
PNODE CBalBST::GetDataPtr()
{
	return (m_pRoot);
}

/***************************************************************************************************
Function       : SetDataPtr
Description    : set the data pointers and values
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::SetDataPtr(PNODE pNode, LPBYTE pbyBuffer, DWORD nBufferSize)
{
	m_pRoot = pNode;
	m_pBuffer = pbyBuffer;
	m_nBufferSize = nBufferSize;
	m_bIsModified = true;
	return (true);
}

/***************************************************************************************************
Function       : GetLargestKey
Description    : return true the largest key
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPVOID CBalBST::GetLargestKey()
{
	for(m_pTemp = m_pRoot; m_pTemp && m_pTemp->pRight ; m_pTemp = m_pTemp->pRight)
	{
		;
	}

	if(m_pTemp)
	{
		return (LPVOID)(m_pTemp->dwKey);
	}
	else
	{
		return NULL;
	}
}

/***************************************************************************************************
Function       : IsModified
Description    : return true if object modified else false
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::IsModified()
{
	return m_bIsModified;
}

/***************************************************************************************************
Function       : SetModified
Description    : sets the object modified flag true
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CBalBST::SetModified(bool bModified)
{
	m_bIsModified = bModified;
}

/***************************************************************************************************
Function       : SearchObject
Description    : when 'bAllPresent' is true the function searches to see that all entries
				 in 'objToSearch' are present in 'this' object. when 'bAllPresent' is false
				 the function searches to see that all entries in 'objToSearch' are absent
				 in 'this' object.
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CBalBST::SearchObject(CBalBST& objToSearch, bool bAllPresent)
{
	return true;
}

/***************************************************************************************************
Function       : Allocate
Description    : allocate dynamic heap memory
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPVOID Allocate(DWORD dwSize)
{
	return (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize));
}

/***************************************************************************************************
Function       : Release
Description    : release the allocated memory
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void Release(LPVOID& pVPtr)
{
	if(pVPtr)
	{
		HeapFree(GetProcessHeap(), 0, pVPtr);
		pVPtr = NULL;
	}
}

/***************************************************************************************************
Function       : DuplicateBuffer
Description    : allocate and copy contents and return this duplicate buffer pointer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPBYTE DuplicateBuffer(LPBYTE pbyBuffer, DWORD nBufferSize)
{
	LPBYTE pHold = NULL;

	if(NULL == pbyBuffer)
	{
		return (NULL);
	}

	pHold =(LPBYTE)Allocate(nBufferSize);
	if(NULL == pHold)
	{
		return (NULL);
	}

	memcpy(pHold, pbyBuffer, nBufferSize);
	return (pHold);
}

/***************************************************************************************************
Function       : DuplicateString
Description    : allocate and copy string and return this duplicate string pointer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPTSTR DuplicateString(LPCTSTR szString)
{
	DWORD iStrSize = 0;
	LPTSTR szNewString = NULL;

	if(NULL == szString)
	{
		return (NULL);
	}

	iStrSize =(DWORD)_tcslen(szString) + 1;
	szNewString =(LPTSTR)Allocate(iStrSize * sizeof(TCHAR));
	if(NULL == szNewString)
	{
		return (NULL);
	}

	_tcscpy_s(szNewString, iStrSize, szString);
	return (szNewString);
}

/***************************************************************************************************
Function       : DuplicateStringA
Description    : allocate and copy ansi string and return this duplicate ansi string pointer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
LPSTR DuplicateStringA(LPCSTR szString)
{
	DWORD iStrSize = 0;
	LPSTR szNewString = NULL;

	if(NULL == szString)
	{
		return (NULL);
	}

	iStrSize =(DWORD)strlen(szString) + 1;
	szNewString =(LPSTR)Allocate(iStrSize);
	if(NULL == szNewString)
	{
		return (NULL);
	}

	strcpy_s(szNewString, iStrSize, szString);
	return (szNewString);
}

/***************************************************************************************************
Function       : MakeFullFilePath
Description    : prepare full path by attaching path if no path present in name
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool MakeFullFilePath(LPCTSTR szFileName, LPTSTR szFullFileName, DWORD dwFullFileNameSize)
{
	if(NULL == szFileName)
	{
		return (false);
	}

	UINT iFileNameLen = 0;
	LPCTSTR szPtr = szFileName;

	while(*szPtr && *szPtr != _T('\\'))
	{
		szPtr++;
	}

	if(*szPtr)
	{
		while(*szPtr)
		{
			szPtr++;
		}

		iFileNameLen =(UINT)(szPtr - szFileName);

		if(iFileNameLen >= dwFullFileNameSize)
		{
			return (false);
		}

		_tcscpy_s(szFullFileName, dwFullFileNameSize, szFileName);
	}
	else
	{
		TCHAR szPath[MAX_PATH]={0};

		iFileNameLen =(UINT)(szPtr - szFileName);
		if(0 == iFileNameLen)
		{
			return (false);
		}

		if(0 == GetModuleFileName(NULL, szPath, _countof(szPath)))
		{
			return (false);
		}

		if(_tcsrchr(szPath, _T('\\')))
		{
			*_tcsrchr(szPath, _T('\\'))= 0;
		}

		if(iFileNameLen + _tcslen(szPath) + 6 >= dwFullFileNameSize)
		{
			return (false);
		}

		_tcscpy_s(szFullFileName, dwFullFileNameSize, szPath);
		_tcscat_s(szFullFileName, dwFullFileNameSize, _T("\\Data\\"));
		_tcscat_s(szFullFileName, dwFullFileNameSize, szFileName);
	}

	return (true);
}

/***************************************************************************************************
Function       : CryptBlock
Description    : crypt a buffer of data
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void CryptBlock(DWORD * Data, DWORD dwDataSize)
{
	const unsigned short MAX_BOX_ITEMS = 0x100;
	DWORD Sbox1[MAX_BOX_ITEMS]={0};
	DWORD Sbox2[MAX_BOX_ITEMS]={0};
	DWORD i = 0, j = 0, x = 0, temp = 0;
	static const DWORD OurUnSecuredKey[] ={0xFFAAFFAA, 0xAAFCAAFC, 0xA37EA37E, 0xB34EB34E,
											0xFFFFFFFF, 0x3BE73BE7, 0xCBA9CBA9, 0x23C123C1,
											0x2E6C2E6C, 0x13CB13CB, 0x64E764E7, 0x34D234D2,
											0xFF6C2E6C, 0x13FF13CB, 0x64E7FFE7, 0x34D234FF};

	// initialize Sbox1 with defined numbers
	for(i = 0; i < _countof(Sbox1); i++)
	{
		Sbox1[i]= 0xFFFFFFFF /(i + 1);
	}

	// initialize Sbox2 with our key
	for(i = 0; i < _countof(Sbox2); i++)
	{
		Sbox2[i]= OurUnSecuredKey[i % _countof(OurUnSecuredKey)];
	}

	// scramble Sbox1 by Sbox2
	for(i = 0; i < _countof(Sbox1); i++)
	{
		j =(j + Sbox1[i]+ Sbox2[i])% _countof(Sbox1);

		if(Sbox1[i]!= Sbox1[j])
		{
			Sbox1[i]= Sbox1[i]^ Sbox1[j];
			Sbox1[j]= Sbox1[i]^ Sbox1[j];
			Sbox1[i]= Sbox1[i]^ Sbox1[j];
		}
	}

	for(i = j = x = 0; x < dwDataSize; x++)
	{
		//increment i
		i =(i + 1U)% _countof(Sbox1);

		//increment j
		j =(j + Sbox1[i])% _countof(Sbox1);

		//Scramble Sbox1 further so encryption routine will
		//will repeat itself at great interval
		if(Sbox1[i]!= Sbox1[j])
		{
			Sbox1[i]= Sbox1[i]^ Sbox1[j];
			Sbox1[j]= Sbox1[i]^ Sbox1[j];
			Sbox1[i]= Sbox1[i]^ Sbox1[j];
		}

		//Get ready to create pseudo random  byte for encryption key
		temp =(Sbox1[i]+ Sbox1[j])%  _countof(Sbox1);

		//get the random byte
		temp = Sbox1[temp];

		//xor with the data and done
		Data[x]= Data[x]^ temp;
	}

	return;
}

/***************************************************************************************************
Function       : CryptBuffer
Description    : crypt buffer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CryptBuffer(LPBYTE pbyBuffer, DWORD dwBufferSize)
{
	DWORD dwBlockData = 0;
	DWORD dwRemainingData = 0;

	dwRemainingData = dwBufferSize % 4;
	dwBlockData = dwBufferSize - dwRemainingData;

	CryptBlock((LPDWORD)pbyBuffer, dwBlockData / 4);
	for(DWORD dwIndex = 0; dwIndex < dwRemainingData; dwIndex++)
	{
		pbyBuffer[dwBlockData + dwIndex]^= 141;
	}

	return (true);
}

/***************************************************************************************************
Function       : CryptFileData
Description    : crypt a file
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CryptFileData(HANDLE hFile, DWORD dwStartOffset)
{
	LPBYTE pbyBuffer = 0;
	DWORD dwBufferSize = 0, dwBytesRead = 0;

	dwBufferSize = GetFileSize(hFile, 0);
	if(dwStartOffset >= dwBufferSize)
	{
		return (false);
	}

	dwBufferSize -= dwStartOffset;
	pbyBuffer =(LPBYTE)Allocate(dwBufferSize);
	if(NULL == pbyBuffer)
	{
		return (false);
	}

	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwStartOffset, 0, FILE_BEGIN))
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	if(!ReadFile(hFile, pbyBuffer, dwBufferSize, &dwBytesRead, 0))
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	if(dwBufferSize != dwBytesRead)
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	if(!CryptBuffer(pbyBuffer, dwBufferSize))
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwStartOffset, 0, FILE_BEGIN))
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	if(!WriteFile(hFile, pbyBuffer, dwBufferSize, &dwBytesRead, 0))
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	if(dwBufferSize != dwBytesRead)
	{
		Release((LPVOID&)pbyBuffer);
		return (false);
	}

	Release((LPVOID&)pbyBuffer);
	return (true);
}

/***************************************************************************************************
Function       : CopyAndCryptFile
Description    : copy szOrgfile to szNewFile and crypt the new file
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CopyAndCryptFile(LPCTSTR szOrgFile, LPCTSTR szNewFile, DWORD dwMaxMemLimit, DWORD dwStartOffset)
{
	bool bError = false;
	LPBYTE byBuffer = NULL;
	HANDLE hFileOrg = NULL, hFileNew = NULL;
	DWORD dwBytesToCrypt = 0, dwBytesCrypted = 0, dwBytesToRead = 0, dwBytesRead = 0;
	BYTE byXorByte = 0;

	hFileOrg = CreateFile(szOrgFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFileOrg)
	{
		return false;
	}

	hFileNew = CreateFile(szNewFile, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFileNew)
	{
		CloseHandle(hFileOrg);
		return false;
	}

	byBuffer = (LPBYTE)Allocate(dwMaxMemLimit);
	if(NULL == byBuffer)
	{
		goto ERROR_EXIT;
	}

	if(!ReadFile(hFileOrg, byBuffer, dwStartOffset, &dwBytesRead, 0))
	{
		goto ERROR_EXIT;
	}

	if(!WriteFile(hFileNew, byBuffer, dwStartOffset, &dwBytesRead, 0))
	{
		goto ERROR_EXIT;
	}

	dwBytesToCrypt = GetFileSize(hFileOrg, 0);
	dwBytesCrypted = dwStartOffset;

	while(dwBytesCrypted < dwBytesToCrypt)
	{
		dwBytesToRead = dwBytesToCrypt - dwBytesCrypted;
		dwBytesToRead = dwBytesToRead > dwMaxMemLimit ? dwMaxMemLimit : dwBytesToRead;

		if(!ReadFile(hFileOrg, byBuffer, dwBytesToRead, &dwBytesRead, 0))
		{
			bError = true;
			goto ERROR_EXIT;
		}

		if(dwBytesToRead != dwBytesRead)
		{
			bError = true;
			break;
		}

		CryptBlock((LPDWORD)byBuffer, (dwBytesRead / sizeof(DWORD)));
		if(!WriteFile(hFileNew, byBuffer, dwBytesToRead, &dwBytesRead, 0))
		{
			bError = true;
			goto ERROR_EXIT;
		}

		dwBytesCrypted += dwBytesRead;
	}

	if(bError)
	{
		goto ERROR_EXIT;
	}

	Release((LPVOID&)byBuffer);
	CloseHandle(hFileOrg);
	CloseHandle(hFileNew);
	return true;

ERROR_EXIT:

	CloseHandle(hFileOrg);
	CloseHandle(hFileNew);
	DeleteFile(szNewFile);
	Release((LPVOID&)byBuffer);
	return false;
}

/***************************************************************************************************
Function       : CryptFile
Description    : open file and crypt data
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CryptFile(LPCTSTR szFileName, const short iMAX_HEADER_SIZE)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szFullFileName[MAX_PATH]={0};

	if(!MakeFullFilePath(szFileName, szFullFileName, _countof(szFullFileName)))
	{
		return (false);
	}

	hFile = CreateFile(szFullFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return (false);
	}

	if(!CryptFileData(hFile, iMAX_HEADER_SIZE))
	{
		CloseHandle(hFile);
		return (false);
	}

	CloseHandle(hFile);
	return (true);
}

/***************************************************************************************************
Function       : CreateCRC64
Description    : create crc64 of string
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CreateCRC64(LPCTSTR szString, ULONG64& ul64CRC)
{
	if(NULL == szString)
	{
		return (false);
	}

	ul64CRC = 0xFFFFFFFFFFFFFFFFUL;

	do
	{
		if(*szString > 0xFF)
		{
			BYTE byChar = 0;
			byChar = (BYTE)(*szString);
			ul64CRC = CRC64Table [(ul64CRC >> 56)^ byChar]^(ul64CRC << 8);
			byChar = (BYTE)(*szString >> 4);
			ul64CRC = CRC64Table [(ul64CRC >> 56)^ byChar]^(ul64CRC << 8);
		}
		else
		{
			ul64CRC = CRC64Table [(ul64CRC >> 56)^ *szString]^(ul64CRC << 8);
		}
	} while(*++szString);

	ul64CRC = ~ul64CRC;
	return (true);
}

/***************************************************************************************************
Function       : CreateCRC64Buffer
Description    : create crc64 of buffer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CreateCRC64Buffer(LPBYTE byBuffer, SIZE_T cbBuffer, ULONG64& ul64CRC)
{
	if(NULL == byBuffer || 0 == cbBuffer )
	{
		return (false);
	}

	ul64CRC = 0xFFFFFFFFFFFFFFFFUL;

	for(SIZE_T i = 0; i < cbBuffer; i++)
	{
		ul64CRC = CRC64Table [(ul64CRC >> 56)^ byBuffer[i]] ^(ul64CRC << 8);
	}

	ul64CRC = ~ul64CRC;
	return (true);
}

/***************************************************************************************************
Function       : CreateHeaderData
Description    : create header data for file integrity check
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CreateHeaderData(HANDLE hFile, LPCTSTR szFullFileName, LPBYTE Buffer, DWORD cbBuffer, ULONG64 ulCount)
{
	LPCTSTR ptrLastSlash = NULL;
	ULONG64 ul64CRC = 0;
	ULONG64 ul64FileSize = 0;
	DWORD dwFileSizeHigh = 0;
	TCHAR* szDuplicateFileName = 0;

	if(NULL == szFullFileName)
	{
		return (false);
	}

	ptrLastSlash = _tcsrchr(szFullFileName, _T('\\'));
	if(NULL == ptrLastSlash)
	{
		ptrLastSlash = szFullFileName;
	}

	szDuplicateFileName = _tcsdup(ptrLastSlash);
	if(NULL == szDuplicateFileName)
	{
		return (false);
	}

#pragma warning(disable:4996)
	_tcslwr(szDuplicateFileName);
#pragma warning(default:4996)

	if(!CreateCRC64(szDuplicateFileName, ul64CRC))
	{
		free(szDuplicateFileName);
		return (false);
	}

	free(szDuplicateFileName);

	FlushFileBuffers(hFile);
	ul64FileSize = GetFileSize(hFile, &dwFileSizeHigh);
	ul64FileSize = MKQWORD(dwFileSizeHigh, ((DWORD)ul64FileSize));

	if(cbBuffer <(sizeof(ul64CRC) + sizeof(ul64FileSize) + sizeof(ulCount)))
	{
		return (false);
	}

	memcpy(Buffer, &ul64FileSize, sizeof(ul64FileSize));
	memcpy(Buffer + sizeof(ul64FileSize), &ul64CRC, sizeof(ul64CRC));
	memcpy(Buffer + sizeof(ul64FileSize) + sizeof(ul64CRC), &ulCount, sizeof(ulCount));
	return (true);
}

/***************************************************************************************************
Function       : CreateCRC32
Description    : create crc32 of string
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CreateCRC32(LPCTSTR szString, DWORD& dwCRC32)
{
	dwCRC32 = 0xFFFFFFFF;
	do
	{
		dwCRC32 = (dwCRC32 >> 8) ^ CRC32Table[ (dwCRC32 ^ *szString) & 0xFF ];
	}while(*szString++);

	dwCRC32 ^= 0xFFFFFFFF;
	return true;
}

/***************************************************************************************************
Function       : CreateCRC32Buffer
Description    : create crc32 of buffer
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool CreateCRC32Buffer(LPBYTE byBuffer, size_t cbBuffer, DWORD& dwCRC32)
{
	dwCRC32 = 0xFFFFFFFF;
	for(size_t i = 0; i < cbBuffer; i++)
	{
		dwCRC32 = (dwCRC32 >> 8) ^ CRC32Table[ (dwCRC32 ^ byBuffer[i]) & 0xFF ];
	}

	dwCRC32 ^= 0xFFFFFFFF;
	return true;
}

/***************************************************************************************************
Function       : PrepareName
Description    : prepare name using process name and dbfile name
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
bool PrepareName(LPTSTR szPath, SIZE_T cchPath, LPCTSTR szFilePath, LPCTSTR szAppendName)
{
	LPCTSTR Ptr = 0, Ptr1 = 0;
	TCHAR szProcessName[MAX_PATH] = {0};

	GetModuleFileName(0, szProcessName, _countof(szProcessName));
	if(!szProcessName[0])
	{
		return false;
	}

	Ptr1 = _tcsrchr(szProcessName, _T('\\'));
	if(!Ptr1)
	{
		return false;
	}

	Ptr1++;

	Ptr = _tcsrchr(szAppendName, _T('\\'));
	if(!Ptr)
	{
		return false;
	}

	Ptr++;

	if(_tcslen(szFilePath) + 1 + _tcslen(Ptr1) + 1 + _tcslen(Ptr) + _tcslen(_T(".txt")) >= cchPath)
	{
		return false;
	}

	_tcscpy_s(szPath, cchPath, szFilePath);
	_tcscat_s(szPath, cchPath, _T("_"));
	_tcscat_s(szPath, cchPath, Ptr1);
	_tcscat_s(szPath, cchPath, _T("_"));
	_tcscat_s(szPath, cchPath, Ptr);
	_tcscat_s(szPath, cchPath, _T(".txt"));
	return true;
}

/***************************************************************************************************
Function       : DumpBuffer
Description    : dump buffer in filename using DebugData name
Author         : Ram Shelke
Date           : 13-JUL-2019
***************************************************************************************************/
void DumpBuffer(LPBYTE byBuffer, SIZE_T cbBuffer, LPCTSTR szFilePath, LPCTSTR szAppendName)
{
	HANDLE hFile = 0;
	DWORD dwBytesWritte = 0;
	TCHAR szFinalPath[MAX_PATH] = {0};

	if(!PrepareName(szFinalPath, _countof(szFinalPath), szFilePath, szAppendName))
	{
		return;
	}

	hFile = CreateFile(szFinalPath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return;
	}

	WriteFile(hFile, byBuffer, (DWORD)cbBuffer, &dwBytesWritte, 0);
	CloseHandle(hFile);
	return;
}

LPVOID VAllocate(SIZE_T nRegionSize)
{
	return VirtualAlloc(0, nRegionSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

LPVOID VRelease(LPVOID lpvMemBase)
{
	return VirtualFree(lpvMemBase, 0, MEM_RELEASE)?lpvMemBase:NULL;
}

BOOL VChangeProtection(LPVOID lpvMemBase, SIZE_T nRegionSize, BOOL bEnable)
{
	DWORD dwPreviousProtecttion = 0;
	return VirtualProtect(lpvMemBase, nRegionSize, PAGE_READONLY, &dwPreviousProtecttion);
}
