/*
 *  Autostar.h
 *  AutoStarX
 *
 *  Created by roro on 6/3/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
#include "SerialPort.h"
#include "AutostarModel.h"
#include "Model494.h"
#include "Model497.h"
#include "ModelLX.h"

enum eAutostarCmnd {MODE, VERSION, SET_DOWNLOAD_MODE, INIT, READ, WRITE_FLASH, ERASE_BANK, PROGRAM_EE, TYPE, TELESCOPECMND, SET_BAUD_RATE};
enum eAutostarMode {UNKNOWN, OPERATIONAL, DOWNLOAD, SAFE_MODE, BUSY};
enum eAutostarStat {AUTOSTAR_OK, AUTOSTAR_DOWNLOADING, AUTOSTAR_UPLOADING, AUTOSTAR_BUSY, 
					BAD_COMM_PORT, NO_AUTOSTAR_RESPONSE, UNKNOWN_AUTOSTAR, OUT_OF_MEMORY, 
					ERASE_ERROR, WRITE_ERROR, READ_ERROR, UNKNOWN_ERROR, WRONG_MODE, BAD_FILE, 
					BAD_CHECKSUM, NO_PAGE7_FILE, NOT_ALLOWED, VERIFY_FAILED, COMMAND_FAILED,
					USEROBJEX_RETRIEVE_ERROR, OLD_FIRMWARE};
					
typedef enum {ERROR=-1, TYPE_UNKNOWN = 0, TYPE_AUTOSTAR = 1, TYPE_AUTOSTAR2 = 2, TYPE_RCX = 3} ASType;

class Autostar  
{
	friend class AutostarModel;
	friend class Model494_497;
	friend class Model494;
	friend class Model497;
	friend class ModelLX;

public:
	Autostar();
	virtual ~Autostar();

	eAutostarStat SendCommand(eAutostarCmnd cmd, Byte *data, Byte *resp, UInt32 &count);
	
	eAutostarStat ConnectToAutostar(const char *bsdPath);
	void DisconnectFromAutostar();
	eAutostarMode CheckDownLoadMode();
	ASType GetModel();
	
	eAutostarStat m_lastError;
	eAutostarMode m_mode;
	bool getModelName(char *buffer);
	bool isConnected();
	
protected:
	SerialPortIO *mPortIO;	

private:
	AutostarModel *m_model;
	CFStringRef m_modelName;
	bool m_hbxSafeMode;
	bool m_connected;
	
	eAutostarStat SendDownloadMode();

	
};

