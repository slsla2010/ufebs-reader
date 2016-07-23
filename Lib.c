#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "Main.h"

static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define B64_SYNTAX_ERROR        1
#define B64_FILE_ERROR          2
#define B64_FILE_IO_ERROR       3
#define B64_ERROR_OUT_CLOSE     4
#define B64_LINE_SIZE_TO_MIN    5
#define B64_DEF_LINE_SIZE       72
#define B64_MIN_LINE_SIZE       4

#define BACK7(X,Y1,Y2,Y3)   ( *(X-7) == Y1 && *(X-6)==Y2 && *(X-5)==Y3 )
#define FWD3(X,Y1,Y2,Y3)    ( *(X+1) == Y1 && *(X+2)==Y2 && *(X+3)==Y3 )

static FILE *sdr ;
extern struct g_flags _gFlags[SETTINGS_COUNT] ;
extern int gFlags[SETTINGS_COUNT] ;

extern LPSTR GlobalBuf ;

void ChangeNumb( char *p){ // на входе текст вида "123" рандомизим этот номер  char  buf[16] ;  int i , nd ;  i=0;   p++;  while( *(p+i) != '"' ) {buf[i]=*(p+i);i++;} buf[i]=0;  nd = atoi(buf);  nd = nd*7;  sprintf( buf,"%d",nd); buf[i]=0;  i--;  for(;i>=0;i--) { *(p+i)=buf[i]; }}
void start_logging( void ) 
{
 char buf[46];
 time_t lt;
 struct tm *ptr;
 if( !gF_logging ) return;
 lt = time(NULL);
 ptr = localtime(&lt);
 sprintf(buf,"ur_%02d-%02d-%04d.log",ptr->tm_mday,ptr->tm_mon+1,1900+ptr->tm_year);
 sdr = fopen(buf,"w"); 
 fprintf(sdr,"start logging\n"); 
}
void stop_logging( void ) { if( !gF_logging ) return; fprintf(sdr,"stop logging\n"); fflush(sdr);fclose(sdr); }
void logging( char *format, ...)
{
	int l ;
	char *buf;
	va_list args;
	if( !gF_logging ) return;
	va_start( args , format );
	l = _vscprintf( format,args)+1 ;
	buf = malloc( l );
	vsprintf( buf, format , args );

	fprintf(sdr,buf );
	free( buf);
}

char *reformatSumInt( unsigned long lSum , char *buf ){char *p;int i,l;strcpy(buf,ltoa(lSum,buf,10));l=strlen(buf);p = buf+l;for(i=1;i<4;i++) *(p+i)=0;*(p  )=*(p-1);*(p-1)=*(p-2);*(p-2)='.';l=strlen(buf);if ( l<6) return ( buf );p=buf+l;for(i=0;i<6;i++)  *(p-i)=*(p-i-1);*(p-6)='`';l=strlen(buf);if ( l<10) return ( buf );p=buf+l;for(i=0;i<10;i++)  *(p-i)=*(p-i-1);*(p-10)='`';l=strlen(buf);if ( l<14) return ( buf );p=buf+l;for(i=0;i<14;i++)  *(p-i)=*(p-i-1);*(p-14)='`';return ( buf );}void bprintf( char *format, ...)
{
	int l ;
	char *buf;
	va_list args;
	va_start( args , format );
	l = _vscprintf( format,args)+1 ;
	buf = malloc( l );
	vsprintf( buf, format , args );
	strcat( GlobalBuf , buf );
	free( buf);
}

void utf8cp1251(const char *in, char *out)
{
    unsigned char i;
    int c,c_out;
    unsigned long c1=32,new_c1, new_c2, new_i,out_i;
    bool byte2=false;
    c_out=0;
    for (c=0;c<strlen(in);c++)
    {
        i=in[c];
        if (i<=127) out[c_out++]=in[c];
        if (byte2)
        {
            new_c2=(c1&3)*64+(i&63);
            new_c1=(c1>>2)&5;
            new_i=new_c1*256+new_c2;
            if (new_i==1025)
            {
                out_i=168;
            }
            else
            {
                if (new_i==1105)
                {
                    out_i=184;
                }
                else
                {
                    out_i=new_i-848;
                }
            }
            out[c_out++]=(char)out_i;
            byte2=false;
        }
        if ((i>>5)==6)
        {
            c1=i;
            byte2=true;
        }
    }
    out[c_out]=0;
}

int trS(char *src, char *dest, char *ch, char *newch)
{ // src и dest могут указывать на один буфер
char *np;
char *pbuf , *pbuf0 ;
int sl , dl , i , j , dif ;
int count;

count = 0 ;
pbuf=(char *)malloc(strlen(src)*2); pbuf0=pbuf;
sl = strlen(ch); dl = strlen(newch);
strcpy(pbuf,src);

np = strstr(pbuf,ch);
if( np == NULL ) { free(pbuf);strcpy(dest,src);return src;}
do {
if( sl < dl ) { // сдвинуть остаток строки на dl-sl вперед
		i=strlen(np);
		while(i>=0){*(np+i+(dl-sl))=*(np+i);i--;} ; // сдвинуть остаток строки вперед
		i=dl;
		while(i){ *(np+i-1)=*(newch+i-1); i--;}
		np+=sl;
	       }
if( sl > dl ) {
		i = strlen(np);
		j = dl;
		dif = sl-dl;
		while(j<i){*(np+j)=*(np+j+dif);j++; } // cдвинуть остаток строки назад
		for(i=0;i<dl;i++) *(np+i)=*(newch+i);
		np+=dl;
	      }
if( sl == dl ) { for(i=0;i<dl;i++) *(np+i)=*(newch+i); np+=dl;}
count++;
} while( np=strstr(np,ch) );
strcpy(dest,pbuf0);
free(pbuf0);
return count;
}

char *trC(char *src, char *dest, int ch, char *newch)
{ 
 char  *srcp, *destp , *chrp;
 int i,j,l;
 l    = strlen(newch);
 srcp = src;
 destp= dest;
 chrp = src;
 do {	chrp = strchr(srcp , ch );
	if ( chrp ) { 
		i = chrp-srcp;
		strncpy(destp,srcp,i) ;destp = destp+i;
		srcp=chrp+1;
		strncpy(destp,newch,l);destp = destp+l;
		} 
 } while ( chrp );
 strcpy( destp,srcp );
 return( dest );
}	

char *tr(char *src, char *dest, int ch, char *newch)
{
char  *srcp, *destp , *chrp;
int i,j,l;
l = strlen(newch);
srcp = src;
destp= dest;
chrp = src;
 do {	chrp = strchr(srcp , ch );
	if ( chrp ) { 
		i = chrp-srcp;
		strncpy(destp,srcp,i) ;destp = destp+i;
		srcp=chrp+1;
// вытянуть все документы в строку, одно ED101 на строке
// идем по буферу и удаляем все переводы строки пока не встретим <ED101>
// 
		sprintf(destp++,">");   // </ED101>
	
	if ( BACK7( srcp , '/','E','D') || BACK7( srcp, ':','v','1') || BACK7( srcp,'g','V','a') || BACK7( srcp,'1','2','5' ) || FWD3( srcp, 'A','c','c') )	 
		{ sprintf(destp,"%c%c",0x0D,0x0A); 	destp = destp+2; }
	if ( FWD3(srcp,'A','c','c') )  { sprintf(destp,"  "                ); destp = destp+2; }
	if ( FWD3(srcp,'P','a','y') )  { sprintf(destp,"%c%c    ",0x0D,0x0A); destp = destp+6; }
	if ( FWD3(srcp,'P','u','r') )  { sprintf(destp,"%c%c  "  ,0x0D,0x0A); destp = destp+4; }
	} 
 } while ( chrp );
 strcpy( destp,srcp );
 return( dest );
}	

char *b64_message( int errcode )
{
    #define B64_MAX_MESSAGES 6
    char *msgs[ B64_MAX_MESSAGES ] = {
            "b64:000:Invalid Message Code.",
            "b64:001:Syntax Error -- check help for usage.",
            "b64:002:File Error Opening/Creating Files.",
            "b64:003:File I/O Error -- Note: output file not removed.",
            "b64:004:Error on output file close.",
            "b64:004:linesize set to minimum."
    };
    char *msg = msgs[ 0 ];

    if( errcode > 0 && errcode < B64_MAX_MESSAGES ) {
        msg = msgs[ errcode ];
    }

    return( msg );
}
/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
void encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/
void b64encode( char *inbuffer, char *outbuffer, int length_in_buff , int linesize )
{
    unsigned char in[3], out[4];
    int i, len, blocksout = 0 , o , j ;
    o = 0 ;
    j = 0 ;
    while(j<length_in_buff){
        for( len = 0, i = 0; i < 3 && j<length_in_buff; i++ ) {
              in[i] = (unsigned char) inbuffer[j++];
              if( j<length_in_buff )  len++;
              else                    in[i] = 0;
             }
        if( len ) {
#ifdef TEST_B64
	    printf("coding block: %c%c%c\n",in[0],in[1],in[2]);
#endif
            encodeblock( in, out, len );
#ifdef TEST_B64
	    printf("result: %c%c%c%c\n",out[0],out[1],out[2],out[3]);
#endif
            for( i = 0; i < 4; i++ ) outbuffer[o++] = out[i] ;
            blocksout++;
        }
        if( blocksout >= (linesize/4) || (j>=length_in_buff) ) {
            if( blocksout ) {
                outbuffer[o++] = '\r' ;
                outbuffer[o++] = '\n' ;
            }
            blocksout = 0;
        }
    }
}

void decodeblock( unsigned char in[4], unsigned char out[3] )
{   
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}
void decode( char *inbuffer, char *outbuffer, int length_in_buff )
{
    unsigned char in[4], out[3], v;
    int i, len, j , o ;
    o = 0 ;
    j = 0 ;
    while(j<length_in_buff){
            for( len = 0, i = 0; i < 4 && j<length_in_buff; i++ ) {
            v = 0;
            while( j<length_in_buff && v == 0 ) {
                v = (unsigned char) inbuffer[j++];
                v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if(v)v=(unsigned char)((v=='$')?0:v-61);
            }
            if( j<length_in_buff ) {
                len++;
                if(v)in[i]=(unsigned char)(v-1);
            }
            else in[i] = 0;
        }
        if( len ) {
            decodeblock( in, out );
            for( i = 0; i < len - 1; i++ ) {
                outbuffer[o++] = out[i] ;
            }
        }
    }
}

int parseSEDcmd( char *cs, char *cmdr , char *chgt )
{
char buf[256], *pb , *pb0 ;
strcpy(buf,cs);
pb = &buf[0];
if( *pb != 's' ) return 1;
pb++;
if( *pb != '/' ) return 2;
pb++;
while( *pb && *pb !='/' ) pb++;
if( *pb != '/' ) return 3;

//printf("BUF: %s\n",buf);
*pb = 0 ;
//printf("CMD:%s\n",&buf[2]);
strcpy(cmdr,&buf[2]);
pb++;
pb0=pb;
while( *pb && *pb !='/' ) pb++;
if( *pb == '/' ) *pb = 0 ;
//printf("CHA:%s\n",pb0);
strcpy(chgt,pb0);
return 0;
}


void one_line_xml_build ( char *decodedText ) { return ; }
