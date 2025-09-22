#pragma once
#define DDS_DLIL_HARDDRIVES 1   // hard drives
#define DDS_DLIL_CDROMS     2   // CD-ROM drives
#define DDS_DLIL_REMOVABLES 4   // other removable drives (floppy, Zip, etc.)
#define DDS_DLIL_NETDRIVES  8   // mapped network drives
#define DDS_DLIL_RAMDRIVES  16  // RAM drives

#define DDS_DLIL_ALL_REMOVABLE ( DDS_DLIL_CDROMS | DDS_DLIL_REMOVABLES )

#define DDS_DLIL_ALL_LOCAL_DRIVES ( DDS_DLIL_HARDDRIVES | DDS_DLIL_CDROMS | \
                                    DDS_DLIL_REMOVABLES | DDS_DLIL_RAMDRIVES )
#define DDS_DLIL_ALL_DRIVES_NO_CDROM ( DDS_DLIL_HARDDRIVES |  \
                              DDS_DLIL_REMOVABLES | DDS_DLIL_NETDRIVES | \
                              DDS_DLIL_RAMDRIVES )

#define DDS_DLIL_ALL_DRIVES ( DDS_DLIL_HARDDRIVES | DDS_DLIL_CDROMS | \
                              DDS_DLIL_REMOVABLES | DDS_DLIL_NETDRIVES | \
                              DDS_DLIL_RAMDRIVES )


class CDrivePickerListCtrl : public CListCtrl
{
	
	public:
		CDrivePickerListCtrl();
		void InitList ( int nIconSize, DWORD dwFlags );
		void  SetSelection ( const DWORD dwDrives );
		void  UnCheckAll();
		void  SetSelection ( LPCTSTR szDrives );
		BYTE  GetNumSelectedDrives() const;
		void  GetSelectedDrives ( DWORD* pdwSelectedDrives ) const;
		void  GetSelectedDrives ( LPTSTR szSelectedDrives ) const;
		virtual ~CDrivePickerListCtrl();

	protected:
		CImageList  m_ImgList;
		void CommonInit ( BOOL bUseLargeIcons, DWORD dwFlags );
		DECLARE_MESSAGE_MAP()
};
