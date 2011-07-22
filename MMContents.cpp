#include "stdafx.h"
#include "MrMAPI.h"
#include "MMContents.h"
#include "MMFolder.h"
#include "DumpStore.h"
#include "File.h"

void DumpContentsTable(
	_In_z_ LPWSTR lpszProfile,
	_In_z_ LPWSTR lpszDir,
	_In_ bool bContents,
	_In_ bool bAssociated,
	_In_ bool bRetryStreamProps,
	_In_ bool bMSG,
	_In_ bool bList,
	_In_ ULONG ulFolder,
	_In_z_ LPWSTR lpszFolder,
	_In_opt_ LPSRestriction lpRes)
{
	InitMFC();
	DebugPrint(DBGGeneric,"DumpContentsTable: Outputting folder %i / %ws from profile %ws to %ws\n", ulFolder, lpszFolder?lpszFolder:L"", lpszProfile, lpszDir);
	if (bContents)  DebugPrint(DBGGeneric,"DumpContentsTable: Outputting Contents\n");
	if (bAssociated) DebugPrint(DBGGeneric,"DumpContentsTable: Outputting Associated Contents\n");
	if (bMSG) DebugPrint(DBGGeneric,"DumpContentsTable: Outputting as MSG\n");
	if (bRetryStreamProps) DebugPrint(DBGGeneric,"DumpContentsTable: Will retry stream properties\n");
	if (bList) DebugPrint(DBGGeneric,"DumpContentsTable: List only mode\n");
	HRESULT hRes = S_OK;
	LPMAPISESSION lpMAPISession = NULL;
	LPMDB lpMDB = NULL;
	LPMAPIFOLDER lpFolder = NULL;

	WC_H(MAPIInitialize(NULL));

	WC_H(MrMAPILogonEx(lpszProfile,&lpMAPISession));
	if (lpMAPISession)
	{
		WC_H(OpenExchangeOrDefaultMessageStore(lpMAPISession, &lpMDB));
	}
	if (lpMDB)
	{
		if (lpszFolder)
		{
			WC_H(HrMAPIOpenFolderExW(lpMDB, lpszFolder, &lpFolder));
		}
		else
		{
			WC_H(OpenDefaultFolder(ulFolder,lpMDB,&lpFolder));
		}
	}
	if (lpFolder)
	{
		CDumpStore MyDumpStore;
		MyDumpStore.InitMDB(lpMDB);
		MyDumpStore.InitFolder(lpFolder);
		MyDumpStore.InitFolderPathRoot(lpszDir);
		MyDumpStore.InitFolderContentsRestriction(lpRes);
		if (bMSG) MyDumpStore.EnableMSG();
		if (bList) MyDumpStore.EnableList();
		if (bRetryStreamProps) MyDumpStore.EnableStreamRetry();
		MyDumpStore.ProcessFolders(
			bContents,
			bAssociated,
			false);
	}

	if (lpFolder) lpFolder->Release();
	if (lpMDB) lpMDB->Release();
	if (lpMAPISession) lpMAPISession->Release();
	MAPIUninitialize();
} // DumpContentsTable

void DumpMSG(_In_z_ LPCWSTR lpszMSGFile, _In_z_ LPCWSTR lpszXMLFile, _In_ bool bRetryStreamProps)
{
	InitMFC();
	HRESULT hRes = S_OK;
	LPMESSAGE lpMessage = NULL;

	WC_H(MAPIInitialize(NULL));

	WC_H(LoadMSGToMessage(lpszMSGFile, &lpMessage));

	if (lpMessage)
	{
		CDumpStore MyDumpStore;
		MyDumpStore.InitMessagePath(lpszXMLFile);
		if (bRetryStreamProps) MyDumpStore.EnableStreamRetry();

		// Just assume this message might have attachments
		MyDumpStore.ProcessMessage(lpMessage,true,NULL);
		lpMessage->Release();
	}

	MAPIUninitialize();
} // DumpMSG

void DoContents(_In_ MYOPTIONS ProgOpts)
{
	SRestriction sResTop = {0};
	SRestriction sResMiddle[2] = {0};
	SRestriction sResSubject[2] = {0};
	SRestriction sResMessageClass[2] = {0};
	SPropValue sPropValue[2] = {0};
	LPSRestriction lpRes = NULL;
	if (ProgOpts.lpszSubject || ProgOpts.lpszMessageClass)
	{
		// RES_AND
		//   RES_AND (optional)
		//     RES_EXIST - PR_SUBJECT_W
		//     RES_CONTENT - lpszSubject
		//   RES_AND (optional)
		//     RES_EXIST - PR_MESSAGE_CLASS_W
		//     RES_CONTENT - lpszMessageClass
		int i = 0;
		if (ProgOpts.lpszSubject)
		{
			sResMiddle[i].rt = RES_AND;
			sResMiddle[i].res.resAnd.cRes = 2;
			sResMiddle[i].res.resAnd.lpRes = &sResSubject[0];
			sResSubject[0].rt = RES_EXIST;
			sResSubject[0].res.resExist.ulPropTag = PR_SUBJECT_W;
			sResSubject[1].rt = RES_CONTENT;
			sResSubject[1].res.resContent.ulPropTag = PR_SUBJECT_W;
			sResSubject[1].res.resContent.ulFuzzyLevel = FL_FULLSTRING | FL_IGNORECASE;
			sResSubject[1].res.resContent.lpProp = &sPropValue[0];
			sPropValue[0].ulPropTag = PR_SUBJECT_W;
			sPropValue[0].Value.lpszW = ProgOpts.lpszSubject;
			i++;
		}
		if (ProgOpts.lpszMessageClass)
		{
			sResMiddle[i].rt = RES_AND;
			sResMiddle[i].res.resAnd.cRes = 2;
			sResMiddle[i].res.resAnd.lpRes = &sResMessageClass[0];
			sResMessageClass[0].rt = RES_EXIST;
			sResMessageClass[0].res.resExist.ulPropTag = PR_MESSAGE_CLASS_W;
			sResMessageClass[1].rt = RES_CONTENT;
			sResMessageClass[1].res.resContent.ulPropTag = PR_MESSAGE_CLASS_W;
			sResMessageClass[1].res.resContent.ulFuzzyLevel = FL_FULLSTRING | FL_IGNORECASE;
			sResMessageClass[1].res.resContent.lpProp = &sPropValue[1];
			sPropValue[1].ulPropTag = PR_MESSAGE_CLASS_W;
			sPropValue[1].Value.lpszW = ProgOpts.lpszMessageClass;
			i++;
		}
		sResTop.rt = RES_AND;
		sResTop.res.resAnd.cRes = i;
		sResTop.res.resAnd.lpRes = &sResMiddle[0];
		lpRes = &sResTop;
		DebugPrintRestriction(DBGGeneric,lpRes,NULL);
	}
	DumpContentsTable(
		ProgOpts.lpszProfile,
		ProgOpts.lpszOutput?ProgOpts.lpszOutput:L".",
		ProgOpts.bDoContents,
		ProgOpts.bDoAssociatedContents,
		ProgOpts.bRetryStreamProps,
		ProgOpts.bMSG,
		ProgOpts.bList,
		ProgOpts.ulFolder,
		ProgOpts.lpszFolderPath,
		lpRes);
} // DoContents

void DoMSG(_In_ MYOPTIONS ProgOpts)
{
	DumpMSG(
		ProgOpts.lpszInput,
		ProgOpts.lpszOutput?ProgOpts.lpszOutput:L".",
		ProgOpts.bRetryStreamProps);
} // DoMAPIMIME