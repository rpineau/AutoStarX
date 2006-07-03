/*****************************************************************************
*                 AutoStar Global Data Header 
*    by John E. Hoot
*       Software Systems Consulting
*       615 S. El Camino Real
*       San Clemente, CA 92672
*       714/498-5784
*
*    For:   
*     Meade Instruments Corporation
*     16542 Millikan Ave
*     Irvine, CA 92714
*     714/756-2291
*
* (c) 1999 Meade Instrument Corporation.  All rights reserved.
*******************************************************************************/
#define REVISION "Version "

// defintiions for 4501 & 494 Handbox
#define CADET 0
#define GERMAN_MOUNT 0
// definitions of 497 handboxes
#define LX90 1

// Version Location
#define MajorNumber (*((char *)(0x32FC)))
#define dot (*((char *)(0x32FD)))
#define MinorNumber (*((char *)(0x32FE)))
#define MinorLetter (*((char *)(0x32FF)))

#ifndef MAGGLOB
#define MAGGLOB 1

#ifndef WIN32
#ifndef _cplusplus
#define TRUE 0xff
#define FALSE 0x00
#endif
#endif

typedef unsigned char byte;
typedef unsigned short word;

typedef struct {
    byte page;
    word offset;
    } poolposition;

///// Procedures exported from Magellan.c
void globalinit(char ForceInit);
void DisplayState(void);
void ShowSpeed(char speed);
void BackgroundProcess(void);
char ChkStdKeys(char c);
void ProcessSerial(void);
short abs( short x);
void DispRADec( unsigned short ra, short dec);
void showplanet( short idx);
void DoMotorCalib();

/////// Telescope state machine definitions for Mode variable///////////////////////
#define SM_RADec        0x01
#define SM_Time         0x02 
#define SM_AltAz        0x03
#define SM_Accessory    0x04
#define SM_Battery      0x05
#define SM_Timers       0x06
#define SM_DateSite     0x07 
#define SM_Lat          0x08
#define SM_Lon          0x09
#define SM_ModeMask     0x3f
#define FocuserEnable   0x40 
#define SlewInhibit     0x80

////////////// controller key/char codes ///////////
#define UPKEY        '^'
#define DNKEY        'v'
#define MODEKEY      0x09
#define LONGMODE     0x0b
#define ENTERKEY     0x0d
#define LONGENTER    0x0a
#define MESSIERKEY   '9'
#define STARKEY      '6'
#define NGCKEY       '3'
#define USERKEY      '5'
#define ICKEY        '2'
#define X2KEY        '0'
#define X8KEY        '1'
#define X16KEY       '4'
#define X32KEY       '7'
#define FINDKEY      'G'
#define GOTOKEY      'G'
#if CADET
#define INFOKEY      0x11 // was '?'
#else 
#define INFOKEY      '?'
#endif
#define SPEEDKEY     0x10  // cadet only
#define ARROW        0x7e
#define CHECK        0xe8
#define DEGREES      223


/////////////////////// Serial State constants ////////////////////////
#define SerXmtMask     0x0F
#define SerXmtRA       0x01
#define SerXmtDec      0x02
#define SerXmtGMT      0x03
#define SerXmtLMT      0x04
#define SerXmtLat      0x05
#define SerXmtLon      0x06
#define SerXmtAlign    0x07
#define SerXmtDate     0x08
#define SerXmtOK       0x09
#define SerRcvMask     0xF0
#define SerRcvIdle     0x10

/////// conversion constants ///////////////
#define sectorad 4.848136811E-6F
#define mintorad 0.0002908882087F
#define degtorad 0.01745329252F
#define cputosidereal 2.986872869E-7F
#define rad_per_tick 0.001533980788F
#define pi 3.141592654F
#define pi2 6.283185307F
#define halfpi 1.570796327F
#define secsperrev  1296000
#define AltAzCordStop 972000
#define Long90  324000L
#define Long180 648000L
#define Long270 972000L
#define Long360 1296000L
#define Long450 1620000L
#define Bighard 0x7f800000
#define SATSTEPS 100
#define AlgolPeriod 2.86732442

