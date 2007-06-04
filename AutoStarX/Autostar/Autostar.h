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
#include "AutostarStat.h"
#include "AutostarModel.h"
#include "Model494.h"
#include "Model497.h"
#include "ModelLX.h"

enum eAutostarCmnd {MODE, VERSION, SET_DOWNLOAD_MODE, INIT, READ, WRITE_FLASH, ERASE_BANK, PROGRAM_EE, TYPE, TELESCOPECMND, SET_BAUD_RATE};
enum eAutostarMode {UNKNOWN, OPERATIONAL, DOWNLOAD, SAFE_MODE, BUSY};

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

	eAutostarStat SendCommand(eAutostarCmnd cmd, Byte *data, Byte *resp, unsigned int &count);
	
	eAutostarStat ConnectToAutostar(const char *bsdPath);
	void DisconnectFromAutostar();
	eAutostarMode CheckDownLoadMode();
	ASType GetModel();
	
	eAutostarStat m_lastError;
	eAutostarMode m_mode;
	const char* getModelName(char *buffer);
	
protected:
	SerialPortIO *mPortIO;	

private:
	AutostarModel *m_model;
	CFStringRef m_modelName;
	bool m_hbxSafeMode;
	bool m_connected;
	
	eAutostarStat SendDownloadMode();

	
};

