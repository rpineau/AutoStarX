/*
	linux command line flasher for autostar
	
*/

#include "serial.h"
#include "lin_autostar.h"

int main(int argc, char **argv)
{
	int err;
	bool wrong;
	
	if(argc<3)
		{
		printf("Usage:\n");
		printf("      lin_autostar /dev/ttySx romfile\n");
		printf( "      /dev/ttySx is the serial port on which the autostar is connected\n");
		printf("      romfile is the autostar ROM to be flashed (like Build33Ef.ROM)\n");
		exit(0);
		}

	signal(SIGINT, finish);
	// lib curses init
	initscr();      /* initialize the curses library */
	keypad(stdscr, TRUE);  /* enable keyboard mapping */
	nonl();         /* tell curses not to do NL->CR/NL on output */
	cbreak();       /* take input chars one at a time, no wait for \n */
	noecho();       /* don't echo input */

	
	// check if the first arg is a device and a serial port
	if(stat(argv[1],&Stat))
		{
		mvprintw(16,0,"%s doesn't exist\n",argv[1]);
		refresh();
		finish(0);
		}
	mvprintw(0,0,"ROM file :");
	mvprintw(1,0,"ROM file version :");
	mvprintw(2,0,"Device Type :");
	mvprintw(3,0,"ROM version :");
	refresh();

	mvprintw(0,0,"ROM file : %s\n", argv[2]);
	if(!(mROMFileHandle=open(argv[2],O_RDONLY)))
		{
		mvprintw(16,0,"Error opening %s\n",argv[2]);
		refresh();
		finish(0);
		}

	refresh();
	
	newRom=(ROM *)malloc(sizeof(ROM));
	fstat(mROMFileHandle,&Stat);
	read(mROMFileHandle,newRom,Stat.st_size);
	close(mROMFileHandle);

	switch(newRom->key)
		{
			case 0x00000000:    // autostar 495 & 497
					romType=1;
					break;
			case 0x00028000:    // autostar II
					romType=2;
					break;
			default :           // unknown rom file
					romType=0xffff;
					break;
		}
	
	mvprintw(1,0,"ROM file version : %c%c%c%C",newRom->version[0],newRom->version[1],newRom->version[2],newRom->version[3]);
	refresh();
	
	AutoStarConnect(argv[1]);

	if(deviceType!=0xFFFF && romType!=0xFFFF)
		{
		if(deviceType==romType)
			{
			wrong=false;
			switch(deviceType)
				{
				case 1: // autostar 495 & 497
					
					Flash();
					break;
				
				case 2: // autostar II
					break;

				default:
					wrong=true;
					break;
				}
			}
		}
	else
		wrong=true;
		
	if(wrong)
		{
		mvprintw(16,0,"Wrong device or ROM file!\nThe ROM file doesn't correspond to the autostar you're trying to flash.\nPlease verify your ROM file.");
		refresh();
		}
	
	AutoStarReset();
	AutoStarDisconnect();	
	finish(0);
	return 0;
}


static void finish(int sig)
{
    endwin();
    exit(0);
}