//////Packed Boolean Constants Mask Positions for State Flags /////////////
#define AzElAligned      0x01
#define Unused1          0x02
#define AlarmSet         0x04
#define TimerOn          0x08
#define ForkFlipBit      0x80   
#define PicBusy          0x10
#define NSReverse        0x20
#define EWReverse        0x40
#define NHemi            0x80

////// Motor Flags ///////////////
#define GuideUpdate      0x01         // set every 2 seconds                                       
#define Tracking         0x02         // set if aligned in !land mode                              
#define LandTarget       0x04         // if current target is a landmark                           
#define HiPrecision      0x08
                                                                                                   
#define WasSlewed        0x10         // set whenever the user slews to a new position manually    
#define PressedAz        0x40         // Az backlash tracking flag                                 
#define PressedEl        0x20         // El Backlash tracking flag                                 
#define BacklashComp     0x80         // Enables backlash calibration                              

// Start Flags //

#define WarnOn           0x01
#define StartOn          0x02
#define Parkwrap         0x04                            
                                                                                                   

///////// Accessories //////////
#define FOCUSER          0x01
#define RETICLE          0x02
#define GPS              0x04
#define AUDIO            0x08
#define CARTRIDGE        0x10

typedef struct {
    word Larger;                 // Min obj size (in minutes)
    word Smaller;                // Max obj size (in minutes)
    short Brighter;              // Max Magnitude * 10 
    short Fainter;               // Minimum Manitude
    word Above;                  // Min Elevation degrees
    word KindMask;               // Type Mask
    short NextIC;                // SearchIndexes
    short NextNGC;
    short NextDeep;
    short LastDeep;
    } BrowseType;
extern BrowseType Browse;    

typedef struct {
   unsigned char hr;
   unsigned char mins;
   unsigned char secs;
   } TimeStruct;
//extern char outnum;

#if CADET
/// 494 Autostar

extern char azenclsb;            // az encoder count msb
extern char azencmid;            // az encoder count mid
extern char azencmsb;            // az encoder count lsb
extern char elenclsb;            // el encoder count msb
extern char elencmid;            // el encoder count mid
extern char elencmsb;            // el encoder count lsb
extern char motorsleep;

extern char inrasleep;           // ra motor in sleep mode
extern char indecsleep;          // dec motor in sleep mode
extern char Poscnt;              // position count to check for motor position
extern char bricnt;              // Backlight counter
extern char concnt;              //contrast count
extern char Bksave;              // Back light save value

extern char highcnt;
extern char lowcnt;
extern char pwmcnt;
extern char decstallcnt;
extern char rastallcnt;

extern char dlyaux;

extern char meadetest;

extern char auxbuff[34];
extern char buffcnt;
extern char startbuff;
extern char endbuff;
extern char serialmode;

extern long lastaux;
extern short auxcheck;
#else

extern char tempout;

#endif

extern char allowAuxchk;
extern char motorversion;
extern char lineone[33];         // used to save line 1 
extern char focfromspeed;
extern char DbasePage;
extern char curkeystroke;
extern char inradec;
extern char inaltaz;
extern char polealign;
extern char focscroll;

extern long lastdec;
extern long lastra;

extern char LX90scope;                      // working with LX90

///// Current Scope Info ////////
extern long ElOffset;           // Encoder offset from 0 in encoder ticks 
extern long AzOffset;           // Encoder offset from 0 in encoder ticsj
extern long AzZeroOffset;       // Encoder offset from 0 degrees
extern long LastAz;             // keep track of cord rap
extern short cnt270;            // cord wrap state
extern long Timer;              // Photographic Timer 
extern long Alarm;              // Event Alarm 
extern unsigned short lasttime; // last time scope position  1= 1 sec  
extern long cputime;            // used to keep precise tracking time 1 tick = 4.096 msec 
extern long  powerdowntime;     // time since last keystroke
extern char LightsOn;           // keyboard sleep timer
extern unsigned long  HT;       // cputime since last alignment 
extern char   mm,dd;            // current date 
extern short yy;                // current year
extern unsigned short GST;      // GST at midnight in minutes 
extern char RAf;                // new RA
extern char Decf;               // new Dec

