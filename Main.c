#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <commctrl.h>
#include <richedit.h>
#define TEST_OFF
#include "Main.h"
#include "stdint.h"
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <regex.h>

#pragma comment(linker,:/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccfldf' language='*'\"")

char  tmpBuf[4096] ;
char  tempBuf[4096] ;
char  tempfln[32] = "!bout.txt" ;
int   searchPosition = 0 ;
// глобальные флаги для настроек
int             gF_checkXML[8]  ;
struct g_flags _gFlags[SETTINGS_COUNT] = {
	{ 0,"логирование %s"},
	{ 1,"поменять AccDocNo %s"},
	{ 1,"поменять EDNo %s"},
	{ 0,"поменять Sum %s"},
	{ 0,"проверка ED501 %s" } ,
	{ 1,"при кодировании в BASE64 сжать пробелы %s"},
	{ 1,"сохранить с правильным именем %s"},
	{ 0,"пересчет EDQuantity %s"},
	{ 0,"полное меню %s"},
	{ 1,"предформат XML %s"}
	};
	
int   godMode = 0 ;
int   node_count;
int   g_EDType ;

char  findInText[128] ;
char  xpathInText[128] ;
char  replInText[128] ;
char  loadedFile[128] ; // последний загруженный файл

LPSTR g_Cmd ;
LPSTR GlobalBuf ;
RECT  gRect ;
xmlDocPtr doc = NULL;
HWND g_hToolBar , g_hStatusBar ,  g_hWnd , hRich ;
HINSTANCE g_hInst;
#define CAT		strcat(GlobalBuf,tempBuf);
#ifdef WALK_LEVEL
static int level = -1 ;
#define  INDENT      int i;for(i=0;i<level;i++)strcat(GlobalBuf,"  ");
#define  INDENT_INC  level++;
#define  INDENT_DEC  level--;
#else
#define INDENT ;
#define INDENT_INC  ;
#define INDENT_DEC  ;
#endif

#define SET_CHECK(X,S)  X=X?0:1;ShowComment(S, X ? "ON" : "OFF");CheckMenuItem(hSettingsMenu,LOWORD(wParam),(X)?MF_CHECKED:MF_UNCHECKED);
#define SET_CHECK_XML(X,S)  X=X?0:1;ShowComment(S,get_rules_mask_item((LOWORD(wParam))-(SET_XML40817+55)), X ? "ON" : "OFF");CheckMenuItem(hSettingsMenu,LOWORD(wParam),(X)?MF_CHECKED:MF_UNCHECKED);

char *iconv_xml_get_content( xmlNodePtr node ) ;
char *reformatSum( char *str ) { return ( str ); }

