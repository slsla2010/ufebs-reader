#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <regex.h>

#include "Main.h"

extern        HINSTANCE g_hInst;
extern        HWND      g_hStatusBar , g_hToolBar , hRich ;
extern        xmlDocPtr doc ; 
char          szFileName[MAX_PATH];
extern char   tempfln[32] ;
extern char   findInText[128] ;
extern char   replInText[128] ;
extern int    godMode ;
extern struct g_flags _gFlags[SETTINGS_COUNT] ;
extern HWND   g_hWnd  ;
extern char   loadedFile[128] ;
extern LPSTR  GlobalBuf ;

TBBUTTON     tbb[11];
struct { // ToolBar init
	int idIcon;
	int idCmd; 
	char *str; 
	} pToolTipDesc[16] = {
{ID_ICON_LOAD	,CM_FILE_OPEN	 ,"Load UFEBS"         },
{ID_ICON_VISUAL	,CM_FILE_VISUAL	 ,"Unpack&view UFEBS"  },
{ID_ICON_CHDAT  ,CM_CHANGE_DATEF ,"Set new date,pack"  },
{ID_ICON_SAVE	,CM_FILE_SAVE	 ,"Save"               },{0,0,NULL},
{ID_ICON_FIND	,CM_FILE_FIND	 ,"Search&replace"     },{0,0,NULL},
{ID_ICON_PRETTY	,CM_FILE_PRETTY	 ,"Pretty XML"         },{0,0,NULL},
{ID_ICON_IDKFA	,CM_CUT_ED101	 ,"Split PacketEPD & construct new for rules"},
{ID_ICON_PERO	,CM_CHANGE_ANALIZE,"Analyzer"},
{ID_ICON_B64E	,CM_FILE_ENCODE   ,"Encode to B64"},
{ID_ICON_B64D	,CM_FILE_DECODE   ,"Decode from B64"},
{ID_ICON_SETT	,CM_SETTINGS      ,"Settings"},
{0,0,"ZERO"} } ;

void  rec(xmlNodePtr node,HWND hEdit, char *sNode) ;

//void  NormalizeFileName( char *xFile , char *newDate ) {  *(xFile+28 ) = *(newDate+8); *(xFile+29 ) = *(newDate+9); }	
void  NormalizeFileName( char *xFile, int day ) 
{ 
char buf[4];
int l;
char *p ;

ShowComment("Нормализация имени файла");
//*(xFile+53 ) = *(newDate+8); *(xFile+54 ) = *(newDate+9); 
sprintf(buf,"%02d",day);
l = strlen( xFile );
p = xFile + l - 10 ;
*p     = buf[0];
*(p+1) = buf[1];

ShowComment("Нормализация имени файла:%s",xFile);
}	

HWND  CreateToolBar( HWND hWndParent )
{
         TBADDBITMAP tbab;
	     int i ;
         g_hToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,  WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER | CCS_ADJUSTABLE  , 0, 0, 0, 0, hWndParent, (HMENU)ID_TOOLBAR, g_hInst, NULL);
         SendMessage(g_hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	     SendMessage(g_hToolBar, TB_SETBITMAPSIZE, (WPARAM)0,(LPARAM)MAKELONG(50,48));
         tbab.hInst = (HINSTANCE)GetWindowLong(hWndParent,GWL_HINSTANCE);
         tbab.nID = IDB_TOOLBITMAP ;
         SendMessage(g_hToolBar, TB_ADDBITMAP, (WPARAM)11, (LPARAM)&tbab);
         ZeroMemory(tbb, sizeof(tbb));
	     for(i=0;i<14;i++) { tbb[i].fsState   = TBSTATE_ENABLED; 
			     tbb[i].fsStyle   = TBSTYLE_FLAT|TBSTYLE_ALTDRAG|BTNS_SHOWTEXT;
			     tbb[i].idCommand = pToolTipDesc[i].idCmd;
			     tbb[i].iBitmap   = pToolTipDesc[i].idIcon;
			     tbb[i].iString   = (char *)pToolTipDesc[i].str;
			  }

         tbb[4].fsStyle = TBSTYLE_SEP;
         tbb[6].fsStyle = TBSTYLE_SEP;
         tbb[8].fsStyle = TBSTYLE_SEP;

         SendMessage(g_hToolBar, TB_ADDBUTTONS, 14, (LPARAM)&tbb);
         SendMessage(g_hToolBar, TB_SETMAXTEXTROWS,0 ,0);
	     ShowWindow( g_hToolBar,SW_SHOW);
}

