/*
 *  fileselect.c
 *  AutoStar
 *
 *  Created by roro on 11/16/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */


#include "FileSelector.h"
FileSelector::FileSelector()
{
}

FileSelector::~FileSelector()
{
}

// Call the Nav dialog
// Return a FSSPec of the selected file
//
FSSpec FileSelector::FileSelect()
{
	OSStatus status;
	OSErr err;
	FSSpec FileSpec;
    
	AEKeyword keyword;
	DescType ActualType;
	Size ActualSize=0;
	
    NavDialogOptions FileSelectorOptions;
    NavTypeListHandle mFileList;
	NavReplyRecord reply;
	
	// Get a resource to open all file type
    mFileList = (NavTypeListHandle)GetResource('open', 128);
	if(mFileList != NULL)
		HLock((Handle)mFileList);
	
	// Get the default options
	status=NavGetDefaultDialogOptions(&FileSelectorOptions);
	//disable the preview
 	FileSelectorOptions.dialogOptionFlags &= ~kNavAllowPreviews;
	//only one file
	FileSelectorOptions.dialogOptionFlags &= ~kNavAllowMultipleFiles;
	
	// call the Nav Dialog
    err=NavGetFile(NULL, &reply, &FileSelectorOptions, NULL, NULL, NULL, mFileList, NULL);
	if((err == noErr) && reply.validRecord)
		{ // if we have no error and a valid selection, get the FSSpec for this file
        err=AEGetNthPtr(&(reply.selection), 1, typeFSS, &keyword, &ActualType, &FileSpec, sizeof(FSSpec), &ActualSize);
		NavDisposeReply(&reply);
		}

	// free the resource list
	if(mFileList != NULL)
		{
		HUnlock((Handle)mFileList);
		DisposeHandle((Handle)mFileList);
		}
		
    return FileSpec;
}

