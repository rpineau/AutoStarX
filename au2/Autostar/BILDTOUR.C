////////////////////////////////////////////////////////////////////////
// BildTour.c : Defines the entry point for the console application.
//
//
//  Tour Encoding---------------------------------
//     !                                 statment following is auto select
//     {                                 Start pick list
//     }                                 End pick list
//     A <string>                        Asteroid
//   * B <word>                          Abell Cluster
//     C <byte>                          Caldwell Object
//     E                                 Lunar Eclipse
//     F <string>                        Named Deep Sky Object
//     G <byte>                          AutoSlew on/off
//   * H <long>                          Hipparcos Star
//     I <word>                          IC Object
//   * J <string>                        Selenographic Feature (by name)
//   * j <word)<word><byte><byte><string><string>
//                                       User Defined Selenographic Feature            
//   * K <word>                          UGC Galaxy
//     L <word><short><string><string>   Landmark
//     M <byte>                          Messier
//     N <word>                          NGC Object
//     O <string>                        Comet
//     P <byte>                          Planet (moon = 9)
//   * R <word>                          Arp Object
//     S <long>                          SAO Star
//     T <string><string>                Text Message
//     U <word><short><string><string>   User Object
//     V <bye><byte><byte>               GCVS entry
//     W <string>                        Name Star
//     X <string>                        Constellation
//     Y                                 Meteor Showers
//     Z <string>                        Satellite
//     #                                 End of tour
//
//  Notes:
//    * designates extended syntax supported only by LX200GPS
//    A tour body is contained in a tour record on the heap.
//    It is a concatentation of statement encodings above.
//    All strings are null terminated.
//
//  (c) 2002 Meade Instruments Corp. All rights reserved.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Parser State Variables ////////
#define St_NeedTitle 1
#define St_InString 2
#define St_GetStmt 3

FILE *srcfile, *destfile;
unsigned char srcline[256],wrkline[256];
char ParseState = 1;
char AutoSlew;
char tourname[17];
char mstring[256];
short linenum;
short PickCnt;
short rtncode;
typedef struct  {
   unsigned char page;
   unsigned short offset;
   } poolposition;

poolposition HeapTop;

short swap( short x)
{
    x = ((x>>8) & 0xff) | (x<<8);
    return(x);
}

short parseNum( short i, short *x )
{
	int		tmp;

     for (;srcline[i]==' '; i++);
     sscanf( &srcline[i],"%d",&tmp);
	 *x = tmp;
     for (;(srcline[i]!=' ' && srcline[i]); i++);
     return(i);
}


unsigned char hexval(char *x)
{
   unsigned char v;

   if (x[0]<'9') v = x[0] & 0x0f;
   else v = toupper(x[0]) -'A' + 10;
   v<<=4;
   if (x[1]<'9') v += x[1] & 0x0f;
   else v += toupper(x[1]) -'A' + 10;
   return(v);
}


short TranslateHex( char *src)
{
   short i,j;

  if (src[0]=='$') { // hex
      i=0;
      for (j=1;j<strlen(src);j++) {
//printf("%c",src[j]);
         if (src[j]=='[') break;
         if (src[j]!=' ') {
            if (src[j]<='9') src[i]=(src[j++]&0x0f)<<4;
            else src[i]=(toupper(src[j++])-'A'+10)<<4;
//printf("%c",src[j]);
            if (src[j]<='9') src[i]+=(src[j]&0x0f);
            else src[i]+=(toupper(src[j])-'A'+10);
            i++;
            }
         }
      src[i]=0;
      }
//else printf(src);
//printf("\n");
   for (i=strlen(src); (i && src[i-1]<=' '); src[--i]=0);
   return(i);
}

      
short parseLong( short i, long *x )
{
     int dd,mm,sgn;
      
     for (;srcline[i]==' '; i++);
     sscanf( &srcline[i],"%ld",x);
     for (;(srcline[i]!=' ' && srcline[i]); i++);
     return(i);
}