void FixToolBar( HWND hWnd )
{
	 int ptWidth[3]; 
         RECT rectClient, rectStatus, rectTool;     
         UINT uToolHeight, uStatusHeight, uClientAreaHeight;
         SendMessage(g_hToolBar, TB_AUTOSIZE, 0, 0);
         SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
         GetClientRect(hWnd, &rectClient);
         GetWindowRect(g_hStatusBar, &rectStatus);
  // Рассчитываем размеры областей Statusbar
  ptWidth[0] = 62;
  ptWidth[1] = 3*((rectStatus.right-rectStatus.left)/4);
  ptWidth[2] = -1;
  
  // Устанавливаем новые размеры Statusbar
  SendMessage(g_hStatusBar, SB_SETPARTS, 3, (LPARAM)ptWidth);

         GetWindowRect(g_hToolBar, &rectTool);
         uToolHeight = rectTool.bottom - rectTool.top;
         uStatusHeight = rectStatus.bottom - rectStatus.top;
         uClientAreaHeight = rectClient.bottom;
         MoveWindow( GetDlgItem(hWnd, IDC_MAIN_TEXT), 0, uToolHeight, rectClient.right, uClientAreaHeight - uStatusHeight - uToolHeight, TRUE);
}


void LoadNotifyTips()
{
int i;
for(i=0;i<14 ;i++) {
	if(pToolTipDesc[i].idCmd) {
		pToolTipDesc[i].str = malloc(64) ;
		LoadString(g_hInst,pToolTipDesc[i].idCmd,pToolTipDesc[i].str,64);
		}
	}
}


LRESULT WndProc_OnNotify(HWND hWnd, int idFrom, NMHDR* pnmhdr)
{
  LPTOOLTIPTEXT lpToolTipText;
  int i;
  if( pnmhdr->code == TTN_GETDISPINFO )
	{
    lpToolTipText = (LPTOOLTIPTEXT)pnmhdr;
	lpToolTipText->hinst = g_hInst;
	for(i=0;i<13 && pToolTipDesc[i].idCmd != lpToolTipText->hdr.idFrom;i++) ;
	if(pToolTipDesc[i].idCmd != CM_CHANGE_ANALIZE &&  pToolTipDesc[i].idCmd != CM_CUT_ED101 ) // ShowComment затирал результат выполнения этих операций
		ShowComment((char *)pToolTipDesc[i].str);
	}
  return FALSE;
}

void  ShowComment( char *format, ...)
{
	char buf[128];
	va_list args;
	va_start( args , format );
	vsprintf( buf, format , args );
	SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	strcat(buf,"\n");
	logging( buf );
}

void ShowLinesCount( void )
{
 int i;
 char bb[12];
 i=SendMessage(hRich, EM_GETLINECOUNT,0,0);
 sprintf(bb,"L:%d",i);
 SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)bb);
}

char *iconv_xml_get_content( xmlNodePtr node )
{
 static char outb[512];
 memset(outb,0,512);
 utf8cp1251( outb ,XML_GET_CONTENT( node ));    
 return( &outb[0] );
}

void Enveloping( HWND hEdit)
{
  DWORD dwTextLength;
  LPSTR pszText, envelopedText;
  ShowComment("Заворачиваем в конвертик..");
  dwTextLength = GetWindowTextLength(hEdit) ; 

  pszText       = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
  envelopedText = (LPSTR)GlobalAlloc(GPTR, dwTextLength + 512);
  strcpy(envelopedText,"<?xml version=\"1.0\" encoding=\"windows-1251\"?><sen:SigEnvelope xmlns:sen=\"urn:cbr-ru:dsig:env:v1.1\"><sen:SigContainer><dsig:MACValue xmlns:dsig=\"urn:cbr-ru:dsig:v1.1\">bzAwMDAwMCuMJakwM8kgQoSrIylscI4i2DSxZFpjwBqbr8IIzXXcL0TAiSNfIP5HHt6ZKmsSsbHAhqDe9spYcIUzAhWESNE2MDg0TVZOQllIMDH+L2FTnZaPTUVNAd8qAAA=</dsig:MACValue></sen:SigContainer><sen:Object>" );
  strcat(envelopedText,pszText);
  strcat(envelopedText,"</sen:Object></sen:SigEnvelope>");
  SetWindowText(hEdit, envelopedText );
  GlobalFree(envelopedText);
  ShowComment("законвертировали");
  ShowLinesCount(); 
}