/// Backlash
extern char azmax;             // number of counts at max speed
extern char aztime;            // time for current step
extern char azrevdir;          // direction
extern short azpos;            // current step change
extern char sazmax;
extern short sazpos;
extern char elmax;             // number of counts at max speed
extern char eltime;            // time for current step
extern char elrevdir;          // direction
extern short elpos;            // current step change
extern char selmax;
extern short selpos;
extern char nokey;

///// Effective user location /////
extern short Lat;               // current effective latitude in arc mins 
extern float LST;               // local sidereal time in rads at time of alignemnt   
extern unsigned long  HT;       // cputime since last alignment or sync (LST relative) 
extern float InitLST;           // Used for determining horizons
extern long  InitHT;            // Used for determining Sidereal time

/***** Telescope Values ****/
extern float RA;                // current RA expressed in rads 
extern float Dec;               // current Dec expressed in rads 
extern long Az;                 // Azimuth Encoder Reading in secs of arc 
extern long El;                 // Elevation Encouder Reading in secs of arc
extern word TargetRA;           // Positon of Last Object Retrieved in arc min 
extern short TargetDec;         // Position of Last Object Retreived in arc min 
extern unsigned char TargetDiam;// Anglular size of target in arc mins
extern char TargetFlags;        // Flags about target
extern word CurRA;              // Current scope position
extern short CurDec;            // Current Scope Position
extern long LastSlewTime;       // Time of last telescope move
extern char InSlew;             // Interlock for serial control
extern char PosChanged;         // Interlock on guide update
extern char AlignQuality;       // align quality indicator 0..100 + MSBit = ForkFlipBit 
extern short TrackingRate;      // Adjustment to sidereal rate

/////////////// Target Flags ////////////////
#define TargetDeep       0x01   // target is deep sky obj

typedef struct {                // Alignment Info 
   long az;    
   long el;
   long timehack;
   word RA;
   short Dec;
   word  idx;
   } AlignStructType;
extern char AlignQuality;       // align quality indicator 0..100 

extern unsigned char Accessory; // Flags for Accessories 
typedef struct {
   char msb;
   short lsw;
   } ScopeRate;
extern ScopeRate SlewRate;           // Handbox slew speed 
extern ScopeRate TrackRA;            // Sidereal Tracking Rate 
extern ScopeRate TrackDec;           // Sidereal Tracking Rate 
extern long lastEl;                  // last read of getencoder(0)
extern long Elback;                  // where we are in backlash
extern long validEl;                 // last read the "moved" the scope
extern long lastAz;                  // last read of getencoder(0)
extern long Azback;                  // where we are in backlash
extern long validAz;                 // last read the "moved" the scope
extern char firsttrack;              // control ground bounce
extern char MotorFlags;              // LastAzWest:LastElSouth:BacklashComp:WasSlewed: Spool[4] 
extern char ModifyDir;               // Inidicates Backlash Flags should be updated
extern short MotorTimer;             // Used when dampening scope movement 
extern char MotorStalls;             // Count of consecutive Motor Stalls
extern char Decerrbad;               // pic encoder error cnt ck
extern char RAerrbad;                // pic encoder error cnt ck
extern char RAInfo;                  // motor rev states
extern char DecInfo;

extern long daycnt;

extern long azspeed;
extern long elspeed;

extern char quitmove;           // terminate slew
extern char curfocspeed;


///////State Machine Values /////////
typedef struct menu_struct {
   word  line1;
   word  line2;
   struct menu_struct *nextstate;
   struct menu_struct *prevstate;
   struct menu_struct *parentstate;
   struct menu_struct *childstate;
   void (*action)(void);
   } Menu;

enum MagModes {ScopeMode, MenuMode, OnTour};
extern enum MagModes CurMode;     // Pointer To Current MenuMode 
extern char ModeSwap;             // Signals Chage to Scope Mode 
extern Menu *CurState;            // Pointer to Active Menu 
extern unsigned char Mode;        // Current Mode Value 
extern unsigned char Entry;       // Current Entry Index 
extern char StateFlags;           // AlignMode : TimerOn : AlarmSet 