// 
// connect to the AutoStarX at 9600 bauds and get device type and ROM version
//
void AutoStarConnect(char *serialPort)
{
    unsigned long index;
	bool bSafeLoad;
    char bsdPath[255];
    Byte ioBuffer[64];
    Byte cmd[2];
	int i;


    // open port at 9600  
    if(!OpenSerialPort(serialPort, 9600))
        { // error opening port
        mvprintw(16,0,"Communication error !\nThe serial port is used by another application\n");
        refresh();
        finish(0);
        }
        
    bConnected=true;
    bSafeLoad=true;
	// flush all data before starting
	ReadData(ioBuffer,64);

    for(i=0;i<11;i++)
        {
        // check AutoStarX status : cmd=0x06
        cmd[0]=0x06;    // ^F (1 byte response)
        cmd[1]=0;
        if(!SendData(cmd,1))
            {
            AutoStarDisconnect();		
            mvprintw(16,0,"Write error !\n");
            refresh();
            finish(0);
            }
        usleep(100000);
        if(!ReadData(ioBuffer,1))
            {
            AutoStarDisconnect();
            mvprintw(16,0,"Read error ! the Aurostar isn't responding\n");
            refresh();
            finish(0);
            }
 
        if(ioBuffer[0]!='?')
            {
            bSafeLoad=false;
            break;
            }            

        } 

    if(ioBuffer[0]!='D' && !bSafeLoad) // not in download mode
        {
        // switch to download mode
        cmd[0]=0x04;    // ^D (1 byte response)
        cmd[1]=0;
		if(!SendData(cmd,1))
			{
			AutoStarDisconnect();
			mvprintw(16,0,"Write error !\n");
			refresh();
            finish(0);
			}
		usleep(500000);	
		if(!ReadData(ioBuffer,1))
			{
			AutoStarDisconnect();
			mvprintw(16,0,"Read error ! the Aurostar isn't responding\n");
			refresh();
            finish(0);;
			}

        }
    // otherwize we are already in download mode !!!!  most probably in safe load
    
    // get device type (495, 497, ....)
    cmd[0]='T';    // ask for AutoStarX type 0x0F=497 0x0A=495 0x05=??? (1 byte response)
    cmd[1]=0;
    if(!SendData(cmd,1))
		{
		AutoStarDisconnect();
		mvprintw(16,0,"Write error !\n");
		refresh();
        finish(0);
		}
	usleep(500000);
	if(!ReadData(ioBuffer,1))
		{
		AutoStarDisconnect();
		mvprintw(16,0,"Read error ! the Aurostar isn't responding\n");
		refresh();
        finish(0);
		}
		
    // set the device type control to the AutoStarX type
    switch(ioBuffer[0])
        {
        case 0x0f:
        	mvprintw(2,0,"Device Type : 497\n");
            deviceType=1;
            break;
            
        case 0x0A:
        	mvprintw(2,0,"Device Type : 495\n");
            deviceType=1;
            break;
        default:
        	mvprintw(2,0,"Device Type : Other\n");
            deviceType=0xFFFF;
            break;
        }

     refresh();   
     
    if(!bSafeLoad)
        {
        // get ROM version
        cmd[0]='V';    // ask for ROM version (4 bytes response)
        cmd[1]=0;
        if(!SendData(cmd,1))
            {
            AutoStarDisconnect();
            mvprintw(16,0,"Write error !\n");
            refresh();
            finish(0);
            }
        usleep(500000);    
        if(!ReadData(ioBuffer,4))
            {
            AutoStarDisconnect();
            mvprintw(16,0,"Read error ! the Aurostar isn't responding\n");
            refresh();
            finish(0);
            }
        
        // set the rom version control to the AutoStarX current version
        mvprintw(3,0,"ROM version : %c%c%c%C\n",ioBuffer[0],ioBuffer[1],ioBuffer[2],ioBuffer[3]);
        }
    else
        mvprintw(3,0,"ROM version: SAfe Load\n");

    refresh();
    
}

void AutoStarDisconnect()
{
    Byte cmd[2];
    
    cmd[0]='I'; // Initialize .. proper way of exiting download mode (0 byte response)
    cmd[1]=0;
    SendData(cmd,1);
    CloseSerialPort();
    bConnected=false;
}


void AutoStarReset()
{
    Byte cmd[16];
    int timeout;
    Byte ioBuffer[64];
    
    cmd[0]='I'; // Initialize .. proper way of exiting download mode (0 byte response)
    cmd[1]=0;
    SendData(cmd,1);

    // wait for the "X" from the autostar boot (~ 10 secondes)
	timeout=0;
    while(!ReadData(ioBuffer,1))
		{
        timeout++;
        if(timeout==15) // 15 secondes just in case
            {
            mvprintw(16,0,"Communication error ! The autostar doesn't respond to the reset command");
            refresh();
            AutoStarDisconnect();
            finish(0);
            }
		}


    // we flush the port to be sure
    ReadData(ioBuffer,16);
    
    // get ROM version :GVN#
    cmd[0]=':';    // ask for ROM version (5 bytes response)
    cmd[1]='G';
    cmd[2]='V';
    cmd[3]='N';
    cmd[4]='#';
    
    if(!SendData(cmd,5))
		{
		AutoStarDisconnect();
        mvprintw(16,0,"Communication error ! Write error !\n");
        refresh();
        finish(0);
		}
		
	if(!ReadData(ioBuffer,5))
		{
		AutoStarDisconnect();
        mvprintw(16,0,"Communication error ! Read error !\n");
        refresh();
        finish(0);
		}
		
    // set the rom version control to the AutoStarX current version

	mvprintw(3,0,"ROM version: %c%c%c%C\n",ioBuffer[0],ioBuffer[1],ioBuffer[2],ioBuffer[3]);
	refresh();

}