void EncodeToBASE64(HWND hEdit)
{
 DWORD dwTextLength;
 LPSTR pszText, encodedText;
 ShowComment("Кодируем в BASE64");
 dwTextLength = GetWindowTextLength(hEdit) ; 
 // делаю запас для дальнешего использования в tr 
 pszText     = (LPSTR)GlobalAlloc(GPTR, dwTextLength/3 * 5 + 4); 
 encodedText = (LPSTR)GlobalAlloc(GPTR, dwTextLength/3 * 5 + 4);

 GetWindowText(hEdit, pszText, dwTextLength + 1);

 if ( gF_compressSpaces )
	{
	trC( pszText , encodedText , 0xA , "" ) ;
	trC( encodedText , pszText , 0xD , "" ) ;
	trC( pszText , encodedText , 0x9 , "" ) ;
	trS( encodedText, pszText  , "   "," ") ;
	trS( encodedText, pszText  , "   "," ") ;
	trS( encodedText, pszText  , "  " ," ") ;
	trS( encodedText, pszText  , "> " ,">") ;
 }

 b64encode( pszText , encodedText , strlen(pszText)+2 , 32000 );

 SetWindowText(hEdit, encodedText );
 GlobalFree(pszText);
 GlobalFree(encodedText);
 ShowComment("Закодировали в BASE64");
 ShowLinesCount(); 
}

void DecodeFromBASE64(HWND hEdit)
{
DWORD dwTextLength;
LPSTR pszText, decodedText;
ShowComment("Декодируем из BASE64");
dwTextLength = GetWindowTextLength(hEdit) ; 
pszText     = (LPSTR)GlobalAlloc(GPTR, dwTextLength/3 * 5 + 32); 
// делаю запас для дальнешего использования в tr 
decodedText = (LPSTR)GlobalAlloc(GPTR, dwTextLength/3 * 5 + 64);

GetWindowText(hEdit, pszText, dwTextLength + 1);
decode( pszText , decodedText , dwTextLength + 1 ) ;

tr( decodedText , pszText , '>' , ">" ) ;
one_line_xml_build ( pszText );

SetWindowText(hEdit, pszText );

GlobalFree(pszText);
GlobalFree(decodedText);
ShowComment("Декодировали!");
}

DWORD ExtractSenObject( HWND hEdit , char *sNode)
{
	LPSTR pszText;
	DWORD dwWritten , dwTextLength , l ;
	xmlNodePtr root ;
	ShowComment("Выделяем тэг  %s",sNode);
	dwTextLength = GetWindowTextLength(hEdit);
	pszText = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
        doc     = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
        root    = xmlDocGetRootElement(doc);

#ifdef MULTI_DOC
	 GlobalBuf = (LPSTR)GlobalAlloc(GPTR, dwTextLength * 2 );	 ZeroMemory( GlobalBuf, dwTextLength * 2 ); 
	 bprintf("PT09PT09PT09PT09PQA=" );
#endif
        rec(root,hEdit,sNode);
#ifdef MULTI_DOC
        SetWindowText( hEdit , GlobalBuf );
    	GlobalFree(GlobalBuf);
#endif
	xmlFreeDoc(doc);
	ShowComment("Выделили тэг!");
	ShowLinesCount(); 
	return 0;
}

DWORD TruncSignature(HWND hEdit) 
{
DWORD dwWritten , dwTextLength , l ;
LPSTR pszText;

ShowComment("Обрезаем ЭЦП ..");
dwTextLength = GetWindowTextLength(hEdit);
pszText      = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
if( pszText[dwTextLength-1] == 'S' && pszText[dwTextLength-2] == 'C' &&
    pszText[dwTextLength-3] == 'I' && pszText[dwTextLength-4] == 'B')
	{
	DWORD i ;
	i = dwTextLength-2 ;
	while( i-- ) { if( pszText[i] == '>' && pszText[i-1] == 'e' ) break ; }
	l = i+1;
	while( i++ < dwTextLength ) pszText[i] = 0 ; 
//	SetWindowText(hEdit, pszText );
	}
ShowComment("ЭЦП отрезана! Для обновления экрана PgUp/PgDn");
ShowLinesCount();
return l ;
}

BOOL LoadFile(HWND hEdit, LPSTR pszFileName) 
{	HANDLE hFile;
	BOOL bSuccess = FALSE;

	ShowComment("Загружаем файл %s",pszFileName);
	
	hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(hFile != INVALID_HANDLE_VALUE) {
		DWORD dwFileSize;
		dwFileSize = GetFileSize(hFile, NULL);
		ShowComment("Size %d",dwFileSize);
	
		if(dwFileSize != 0xFFFFFFFF) {
			LPSTR pszFileText;
			pszFileText = (LPSTR)GlobalAlloc(GPTR, dwFileSize + 1);
			if(pszFileText != NULL) {
				DWORD dwRead;
				if(ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
					pszFileText[dwFileSize] = 0; // Null terminator
					strcpy(loadedFile,pszFileName);
					if(SetWindowText(hEdit, pszFileText))
						bSuccess = TRUE; // It worked!
				}
				GlobalFree(pszFileText);
			}
		}
		CloseHandle(hFile);
	}
	ShowComment("Загружен файл:%s",pszFileName);
	SetWindowText(g_hWnd,loadedFile);
	ShowLinesCount(); 
	return bSuccess;
}