short parseRA( short i, short *ra)
{
     int hh,mm,ss;
     
     for (;srcline[i]==' '; i++);
     if (srcline[i]) {
        sscanf(&srcline[i],"%02d:%02d:%02d",&hh,&mm,&ss);
        i+=8;
        *ra = hh*900 + mm*15 + ss/4;
        *ra  = swap(*ra);
        return(i);
        }
     *ra = 0;
     return(i);
}     


short parseAz( short i, short *az)
{
     int hh,mm,ss;
     
     for (;srcline[i]==' '; i++);
     if (srcline[i]) {
        sscanf(&srcline[i],"%03d:%02d:%02d",&hh,&mm,&ss);
        i+=8;
        *az = hh*60 + mm;
        *az  = swap(*az);
        return(i);
        }
     *az = 0;
     return(i);
}     

short parseDec( short i, short *dec)
{
     int dd,mm,sgn;
      
     for (;srcline[i]==' '; i++);
     sgn = 1;
     if (srcline[i]) {
        if (srcline[i]=='+') i++;
        else if (srcline[i]=='-') {
           sgn = -1;
           i++;
           }
        srcline[i+2]=' '; srcline[i+5]=' ';
//printf("%s\n",&srcline[i]);
        sscanf(&srcline[i],"%02d %02d",&dd,&mm);

        i+=6;
        while (srcline[i]>' ') i++;
//printf("dd=%d mm=%d\n",dd,mm);
        *dec = sgn * (dd*60 + mm);
        *dec  = swap(*dec);
        return(i);
        }
     *dec = 0;
     return(i);
}     

short parseStr( short i, char *p)
{
   short j;
   char c;
   char bb,even;

   for (j = i; (srcline[j] != '"' && srcline[j]); j++);
   if (srcline[j]=='"') j++;
   else {
      printf("Error- Invalid String line %d:\n %s\n",linenum,srcline);
      rtncode=1;
      return(j);
      }
   i = j;
   if (srcline[j]!='$') // not hex
      {
      do {
         for (j = i; (srcline[j]!='"' && srcline[j]>=' '); j++) {
            *p++ = srcline[j];
            }
         c = srcline[j];
         if (c == '"') {
            if (srcline[j+1]=='"') {
               *p++ = c;
               i=j+2;
               }
            else {
               c = ' ';
               p--;
               if (*p != ' ') {
                 ++p;
                 *p++ = c;
                 }
               i=j+1;
               }
            }
         else {
            printf("\nError- Missing (%c) line %d:\n %s\n",'"',linenum,srcline);
            rtncode=1;
            c = ' ';
            i=j;
            }
         }
      while ( c != ' ');
      }
   else { // process hex
      even=1;
      i++;
      for (j=i; (srcline[j]!='"' && srcline[j]>=' '); j++) {
         if (srcline[j]!=' ') {
            bb = srcline[j]-'0';
            if (bb>9) bb = toupper(srcline[j])-'A'+10;
            if (even) *p = bb<<4;
            else {
               *p |= bb;
               p++;
               }
            even = !even;
            }
         }
      if (srcline[j]!='"') {
         printf("\nError- Missing (%c) line %d:\n %s\n",'"',linenum,srcline);
         rtncode=1;
         c = ' ';
         i=j;
         }
      else i=j+1;
      }
   *p=0;
   return(i);
}


short MapPlanet( char *mstring)
{
   short j;

   for (j=0;j<strlen(mstring);j++)
      mstring[j]=toupper(mstring[j]);
   mstring[strlen(mstring)-1]=0;
//printf("->%s<-\n",mstring);
   j= 0;
   if (!strcmp("MERCURY",mstring))
      j=0;
   else if (!strcmp("VENUS",mstring))
      j=1;
   else if (!strcmp("EARTH",mstring))
      j=2;
   else if (!strcmp("MARS",mstring))
      j=3;
   else if (!strcmp("JUPITER",mstring))
      j=4;
   else if (!strcmp("SATURN",mstring))
      j=5;
   else if (!strcmp("URANUS",mstring))
      j=6;
   else if (!strcmp("NEPTUNE",mstring))
      j=7;
   else if (!strcmp("PLUTO",mstring))
      j=8;
   else {
      printf("\nError- Unknown planet. line %d\n",linenum);
      printf("  %s\n",srcline);
      rtncode=1;
      }
   return(j);
}  

