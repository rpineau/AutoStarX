/*
 *  Autostar.cpp
 *  AutoStarX
 *
 *  Created by roro on 6/3/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Autostar.h"

Autostar::Autostar()
{
	m_model				= NULL;
	m_modelName			= CFSTR("Unknown");
	m_hbxSafeMode		= FALSE;
}

Autostar::~Autostar()
{
	if (m_model)
		delete m_model;
}

