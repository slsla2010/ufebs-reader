#define CM_FILE_VISUAL	3077
#define CM_FILE_SAVE	CM_FILE_VISUAL+1
#define CM_FILE_PRETTY	CM_FILE_VISUAL+2
#define CM_FILE_EXTRACT	CM_FILE_VISUAL+3
#define CM_FILE_SWIFT   CM_FILE_VISUAL+20
#define CM_FILE_PROP    CM_FILE_VISUAL+4 
#define CM_FILE_TRUNC	CM_FILE_VISUAL+5
#define CM_FILE_DECODE	CM_FILE_VISUAL+6
#define CM_FILE_EXIT	CM_FILE_VISUAL+7
#define CM_FILE_OPEN	CM_FILE_VISUAL+8
#define CM_FILE_REPLACE CM_FILE_VISUAL+9
#define CM_FILE_ENCODE  CM_FILE_VISUAL+10
#define CM_FILE_HEAD    CM_FILE_VISUAL+11
#define CM_FILE_CHANGE  CM_FILE_VISUAL+12
#define CM_FILE_FIND	CM_FILE_VISUAL+13
#define CM_FIND_NEXT	CM_FILE_VISUAL+14
#define CM_FILE_SED_DIA CM_FILE_VISUAL+15
#define CM_CHANGE_DATE  CM_FILE_VISUAL+17
#define CM_CHANGE_DATEF CM_FILE_VISUAL+18
#define CM_XPATH_LOOK   CM_FILE_VISUAL+19
#define CM_CHANGE_EDNO         CM_FILE_VISUAL+31
#define CM_CHANGE_ACCDOCNO     CM_FILE_VISUAL+32
#define CM_CHANGE_SUM	       CM_FILE_VISUAL+33
#define CM_CHANGE_RECALCULATE  CM_FILE_VISUAL+34
#define CM_CHANGE_ANALIZE      CM_FILE_VISUAL+35
#define CM_CUT_ED101	       CM_FILE_VISUAL+36
#define CM_SETTINGS	       CM_FILE_VISUAL+37

#define SETTINGS_COUNT		10
#define SETTINGS_ID_BASE       5087

#define SET_LOGGING	       SETTINGS_ID_BASE
#define SET_CHANGE_ACCDOCNO    SETTINGS_ID_BASE+1
#define SET_CHANGE_EDNO        SETTINGS_ID_BASE+2
#define SET_CHANGE_SUM         SETTINGS_ID_BASE+3
#define SET_CHECK_ED501        SETTINGS_ID_BASE+4
#define SET_COMPRESS_SPACES    SETTINGS_ID_BASE+5
#define SET_SAVE_PROPER_NAME   SETTINGS_ID_BASE+6
#define SET_CHANGE_RECALCULATE SETTINGS_ID_BASE+7
#define SET_FULLMENU	       SETTINGS_ID_BASE+8
#define SET_CHANGE_FORMAT_XML  SETTINGS_ID_BASE+9
#define SET_XML40817           SETTINGS_ID_BASE+10


//#define	SET_XMLCONSTRUCT       SETTINGS_ID_BASE+10




#define gF_logging  		_gFlags[SET_LOGGING-SETTINGS_ID_BASE].value
#define gF_changeAccDocNo	_gFlags[SET_CHANGE_ACCDOCNO-SETTINGS_ID_BASE].value
#define gF_changeEDNo		_gFlags[SET_CHANGE_EDNO-SETTINGS_ID_BASE].value
#define gF_changeSum		_gFlags[SET_CHANGE_SUM-SETTINGS_ID_BASE].value
#define gF_checkED501		_gFlags[SET_CHECK_ED501-SETTINGS_ID_BASE].value
#define gF_compressSpaces	_gFlags[SET_COMPRESS_SPACES-SETTINGS_ID_BASE].value
#define gF_saveProper		_gFlags[SET_SAVE_PROPER_NAME-SETTINGS_ID_BASE].value
#define gF_recalculate		_gFlags[SET_CHANGE_RECALCULATE-SETTINGS_ID_BASE].value
#define gF_fullMenu		_gFlags[SET_FULLMENU-SETTINGS_ID_BASE].value
#define gF_formatXML 		_gFlags[SET_CHANGE_FORMAT_XML-SETTINGS_ID_BASE].value

#define CM_HELP_ABOUT	4077
#define CM_ABOUT        CM_HELP_ABOUT
#define CM_EDIT_PASTE	CM_HELP_ABOUT+1
#define CM_EDIT_COPY	CM_HELP_ABOUT+2
#define CM_EDIT_CUT		CM_HELP_ABOUT+3
#define CM_EDIT_REDO	CM_HELP_ABOUT+4
#define CM_EDIT_UNDO	CM_HELP_ABOUT+5
#define CM_FILE_SAVEAS	CM_HELP_ABOUT+6

