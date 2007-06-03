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
	
protected:
	SerialPortIO *mPortIO;	
private:
	AutostarModel *m_model;
	CFStringRef m_modelName;
	bool m_hbxSafeMode;
	
};