/////////////// LX200 Declarations ///////////////////////
#define ProductName "Autostar"
#define ProductionDesignator "A"
#define VersionDate __DATE__
#define VersionTime __TIME__
#define MAX_CMD_BUFR_SIZE 32
#define LX200FocusFast 0x04
#define LX200FocusSlow 0x01
#define LX200FocusStop 0x00
#define LX200FocusIn   0x00
#define LX200FocusOut  0x80

#ifdef CADET
extern char serbuf [ 40 ];               // Serial Buffer for Cadet.
extern int  outser;                      // index into serbuf for Cadet.
#endif

extern char LX200Cmd[MAX_CMD_BUFR_SIZE]; // temp buffer for parsing LX200 cmd
extern char SerialRcvQ[64];              // Serial Request Queue 
extern char SerialRcvHead;               // Head ptrs
extern char SerialRcvTail;               // Tail ptrs 
extern char SerialRcvCnt;                // Characters in the Queue
extern char LX200idx;                    // pointer into the lxdecode register
extern char SerBuffer[32];               // Working Serial Formating buffer
extern char LongFormat;                  // Bool state of the I/O format mode for the LX200 commands.  Normally false.
extern char LX200SlewNS;                 // If TRUE indicates we are in a Move command.  If cleared we have stopped.
extern char LX200SlewEW;
extern char Version[];
extern char VersionUp[];
extern char VersionLow[];
extern char LongFormat;                  // Bool state of the I/O format mode for the LX200 commands.  Normally false.
extern char QualityType;                 // Index into the Find Quality array.
extern char Lx200Field;                  // The current field of view for an LX200.
extern char Lx200Smaller;
extern char Lx200Larger;
extern float Lx200Brighter;
extern float Lx200Fainter;
extern unsigned char  LX200FocusDirection; // How fast we focus.
extern unsigned char  LX200FocusSpeed;     // What speed we focus at.
extern char PrevChar;                    // Previous character in the input stream.
extern char LXRentered;                  // Kludge for rentrancy during slewing                                   
extern char FindType [ 6 ];              // Current Find type string.
extern char EndClarksVars;               // Protection byte.  Remove when debugged if you need the byte.


////// Handbox State /////////
extern char DspContrast;
extern char BkLite;             // BkLite level + Hemisphere & Rerverse Switches 
extern char cursor;             // current cursor 
extern char NoSync;             // Sync Interlock
extern short MeanVoltage;       // Moving Average Voltage
extern char Lite;               // Handbox Lite
extern char LowBatteryOn;       // Alarm on Low Battery
extern unsigned char KeyMask;   // locks out move keys
extern unsigned char KeyState0; // key flags
extern unsigned char KeyState1; // key flags
extern unsigned char KeyState2; // key flags
extern unsigned char LastKey2;  // Key Motion Changes
extern unsigned char repeated;  // flag for keyboard processing
extern unsigned char lastkeystroke; // used for spiral searching
extern char FocusSpeed;         // Focuser Space
extern char donesleep;          // Interlock with Picio
extern char time_sleep;         // facilitate wake up on alarm
extern char motor_bits;         // Support Batter state
extern char GotoComplete;       // Count Down timer on gotos for Beep
extern char BusyPos;            // Busy spinner state
extern char IsBusy;             // Show spinner flag
extern char BusyCnt;            // Delay between spinner 1/8 turns

//// scroll text //////
extern int scrollpos;
extern long lastscroll;
extern char scrollspeed,inscroll;


///////////////////////////////////////////////////////////
/////////////////// Database Variables ////////////////////
///////////////////////////////////////////////////////////
#define CONSTELLCNT 88
#define GREEKLETTERS 24

///// constant arrays in magmenus.c /////
extern char Languages[];
extern char Japanese[];
extern short yrdays[13];
extern const char *ConstAbbrev[CONSTELLCNT];
extern const char *Greek[GREEKLETTERS];
extern const char *GreekAbbrev[GREEKLETTERS];
extern const char StarClass[];
extern const char *StarType[];

