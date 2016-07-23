#include <windows.h>
#include <commctrl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <regex.h>

#include "Main.h"

extern char  tempfln[32] ;

typedef struct rule_descr { 
	char     field[48] ;
	char     mask[32];
	int      action ;
        regex_t  reg_mask ; 
	} ;

//////////////////////////////////////////////////////
#include "rules.h"
//////////////////////////////////////////////////////

#define PacketEPD_GetEDAuthor     _ED101_GF_(root,"EDAuthor")
#define PacketEPD_GetEDDate       _ED101_GF_(root,"EDDate")
#define PacketEPD_GetEDNo         _ED101_GF_(root,"EDNo")
#define PacketEPD_GetEDQuantity   _ED101_GF_(root,"EDQuantity")
#define PacketEPD_GetEDReceiver   _ED101_GF_(root,"EDReceiver")
#define PacketEPD_GetSum          _ED101_GF_(root,"Sum")
#define PacketEPD_GetSystemCode   _ED101_GF_(root,"SystemCode")
#define ED101_START      	  xmlTextWriterStartElement(writer, BAD_CAST "ED101")
#define ED101_PROP( X )  	  xmlTextWriterWriteAttribute(writer, BAD_CAST X, BAD_CAST _ED101_GF_( c , X ) );

static char *not_found_str="0" ;
extern int   gF_checkXML[8];
extern int   gFlags[SETTINGS_COUNT] ;

extern struct g_flags _gFlags[SETTINGS_COUNT] ;

void set_rules_action()
{
int i,rules_cnt;
rules_cnt = get_rules_count() ;
for( i = 0 ; i < rules_cnt ; i++) {
//	rules[i].action = _WRITE_ED101_;
	if( gF_checkXML[i] ) rules[i].action = _SKIP_ED101_ ;
	else rules[i].action = _WRITE_ED101_ ;
	}
}

char *get_rules_mask_item(int uItem){ return ( rules[uItem].mask) ; }
char *get_rules_action_item(int uItem){ return ( rules[uItem].action) ; }
int   get_rules_count( void )
 {
 int i  ;
 i = sizeof( rules )/sizeof( struct rule_descr) ;
 while( strlen(rules[i].field) < 4 ) i-- ;
 return( i ); 
  }

void compileRegMask()
{
int rules_cnt,i;
rules_cnt = sizeof( rules )/sizeof( struct rule_descr) ;
for( i=0;i<rules_cnt;i++)
	{
	if( rules[i].mask[0] == 0 ) break;
    logging("Kомпил REGEX[%d]:%s\n",i,rules[i].mask);
	regcomp(&rules[i].reg_mask, rules[i].mask , 0) ;
	}

set_rules_action();
}

char *_ED101_GF_(xmlNodePtr p,const char *fn)
{   
 xmlAttr   *a ;
 xmlNodePtr d , d0 ;
 int i ;
 char *fn1 , buf[48]; 
 
 strcpy(buf,fn);
 fn1 = &buf[0] ;
 a = p->properties ;
 i = 0 ;
 d = p ;
 // если fn начинается с ":"  типа ":Payer:INN"
 // то переходим на узел Payer
 if( buf[0] == ':' )
	{
	d = p->children   ;
	fn1 = &buf[1];
        i = 1; while( buf[i] != ':') i++; buf[i]=0;
	//printf("fn1[%s]\n",fn1);
        while ( d && strcmp(d->name, fn1 ) ) d = d->next;
	a = d->properties;
	fn1 = &buf[i+1];

	// возможен третий уровень ":Payer:Bank:CorrespAcc"
	// сейчас fn1 = "Bank:CorrespAcc"
	// ищем ':' если находим - повторяем прыг
	i++;
	while( buf[i] && buf[i] != ':') i++;
	if ( buf[i] == ':' )
		{ // мы на "Bank^CorrespAcc"
		buf[i]=0;
		d = d->children   ;
	        while ( d && strcmp(d->name, fn1 ) ) d = d->next;
		a = d->properties;
		fn1 = &buf[i+1];
		}	

	}

// Ёто может быть Node или Attr
// ищем среди Node
if( d->children ){ d0=d->children;
while ( d0 && strcmp(d0->name, fn1 ) ) d0 = d0->next;
if( d0 ) { return(XML_GET_CONTENT(d0->children)); }
}


// поиск среди атрибутов
//printf("--fn1[%s]\n",fn1);
 while( a && strcmp(a->name,fn1) ) 
	{ // printf("gf:%p %s\n",a,a->name);
	 a = a->next ;
	}
// printf("gf:%x  \n",a);

 if( a && a->children ) {
	return( XML_GET_CONTENT(a->children) );
	}
 else {
	//printf("attr not found\n");
	return( not_found_str );
	}

}

