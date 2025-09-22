#ifndef INCLUDE_SCAN_API_CPP_H
#define INCLUDE_SCAN_API_CPP_H

#include "scanapi.h"

#define CHECK_AND_THROW_ERROR( A )  \
        int errcode = A; \
        if ( errcode != 0 ) \
            throw CThreatScannerException( errcode );

// This is an exception generated if any SDK operation returned error.
class CThreatScannerException
{
	public:
		CThreatScannerException( int code )
		{
			m_code = code;
		}

		virtual ~CThreatScannerException()
		{
		}

		//! Returns error code
		int GetCode() const
		{
			return m_code;
		}

	protected:
		int		m_code;
};


// This is a C++ class which wraps all the SDK functions into object-oriented interface
class CThreatScannerIface
{
	protected:
		CThreatScanner *	m_scanner;

	public:
		// A constructor might throw exception!
		CThreatScannerIface()
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_CreateInstance( &m_scanner ) );
		}

		// If the scanner instance was not properly destroyed, the destructor may 
		// also throw exception. This will lead to crash if the CThreatScannerIface
		// object is destroyed during stack unwinding by another exception handler.
		virtual ~CThreatScannerIface()
		{
			if ( m_scanner )
				DestroyInstance();
		}

		void DestroyInstance()
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_DestroyInstance( m_scanner ) );
			m_scanner = 0;
		}

		void SetScanPriority( int priority )
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_SetScanPriority( m_scanner, priority) );
		}

		void SetScanCallback( SCAN_CALLBACK pfnCallback, void * pContext )
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_SetScanCallback( m_scanner, pfnCallback, pContext ) );
		}

		void SetScanCallback2( SCAN2_CALLBACK pfnCallback2, void * pContext )
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_SetScanCallback2( m_scanner, pfnCallback2, pContext ) );
		}

		void ScanPath( int objectType, const CHAR_T * szPath, int accessorPID)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanPath( m_scanner, objectType, szPath, accessorPID) );
		}

		void ScanObject( int objectType, const CHAR_T * szObjectPath, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int accessorPID = 0, const CHAR_T * szObjectName = 0 )
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanObject( m_scanner, objectType, szObjectPath, attemptClean, pnScanStatus, pnThreatType, szThreatInfo, accessorPID, szObjectName) );
		}

		void ScanObjectByHandle( void * hObject, const CHAR_T * szObjectName, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo, int accessorPID)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanObjectByHandle( m_scanner, hObject, szObjectName, attemptClean, pnScanStatus, pnThreatType, szThreatInfo, accessorPID) );
		}

		void InitializeMemoryScan( const CHAR_T * mappingName, unsigned long mappingSize )
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_InitializeMemoryScan( m_scanner, mappingName, mappingSize ) );
		}
		
		void InitializeMemoryScanEx( const CHAR_T * mappingName, unsigned long mappingSize, int isReadOnly )
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_InitializeMemoryScanEx( m_scanner, mappingName, mappingSize, isReadOnly ) );
		}

		void UninitializeMemoryScan()
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_UninitializeMemoryScan(m_scanner) );
		}

		void ScanMemory( int nObjectType, unsigned long * pdwObjectSize, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanMemory( m_scanner, nObjectType, pdwObjectSize, attemptClean, pnScanStatus, pnThreatType, szThreatInfo) );
		}

		void ScanMemoryEx( const CHAR_T * objectName, int nObjectType, unsigned long * pdwObjectSize, int attemptClean, int * pnScanStatus, int * pnThreatType, const CHAR_T **szThreatInfo)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanMemoryEx( m_scanner, objectName, nObjectType, pdwObjectSize, attemptClean, pnScanStatus, pnThreatType, szThreatInfo) );
		}
		
		void SetOption( int option, int value)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_SetIntOption( m_scanner, option, value) );
		}

		void SetOption( int option, const CHAR_T * value)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_SetStringOption( m_scanner, option, value) );
		}

		void GetLicenseInformation( unsigned long * expirationTime)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_GetLicenseInformation( m_scanner, expirationTime ) );
		}

		void EnumerateDatabaseRecords( DB_ENUM_CALLBACK enumCallback, void * context)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_EnumerateDatabaseRecords( m_scanner, enumCallback, context) );
		}

		void GetDatabaseInformation( unsigned long * numberOfSignatures, unsigned long * lastUpdateTime)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_GetDatabaseInformation( m_scanner, numberOfSignatures, lastUpdateTime) );
		}

		void SetPasswordCallback( PASSWORD_CALLBACK passwordCallback, void * context)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_SetPasswordCallback( m_scanner, passwordCallback, context) );
		}

		void ScanBuffer( void * bufPtr, int objectType, unsigned long bufferSize, unsigned long * pulObjectSize, int attemptClean, int * objectStatus, int * pnObjectThreatType, const CHAR_T **objectThreatInfo)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanBuffer( m_scanner, bufPtr, objectType, bufferSize, pulObjectSize, attemptClean, objectStatus, pnObjectThreatType, objectThreatInfo) );
		}
		
		void ScanBufferEx( void * bufPtr, const CHAR_T * objectName, int objectType, unsigned long bufferSize, unsigned long * pulObjectSize, int attemptClean, int * objectStatus, int * pnObjectThreatType, const CHAR_T **objectThreatInfo)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanBufferEx( m_scanner, bufPtr, objectName, objectType, bufferSize, pulObjectSize, attemptClean, objectStatus, pnObjectThreatType, objectThreatInfo) );
		}
		
		void ScanStream( IStream * stream, int objectType, int attemptClean, int * objectStatus, int * objectThreatType, const CHAR_T **objectThreatInfo)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanStream( m_scanner, stream, objectType, attemptClean, objectStatus, objectThreatType, objectThreatInfo) );
		}
		
		void ScanMappedMemory( int nObjectType, const CHAR_T * szObjectName, UINT64 hGzHandle, LONGLONG dwObjectSize, int * pnScanStatus, int * pnThreatType, const CHAR_T ** szThreatInfo)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_ScanMappedMemory( m_scanner, nObjectType, szObjectName, hGzHandle, dwObjectSize, pnScanStatus, pnThreatType, szThreatInfo) );
		}
		
		void GetOption( int option, void * value)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_GetOption( m_scanner, option, value) );
		}

		void GetVersion(int * versionMajor, int * versionMinor)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_GetVersion( versionMajor, versionMinor) );
		}

		void GetScanStatistics(ScanStatistics **ppStatistics)
		{
			CHECK_AND_THROW_ERROR( ThreatScanner_GetScanStatistics(m_scanner, ppStatistics));
		}
};

#undef CHECK_AND_THROW_ERROR

#endif // INCLUDE_SCAN_API_CPP_H