/////  Star Records ////////////
typedef struct {
   byte Bayerid;                  // bayer code ' ' non 1=alph..24=omega          
   byte Constel;                  // constellation identifier                     
   poolposition CommonName;       // Index to string pool description             
   word RA;                       // expressed in arc minutes                     
   short DEC;                     // epxressed in arc minutes                     
   byte MAG;                      // expressed as 10*(mag+5)                      
   byte Spectra;                  // bits 0..3 Color =  O B A F G K M N S R       
                                  // bits 4..7 Class = I .. IV                    
   byte TypeClass;                // I,II,III,IV,V                                
   byte SAO_H;                    // upper 8 bits of SAO Number                   
   word SAO_L;                    // Lower 16 bits of SAO Number                  
//   short B_V;                     // (b-v)*100;                                   
   word Dist;                     // light_yr/10                                  
   word Sep;                      // Dbl Separation in seconds/10                 
   poolposition Notes;            // Other Decriptive Text                        
   } star;

typedef struct{
   byte SAO_H;
   word SAO_L;
   word RA;
   short Dec;
   byte Mag;
   byte Spectra;
   } SAOType;

typedef struct {
   long SAOnum;                   // id of star
   poolposition Lore;             // decription of planets
   } WithPlanetsType;

///////////  Variable Stars Index ///////////////////////
typedef struct {
   word StarIdx;                  // Index into star table for balance of info    
   poolposition VarName;          // Pool Index of Variable Name                    
// word Period;                   // Variable Period*10 in Days if >0 else -yrs   
// byte Max;
// byte Min;
   } variable;

////////// Contellation Records //////////////////////
typedef struct {
    short Member[10];           // index of the 10 brightest members
    poolposition Lore;          // index into the string pool of the lore
    word EastRA;                // in arc minutes
    word WestRA;                // in arc minutes
    short NorthDec;             // in arc minutes
    short SouthDec;             // in arc minutes
    } constelltype;             // in arc minutes
   

//////////////////   Deep Sky Objs ////////////////////////////
enum deepkind {black_hole,diffuse_nebula, dark_nebula, double_star,
               elliptical_galaxy, globular_cluster, irregular_galaxy,
               open_cluster, planetary_nebula, quasar, spiral_galaxy, 
               galaxy, unknown};
               
typedef struct {
    short NGC;                 // ngc index (negative => IC) 0000=> neither
    char CaldwellMessier;      // messier number or 0
    word RA;                   // expressed in arcminutes             
    short Dec;                 // in arch minutes                                
    char Width;                // in mins/2 (secs if pl)
    char Kind;                 // Kind
    char Magnitude;            // (brightness * 10)+55;
    char Constell;             // Constellation Number
    poolposition Lore;         // Pointer to string pool;
    } deeptype;



//////////////////// Planet Record ///////////////////////////////
typedef struct {
   float Period;
   float LonAtEpoch;
   float LonAtPer;
   float Eccen;
   float MajAxis;
   float Inclination;
   float LonAscNode;
   float Diameter;
   float Mag;
   short Epoch_y;
   char  Epoch_m;
   char  Epoch_d;
   } PlanetInfoRec;
extern PlanetInfoRec PlanetInfo[];

typedef struct {
   poolposition Name;    // National Language planet name, do not move!!!!!
   short Mass;           // in 1/100ths of earth masses or 10000th of earth masses
   word Diameter;        // in 1/100ths of earth
   word Distance;        // 1/100 au
   word Day;             // earth days * 100;
   word Year;            // in earth years * 100 ;
   word Gravity;         // *100
   short DayTemp;
   short NightTemp;
   char Moons;
   poolposition Lore;
   } planettype;
   
//////////////////////////// Geographic Record /////////////////////
typedef struct {
   short lat;               // lat in arc mins 
   short lon;               // lon in arch mins 
   char zone;               // time zone (0..24) 
   poolposition cityname;
   poolposition state;
   } CityType;

////////////////////////// Event Structures //////////////////////////   
typedef struct {
    poolposition   Name;            // Name of shower
    char           Duration;        // Duration in days
    char           Month;           // Month of peak epoch 1992
    char           Day;             // Day of peak epcoh 1992
    char           UT;              // Hour of Peak
    unsigned short RA;              // arc minutes
    short          Dec;             // arc minutes
    char           Rate;            // meteors per hour
    poolposition   Comet;           // source comet
    } MeteorShowerType;