void putAttr( xmlAttr *prop )
{ // прогон по аттрибутам(пропертям) прописанным внутри XML тэга
 xmlAttr *Attr;
 if ( prop == NULL ) return;
 Attr = prop ;
 INDENT_INC  ;
 do {
	 INDENT ;
	 bprintf("\t%s=\"%s\"%c%c",Attr->name , XML_GET_CONTENT(Attr->children),0xd,0xa) ;
	 Attr = Attr->next ;
	}  while( Attr );
  bprintf("%c%c",0xd,0xa);
  INDENT_DEC  ;
}
int CheckED501(  HWND hEdit )
{
  DWORD    dwTextLength  ;
  LPSTR    pszText       ;
  int 	   retval;
  DWORD    dwFileSize;
  DWORD    dwRead;
  xmlDocPtr            docx;
  xmlXPathContextPtr   xpathCtx;
  xmlXPathObjectPtr    xpPack;
  ShowComment("Проверим ED501?");
  dwTextLength = GetWindowTextLength(hEdit);
  if ( dwTextLength < 40 ) { ShowComment("Нечего проверять");return 0; }
  pszText   = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));

  docx      = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
  xpathCtx  = xmlXPathNewContext(docx);
  xmlXPathRegisterNs(xpathCtx, "my", "urn:cbr-ru:ed:v2.0" ) ;
  retval=0;

  xpPack    =  xmlXPathEvalExpression("/my:ED501", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=501;

  xpPack    =  xmlXPathEvalExpression("/my:ED503", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=503;

  xpPack    =  xmlXPathEvalExpression("/my:PacketEPD", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=101;

  xpPack    =  xmlXPathEvalExpression("/my:ED205", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=205;

  xpPack    =  xmlXPathEvalExpression("/my:PacketESID", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=206;

  xpPack    =  xmlXPathEvalExpression("/my:PacketEID", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=208;

  xpPack    =  xmlXPathEvalExpression("/my:ED219", xpathCtx);
  if ( xpPack && xpPack->nodesetval->nodeNr ) retval=219;
  g_EDType = retval ;
  ShowComment("ED%03d",retval);

  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(docx);
  return (retval);
}

void Analyzer(  HWND hEdit , char *ed101xpath )
{ //  ed101xpath = "/my:PacketEPD/my:ED101/my:Payee/@PersonalAcc"
  DWORD                dwTextLength  ;
  LPSTR                pszText,pszText1       ;
  int 		i,j,k;
  char PersonalAcc[32] ;

  HANDLE hFile;
  DWORD dwFileSize;DWORD dwRead;
  xmlDocPtr            docx;
  xmlXPathContextPtr   xpathCtx;
  xmlXPathObjectPtr    xpPack,xpathObjEDs;
  xmlNodePtr curEDs;
  ShowComment("Анализируем документы");
  dwTextLength = GetWindowTextLength(hEdit);
  if ( dwTextLength < 40 ) {ShowComment("Буфер так мал :(  нечего анализировать");return;}
  pszText   = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
  docx = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(docx);
    /* Register namespaces from list (if any) */
    xmlXPathRegisterNs(xpathCtx, "my", "urn:cbr-ru:ed:v2.0" ) ;
    xpPack =  xmlXPathEvalExpression("/my:PacketEPD", xpathCtx);
    if( xpPack == NULL || xpPack->nodesetval->nodeNr == 0 )
	 { ShowComment("Это же не PacketEPD!");return; }
//    xpathObjEDs=  xmlXPathEvalExpression("/my:PacketEPD/my:ED101/my:Payee/@PersonalAcc"	, xpathCtx);
    xpathObjEDs=  xmlXPathEvalExpression(ed101xpath , xpathCtx);
    if( xpathObjEDs == NULL ) { ShowComment("По выражению XPATH документов ED101 нет");return; }
    j = xpathObjEDs->nodesetval->nodeNr ;
    if( j )
	{
	  k = 0 ;
	  set_rules_action();
	  compileRegMask();
	  for(i=0;i<j;i++)
		{
	         curEDs = xpathObjEDs->nodesetval->nodeTab[i];
	         //if(curEDs->type != XML_NAMESPACE_DECL && curEDs->type != XML_ELEMENT_NODE) {
			 if(curEDs->type != XML_NAMESPACE_DECL ) {
                 //if( curEDs->type==2)
             		{
					if ( checkED101_KZ( curEDs ) == _SKIP_ED101_ ) k++;
			}
	   }
	}
	 ShowComment("В пакете %d документов, на КЗ УПД предположительно %d",j,k );

    }
   xmlXPathFreeContext(xpathCtx);
   xmlFreeDoc(docx);
}

int XPATHLooker(  HWND hEdit , char *ed101xpath )
{ //  ed101xpath = "/my:PacketEPD/my:ED101/my:Payee/@PersonalAcc"
  DWORD                dwTextLength  ;
  LPSTR                pszText       ;
  int 		     j;
  DWORD dwFileSize;DWORD dwRead;
  xmlDocPtr            docx;
  xmlXPathContextPtr   xpathCtx;
  xmlXPathObjectPtr    xpPack,xpathObjEDs;
  ShowComment("Поиск XPATH выражения"); dwTextLength = GetWindowTextLength(hEdit);
  if ( dwTextLength < 40 ) return 0;
  pszText   = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
  docx = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
  xpathCtx = xmlXPathNewContext(docx);
  xmlXPathRegisterNs(xpathCtx, "my", "urn:cbr-ru:ed:v2.0" ) ;
  xpPack =  xmlXPathEvalExpression("/my:PacketEPD", xpathCtx);
  if( xpPack == NULL || xpPack->nodesetval->nodeNr == 0 )  { ShowComment("Это не PacketEPD!");return 0; }
  xpathObjEDs =  xmlXPathEvalExpression(ed101xpath , xpathCtx);
  ShowComment("Поиск закончен");
  if( xpathObjEDs == NULL ) { ShowComment("По выражению XPATH документов ED101 нет");return 0; }
  j = xpathObjEDs->nodesetval->nodeNr ;
  ShowComment("выражению XPATH соответствуют %d документов",j );
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(docx);
  return( j );
}

void RecalculateED(  HWND hEdit )
{
  DWORD                dwTextLength  ;
  LPSTR                pszText,pszText1       ;
  int 		i,j;
  unsigned long lSum;
  char sumStr[32] ;
  FILE *f;
  HANDLE hFile;
  DWORD dwFileSize;DWORD dwRead;
  xmlDocPtr            docx;
  xmlXPathContextPtr   xpathCtx;
  xmlXPathObjectPtr    xpathObjQ,xpathObjS,xpPack,xpathObjEDs;
  xmlNodePtr curS,curQ,curEDs;

    dwTextLength = GetWindowTextLength(hEdit);
  if( dwTextLength < 10 ) { ShowComment("Буфер так мал :(  Даже нечего пересчитывать");return ;}
  ShowComment("Запрос EDQuantity и суммы документов");
  pszText   = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
  docx = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(docx);
    /* Register namespaces from list (if any) */
    xmlXPathRegisterNs(xpathCtx, "my", "urn:cbr-ru:ed:v2.0" ) ;
    xpPack =  xmlXPathEvalExpression("/my:PacketEPD", xpathCtx);
    if( xpPack == NULL || xpPack->nodesetval->nodeNr == 0 )
	 { ShowComment("Это не PacketEPD!");return; }

    xpathObjS =  xmlXPathEvalExpression("/my:PacketEPD/@Sum"	   , xpathCtx);
    xpathObjQ =  xmlXPathEvalExpression("/my:PacketEPD/@EDQuantity", xpathCtx);

    if( xpathObjS->nodesetval->nodeNr )
    {
         if(xpathObjS->nodesetval->nodeTab[0]->type != XML_NAMESPACE_DECL
 	 && xpathObjS->nodesetval->nodeTab[0]->type != XML_ELEMENT_NODE) {
	    curS = xpathObjS->nodesetval->nodeTab[0];
	    if( curS->type==2) ShowComment("Сумма всех документов:%s", reformatSum(XML_GET_CONTENT( curS->children )) );
	   }
    }
    if( xpathObjQ->nodesetval->nodeNr )
    {
         if(xpathObjQ->nodesetval->nodeTab[0]->type != XML_NAMESPACE_DECL
 	 && xpathObjQ->nodesetval->nodeTab[0]->type != XML_ELEMENT_NODE) {
	    curQ = xpathObjQ->nodesetval->nodeTab[0];
	    if( curQ->type==2) ShowComment("Количество документов:%s", XML_GET_CONTENT( curQ->children ) );
	   }
    }
  ShowComment("EDQuantity:%s  Sum:%s", XML_GET_CONTENT( curQ->children ), reformatSum(XML_GET_CONTENT( curS->children )));
// TODO запрос всех ED101 и сумм документов
//      /my:PacketEPD/my:ED101/@Sum
//      если не совпадает, то апдейт EDQuantity и Sum
    xpathObjEDs=  xmlXPathEvalExpression("/my:PacketEPD/my:ED101/@Sum"	, xpathCtx);
    if( xpathObjEDs == NULL ) { ShowComment("Не нашел ED101 !");return; }
    j = xpathObjEDs->nodesetval->nodeNr ;
    if( j )
	{
	  lSum = 0L ;
	  for(i=0;i<j;i++)
		{
	    curEDs = xpathObjEDs->nodesetval->nodeTab[i];
	         if(curEDs->type != XML_NAMESPACE_DECL
 		 && curEDs->type != XML_ELEMENT_NODE) {
		    if( curEDs->type==2)
			{
			lSum += atol(XML_GET_CONTENT( curEDs->children ) );
			}
	   }
	}
	ShowComment("Пересчитанная сумма %d документов: %s",i,reformatSumInt(lSum,sumStr) );
// update EDQuantity
	xmlNodeSetContent( curQ , itoa(i,sumStr,10) );
	xmlNodeSetContent( curS , ltoa(lSum,sumStr,10) );
	f = fopen( tempfln,"w");xmlDocDump( f , docx );fclose(f);
	hFile = CreateFile( tempfln, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	dwFileSize = GetFileSize(hFile, NULL);
	ReadFile(hFile, pszText, dwFileSize, &dwRead, NULL);
	CloseHandle(hFile);
	DeleteFile( tempfln);
	pszText[dwFileSize] = 0;
	SetWindowText(hEdit, pszText);
    }
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(docx);
}

void rec_walk(xmlNodePtr node)
{
   char *value;
   xmlNodePtr cur_node;
   cur_node = node;
   INDENT_INC ;
   while (cur_node ) {
    if ( !(++node_count % 50) ) ShowComment("Обработано %-6d XML узлов",node_count);
    if ( cur_node->type == 1)
	{
	INDENT ;
	bprintf("<%s>%c%c",cur_node->name,0xd,0xa);
	putAttr( cur_node->properties );
	}
    rec_walk(cur_node->children);
//    if( cur_node->type==1){INDENT;sprintf(tempBuf,"</%s>%c%c",cur_node->name,0xd,0xa); CAT ; }
    if( cur_node->type==1){INDENT;bprintf("</%s>%c%c",cur_node->name,0xd,0xa);  }
    if( cur_node->type==3 && XML_GET_CONTENT(cur_node) )
		{ utf8cp1251(XML_GET_CONTENT( cur_node) , tmpBuf); INDENT ;
		  bprintf("   %s%c%c",tmpBuf,0xd,0xa );
		}
    cur_node = cur_node->next;
   }
   INDENT_DEC ;
   bprintf("%c%c",0xd,0xa);
 return ;
}

void WalkerXML( HWND hEdit )
{
DWORD dwTextLength  ;
LPSTR pszText , pszWalkText ;
xmlNodePtr root ;

	 ShowComment("Форматирование XML ..");
	 dwTextLength = GetWindowTextLength(hEdit);
	 pszText   = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
	 GlobalBuf = (LPSTR)GlobalAlloc(GPTR, dwTextLength * 2 );
	 ZeroMemory( GlobalBuf, dwTextLength * 2 );
	 doc = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
	 root = xmlDocGetRootElement(doc);
	 node_count=0;
	 rec_walk(root) ;
	 xmlFreeDoc(doc);
	 SetWindowText(hEdit, GlobalBuf );
	 GlobalFree(GlobalBuf);
	 ShowComment("Форматирование XML законченo!");
	 ShowLinesCount();
}

void putNode(xmlNodePtr node,HWND hEdit)
{
    const xmlChar *value;
    value = XML_GET_CONTENT( node->children );
#ifdef MULTI_DOC
    bprintf(value);
    bprintf("PT09PT09PT09PT09PQA=" );
#else
    SetWindowText(hEdit, value );
#endif
// срабатывает для последнего найденного тэга
// надо переделать для наращивания буфера чтобы все найденные показались в ричедите
// актуально для ED503 в котором много SWIFTDocument-ов
// между блоками вставить "PT09PT09PT09PT09PQA=" - это разделитель ===========
}

void rec(xmlNodePtr node,HWND hEdit , char *sNode )
{ // sNode = ProprietoryDocument | Object  | SWIFTDocument
   xmlNodePtr cur_node = node;
   while (cur_node)
   {
    if (cur_node->type == XML_ELEMENT_NODE) //проверяем что попали на узел
       {
	if( !strcmp(cur_node->name, sNode ))  //"Object") )
		putNode(cur_node,hEdit);
       }
    rec(cur_node->children,hEdit, sNode ); // sNode: Object или ProprietoryDocument или SWIFTDocument
    cur_node = cur_node->next;
// между блоками вставить "PT09PT09PT09PT09PQA=" - это разделитель ===========
    }
}

void Replacing()
{
regex_t r;
regmatch_t pmatch[4];
int i , count ;
char buf[64] ;
DWORD dwTextLength ;
LPSTR pszText,newText, tempPtr ,tempOutPtr ;
HCURSOR  ghcurSave;  // previous cursor

	ShowComment("Контекстная замена текста..");
	count=0;
        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
	dwTextLength = GetWindowTextLength(hRich);
	pszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength * 2 );
	newText = pszText ;
//	newText = (LPSTR)GlobalAlloc(GPTR, dwTextLength * 2 );
	GetWindowText(hRich, pszText, dwTextLength + 1);
// теперь поехали по буферу регуляркой из findInText
//       ShowComment("Compile regex:%s\n",findInText);
	strcpy(newText,pszText);
	if( regcomp(&r, findInText , REG_EXTENDED) )
		ShowComment("Регулярное выражение некорректно!");
	else {
	while ( regexec(&r, pszText , 4,&pmatch[0],0) != REG_NOMATCH )
		{
			strncpy(buf,&pszText[pmatch[0].rm_so],pmatch[0].rm_eo-pmatch[0].rm_so);
			buf[pmatch[0].rm_eo-pmatch[0].rm_so]=0;
// это для избегания зацикливания - если строка поиска равна строке замены
			if ( !strcmp(buf,replInText ) ) break ;
			count += trS( pszText , newText , buf , replInText );
			strcpy(pszText,newText);
		}
	}
	regfree(&r);
	SetWindowText(hRich, newText );
        GlobalFree(pszText);
//	GlobalFree(newText);
        SetCursor(ghcurSave);
	ShowComment("Произведено %d замен",count);
}

void ChangerNumberDocs(char *xmldo )
{ // xmldo - реквизит который меняем. Может быть EDNo=  либо AccDocNo=
 regex_t r;
 regmatch_t pmatch[4];
 DWORD    dwTextLength ;
 LPSTR    pszText, pbuf ;
 HCURSOR  ghcurSave;  // previous cursor
  ShowComment("Изменяем номера документов в реквизитах:%s ...",xmldo);
  ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
  dwTextLength = GetWindowTextLength(hRich);
  pszText =(char *)LocalLock((HLOCAL)SendMessage( hRich, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
  if( regcomp(&r, xmldo , 0) )	SendMessage(g_hStatusBar, SB_SETTEXT,1,(LPARAM)"Ошибка компиляции регулярного выражения!" ) ;
  else {
// теперь поехали по буферу регуляркой
// можно сразу буфер EDIT править:	*(pszText+searchPosition)='#';
      pbuf = pszText ;
      while( regexec(&r, pbuf , 2, &pmatch[0],0) != REG_NOMATCH )
	{
	pbuf = pbuf + pmatch[0].rm_eo ;
	ChangeNumb( pbuf );
	pbuf+=10;
	}
	regfree(&r);
        SetCursor(ghcurSave);
	}
}

void Searching( )
{
regex_t r;
regmatch_t pmatch[4];
int  slen ;
DWORD dwTextLength ;
LPSTR pszText  ;
HCURSOR  ghcurSave;  // previous cursor
	ShowComment("Ищем текст:%s ...",findInText);
        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
	dwTextLength = GetWindowTextLength(hRich);
//	pszText = (LPSTR)GlobalAlloc(GPTR, dwTextLength * 2 );
//	GetWindowText(hRich, pszText, dwTextLength + 1);
	pszText =(char *)LocalLock((HLOCAL)SendMessage( hRich, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
	if( regcomp(&r, findInText , REG_EXTENDED) )
		SendMessage(g_hStatusBar, SB_SETTEXT,1,(LPARAM)"Ошибка компилЯции регулЯрного выражениЯ!" ) ;
	else {
// теперь поехали по буферу регулЯркой
	if ( regexec(&r, (pszText+searchPosition) , 4,&pmatch[0],0) != REG_NOMATCH )
		{
			slen = pmatch[0].rm_eo - pmatch[0].rm_so ;
			ShowComment("Found!");
			SendMessage( hRich, EM_SETSEL , searchPosition + pmatch[0].rm_so , searchPosition + pmatch[0].rm_so + slen );
			SendMessage( hRich, EM_SCROLLCARET , 0,0);
			searchPosition = searchPosition  +  pmatch[0].rm_so + slen ;
// можно сразу буфер EDIT править:	*(pszText+searchPosition)='#';
		}
	else ShowComment("Текст %s не найден!",findInText);
	}
	regfree(&r);
//        GlobalFree(pszText);
        SetCursor(ghcurSave);
}

BOOL WINAPI SEDDlgProc( HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
   char lst[256];
   switch (wMsg)
   {
   case WM_INITDIALOG:
	sprintf(lst,"s/%s/%s/",findInText,replInText);
	SetDlgItemText(hWnd, ID_FIND_TEXT, lst);
	break;
   case WM_COMMAND:
      switch (wParam)
      {
         case ID_REPL: // "Done"
	    GetDlgItemText(hWnd, ID_FIND_TEXT, findInText, sizeof(findInText));
            EndDialog(hWnd, TRUE);
	    if( !strlen( findInText ) )  ShowComment("Нечего искать, не на что заменЯть", "SED:",0);
	    else {  strcpy( lst, findInText );
		    if( parseSEDcmd( lst , findInText , replInText ) )
			MessageBox(NULL, "Не распознана команда замены\nВведите:\n\ts/текст/новыйтекст/\n" , "SED CMD:",0);
			else    Replacing();
		}
            break;
      }
      break;
   }
   return FALSE;
}

BOOL WINAPI XPATHDlgProc( HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
   char lst[256];
   switch (wMsg)
   {
   case WM_INITDIALOG:
//	sprintf(lst,"s/%s/%s/",findInText,replInText);
//	strcpy(lst,"/my:PacketEPD/my:ED101/my:Payee/@PersonalAcc");
	strcpy(lst,xpathInText);
	SetDlgItemText(hWnd, ID_XPATH_TEXT, lst);
	break;
   case WM_COMMAND:
      switch (wParam)
      {
         case ID_XPATH_GO: // "Done"
	    GetDlgItemText(hWnd, ID_XPATH_TEXT, xpathInText, sizeof(xpathInText));
            EndDialog(hWnd, TRUE);
	    if( !strlen( xpathInText ) )  ShowComment("Пустое выражение XPATH", "SED:",0);
	    else {  strcpy( lst, xpathInText );
		    XPATHLooker(hRich, xpathInText );

		}
            break;
      }
      break;
   }
   return FALSE;
}

/* BOOL WINAPI XmlConstructDlgProc( HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
   switch (wMsg)
   {
   case WM_INITDIALOG:

	break;
   case WM_CLOSE:
   case WM_DESTROY:
	EndDialog(hWnd,FALSE);
	break;
   case WM_COMMAND:
      switch (wParam)
      {
         case ID_FIND:
            EndDialog(hWnd, TRUE);
	}
	break;
  }
return FALSE;
 }  */

BOOL WINAPI SettingsDlgProc( HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
   switch (wMsg)
   {
   case WM_INITDIALOG:
	CheckDlgButton(hWnd, ID_SETT_DLG_LOG      , gF_logging        );
	CheckDlgButton(hWnd, ID_SETT_DLG_SUM      , gF_changeSum      );
	CheckDlgButton(hWnd, ID_SETT_DLG_EDNO     , gF_changeEDNo     );
	CheckDlgButton(hWnd, ID_SETT_DLG_ACCDOCNO , gF_changeAccDocNo );
	CheckDlgButton(hWnd, ID_SETT_DLG_COMPRESS_SPACE  , gF_compressSpaces);
	CheckDlgButton(hWnd, ID_SETT_DLG_FORMAT_XML   , gF_formatXML );
	break;
   case WM_CLOSE:
   case WM_DESTROY:
	EndDialog(hWnd,FALSE);
	break;
   case WM_COMMAND:
//      MoveWindow(hWnd, gRect.left+30,gRect.top+20,gRect.left+140,gRect.top+36,TRUE);
      switch (wParam)
      {
         case ID_FIND:
	    gF_changeEDNo     = ( IsDlgButtonChecked(hWnd, ID_SETT_DLG_EDNO) == BST_CHECKED ? 1:0) ;
	    gF_changeAccDocNo = ( IsDlgButtonChecked(hWnd, ID_SETT_DLG_ACCDOCNO) == BST_CHECKED ? 1:0) ;
	    gF_changeSum      = ( IsDlgButtonChecked(hWnd, ID_SETT_DLG_SUM) == BST_CHECKED ? 1:0) ;
	    gF_logging        = ( IsDlgButtonChecked(hWnd, ID_SETT_DLG_LOG) == BST_CHECKED ? 1:0) ;
	    gF_compressSpaces = ( IsDlgButtonChecked(hWnd, ID_SETT_DLG_COMPRESS_SPACE) == BST_CHECKED ? 1:0) ;
	    gF_formatXML      = ( IsDlgButtonChecked(hWnd, ID_SETT_DLG_FORMAT_XML) == BST_CHECKED ? 1:0) ;
            EndDialog(hWnd, TRUE);
            break ;
         case ID_REPL:
            EndDialog(hWnd, TRUE);
            break;
      }
      break;
   }
   return FALSE;
}

BOOL WINAPI FindDlgProc( HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
   switch (wMsg)
   {
   case WM_INITDIALOG:
        SetDlgItemText(hWnd, ID_FIND_TEXT, findInText );
	SetDlgItemText(hWnd, ID_REPL_TEXT, replInText );
	break;
   case WM_CLOSE:
   case WM_DESTROY:
	EndDialog(hWnd,FALSE);
	break;
   case WM_COMMAND:
//      MoveWindow(hWnd, gRect.left+30,gRect.top+20,gRect.left+140,gRect.top+36,TRUE);
      switch (wParam)
      {
         case ID_FIND:
            EndDialog(hWnd, TRUE);
	    GetDlgItemText(hWnd, ID_FIND_TEXT, findInText, sizeof(findInText));
	    searchPosition = 0;
	    if( strlen( findInText ) > 0 ) Searching();
            break ;
         case ID_REPL:
	    GetDlgItemText(hWnd, ID_FIND_TEXT, findInText, sizeof(findInText));
	    GetDlgItemText(hWnd, ID_REPL_TEXT, replInText, sizeof(replInText));
            EndDialog(hWnd, TRUE);
	    if( !strlen( findInText ) && !strlen(replInText) )  MessageBox(NULL, "Нечего искать, не на что заменЯть", "Find-Repl:",0);
	    else Replacing();
            break;
      }
      break;
   }
   return FALSE;
}

BOOL WINAPI DatDlgProc( HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
   SYSTEMTIME DT ;
   HWND hWndDT;
   switch (wMsg)
   {
//   case WM_INITDIALOG:
//        SetDlgItemText(hWnd, ID_FIND_TEXT, findInText );
//	SetDlgItemText(hWnd, ID_REPL_TEXT, replInText );
//	break;
   case WM_CLOSE:
   case WM_DESTROY:
	EndDialog(hWnd,FALSE);
	break;
   case WM_COMMAND:
	hWndDT = GetDlgItem( hWnd, IDC_DATEPICK);
//      MoveWindow(hWnd, gRect.left+30,gRect.top+20,gRect.left+140,gRect.top+36,TRUE);
//      MonthCal_GetCurSel( hWnd, lpDateTime );
      switch (wParam)
      {
         case ID_CANCEL:
	    SendMessage( hWndDT,MCM_GETCURSEL,(WPARAM)0,(LPARAM)&DT);
            EndDialog(hWnd, TRUE);
            ShowComment( "ИзменЯем дату платежей на %d-%02d-%02d",DT.wYear,DT.wMonth,DT.wDay);
	    strcpy(findInText,"201[4-8]-[01][0-9]-[0-3][0-9]");
	    sprintf(replInText,"%d-%02d-%02d",DT.wYear,DT.wMonth,DT.wDay);
	    NormalizeFileName( loadedFile, DT.wDay ) ;
	    Replacing();
	    return TRUE ;
      }
      break;
   }
   return FALSE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  char  szBuf[2048];
  HMENU hMenu,hFileMenu, hChangeMenu , hSettingsMenu , hSystemMenu ;
  HCURSOR  ghcurSave;  // previous cursor
  WNDCLASSEX 	wc;
  HWND hR ;
  int i ;
  hMenu         = GetMenu(hwnd);
  hFileMenu     = GetSubMenu(hMenu, 0);
  hChangeMenu   = GetSubMenu(hMenu, 1);
  hSettingsMenu = GetSubMenu(hMenu, 2);
 switch(Message) {
		case WM_CREATE:
			hRich = CreateWindow("EDIT", "" ,WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,hwnd, (HMENU)IDC_MAIN_TEXT, g_hInst , NULL);
			//hRich = CreateWindow("RICHEDIT", "" ,WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,hwnd, (HMENU)IDC_MAIN_TEXT, g_hInst , NULL);
		    SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_SETFONT,(WPARAM)GetStockObject(SYSTEM_FIXED_FONT), MAKELPARAM(TRUE, 0));
			LoadNotifyTips();
			CreateToolBar( hwnd );
			CreateStatusBar( hwnd );
			SysMenuAppend( hwnd );
			for(i=0;i<SETTINGS_COUNT;i++)CheckMenuItem(hSettingsMenu,SETTINGS_ID_BASE+i,_gFlags[i].value?MF_CHECKED:MF_UNCHECKED);				
			for(i=0;i<get_rules_count();i++)
				{ // забиваем в меню настроек строки из структуры rules
				AppendMenu(hSettingsMenu,MF_STRING|(get_rules_action_item(i)==_SKIP_ED101_?MF_CHECKED:MF_UNCHECKED),SET_XML40817+55+i,get_rules_mask_item(i));
				gF_checkXML[i] = (get_rules_action_item(i)==_SKIP_ED101_?1:0);
				}
			break;
		case WM_SIZE:
			if(wParam != SIZE_MINIMIZED) MoveWindow(hRich, 0, 0, LOWORD(lParam),HIWORD(lParam), TRUE);
			FixToolBar ( hwnd );
			//FixToolBar ( hRich );
			break;
		case WM_SETFOCUS: SetFocus( hRich );break;
		case WM_SYSCOMMAND:
		      switch (LOWORD(wParam))
		        {
			case 887:SendMessage( hwnd,WM_COMMAND, CM_ABOUT,  0L ); break;
			case 888:SendMessage( hwnd,WM_COMMAND, CM_IDKFA,  0L );
	  			   hSystemMenu = GetSystemMenu(hwnd, FALSE);
				   CheckMenuItem(hSystemMenu,888,godMode?MF_CHECKED:MF_UNCHECKED);
				   //CheckMenuItem(hSettingsMenu,CM_IDKFA,godMode?MF_CHECKED:MF_UNCHECKED);
				   break;
			  default:DefWindowProc(hwnd, Message, wParam, lParam); break;
			} break;
		    HANDLE_MSG(hwnd, WM_NOTIFY, WndProc_OnNotify); break ;
		case WM_COMMAND:
			ShowLinesCount();
//////////// Для чеканья меню Настройки -  крыжить счета которые не включать в новый файл ////////////
			i = (LOWORD(wParam)) - (SET_XML40817+55) ;
			if  ( i >= 0 && i < get_rules_count() )
				{  SET_CHECK_XML( gF_checkXML[i], "Удалять при выборке ED101 со счетом получателя %s : %s"); 
   				   logging("для маски %s установлено правило %d\n", get_rules_mask_item( i ), get_rules_action_item( i ));
				   set_rules_action();
				}
				
//////////// чеканье меню Настройки  ////////////
			i = LOWORD(wParam)-SETTINGS_ID_BASE ;
			if  ( i >= 0 && i <= SETTINGS_COUNT ) {  SET_CHECK( _gFlags[i].value , _gFlags[i].description ); }
///////////////////////////////////////////////
						
			switch(LOWORD(wParam)) {
		        case CM_EDIT_UNDO  :SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, EM_UNDO, 0, 0);		    break;
			    case CM_EDIT_CUT   :SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_CUT, 0, 0);  	    break;
			    case CM_EDIT_COPY  :SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_COPY, 0, 0);		    break;
			    case CM_EDIT_PASTE :SendDlgItemMessage(hwnd, IDC_MAIN_TEXT, WM_PASTE, 0, 0);	    break;
			    case CM_FILE_OPEN  :ShowComment("Выбор файла длЯ загрузки..");
					if ( DoFileOpenSave(hwnd, FALSE) )
						{
						SetWindowText(hwnd,loadedFile);
						//ShowComment("Loaded ED!");
						}
					break;
			   case CM_FILE_VISUAL:
					//SetWindowText( hwnd,"View XML");
					SendMessage( hwnd,WM_COMMAND, CM_FILE_TRUNC, 0L );
					SendMessage( hwnd,WM_COMMAND, CM_FILE_EXTRACT,  0L );
					SendMessage( hwnd,WM_COMMAND, CM_FILE_SWIFT,  0L );
					SendMessage( hwnd,WM_COMMAND, CM_FILE_DECODE,  0L );
					if ( gF_checkED501 )
						switch(CheckED501(  hRich ) )
							{
							case 501:
								ShowComment("ED501");
								SendMessage( hwnd,WM_COMMAND, CM_FILE_PROP,  0L );
								SendMessage( hwnd,WM_COMMAND, CM_FILE_DECODE,  0L );
								break;
							default: break ;
							}
					break;
			   case CM_IDKFA:
					godMode=1;
					//ShowComment("GOD mode!");
			   	        hSystemMenu = GetSystemMenu(hwnd, FALSE);
					CheckMenuItem(hSystemMenu,888,godMode?MF_CHECKED:MF_UNCHECKED);
	                //CheckMenuItem(hSettingsMenu,CM_IDKFA,godMode?MF_CHECKED:MF_UNCHECKED);
					break;
///////////////  Settings block //////////////////////////////
			   case SET_LOGGING: 
				if( gF_logging ) start_logging();
				else stop_logging();
//  FORWARD_WM_MENUSELECT(hwnd, hMenu, CM_CHANGE_RECALCULATE,hFileMenu, 0L, DefWindowProc);



				break ;
///////////////////////////////////////////////////////////////////////////////
			   case CM_CHANGE_RECALCULATE:RecalculateED( hRich );break ;
			   case CM_CHANGE_EDNO       :ChangerNumberDocs("EDNo=");break;
			   case CM_CHANGE_SUM        :ShowComment("Заменить суммы в ED101 (не реализовано)");break;
			   case CM_CHANGE_ACCDOCNO   :ChangerNumberDocs("AccDocNo=");break;
			   case CM_CHANGE_DATE       :DialogBox( g_hInst, "DATDLG", hwnd, (DLGPROC)DatDlgProc) ; break;
			   case CM_CHANGE_DATEF      :
 				        if( gF_changeEDNo     )SendMessage( hwnd,WM_COMMAND, CM_CHANGE_EDNO, 0L );
 				        if( gF_changeAccDocNo )SendMessage( hwnd,WM_COMMAND, CM_CHANGE_ACCDOCNO, 0L );
				        if( gF_recalculate    )SendMessage( hwnd,WM_COMMAND, CM_CHANGE_RECALCULATE, 0L );
						if( gF_changeSum      )SendMessage( hwnd,WM_COMMAND, CM_CHANGE_SUM, 0L );
//////////////////////////////////////  А можно вместо сигнала просто вызвать:RecalculateED( hRich );
			                if ( DialogBox( g_hInst, "DATDLG", hwnd, (DLGPROC)DatDlgProc) )
							{
				  		    SendMessage( hwnd,WM_COMMAND, CM_FILE_ENCODE, 0L );
					  	    SendMessage( hwnd,WM_COMMAND, CM_FILE_HEAD  , 0L );
						    if( gF_saveProper){
		//TODO - заменить в pszFileName день и сохранить
						       WritingFile( hRich , loadedFile );
						    }
						}
					break;

			   case CM_FILE_DECODE:
					//SetWindowText( hwnd,"Decoding...");
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
					DecodeFromBASE64(GetDlgItem(hwnd, IDC_MAIN_TEXT));
			        SetCursor(ghcurSave);
					//SetWindowText( hwnd,"Декодирован из BASE64");
					break;
			   case CM_FILE_PRETTY:
					//SetWindowText( hwnd,"Formating XML code ... please wait...");
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
					WalkerXML(GetDlgItem(hwnd, IDC_MAIN_TEXT));
					//SetWindowText( hwnd,"XML formatted");
			        SetCursor(ghcurSave);
					break;
			   case CM_FILE_SAVE:
					DoFileOpenSave(hwnd, TRUE);
					//SetWindowText( hwnd,"Saved");
					break;
			   case CM_FILE_SED_DIA:
//					GetWindowRect(hwnd, &gRect) ;
					//SetWindowText( hwnd,"Replacing...");
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
	                DialogBox( g_hInst, "SEDDLG", hwnd, (DLGPROC)SEDDlgProc);
					//SetWindowText( hwnd,"Replaced");
			        SetCursor(ghcurSave);
           			break;
			   case CM_FILE_REPLACE:
					DoReplace(hwnd);
					SetWindowText( hwnd,"Replacing");
					break;
			   case CM_FILE_ENCODE:
					//SetWindowText( hwnd,"Encoding...");
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
					EncodeToBASE64(GetDlgItem(hwnd, IDC_MAIN_TEXT));
			        SetCursor(ghcurSave);
					//SetWindowText( hwnd,"Закодирован в BASE64");
					break;
			    case CM_FILE_CHANGE:
					SendMessage( hwnd,WM_COMMAND, CM_FILE_SAVE, 0L );
					break;
			    case CM_FILE_HEAD:
					Enveloping(GetDlgItem(hwnd, IDC_MAIN_TEXT));
					//SetWindowText( hwnd,"Упаковано в XML Envelope");
					break;
			    case CM_FILE_TRUNC:
					TruncSignature(hRich);
					break;
			    case CM_FILE_EXTRACT:
				    ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
					//SetWindowText( hwnd,"Extracted");
					ExtractSenObject(hRich,"Object");
					SetCursor(ghcurSave);
					break;
			    case CM_FILE_PROP:
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
					//SetWindowText( hwnd,"Extracted");
					ExtractSenObject(hRich,"ProprietoryDocument");
			        SetCursor(ghcurSave);
					break;
			    case CM_FILE_SWIFT:
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
					//SetWindowText( hwnd,"SWIFT Extracted");
					ExtractSenObject(hRich,"SWIFTDocument");
			        SetCursor(ghcurSave);
					break;
			    case CM_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;
//			    case SET_XMLCONSTRUCT:
//	                DialogBox( g_hInst, "XMLCONSTRUCTDLG", hwnd, (DLGPROC)XmlConstructDlgProc);
//					break;
			    case CM_FILE_FIND:
	                DialogBox( g_hInst, "FindDlg", hwnd, (DLGPROC)FindDlgProc);
           			break;
           	    case CM_FIND_NEXT : Searching(); break;
			    case CM_CUT_ED101 :	ShowComment("Выборка документов");
					DeleteED101( hRich);
					SendMessage( hwnd,WM_COMMAND, CM_CHANGE_RECALCULATE , 0L );
					//RecalculateED( hRich );break ;
					break;
           	    case CM_XPATH_LOOK: 
				    //ShowComment("Ищем документы по выборке XPATH");
					//SetWindowText( hwnd,"XPATH Look...");
			        ghcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));
	                DialogBox( g_hInst, "XPATHDLG", hwnd, (DLGPROC)XPATHDlgProc);
					//SetWindowText( hwnd,"Looked");
			        SetCursor(ghcurSave);
           			break;
      		    case CM_ABOUT:LoadString(g_hInst, IDS_ABOUT_TEXT, szBuf, sizeof(szBuf));
					LoadString(g_hInst, ID_TIME_COMPILE, tempBuf, 48 );
					MessageBox(NULL,szBuf,tempBuf,0);
					break;

			    case CM_CHANGE_ANALIZE:
					//ShowComment("Анализируем количество документов");
					//Analyzer(hRich, "/my:PacketEPD/my:ED101/my:Payee/@PersonalAcc");
					Analyzer(hRich, "/my:PacketEPD/my:ED101");
					break;
			    case CM_SETTINGS:
					ShowComment("Настройка параметров обработки");
			                DialogBox( g_hInst, "SETT_DLG_UI", hwnd, (DLGPROC)SettingsDlgProc);
					break;
#ifdef TEST_ON

   			    case CM_TEST:
					{
					int i;
					ShowComment("TEST!");
					i=SendMessage(hRich,EM_LINEINDEX,3,0);
					ShowComment("Index 3l:%d",i);
					SendMessage(hRich,EM_REPLACESEL,1,(LPARAM)replInText);
					SendMessage(hRich,EM_SETZOOM,0,(LPARAM)3);
					}
					break;
#endif
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

void ScanParams( char *lpCmdLine )
{
char *ptr;
ptr = lpCmdLine ;
while ( *ptr  )
	{
	while( *ptr == 0x20 ) ptr++;
	if (*ptr=='-')
		{ ptr++;
		switch( *ptr )
			{
			case '$': gF_changeSum       = 1 ;break; 
			case 'l': gF_logging         = 1 ;break; 
			case 'E': gF_changeEDNo      = 1 ;break;
			case 'e': gF_changeEDNo      = 0 ;break;
			case 'B': gF_compressSpaces  = 1 ;break;
			case 'b': gF_compressSpaces  = 0 ;break;
			case 'A': gF_changeAccDocNo  = 1 ;break;
			case 'a': gF_changeAccDocNo  = 0 ;break;
			case 'R': gF_recalculate     = 1 ;break;
			case 'r': gF_recalculate     = 0 ;break;
			case 'S': gF_saveProper      = 1 ;break;
			case 's': gF_saveProper      = 0 ;break;
			case '5': gF_checkED501      = 1 ; if( *(ptr+1)=='-') gF_checkED501 = 0 ;break;
			case '0': gF_checkED501      = 0 ;break;
			case 'f': gF_fullMenu        = 0 ;if( *(ptr+1)=='+') gF_fullMenu = 1 ;break ;
			case 'F': gF_fullMenu        = 1 ;break ;
			}
		  ptr++;
		}
	else { // это уже не ключ
		LoadFile(GetDlgItem(g_hWnd, IDC_MAIN_TEXT),ptr);
		SendMessage( g_hWnd,WM_COMMAND, CM_FILE_VISUAL, 0L );
		return;
		}
	while( *ptr == 0x20 ) ptr++; // идем к следущему параметру
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine, int nCmdShow)
{
  WNDCLASSEX 	wc;
  HWND 		hwnd;
  HACCEL 		hAccel;
  MSG 		Msg;
  // ПрoверЯем, не было ли это приложение запущено ранее
  hwnd = FindWindow("WindowClass", NULL);
  if(hwnd)   {
    if(IsIconic(hwnd))  ShowWindow(hwnd, SW_RESTORE);
    SetForegroundWindow(hwnd);
    return FALSE;
  }

{ // init Window
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, "AppIcon"); //	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = "MAINMENU";
	wc.lpszClassName = "WindowClass" ;
	wc.hIconSm         = LoadIcon(NULL, "AppIcon"); //	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
}

	if( strlen(lpCmdLine) > 1 ) ScanParams( lpCmdLine );
	if ( gF_fullMenu )  wc.lpszMenuName = "FULLMENU" ;
		
#ifdef TEST_CTL3D	
	if( !Ctl3dRegister(hInstance) ) {MessageBox(0,"Ctl3d load lib error","Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);return 0;}
 	Ctl3dAutoSubclass(hInstance);
        InitCommonControls();
#endif
	if(!RegisterClassEx(&wc)) 
		if(!RegisterClass((LPWNDCLASS)&wc.style)) 
			{MessageBox(0,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);return 0;}
	//LoadLibrary("RICHEDIT32.DLL");
	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","UFEBS reader",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, CW_USEDEFAULT, 1200, 480, NULL, NULL, hInstance, NULL);
	if(hwnd == NULL) {MessageBox(0, "Window Creation Failed!", "Error!",MB_ICONEXCLAMATION|MB_OK|MB_SYSTEMMODAL);return 0;}
	g_hInst = hInstance ;
	g_Cmd   = lpCmdLine;
	g_hWnd  = hwnd ;
	start_logging();
	
	ShowWindow(hwnd,nCmdShow);
	UpdateWindow(hwnd);
/// запускаю второй раз, потому что при первом запуске Я разобрал ключи
/// но посылать сигнал WM_COMMAND, CM_FILE_VISUAL  некуда, так как окно и событиЯ
/// еще не проинициализированы - при первом запуске сигнал посылалсЯ в пустоту
	if( strlen(lpCmdLine) > 1 ) ScanParams( lpCmdLine );
///
	hAccel = LoadAccelerators( hInstance,"MenuAccelerators");
   	strcpy(xpathInText,"/my:PacketEPD/my:ED101/my:Payee[@PersonalAcc=\"40802810017000180048\"]");
//"/my:PacketEPD/my:ED101/my:Payee/@PersonalAcc");
	findInText[0]=0;
	replInText[0]=0;
	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
	   if( !TranslateAccelerator(hwnd,hAccel,&Msg) ) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		}
	}
	stop_logging();
#ifdef TEST_CTL3D	
	Ctl3dUnregister(hInstance);
#endif
	return Msg.wParam;
}