xmlNodePtr firstED101 ( xmlNodePtr root ) 
{ 
// return ( root->children->next->next->next ); 
xmlNodePtr  c ;
c = xmlFirstElementChild (root) ;
while( strcmp(c->name,"ED101") )
	{c = xmlNextElementSibling(c) ;
//	if(gF_logging)	printf("FIRST:%s\n", c->name );
	}
return( c );
}

xmlNodePtr nextED101 ( xmlNodePtr cur_node )
{
cur_node = xmlNextElementSibling( cur_node );
return( cur_node );
}

void writeED101(xmlNodePtr c,xmlTextWriterPtr writer)
{
logging("%WRITE ED101 EDNo:%s\n",_ED101_GF_(c,"EDNo"));
ED101_START  ;
ED101_PROP( "ChargeOffDate" )  ;
ED101_PROP( "EDAuthor" );
ED101_PROP( "EDDate" );
ED101_PROP( "EDNo" );
ED101_PROP( "Priority" );
ED101_PROP( "ReceiptDate" );
ED101_PROP( "Sum" );
//EDQuantity++;
//Sum += strtoul( _ED101_GF_(c,"Sum"),NULL,10 );
ED101_PROP( "TransKind" );
xmlTextWriterStartElement(writer, BAD_CAST "AccDoc");
xmlTextWriterWriteAttribute(writer, BAD_CAST "AccDocDate", BAD_CAST _ED101_GF_(c,":AccDoc:AccDocDate") );
xmlTextWriterWriteAttribute(writer, BAD_CAST "AccDocNo"  , BAD_CAST _ED101_GF_(c,":AccDoc:AccDocNo") );
xmlTextWriterEndElement(writer); //AccDoc

xmlTextWriterStartElement(writer, BAD_CAST "Payer");
xmlTextWriterWriteAttribute(writer, BAD_CAST "INN", BAD_CAST _ED101_GF_(c,":Payer:INN") );
xmlTextWriterWriteAttribute(writer, BAD_CAST "KPP", BAD_CAST _ED101_GF_(c,":Payer:KPP") );
xmlTextWriterWriteAttribute(writer, BAD_CAST "PersonalAcc", BAD_CAST _ED101_GF_(c,":Payer:PersonalAcc") );

xmlTextWriterWriteElement(writer, BAD_CAST "Name", BAD_CAST _ED101_GF_(c,":Payer:Name"));
xmlTextWriterStartElement(writer, BAD_CAST "Bank");
xmlTextWriterWriteAttribute(writer, BAD_CAST "BIC", BAD_CAST _ED101_GF_(c,":Payer:Bank:BIC"));
xmlTextWriterWriteAttribute(writer, BAD_CAST "CorrespAcc", BAD_CAST _ED101_GF_(c,":Payer:Bank:CorrespAcc"));
xmlTextWriterEndElement(writer); //Bank;
xmlTextWriterEndElement(writer); //Payer;

xmlTextWriterStartElement(writer, BAD_CAST "Payee");
xmlTextWriterWriteAttribute(writer, BAD_CAST "INN", BAD_CAST _ED101_GF_(c,":Payee:INN") );
xmlTextWriterWriteAttribute(writer, BAD_CAST "KPP", BAD_CAST _ED101_GF_(c,":Payee:KPP") );
xmlTextWriterWriteAttribute(writer, BAD_CAST "PersonalAcc", BAD_CAST _ED101_GF_(c,":Payee:PersonalAcc") );

xmlTextWriterWriteElement(writer, BAD_CAST "Name", BAD_CAST _ED101_GF_(c,":Payee:Name"));

xmlTextWriterStartElement(writer, BAD_CAST "Bank");
xmlTextWriterWriteAttribute(writer, BAD_CAST "BIC", BAD_CAST _ED101_GF_(c,":Payee:Bank:BIC"));
xmlTextWriterWriteAttribute(writer, BAD_CAST "CorrespAcc", BAD_CAST _ED101_GF_(c,":Payee:Bank:CorrespAcc"));
xmlTextWriterEndElement(writer); //Bank;
xmlTextWriterEndElement(writer); //Payee;
xmlTextWriterWriteElement(writer, BAD_CAST "Purpose", BAD_CAST _ED101_GF_(c,"Purpose"));
xmlTextWriterEndElement(writer); //ED101
}

int  checkED101_KZ(xmlNodePtr node)
{
regmatch_t pmatch[2];
char b[24] ;
int i ;
int rules_cnt ;

rules_cnt = get_rules_count() ;

logging("Check ED101 EDNo:%s  PersonalAcc:%s\n",_ED101_GF_( node ,"EDNo"),_ED101_GF_(node,":Payee:PersonalAcc"));
for( i=0;i<rules_cnt;i++)
	{
	strcpy(b, _ED101_GF_( node, rules[i].field ) );
	logging(" поле %s:%s сравниваем с маской %s\n", rules[i].field,b,rules[i].mask ) ;

	//	if( !strncmp( b , rules[i].mask , strlen(rules[i].mask) ) ) {	
        if( regexec(&rules[i].reg_mask, b , 2, &pmatch[0],0) != REG_NOMATCH ) {
		logging(" совпадение rule:%d  action:(%d) EDNo:%-7s ACC:%s SUM:%s\n", i , rules[i].action, _ED101_GF_( node,"EDNo" ) , _ED101_GF_( node, ":Payee:PersonalAcc" ) , _ED101_GF_( node, "Sum") ) ;
		return rules[i].action ;
		}
	}
return _WRITE_ED101_;
}