#define CM_IDKFA	CM_HELP_ABOUT+7
#define CM_TEST		CM_HELP_ABOUT+8

//#define ICONVED_TEXT
#define WALK_LEVEL

// параметры сборки разных версий
//#define MULTI_DOC
//#define TEST_CTL3D	
//#define FULL_MENU
////////////////////////////////////

#define ID_FIND		4008
#define ID_REPL		ID_FIND+1
#define ID_FIND_TEXT	ID_FIND+2
#define ID_REPL_TEXT	ID_FIND+3
#define ID_SED_TEXT	ID_FIND+4
#define ID_NEXT  	ID_FIND+5
#define ID_CANCEL       ID_FIND+6
#define ID_XPATH_TEXT   ID_FIND+7
#define ID_XPATH_GO     ID_FIND+8
#define ID_XML_CHECK1   ID_FIND+9

#define ID_SETT_DLG_EDNO 	 ID_FIND+20
#define ID_SETT_DLG_ACCDOCNO	 ID_FIND+21
#define ID_SETT_DLG_SUM          ID_FIND+22
#define ID_SETT_DLG_LOG          ID_FIND+23
#define ID_SETT_DLG_COMPRESS_SPACE  ID_FIND+24
#define ID_SETT_DLG_FORMAT_XML   ID_FIND+25


#define ID_STATUSBAR    4996
#define ID_TOOLBAR      ID_STATUSBAR+1
//#define IDB_TBBITMAP    4209

#define IDS_ABOUT_TEXT  ID_STATUSBAR+2
#define IDS_PROG_DESCR  ID_STATUSBAR+3

#define TOOLBAR_ICONS	1793
#define IDB_TOOLBITMAP  TOOLBAR_ICONS

// не менять - это порядковый номер нужной иконки в BMP файле.
// для другого файла - будут другие номера
#define ID_ICON_LOAD    6
#define ID_ICON_VISUAL  4 
#define ID_ICON_SAVE    3 
#define ID_ICON_FIND    2 
#define ID_ICON_PRETTY  1 
#define ID_ICON_IDKFA   0 
#define ID_ICON_CHDAT   5
#define ID_ICON_PERO	7
#define ID_ICON_B64E	8
#define ID_ICON_B64D	9
#define ID_ICON_SETT	10


#define false 0
#define true  1
#define bool int

#define IDC_MAIN_TEXT   1001
#define IDC_DATEPICK    1003
#define ID_TIME_COMPILE 7656


#define _SKIP_ED101_    1
#define _DELETE_ED101_  1
#define _WRITE_ED101_   0




void  bprintf( char *format, ...) ;
void  utf8cp1251(const char *in, char *out) ;
char *tr(char *src, char *dest, int ch, char *newch) ;
char *trC(char *src, char *dest, int ch, char *newch) ;
int   trS(char *src, char *dest, char *ch, char *newch)  ;

void  b64encode( char *inbuffer, char *outbuffer, int length_in_buff , int linesize ) ;
char *reformatSumInt( unsigned long lSum , char *buf ) ;
void  decode( char *inbuffer, char *outbuffer, int length_in_buff ) ;
int   parseSEDcmd( char *cs, char *cmdr , char *chgt ) ;
void  ShowComment( char *format, ...) ;
void  ShowLinesCount( void ) ;
void  EncodeToBASE64(HWND hEdit) ;
void  Enveloping( HWND hEdit)  ;
void  DecodeFromBASE64(HWND hEdit) ;
DWORD ExtractSenObject( HWND hEdit , char *sNode) ;
DWORD TruncSignature(HWND hEdit) ;
BOOL  LoadFile(HWND hEdit, LPSTR pszFileName) ;
BOOL  DecodeFile(HWND hEdit, LPSTR pszFileName) ;
BOOL  DoFileOpenSave(HWND hwnd, BOOL bDecode) ; 
HWND  CreateStatusBar( HWND hWndParent ) ;
HWND  CreateToolBar( HWND hWndParent ) ;
void  FixToolBar( HWND hWnd ) ;
void  DoReplace(HWND h) ;
void  LoadNotifyTips() ;
void  Replacing() ;
void  SysMenuAppend( HWND hWnd ) ;
void  NormalizeFileName( char *xFile , int day );
LRESULT WndProc_OnNotify(HWND hWnd, int idFrom, NMHDR* pnmhdr) ;
DWORD DeleteED101( HWND hEdit) ;
void ChangeNumb( char *p);
char *get_rules_mask_item(int uItem);
char *get_rules_action_item(int uItem);
int   get_rules_count( void ) ;
void set_rules_action(void) ;
void one_line_xml_build ( char *pszText );

struct g_flags {
	int value;
	char *description ;
	}  ;