void ProcessLunar(short i, char extsyntax)  // lunar commands
{
   short j;
   short x,y;
   long l;
   char c,d;
   
   if (!extsyntax) {  // etx lunar string only
      c='E';
      fwrite(&c,1,1,destfile);
      }
   else { // extended lunar syntax;
      for (i+=5;wrkline[i]==' ';i++);
      if (!strncmp("ECLIPSE",&wrkline[i],7)) { // eclipse
         c='E';
         fwrite(&c,1,1,destfile);
         }
      else if (!strncmp("FEATURE",&wrkline[i],7)) { // feature
         c='J';
         fwrite(&c,1,1,destfile);
         parseStr(i+7,mstring);
         if (mstring[strlen(mstring)-1]!=' ')
            fwrite(mstring,1,strlen(mstring),destfile);
         else
            fwrite(mstring,1,strlen(mstring)-1,destfile);
         c=0;
         fwrite(&c,1,1,destfile);
         }
      else if (!strncmp("LOCATION",&wrkline[i],8)) { // user location
         c='j';
         fwrite(&c,1,1,destfile); 
         i+=8;
         i=parseDec(i,&x);        // lat
         fwrite(&x,1,2,destfile);
         i=parseDec(i,&x);        // lon
         fwrite(&x,1,2,destfile);
         i=parseDec(i,&x);        // min angle
         fwrite(&x,1,2,destfile);
         i=parseDec(i,&x);        // max angle
         fwrite(&x,1,2,destfile);
         i=parseStr(i,mstring);   // name
         if (mstring[strlen(mstring)-1]!=' ')
            fwrite(mstring,1,strlen(mstring),destfile);
         else
            fwrite(mstring,1,strlen(mstring)-1,destfile);
         c=0; fwrite(&c,1,1,destfile);
         ParseState = St_InString;
         parseStr(i,mstring);
         fwrite(mstring,1,strlen(mstring),destfile);
         }
      else {
         printf("\nError- unrecognized command line %d:\n %s\n",linenum,srcline);
         rtncode=1;
         }
      }
}   

void ProcessExtensions( short i)
{
   short j;
   short x,y;
   long l;
   char c,d;

   if (!strncmp("ABELL",&wrkline[i],5)) { // Abell cluster
      c='B';
      fwrite(&c,1,1,destfile);
      i=parseNum(i+5,&x);
      x = swap(x);
      fwrite(&x,1,2,destfile);
      }
   else if (!strncmp("ARP",&wrkline[i],3)) { // Arp Object
      c='R';
      fwrite(&c,1,1,destfile);
      i=parseNum(i+3,&x);
      if (wrkline[i]=='-')  // suffix
         i=parseNum(i+1,&y);
      else y=1;
      x = swap(x*100+y);
      fwrite(&x,1,2,destfile);
      }
   else if (!strncmp("UGC",&wrkline[i],3)) { // UGC Galaxy
      c='K';
      fwrite(&c,1,1,destfile);
      i=parseNum(i+3,&x);
      x = swap(x);
      fwrite(&x,1,2,destfile);
      }
   else if (!strncmp("HIPPARCOS",&wrkline[i],9)) { // HIPPARCOS star
      c='H';
      fwrite(&c,1,1,destfile);
      i = parseLong(i+9,&l);
      x = (l>>16);
      x = swap(x);
      fwrite(&x,1,2,destfile);
      x = (l & 0xffff);
      x = swap(x);
      fwrite(&x,1,2,destfile);
      }
   else {
      printf("\nUnknown Error line %d:\n %s\n",linenum,srcline);
      rtncode=1;
      }
}