void Flash()
{
	Byte doublepages;
    Byte page;
    int i,j;
    unsigned long progress;
	Byte ioBuffer[64];
	Byte cmd[70];	// 5 byte command + 64 byte of data maximum
	unsigned short addr;
	int erase_dbl_page;
    Byte ff_data[BLOCKSIZE];
	
	memset(ff_data,0xff,BLOCKSIZE);
    progress=0;

#ifndef __TEST

    // we start on page 2 so it's double page 1 and write 2 pages as we need to erase a double page each time
    for( doublepages=1;doublepages<16;doublepages++)
        {
        // update the progress bar and status
	    mvprintw(4,0,"writing page %u/30\tbyte : %lu \n",page,progress);
		refresh();
		// we need to test if the page is full set to $FF 
		// and if yes not erase it a go to the next double page
		erase_dbl_page=0;
		for(i=0;i<32768;i+=BLOCKSIZE)
			{
			erase_dbl_page+=memcmp(&(newRom->pages[doublepages*2][i]),ff_data,BLOCKSIZE);
			erase_dbl_page+=memcmp(&(newRom->pages[doublepages*2+1][i]),ff_data,BLOCKSIZE);
			if(erase_dbl_page)
				break;
			}
			
		if(!erase_dbl_page)
            {
            progress+=(32768*2);
			continue;
            }

#ifdef __COM_DEBUG
                printf("command = E doublepage =%X\n",doublepages);
#endif

        // erase double page
		cmd[0]=0x45;    // E (1 byte response)
		cmd[1]=doublepages;
		cmd[2]=0;
		if(!SendData(cmd,2))
			{
			AutoStarDisconnect();
			mvprintw(16,0,"Communication error !\nThe autostar isn't responding to the flashing commands.\nCheck all connections and restart the autostar in safe load mode (press Enter end down close to ? and power on the autostar)\n");
			refresh();
			finish(0);
			}
#ifdef __COM_DEBUG
                printf("command = E doublepage =%X command sent\n",doublepages);
#endif
        usleep(1000000); // a second
        
		if(!ReadData(ioBuffer,1))
			{
			AutoStarDisconnect();
			mvprintw(16,0,"Communication error !\nThe autostar isn't responding to the flashing commands.\nCheck all connections and restart the autostar in safe load mode (press Enter end down close to ? and power on the autostar)\n");
			refresh();
			finish(0);
			}

#ifdef __COM_DEBUG
                printf("command = E doublepage =%X result= %c\n",doublepages,ioBuffer[0]);
#endif
			
		// check if answer is "Y"
		if(ioBuffer[0]!='Y') // page has been erased
			{
			// if not we don't try to write
	        mvprintw(4,0,"writing page %u/30\tbyte : %lu \n",page,progress);
	        refresh();
			continue;
			}

#ifdef __COM_DEBUG
                printf("command = E doublepage =%X   ERASE OK\n",doublepages);
#endif

		
		for(j=0;j<2;j++)
			{
			// start write page
			addr=0x8000;
			page=doublepages*2+j;
			// we write "BLOCKSIZE" byte each time
			for(i=0;i<32768;i+=BLOCKSIZE)		
				{
                // update the progress bar and status								
                mvprintw(4,0,"writing page %u/30\tbyte : %lu \n",page,progress);
                refresh();

				// we need to avoid the 512 byte of eeprom at B600-B7FF
				// it should be mark by all FF in the file but testing for it is safer
				if( (addr>0xB5FF) && (addr<0xB800) )
					{
					// set addr after the eeprom, increment i and progress by 512
                    addr=0xB800;
                    i+=512-BLOCKSIZE;   // new value for i
                    progress+=512;
					continue;
					}

#ifdef __COM_DEBUG
                printf("command = %W page =%X addresse=%02X%02X size = %02X\n",page,(Byte)((addr&0xFF00)>>8),(Byte)(addr&0xFF),BLOCKSIZE);
#endif

				progress+=BLOCKSIZE;

				// we don't write block that are all $FF
				if( ! memcmp(&(newRom->pages[page][i]),ff_data,BLOCKSIZE))
					{
					// increment addr
					addr+=BLOCKSIZE;
#ifdef __COM_DEBUG
                    printf("all $FF\n");
#endif				
                    continue;
					}
				// write data
				cmd[0]=0x57;    // W (1 byte response)
				cmd[1]=page;
				cmd[2]=(Byte)((addr&0xFF00)>>8);		// HI part address
				cmd[3]=(Byte)(addr&0xFF);		// LOW part address
				cmd[4]=BLOCKSIZE;			// nb byte
				memcpy(&cmd[5],&(newRom->pages[page][i]),BLOCKSIZE);
				if(!SendData(cmd,69))
					{
					AutoStarDisconnect();
					mvprintw(16,0,"Communication error !\nThe autostar isn't responding to the flashing commands.\nCheck all connections and restart the autostar in safe load mode (press Enter end down close to ? and power on the autostar)\n");
					refresh();
					finish(0);
					}
					
				if(!ReadData(ioBuffer,1))
					{
					AutoStarDisconnect();
					mvprintw(16,0,"Communication error !\nThe autostar isn't responding to the flashing commands.\nCheck all connections and restart the autostar in safe load mode (press Enter end down close to ? and power on the autostar)\n");
					refresh();
					finish(0);
					}
					
				// check if answer is "Y"
				if(ioBuffer[0]!='Y') // Data have been writen ?
					{
					// if not we have a problem .. we stop it all
					AutoStarDisconnect();
					mvprintw(16,0,"Communication error !\nThe autostar isn't responding to the flashing commands.\nCheck all connections and restart the autostar in safe load mode (press Enter end down close to ? and power on the autostar)\n");
					refresh();
					finish(0);
					}
				// increment addr
				addr+=BLOCKSIZE;
				
				}
			}
			
        }

#endif				

    
    // quiting the trhread
    mvprintw(4,0,"Update done\n");
    refresh();
    return;
}