typedef struct {
   short Year;
   char  Month;
   char  Day;
   short UTC;
   char  Kind;
   char  Saros;
   short Gamma;
   short PenMag;
   short UmbMag;
   short UmbDur;
   short PenDur;
   short RA;
   short Dec;
   } LunarEclipseType;
   
typedef struct {
   short Year;
   char  Month;
   char  Day;
   short UTC;
   char  Kind;
   char  Saros;
   short Gamma;
   short Magnitude;
   short Latitude;
   short Longitude;
   char  Altitude;
   short Width;
   short Duration;
   } SolarEclipseType;

typedef struct {
   poolposition Name;      // name of eyepiece family
   short FocalLen;         // flocal len in mm*10
   unsigned char Field;    // apparent field of view (degrees)
   } EyePieceType;
   
extern unsigned short catalogidx;  // catalog record index    
extern char objectxt[20];          // text description of object 
extern char keyword[21];           // text description of object 
extern char conststring[80];       // holding location for string constants
extern poolposition HeapTop;       // location of free heap memory 
extern poolposition NextToken;     // pointer to the next token after last decoded item
float arctan(float y, float x);    // arctan that handle all 4 quadrants      
float arcsin(float x);             // rectified

///////////// hyperlink stack ////////////////////////////
#define MAXLINKS 12  
#define MAXUSERSIZE 0x3300
extern char linkdepth;                    // pointer to next avail stack frame 
extern poolposition linkptr[MAXLINKS];    // stack of memory pointers 
extern char linkclass[MAXLINKS];          // stack of link class characters 
extern char TutorialOn;                   // Flag for startup
extern char DecompBuffer[];               // location for decompression/garbage collection
#define CollectBuffer DecompBuffer

//////////////////////////////////////////////////////////////////////////
////////////////////////// Processor EEProm Data /////////////////////////
//////////////////////////////////////////////////////////////////////////

// User Information
//#define U_UsePin   0x01      // Pin number required 
#define U_24clk    0x01      // 24 hour clock 
#define U_Parked   0x02      // Scope was parked at end of last use 
#define U_HasPier  0x04      // Scope mounted on pier 
#define U_Polar    0x08      // Scope is equatorially mounted 
#define U_German   0x10      // German Mount
#define U_SkipCk   0x20      // skip check on german mounts
//#define U_Smart    0x10      // Smart flexure correction enabled 
//#define U_Train    0x20      // Train flexiure Map 
#define U_Daylight 0x40      // Daylight Savings active
#define U_Land     0x80      // Scope is being used for terrestrial viewing 

extern char UserFlags;
extern char UserFirstName[16];    // customer inf 
extern char UserLastName[16];
extern short UserPinNum;
extern char UserSt1[30];
extern char UserSt2[30];
extern char UserCity[16];
extern char UserState[16];
extern char UserPostCode[16];
extern char UserMon, UserDay;
extern short UserYear;
extern char UserCurrentSite;
extern char UserLastValidSite;
extern char UserSerialNum[16];

// Site Information
#define MAXUSERSITE 5
typedef struct {
   short Latitude;         // in arc min 
   short Longitude;        // in arc min 
   word NearestCity;       // offset into city db 
   char TimeZone;          // offset from UTC 
   char SiteName[16];      // User Site name 
   } SiteType;
   
extern SiteType UserSites[MAXUSERSITE];

// Handbox & Scope Values
extern char   EE_Contrast;      // Last LCD Contrast Setting
extern float  EE_ElRatio;       // scope data 
extern float  EE_AzRatio;     
extern short  EE_ElBacklash;    // Backlash correction 
extern short  EE_AzBacklash;    // Backlash corrention 
extern char   EE_RALED;         // encoder current setting
extern char   EE_DecLED;        // encoder current setting
extern short  EE_FocalLen;
extern char   EE_ScrollRate;    // Handbox Scroll Rate
extern char   EE_Backlite;      // Remember Backlighting
extern short  EE_MaxEl;         // Elevation limit for AltAz scopes
extern short  EE_MinEl;         // Elevation linit for AltAz Scopes
extern char   EE_Model;         // Index of telescope model;
extern char   EE_mm;            // Last Month index
extern char   EE_dd;            // Last Day index
extern short  EE_yy;            // Last Year index
extern char   EE_Azper;         // percent basklash remove az
extern char   EE_Elper;         // percent basklash remove el
#define EE_Reset (*((char *)(0xB7FF)))