int ProcessTour(char extsyntax)
{   
   short i,j;
   short x,y;
   long l;
   char c,d;

   ///// Init Globals /////
   linenum    = 0;
   PickCnt    = 0;
   ParseState = St_NeedTitle;
   AutoSlew   = 0;
   rtncode    = 0;
      

   /// parse lines ////
   while (!feof(srcfile)) {
      fgets(srcline,255,srcfile);
      linenum++;
      putchar('.');
      if (!(linenum & 63)) printf("\n");
      for (i=0; srcline[i]; i++) if (srcline[i]==0x92) srcline[i]=0x27;
      for (i = 0; srcline[i]==' '; i++);
      
      // convert line to upper case for parsing
      strcpy(wrkline,srcline);
      for (j=0; wrkline[j]; j++) wrkline[j] = toupper(wrkline[j]);
      switch (srcline[i]) {
         case 0: 
         case 0x0a:
         case 0x0d:
            break; // nada
         case '/': 
            break; // comment
         case '#':
            if (PickCnt) {
               printf("Error- Missing PICK END stmt. Line %d\n %s\n",linenum,srcline);
               rtncode=1;
               }
            if (ParseState == St_NeedTitle) {
               printf("\nError- Title expected line %d:\n %s\n",linenum,srcline);
               rtncode=1;
               }
            if (ParseState == St_InString) {
               c=0x00; // end line character;
               fwrite(&c,1,1,destfile);
               ParseState = St_GetStmt;
               }
            if (strncmp("END",&wrkline[i+1],3)) {
               printf("\nError- Title expected line %d:\n %s\n",linenum,srcline);
               close(srcfile);
               close(destfile);
               return(1);
               }
            else {
               c = '#';
               fwrite(&c,1,1,destfile);
               fclose(destfile);
               fclose(srcfile);
               printf("\nComplete\n");
               return(rtncode);
               }
         case '"':
            if (ParseState == St_InString) {
               i=parseStr(i,mstring);
               fwrite(mstring,strlen(mstring),1,destfile);
               }
            else {
               printf("\nError- Statement Expected line %d:\n %s",linenum,srcline);
               rtncode=1;
               }
            break;
         default:
            // finish off strings in progress
            if (ParseState == St_InString) {
               c=0x00;  // Autostar end of item
               fwrite(&c,1,1,destfile);
               ParseState = St_GetStmt;
               }
            if (ParseState == St_NeedTitle) {
               if (strncmp("TITLE",&wrkline[i],5)) {
                  printf("\nError- Title Expedted line %d:\n %s\n",linenum,srcline);
                  rtncode=1;
                  }
               else {
                  i=parseStr(i+5,mstring);
                  if (strlen(mstring)>17) {
                     printf("\nError- Tour name longer than 16 characters line %d:\n %s\n",linenum,mstring);
                     rtncode=1;
                     mstring[16]=0;
                     }
                  strcpy(tourname,mstring);
                  for (j=strlen(tourname);j<16;j++) tourname[j]=' ';
                  // Tour name
                  fwrite(tourname,16,1,destfile);
                  ParseState = St_GetStmt;
                  }
               }
            else if (ParseState == St_GetStmt ) {
               // check for auto select statement
               if (!strncmp("AUTO",&wrkline[i],4)) {
                  i+=4;
                  for (; srcline[i]==' '; i++);
                  if (!strncmp("SLEW",&wrkline[i],4)) {
                     for (i+=4; srcline[i]==' ';i++);
//printf("Auto Slew");
                     if (!strncmp("ON",&wrkline[i],2)) c = 1;
                     else c=0;
                     // Note we only emitt code for state changes
                     if ((c && !AutoSlew) || (AutoSlew && !c)) {
//printf(" Emitted");
                        d='G';
                        fwrite(&d,1,1,destfile);
                        fwrite(&c,1,1,destfile);
                        AutoSlew = c;
                        }
//printf("\n");
                     break;
                     }
                  else if (strncmp("SELECT",&wrkline[i],6)) {
                    printf("\nError- Expecting SELECT or SLEW. line %d:\n %s\n",linenum,srcline);
                    rtncode=1;
                    }
                  else {
                     i+=6;
                     for (; (srcline[i] && srcline[i]==' '); i++);
                     c='!'; // symbol for immediate
                     fwrite(&c,1,1,destfile);
                     }
                  }
               if (!strncmp("USER",&wrkline[i],4)) { // user definition
                  c='U';
                  fwrite(&c,1,1,destfile);
                  i=parseRA(i+4,&x);
                  fwrite(&x,1,2,destfile);
                  i=parseDec(i,&x);
                  fwrite(&x,1,2,destfile);
                  i=parseStr(i,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0; fwrite(&c,1,1,destfile);
                  ParseState = St_InString;
                  parseStr(i,mstring);
                  fwrite(mstring,1,strlen(mstring),destfile);
                  }
               else if (!strncmp("PICK",&wrkline[i],4)) { // PicK Command
                  i += 4;
                  for (;wrkline[i]==' '; i++);
                  if (!strncmp("ONE",&wrkline[i],3)) {
                     PickCnt++;
                     c='{';
                     fwrite(&c,1,1,destfile);
                     }
                  else if (!strncmp("END",&wrkline[i],3)) {
                     PickCnt--;
                     if (PickCnt<0) {
                        printf("Error- PICK END without PICK ONE. Line %d\n %s\n",linenum,srcline);
                        rtncode=1;
                        PickCnt = 0;
                        }
                     else {
                        c = '}';
                        fwrite(&c,1,1,destfile);
                        }
                     }
                  }
               else if (!strncmp("TEXT",&wrkline[i],4)) { // Text command
                  c='T';
                  fwrite(&c,1,1,destfile);
                  i=parseStr(i+4,mstring);
                  j = strlen(mstring)-1;
                  fwrite(mstring,j,1,destfile);
                  c=0; fwrite(&c,1,1,destfile);
                  ParseState = St_InString;
                  parseStr(i,mstring);
                  fwrite(mstring,1,strlen(mstring),destfile);
                  }
               else if (!strncmp("NGC",&wrkline[i],3)) { // NGC Object
                  c='N';
                  fwrite(&c,1,1,destfile);
                  i=parseNum(i+3,&x);
                  x = swap(x);
                  fwrite(&x,1,2,destfile);
                  }
               else if (!strncmp("IC",&wrkline[i],2)) { // IC
                  c='I';
                  fwrite(&c,1,1,destfile);
                  i=parseNum(i+2,&x);
                  x = swap(x);
                  fwrite(&x,1,2,destfile);
                  }
               else if (!strncmp("MOON",&wrkline[i],4)) { // Moon
                  c='P';
                  fwrite(&c,1,1,destfile);
                  c=9;
                  fwrite(&c,1,1,destfile);
                  }
               else if (!strncmp("SAO",&wrkline[i],3)) { // Star
                  c='S';
                  fwrite(&c,1,1,destfile);
                  i = parseLong(i+3,&l);
                  x = (l>>16);
                  x = swap(x);
                  fwrite(&x,1,2,destfile);
                  x = (l & 0xffff);
                  x = swap(x);
                  fwrite(&x,1,2,destfile);
                  }
               else if (!strncmp("ASTEROID",&wrkline[i],8)) { // Asteroid
                  c='A';
                  fwrite(&c,1,1,destfile);
                  parseStr(i+8,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0;
                  fwrite(&c,1,1,destfile);
                  }
               else if (!strncmp("COMET",&wrkline[i],5)) { // Comet
                  c='O';
                  fwrite(&c,1,1,destfile);
                  parseStr(i+5,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0;
                  fwrite(&c,1,1,destfile);
                  }
               else if (!strncmp("SATELLITE",&wrkline[i],9)) { //  Satellite
                  c='Z';
                  fwrite(&c,1,1,destfile);
                  parseStr(i+9,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0;
                  fwrite(&c,1,1,destfile);
                  }
               else if (!strncmp("STAR",&wrkline[i],4)) { //  Stars
                  c='W';
                  fwrite(&c,1,1,destfile);
                  parseStr(i+4,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0;
                  fwrite(&c,1,1,destfile);
                  }
               else if (!strncmp("DEEP",&wrkline[i],4)) { // Deep Sky
                  for (i+=4;wrkline[i]==' ';i++);
                  if (strncmp("SKY",&wrkline[i],3)) {
                     printf("Error- SKY expected. Line %d\n %s\n",linenum,srcline);
                     rtncode=1;
                     }
                  else{
                     c='F';
                     fwrite(&c,1,1,destfile);
                     parseStr(i+3,mstring);
                     if (mstring[strlen(mstring)-1]!=' ')
                        fwrite(mstring,1,strlen(mstring),destfile);
                     else
                        fwrite(mstring,1,strlen(mstring)-1,destfile);
                     c=0;
                     fwrite(&c,1,1,destfile);
                     }
                  }
               else if (!strncmp("CONSTELLATION",&wrkline[i],13)) { //  constellation
                  c='X';
                  fwrite(&c,1,1,destfile);
                  parseStr(i+13,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0;
                  fwrite(&c,1,1,destfile);
                  }
               else if (!strncmp("MESSIER",&wrkline[i],7)) { // Messier
                  c='M';
                  fwrite(&c,1,1,destfile);
                  i=parseNum(i+7,&x);
                  fwrite(&x,1,1,destfile);
                  }
               else if (!strncmp("CALDWELL",&wrkline[i],8)) { // Caldwell
                  c='C';
                  fwrite(&c,1,1,destfile);
                  i=parseNum(i+8,&x);
                  fwrite(&x,1,1,destfile);
                  }
               else if (!strncmp("PLANET",&wrkline[i],6)) { // Planet
                  c='P';
                  fwrite(&c,1,1,destfile);
                  parseStr(i+6,mstring);
                  j=MapPlanet(mstring);
                  fwrite(&j,1,1,destfile);
                  }
               else if (!strncmp("LUNAR",&wrkline[i],5)) ProcessLunar(i,extsyntax);
               else if (!strncmp("LANDMARK",&wrkline[i],8)) { // landmark
                  c='L';
                  fwrite(&c,1,1,destfile);
                  i=parseAz(i+8,&x);
                  fwrite(&x,1,2,destfile);
                  i=parseDec(i,&x);
                  fwrite(&x,1,2,destfile);
                  i=parseStr(i,mstring);
                  if (mstring[strlen(mstring)-1]!=' ')
                     fwrite(mstring,1,strlen(mstring),destfile);
                  else
                     fwrite(mstring,1,strlen(mstring)-1,destfile);
                  c=0;
                  fwrite(&c,1,1,destfile);
                  i=parseStr(i,mstring);
                  fwrite(mstring,1,strlen(mstring),destfile);
                  ParseState = St_InString;
                  }
               else if (!strncmp("METEOR",&wrkline[i],6)) { // meteor showers
                  c='Y';
                  fwrite(&c,1,1,destfile);
                  }
               else if (extsyntax) ProcessExtensions(i); // lx200 catalog extensions
               else {
                  printf("\nError- unrecognized command line %d:\n %s\n",linenum,srcline);
                  rtncode=1;
                  }
               }
            else { 
               printf("\nUnknown Error line %d:\n %s\n",linenum,srcline);
               rtncode=1;
               }
            break;
         } //switch
      } // while
   printf("Error- missing end statement line %d",linenum);
   c=0xff;
   fwrite(&c,1,1,destfile);
   fclose(srcfile);
   fclose(destfile);
   return(1);
}

void HeapInc( poolposition *p, long inc)
{
    long l;

    l = p->offset;
    while (inc) {
       if (l+inc > 0xffffl) {
          inc -= (0x10000l - l);
          l = 0x8000;
          p->page++;
          }
       else {
          l += inc;
          inc = 0;
          }
       }
    p->offset = (unsigned short) l;
}



void HeapPut( char *c) {

   unsigned short x,i;

   fwrite(c,1,1,destfile);
   if (HeapTop.offset == 0xffff) {
      if (HeapTop.page == 7) {
         printf("Error Heap Overflow");
         HeapTop.offset = 0x0000;
         }
      else {
         HeapTop.offset = 0x8000;
         HeapTop.page++;
         }
      }
   else HeapTop.offset++;
}


void HeapWrite( void *p, unsigned short cnt)
{
    char *pp;

    pp = (char *)p;
    while (cnt) {
      HeapPut(pp++);
      cnt--;
      }
}
    

int main(int argc, char* argv[])
{
   char wrkfname[20];
   char c;
   poolposition NextPos,p;
   long l;
   char buf[256];
   unsigned short len;
   short srcidx;
      
 
   //// open the files ////
   if (argc<2) {
      printf("Usage: bildtour srcfile [srcfile.....]");
      return(1);
      }

   for (srcidx = 1; srcidx<argc; srcidx++) {
      srcfile=fopen(argv[srcidx],"rt");
      if (srcfile==NULL) {
         printf("\nError- cannot open source file : %s",argv[1]);
         }
      else {
         printf("Processing %s\n",argv[srcidx]);
         sprintf(wrkfname,"TEMP%d.ROM",srcidx);
         destfile=fopen(wrkfname,"wb");
         if (destfile == NULL) {
            printf("\nError- cannot open work file: %s\n",wrkfname);
            return(1);
            }
         ProcessTour(1);
         }
         
      }

   // Set Initial Heap Location
   HeapTop.page = 0x60;
   HeapTop.offset = 0x8000;
   destfile=fopen("TOURS.ROM","wb");
   
   /// initialize Head Structure
   c = 0xff;
   while (HeapTop.offset != 0x801b) HeapPut(&c); // not Tour Head
   // place tourhead
   p.offset = swap(0x8079);
   p.page = 0x60;
   HeapWrite(&p,sizeof(p));
   c = 0xff;
   while (HeapTop.offset != 0x8078) HeapPut(&c); // not start end of heap
   c=0;
   HeapPut(&c);

   /// build heap image file ////
   for (srcidx=1; srcidx<argc; srcidx++) {
      sprintf(wrkfname,"TEMP%d.ROM",srcidx);
      srcfile=fopen(wrkfname,"rb");
      fseek(srcfile,0L,SEEK_END);
      l=ftell(srcfile);
      fseek(srcfile,0L,SEEK_SET);
      if (srcidx != (argc-1)) { // compute link ptr
         NextPos = HeapTop;
         HeapInc(&NextPos,l+7);
         }
      else {
         NextPos.offset = 0xffff;
         NextPos.page   = 0xff;
         }
      l -= 16; // compute tour body length
      NextPos.offset = swap(NextPos.offset);
      HeapWrite(&NextPos,sizeof(poolposition));
      c = 0xff;
      HeapPut(&c); // active tag;
      fread(buf,16,1,srcfile);
      HeapWrite(buf,16); // tour name
      len = l;
      len = swap(len);
      HeapWrite( &len,2);
      while (l>=256L) {
         fread(buf,256,1,srcfile);
         HeapWrite(buf,256);
         l -= 256;
         }
      fread(buf,l,1,srcfile);
      HeapWrite( buf,l);
      buf[0]=0;
      HeapWrite(buf,1); // end tag
      close(srcfile);
      remove(wrkfname);
      }
   fclose(destfile);

   return(0);
}