BOOL WritingFile(HWND hEdit, LPSTR pszFileName) 
{	HANDLE hFile;
	LPSTR  pszText;
	DWORD  dwTextLength;
	BOOL   bSuccess = FALSE;

	ShowComment("Записываем в файл ..");
	hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hFile != INVALID_HANDLE_VALUE) {
		dwTextLength = GetWindowTextLength(hEdit);
		if(dwTextLength > 0) {
			pszText  = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
			if(WriteFile(hFile, pszText , dwTextLength , &dwTextLength , NULL))
				bSuccess = TRUE;
		}
		CloseHandle(hFile);
	}
	ShowComment("Файл сохранен!");
	ShowLinesCount(); 
	return bSuccess;
}


BOOL DoFileOpenSave(HWND hwnd, BOOL bWriteOrLoad) 
{	OPENFILENAME ofn;
	
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "ED Files (*.ED)\0*.ED\0All Files (*.*)\0*.*\0\0";

	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "ED";

	if(bWriteOrLoad) {
		ofn.Flags = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
//		if( szFileName[0] == 0 )
			strcpy(szFileName,"EMPTY_FILE");
		if(GetSaveFileName(&ofn)) {
			if(!WritingFile(hRich, szFileName)) {
				MessageBox(hwnd, "Save file failed.", "Error",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}
	} else {szFileName[0] = 0; 
		ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
		if(GetOpenFileName(&ofn)) {
		
			if(!LoadFile(hRich, szFileName)) {
				MessageBox(hwnd, "Load of file failed.", "Error",MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			SetWindowText(hwnd,loadedFile);
		}
	}
	return TRUE;
}

HWND CreateStatusBar( HWND hWndParent )
{
         int iStatusWidths[] = {76, 444, -1};
         g_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWndParent, (HMENU)ID_STATUSBAR, g_hInst, NULL);
         SendMessage(g_hStatusBar, SB_SETBKCOLOR, 0,RGB(192,192,192));
         SendMessage(g_hStatusBar, SB_SETPARTS, 3, (LPARAM)iStatusWidths);
         SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)"UFEBS reader");\
}

void DoReplace(HWND h)
{
DWORD dwTextLength , l ;
LPSTR pszText,newText;
LPSTR lst ;
HANDLE clip;

ShowComment("Запуск операции контекстной замены..");
SendDlgItemMessage(h, IDC_MAIN_TEXT, WM_CUT, 0, 0);
OpenClipboard(NULL);

clip = GetClipboardData(CF_TEXT) ;
lst  = (LPSTR)GlobalLock(clip);					

if( parseSEDcmd( lst , findInText , replInText ) )
	MessageBox(NULL, "Не распознана команда замены\nВведите:\n\ts/текст/новыйтекст/\nВыделите этот блок и повторите команду\n" , "REPLACE TEXT CMD:",0);
else    Replacing();

GlobalUnlock(clip);
CloseClipboard();
ShowComment("Операция контекстной замены закончена!");
}

/* Append "About" menu item to system menu. */
void SysMenuAppend( HWND hWnd )
{
  HMENU hmenuSystem ;
  char tempBuf0[128];

  hmenuSystem = GetSystemMenu(hWnd, FALSE);
  AppendMenu(hmenuSystem, MF_SEPARATOR, 0, NULL);
  LoadString(g_hInst, IDS_PROG_DESCR , tempBuf0, 64 );
  AppendMenu(hmenuSystem, MF_STRING, 887 , tempBuf0 );
  LoadString(g_hInst, ID_TIME_COMPILE, tempBuf0, 48 );
  AppendMenu(hmenuSystem, MF_STRING|MF_CHECKED|MF_GRAYED, 887 , tempBuf0 );
  AppendMenu(hmenuSystem, MF_SEPARATOR, 0, NULL);
  SendMessage(g_hStatusBar, SB_SETTEXT, 2, (LPARAM)tempBuf0);
  AppendMenu(hmenuSystem, MF_STRING|MF_CHECKED, 888 , "Режим полубога" );
  CheckMenuItem(hmenuSystem,888, godMode?MF_CHECKED:MF_UNCHECKED);
}