// Park Values
extern short ParkLat;
extern short ParkAzCorrection;

extern char  EE_Startflags;

//#define SMARTMAPSIZE 29
//extern short EE_AzMap[SMARTMAPSIZE];   // for elevations 15..75, in 30 degree angle increments  + zenith 
//extern short EE_ElMap[SMARTMAPSIZE];   // for elevations 15..75, in 30 degree angle increments  + zenith 
                                       // value is the correction in arc secs +/- 

//////////////////////////////////////////////////////////////////////
/////////////// Flash Heap Page Organization /////////////////////////
//////////////////////////////////////////////////////////////////////
#define HeaderPage      0x06     
#define CollectPage     0x1E
#define HeapTag         0x8000      
#define UserObjHd       0x8001
#define LandmarkHd      0x8004
#define TourHd          0x8007
#define AsteroidHd      0x800a
#define CometHd         0x800d
#define SatelliteHd     0x8010
#define EndHdsTag       0x8013

typedef struct {
      poolposition Next;      // Pointer to next record of this type on heap 
      char Active;            // FF = Active, 00=Deleted, FE = No Delete, FC = No Edit 
      char Name[16];
      word RA;
      short Dec;
      short Size;
      short Mag;
      char Endtag;            // Alway 0 
      } UserObjType;

typedef struct {
      poolposition Next;      // Pointer to next record of this type on heap 
      char Active;            // FF = Active, 00=Deleted 
      char Name[16];
      word Az;
      short El;
      char Endtag;            // Alway 0 
      } LandmarkType;
      
typedef struct {
      poolposition Next;      // Pointer to next record of this type on heap 
      char Active;            // FF = Active, 00=Deleted 
      char Name[16];
      char  Epoch_mm;
      char  Epoch_dd;
      short Epoch_yy;
      float Eccentricity;     // e       
      float SemiMajorAxis;    // a       in AU 
      float Inclination;      // Incl.   Orbital Inclination in degrees 
      float LongAscNode;      // wa
      float ArgPerhelion;     // Peri.   in degrees 
      float MeanAnomaly;      // M      
      float AbsMag;           // H       Absolute Magnitude 
      float Slope;            // G       See "Application of Photmetric Models to Asteroids", Bowell 
      char Endtag;            // Alway 0 
      } AsteroidType;
      
typedef struct {
      poolposition Next;      // Pointer to next record of this type on heap 
      char Active;            // FF = Active, 00=Deleted 
      char Name[16];
      char  Epoch_mm;
      char  Epoch_dd;
      short Epoch_yy;
      float Perhelion;        // q       in AU 
      float Eccentricity;     // e       
      float ArgPerhelion;     // Peri.   in degrees 
      float LongAscNode;      // omega
      float Inclination;      // Incl.   Orbital Inclination in degrees 
      float AbsMag;           // H       Absolute Magnitude 
      char Endtag;            // Alway 0 
      } CometType;
      
typedef struct {
      poolposition Next;      // Pointer to next record of this type on heap 
      char Active;            // FF = Active, 00=Deleted 
      char Name[16];
      short EpochYear;
      float EpochDay;
      float Inclination;      // Incl.   Orbital Inclination in degrees 
      float RAAN;               
      float Eccentricity;     // e       
      float ArgPerigee;
      float MeanAnomaly;
      float MeanMotion;
      char Endtag;            // Alway 0 
      } SatelliteType;

typedef struct {
      poolposition Next;      // Pointer to next record of this type on heap 
      char Active;            // FF = Active, 00=Deleted 
      char Name[16];
      short Len;              // block len: note - hi order bit is flag for extended tours
      char body[1];           // actual length in Len field
      char Endtag;            // Alway 0 
      } TourType;
      
#endif

