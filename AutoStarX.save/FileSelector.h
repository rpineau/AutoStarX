/*
 *  fileselect.h
 *  AutoStar
 *
 *  Created by roro on 11/16/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
class FileSelector
{
public:
    FileSelector();
    virtual ~FileSelector();
    virtual FSSpec FileSelect();
    
private :

};