int   recForDeleteKZ(xmlNodePtr root, xmlTextWriterPtr writer)
{
   int cnt ;
   xmlNodePtr cur_node ;
   cnt = 0 ;
   xmlTextWriterStartElement(writer, "PacketEPD" );
   xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns",                                     BAD_CAST "urn:cbr-ru:ed:v2.0" );
   xmlTextWriterWriteAttribute(writer, BAD_CAST "EDAuthor",                                     BAD_CAST PacketEPD_GetEDAuthor );
   xmlTextWriterWriteAttribute(writer, BAD_CAST "EDDate",                                       BAD_CAST PacketEPD_GetEDDate);
   xmlTextWriterWriteAttribute(writer, BAD_CAST "EDQuantity",                                     BAD_CAST PacketEPD_GetEDQuantity);
   xmlTextWriterWriteAttribute(writer, BAD_CAST "EDReceiver",                                     BAD_CAST PacketEPD_GetEDReceiver);
   xmlTextWriterWriteAttribute(writer, BAD_CAST "Sum",                                            BAD_CAST PacketEPD_GetSum);
   xmlTextWriterWriteAttribute(writer, BAD_CAST "SystemCode",                                     BAD_CAST PacketEPD_GetSystemCode);
   xmlTextWriterWriteElementNS(writer,"dsig","SigValue","urn:cbr-ru:dsig:v1.1" , BAD_CAST _ED101_GF_(root,"SigValue" )  );
   cur_node = firstED101( root ) ; 
   do {
    switch( checkED101_KZ(cur_node) )
	 {  
	case _SKIP_ED101_  : break ;// пропускаем данный документ
	case _WRITE_ED101_ : cnt++ ; writeED101 ( cur_node, writer ); break ;
	}
    cur_node = nextED101( cur_node ); 
    } while ( cur_node ) ;
    xmlTextWriterEndElement(writer ); //PacketEPD
return cnt;
}

DWORD DeleteED101( HWND hEdit)
{
    xmlTextWriterPtr writer;
    LPSTR pszText;
    DWORD dwWritten , dwTextLength , l ;
    xmlDocPtr  docx,dcr  ;
    xmlBufferPtr xbuf;
    FILE *fou;
    xmlNodePtr root ;
    ShowComment("nроизводим выборку ED101");
    dwTextLength = GetWindowTextLength(hEdit);
	if( dwTextLength < 20 ) { ShowComment("Ѕуфер так мал :(  нечего разбирать");return 0;}
    pszText = (LPSTR)LocalLock((HLOCAL)SendMessage( hEdit, EM_GETHANDLE,(WPARAM)0,(LPARAM)0));
    docx    = xmlReadMemory(pszText, dwTextLength , "TESTFILE", NULL, 0);
    if(docx == NULL) { printf("Error parsing memory block PBUF\n"); return 0; }
    xbuf   = xmlBufferCreate();
    writer = xmlNewTextWriterMemory(xbuf, 0);

    root = xmlDocGetRootElement(docx);
    if( strcmp(root->name,"PacketEPD" ) ) 
	{ logging("oперациa пересборки работает только с PacketEPD\n");   xmlFreeDoc(docx); return 0;} 

   xmlTextWriterStartDocument (writer, NULL, "WINDOWS-1251", NULL);
   compileRegMask();
   l=recForDeleteKZ( root , writer );

   xmlTextWriterEndDocument(writer);
    
   ShowComment("—обираем выборку из %d документов",l);
   xmlFreeTextWriter(writer);
   xmlFreeDoc(docx);
   dcr  = xmlReadMemory( xbuf->content , xbuf->use , "TESTFILE", NULL, 0);
   xmlSaveFile	(tempfln,dcr);
   xmlFreeDoc(dcr);

   ShowComment("nакет пересобран, количество документов:%d",l);
   
   fou = fopen( tempfln , "r" );
   memset(pszText,0,dwTextLength);
   dwWritten = fread(pszText,1,xbuf->size,fou);

   fclose( fou );
   unlink( tempfln );

   SetWindowText(hEdit, pszText);

//if( gF_logging)printf("del temp file:%s\n",tempfln);

//  recalculateQuantity( xbuf , pbuf ); 
  logging("write    use:%d  size:%d n", xbuf->use,xbuf->size );
  xmlBufferFree(xbuf);
  return 1;
}
