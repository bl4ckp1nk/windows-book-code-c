#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "../../../../nCom/PipeBase.h"
#include "../../xVMRuntime/VirtualDebugCRT/mmCom.h"
#include "../../../../3rdparty/tinyxml/tinyxml.h"
#include "../../../xHooklib/xhook_inline.h"
#include "../../../xHooklib/xhook_peloader.h"
#include "../../../xHooklib/xhook_typedef.h"
#include "../../xVMRuntime/xvmruntime.h"
#include "../../../../nCom/nautolargebuffer.h"
#include "../xdynamic_ollydbg.h"
#include "../resource.h"

#include "../xvmdebug.h"

#include "VirtualDebug.h"
#include "VirtualDebugApiLayer.h"


LPWaitForDebugEvent gEntryWaitForDebugEvent = NULL;
LPContinueDebugEvent gEntryContinueDebugEvent = NULL;
LPDebugActiveProcess gEntryDebugActiveProcess = NULL;
LPCreateProcessInternalW gEntryCreateProcessInternalW = NULL;

LPGetThreadContext gEntryGetThreadContext = NULL;
LPSetThreadContext gEntrySetThreadContext = NULL;
LPSuspendThread gEntrySuspendThread = NULL;
LPResumeThread gEntryResumeThread = NULL;
LPZwReadVirtualMemory gEntryZwReadVirtualMemory = NULL;
LPZwWriteVirtualMemory gEntryZwWriteVirtualMemory = NULL;
LPOpenThread gEntryOpenThread = NULL;
LPCloseHandle gEntryCloseHandle = NULL;
LPDuplicateHandle gEntryDuplicateHandle = NULL;
LPVirtualAllocEx gEntryVirtualAllocEx = NULL;
LPVirtualFreeEx gEntryVirtualFreeEx = NULL;
LPZwProtectVirtualMemory gEntryZwProtectVirtualMemory = NULL;
LPVirtualQueryEx gEntryVirtualQueryEx = NULL;
LPZwQueryInformationProcess         gEntryZwQueryInformationProcess = NULL;
LPZwSetInformationProcess gEntryZwSetInformationProcess = NULL;
LPZwQueryInformationThread          gEntryZwQueryInformationThread = NULL;
LPZwSetInformationThread gEntryZwSetInformationThread = NULL;
LPZwTerminateProcess gEntryZwTerminateProcess = NULL;
LPZwTerminateThread gEntryZwTerminateThread = NULL;

PROCESS_INFORMATION gProcessInfo;
DWORD gExceptThreadID= 0;

BOOL StartRecvPipe()
{
    TCHAR pPipeName[64];
    _stprintf(pPipeName,_T("\\\\.\\pipe\\xVMDebug%04X"),GetCurrentProcessId());
    gPipeRecv->fpClose();
    return gPipeRecv->fpCreateInst(pPipeName);
}
//???????????????????????????
BOOL WINAPI Proxy_CreateProcessInternalW(HANDLE hToken,
                                         LPCWSTR lpApplicationName,
                                         LPWSTR lpCommandLine,
                                         LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                         LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                         BOOL bInheritHandles,
                                         DWORD dwCreationFlags,
                                         LPVOID lpEnvironment,
                                         LPCWSTR lpCurrentDirectory,
                                         LPSTARTUPINFOW lpStartupInfo,
                                         LPPROCESS_INFORMATION lpProcessInformation,
                                         PHANDLE hNewToken)
{
    if (dwCreationFlags & DEBUG_ONLY_THIS_PROCESS || dwCreationFlags & DEBUG_PROCESS)
    {   //?????????????????????????????????????????????????????????????????????????????????????????????
        NAutoLargeBufferA nla;//???????????????????????????xVMRuntime?????????????????????????????????????????????
        nla.load(gxvmcfg.vdCRT.c_str());    //?????????????????????????????????????????????xVMRuntime32
        int szdata = 0;
        const char* lpdata = nla.data(szdata,0,false);
        if (!lpdata || szdata < 1)    return FALSE;
        //????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
        dwCreationFlags &= ~DEBUG_ONLY_THIS_PROCESS;
        dwCreationFlags &= ~DEBUG_PROCESS;
        dwCreationFlags |= CREATE_SUSPENDED;//??????????????????????????????????????????xVMRuntime32??????
        //????????????
        BOOL retCode = gEntryCreateProcessInternalW(hToken,lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,lpEnvironment,lpCurrentDirectory,lpStartupInfo,lpProcessInformation,hNewToken);
        if (retCode)
        {   //?????????????????????????????????????????????
            retCode = FALSE;
            wchar_t path[MAX_PATH*2];
            GetModuleFileNameW(0,path,sizeof(path)-1);
            wchar_t* curDir = (wchar_t*)malloc((wcslen(path)+50)*sizeof(wchar_t));
            if (curDir)
            {
                wcscpy(curDir,path);

                int slen = wcslen(curDir);
                for (int i=slen-1;i>=0;i--)
                {
                    if (curDir[i] =='\\'||curDir[i] =='/')
                    {
                        curDir[i+1] = 0;
                        break;
                    }
                }

            }
            xvmrutime_args args = {0};
            //?????????????????????????????????????????????xVMRuntime32???????????????????????????????????????????????????????????????
            args.bEnableHeapHook = FALSE;
            args.bBreakAtOEP = FALSE;
            args.bVirtualDebugMode = TRUE;
            long flags = XHOOK_PELOADER_FLAG_DIRECTEXEC| XHOOK_PELOADER_FLAG_OEPCALL;
            if (gxvmcfg.breakOnLdrLoadDll)  //???????????????????????????LdrLoadDll???
                flags |= XHOOK_PELOADER_FLAG_LDRLOADDLLCALL;
            HANDLE hPs = StartInProcess(lpProcessInformation->hProcess,
                                        lpProcessInformation->hThread,
                                        lpProcessInformation->dwThreadId,
                                        lpdata,szdata,(const char*)&args,sizeof(args),
                                        flags);
            if (hPs != INVALID_HANDLE_VALUE)
            {   //???????????????????????????????????????????????????????????????
                //??????????????????????????????????????????????????????????????????????????????????????????????????????
                //????????????????????????????????????????????????????????????????????????????????????????????????
                //????????????????????????????????????????????????????????????
                CloseHandle(lpProcessInformation->hProcess);
                CloseHandle(lpProcessInformation->hThread);
                lpProcessInformation->hProcess = (HANDLE)((LONG)lpProcessInformation->dwProcessId| HANDLE_PROCESS_MASK);
                lpProcessInformation->hThread =(HANDLE)((LONG)lpProcessInformation->dwThreadId| HANDLE_THREAD_MASK);
                gProcessInfo = *lpProcessInformation;
                //???????????????????????????????????????????????????
                TCHAR pipeName[64];
                _stprintf(pipeName,_T("\\\\.\\pipe\\xVMDebug%04X"),lpProcessInformation->dwProcessId);
                BOOL beConned = FALSE;
                for (int i=0;i<20;i++)  //???????????????????????????????????????????????????????????????????????????
                {
                    if (gPipeSend->fnOpenInst(pipeName))
                    {
                        beConned = TRUE;
                        break;
                    }
                    Sleep(100);
                }
                if (!beConned)
                {
                    TerminateProcess(lpProcessInformation->hProcess,0);
                    return FALSE;
                }
                //?????????????????????????????????????????????????????????????????????????????????
                StartRecvPipe();
                s_mm_dbg_connect st_dbg_conn;
                //?????????????????????????????????????????????????????????
                _stprintf(st_dbg_conn.m_dbgPipe,_T("\\\\.\\pipe\\xVMDebug%04X"),GetCurrentProcessId());
                st_dbg_conn.m_int3cmd = gxvmcfg.int3cmd;
                st_dbg_conn.m_int3ecode = gxvmcfg.int3ecode;
                st_dbg_conn.m_stepcmd = gxvmcfg.stepcmd;
                st_dbg_conn.m_stepecode = gxvmcfg.stepecode;
                SIZE_T pszBuf = gPipeSend->fpTransact(&st_dbg_conn,sizeof(st_dbg_conn),&st_dbg_conn,sizeof(st_dbg_conn));
                if (pszBuf != sizeof(st_dbg_conn))
                {
                    TerminateProcess(lpProcessInformation->hProcess,0);
                    return FALSE;
                }
                return st_dbg_conn.m_RetCode;
            }
            curDir?free(curDir):0;
            if (!retCode)
            {
                TerminateProcess(lpProcessInformation->hProcess,0);
                CloseHandle(lpProcessInformation->hProcess);
                CloseHandle(lpProcessInformation->hThread);
            }
        }
        return retCode;
    }
    return gEntryCreateProcessInternalW(hToken,lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,lpEnvironment,lpCurrentDirectory,lpStartupInfo,lpProcessInformation,hNewToken);
}

BOOL WINAPI Proxy_CloseHandle(
        __in HANDLE hObject
        )
{
    if ((LONG)hObject & HANDLE_MASK_BASE)
    {

        return TRUE;
    }
    if ((LONG)hObject & HANDLE_THREAD_MASK)
    {
#ifdef _DEBUG
        printf("close!!!\n");
#endif
        return TRUE;
    }
    return gEntryCloseHandle(hObject);
}

BOOL WINAPI Proxy_DuplicateHandle(
        __in        HANDLE hSourceProcessHandle,
        __in        HANDLE hSourceHandle,
        __in        HANDLE hTargetProcessHandle,
        __deref_out LPHANDLE lpTargetHandle,
        __in        DWORD dwDesiredAccess,
        __in        BOOL bInheritHandle,
        __in        DWORD dwOptions
        )
{
    if ((LONG)hSourceHandle & HANDLE_MASK_BASE)
    {
        *lpTargetHandle = hSourceHandle;
        return TRUE;
    }
    return gEntryDuplicateHandle(hSourceProcessHandle,hSourceHandle,hTargetProcessHandle,lpTargetHandle,dwDesiredAccess,bInheritHandle,dwOptions);
}

void fn_rm_printlog(s_mm_dbg_printlog* ps_dbg_printlog)
{
    //fn_odbg_printlog(ps_dbg_printlog->m_addr,ps_dbg_printlog->m_color,ps_dbg_printlog->m_strbuf);
}

//?????????????????????????????????
s_mm_dbg_event gstdbg;
BYTE* gRecvBuf  = NULL;
int gszRecvBuf= 0;
BOOL MainReceiveLoop()
{
    DWORD pdwByte = 0;
    if (!gPipeRecv->fpPeek(NULL,0,NULL,0,&pdwByte))
        return FALSE;
    if (pdwByte == 0)  return FALSE;
    if (!gRecvBuf)
    {
        gszRecvBuf = pdwByte;
        gRecvBuf = (BYTE*)malloc(gszRecvBuf);
    }else if (gszRecvBuf < pdwByte)
    {
        gszRecvBuf = pdwByte;
        gRecvBuf = (BYTE*)realloc(gRecvBuf,gszRecvBuf);
    }
    if (!gRecvBuf)
        return FALSE;

    BOOL pbRet = FALSE;
    gPipeRecv->fpRead(gRecvBuf,gszRecvBuf);
    switch (*(int*)gRecvBuf)
    {
    case XVM_VD_DBG_EXCEPTION:
    {   //?????????????????????????????????????????????
        gstdbg = *(s_mm_dbg_event*)gRecvBuf;
        pbRet = TRUE;
    }break;
    case XVM_VD_DBG_PRINTLOG:
    {
        fn_rm_printlog((s_mm_dbg_printlog*)gRecvBuf);
        gPipeRecv->fpWrite(gRecvBuf,sizeof(s_mm_dbg_printlog));
    }break;
    }
    return pbRet;
}
//??????????????????
BOOL WINAPI Proxy_WaitForDebugEvent(__in LPDEBUG_EVENT lpDebugEvent,__in DWORD dwMilliseconds)
{
    BOOL looping = TRUE;
    DWORD lastTick = GetTickCount();
    while (looping && !gstdbg.m_Invoked)
    {   //?????????????????????????????????????????????????????????????????????????????????????????????????????????
        if (MainReceiveLoop() && gstdbg.m_Invoked)
            break;
        if (GetTickCount() - lastTick >= dwMilliseconds)
        {
            lastTick = FALSE;
            break;
        }
        if (dwMilliseconds > 0) Sleep(1);
    }
    if (!gstdbg.m_Invoked)  return FALSE;
    *lpDebugEvent = gstdbg.m_dbgEvent;
    gstdbg.m_Invoked = false;
    switch (lpDebugEvent->dwDebugEventCode) //??????????????????
    {
    case CREATE_PROCESS_DEBUG_EVENT:
    {
        BYTE pPath[MAX_PATH];
        ReadProcessMemory(gProcessInfo.hProcess,lpDebugEvent->u.CreateProcessInfo.lpImageName,pPath,sizeof(pPath),NULL);
        lpDebugEvent->u.CreateProcessInfo.hFile = CreateFileA((LPCSTR)pPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
    }break;
    case LOAD_DLL_DEBUG_EVENT:
    {
        BYTE pPath[MAX_PATH];
        ReadProcessMemory(gProcessInfo.hProcess,lpDebugEvent->u.LoadDll.lpImageName,pPath,sizeof(pPath),NULL);
        lpDebugEvent->u.LoadDll.hFile = CreateFileA((LPCSTR)pPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
    }break;
    case EXCEPTION_DEBUG_EVENT:
    {
        if (gxvmcfg.int3cmd)
        {
            if (fn_odbg_checkint3(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionAddress,lpDebugEvent->u.Exception.ExceptionRecord.ExceptionCode))
                lpDebugEvent->u.Exception.ExceptionRecord.ExceptionCode = 0x80000003;
        }
        if (gxvmcfg.singletd)
            gExceptThreadID = lpDebugEvent->dwThreadId;
    }break;
    case CREATE_THREAD_DEBUG_EVENT:
    {
    }break;
    }
    return TRUE;
}
//??????????????????
BOOL WINAPI Proxy_ContinueDebugEvent(DWORD dwProcessId,DWORD dwThreadId,DWORD dwContinueStatus)
{
    if (gxvmcfg.singletd)   //????????????????????????????????????????????????????????????
    {
        if (gstdbg.m_dbgEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
        {
            if (dwThreadId == gExceptThreadID)
                gExceptThreadID = 0;
        }
    }//??????????????????????????????
    if (dwContinueStatus == DBG_CONTINUE)
        gstdbg.m_result = XVM_VD_DBG_CONTINUE;
    else
        gstdbg.m_result = XVM_VD_DBG_EXCEPTION_NOT_HANDLED;
    gPipeRecv->fpWrite(&gstdbg,sizeof(gstdbg)); //??????????????????
    return TRUE;
}

HANDLE WINAPI Proxy_OpenThread(
        __in DWORD dwDesiredAccess,
        __in BOOL bInheritHandle,
        __in DWORD dwThreadId
        )
{
#ifdef _DEBUG
    printf("open thread %d\n",dwThreadId);
#endif
    return gEntryOpenThread(dwDesiredAccess,bInheritHandle,dwThreadId);
}
//?????????????????????
BOOL WINAPI Proxy_GetThreadContext(HANDLE hThread,LPCONTEXT lpContext)
{   //?????????????????????????????????????????????????????????????????????????????????????????????????????????
    if (hThread != GetCurrentThread() && (LONG)hThread & HANDLE_THREAD_MASK)
    {
        BOOL pbRet = FALSE;
        DWORD tid = (LONG)hThread & ~HANDLE_THREAD_MASK;
        if (gxvmcfg.singletd)
            if (gExceptThreadID != tid)
                return FALSE;
        s_mm_dbg_getcontext st_getcon;//?????????????????????
        st_getcon.m_ThreadID = tid;
        SIZE_T pszBuf = gPipeSend->fpTransact(&st_getcon,sizeof(st_getcon),&st_getcon,sizeof(st_getcon));
        if (pszBuf == sizeof(st_getcon))
        {
            if (st_getcon.m_bRet)
            {   //????????????????????????????????????????????????????????????????????????
                pszBuf = gPipeSend->fpRead(lpContext,sizeof(CONTEXT));
                if (pszBuf == sizeof(CONTEXT))
                    pbRet = TRUE;
            }
        }
        return pbRet;
    }
    return gEntryGetThreadContext(hThread,lpContext);
}

BOOL WINAPI Proxy_SetThreadContext(
        __in HANDLE hThread,
        __in CONST CONTEXT *lpContext
        )
{
    if (hThread != GetCurrentThread() && (LONG)hThread & HANDLE_THREAD_MASK)
    {
        DWORD ptid = (LONG)hThread & ~HANDLE_THREAD_MASK;
        if (gxvmcfg.singletd)
            if (gExceptThreadID != ptid)
                return TRUE;

        s_mm_dbg_setcontext ps_dbg_setcontext;
        ps_dbg_setcontext.m_ThreadID = ptid;
        ps_dbg_setcontext.m_context = *lpContext;
        BOOL prret;
        SIZE_T pszBuf = gPipeSend->fpTransact(&ps_dbg_setcontext,sizeof(ps_dbg_setcontext),&prret,sizeof(prret));
        return (pszBuf==sizeof(prret))?prret:FALSE;
    }
    return gEntrySetThreadContext(hThread,lpContext);
}


DWORD WINAPI Proxy_SuspendThread(
        __in HANDLE hThread
        )
{
    if (hThread != GetCurrentThread() && (ULONG_PTR)hThread & HANDLE_THREAD_MASK)
    {
        if (gxvmcfg.singletd)
            return 0;
        s_mm_dbg_suspendtd st_dbg_suspendtd;
        st_dbg_suspendtd.m_ThreadID = (ULONG_PTR)hThread & ~HANDLE_THREAD_MASK;
        SIZE_T pszBuf = gPipeSend->fpTransact(&st_dbg_suspendtd,sizeof(st_dbg_suspendtd),&st_dbg_suspendtd,sizeof(st_dbg_suspendtd));
        return st_dbg_suspendtd.m_RetCode;

    }
    return gEntrySuspendThread(hThread);
}

DWORD WINAPI Proxy_ResumeThread(
        __in HANDLE hThread
        )
{
    if (hThread != GetCurrentThread() && (ULONG_PTR)hThread & HANDLE_THREAD_MASK)
    {
        if (gxvmcfg.singletd)
            return 0;
        s_mm_dbg_resumtd ps_dbg_resumtd;
        ps_dbg_resumtd.m_ThreadID = (ULONG_PTR)hThread & ~HANDLE_THREAD_MASK;
        SIZE_T pszBuf = gPipeSend->fpTransact(&ps_dbg_resumtd,sizeof(ps_dbg_resumtd),&ps_dbg_resumtd,sizeof(ps_dbg_resumtd));
        return ps_dbg_resumtd.m_RetCode;

    }
    return gEntryResumeThread(hThread);
}
//????????????????????????
LONG WINAPI Proxy_ZwReadVirtualMemory(HANDLE hProcess,PVOID BaseAddress,PVOID Buffer,ULONG BytesToRead,PULONG BytesRead)
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
        s_mm_dbg_readmemory st_dbg_readmem;
        st_dbg_readmem.m_lpAddr = (void*)BaseAddress;
        st_dbg_readmem.m_szRead = BytesToRead;
        st_dbg_readmem.m_szReaded = 0;
        LONG plRet = ERROR_ACCESS_DENIED;   //????????????????????????????????????
        SIZE_T pszBuf =gPipeSend->fpTransact(&st_dbg_readmem,sizeof(st_dbg_readmem),&st_dbg_readmem,sizeof(st_dbg_readmem));
        if (pszBuf == sizeof(st_dbg_readmem))
        {
            pszBuf = 0;
            if (st_dbg_readmem.m_bRet == TRUE)
            {
                pszBuf = gPipeSend->fpRead(Buffer,BytesToRead);
                plRet = ERROR_SUCCESS;

                if (gxvmcfg.int3cmd != 0)
                    fn_odbg_getint3(BaseAddress,(unsigned char*)Buffer,pszBuf,gxvmcfg.int3cmd);
            }
        }else
            pszBuf = 0;
        if (BytesRead)
            *BytesRead = pszBuf;
        return plRet;
    }
    return gEntryZwReadVirtualMemory(hProcess,BaseAddress,Buffer,BytesToRead,BytesRead);
}

LONG WINAPI Proxy_ZwWriteVirtualMemory(
        IN HANDLE hProcess,
        IN PVOID BaseAddress,
        IN PVOID Buffer,
        IN ULONG BytesToWrite,
        OUT PULONG BytesWritten
        )
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
        if (BytesToWrite <= 0 || Buffer == 0)
            return ERROR_ACCESS_DENIED;

        if (gxvmcfg.int3cmd != 0)
            fn_odbg_setint3(BaseAddress,(unsigned char*)Buffer,BytesToWrite,gxvmcfg.int3cmd);

        s_mm_dbg_writemem ps_dbg_writemem;
        ps_dbg_writemem.BaseAddress = BaseAddress;
        ps_dbg_writemem.Buffer = Buffer;
        ps_dbg_writemem.BytesToWrite = BytesToWrite;
        ps_dbg_writemem.BytesWritten = 0;
        ps_dbg_writemem.m_cmdid = XVM_VD_CMD_WRITEMEMORY;
        gPipeSend->fpWrite(&ps_dbg_writemem,sizeof(s_mm_dbg_writemem));
        gPipeSend->fpWrite(Buffer,BytesToWrite);
        gPipeSend->fpRead(&ps_dbg_writemem,sizeof(s_mm_dbg_writemem));
        if (BytesWritten)
            *BytesWritten = ps_dbg_writemem.BytesWritten;
        LONG plRet = ps_dbg_writemem.m_RetCode;
        return plRet;

    }
    return gEntryZwWriteVirtualMemory(hProcess,BaseAddress,Buffer,BytesToWrite,BytesWritten);
}



LPVOID WINAPI Proxy_VirtualAllocEx(
        __in     HANDLE hProcess,
        __in_opt LPVOID lpAddress,
        __in     SIZE_T dwSize,
        __in     DWORD flAllocationType,
        __in     DWORD flProtect
        )
{
    return gEntryVirtualAllocEx(hProcess,lpAddress,dwSize,flAllocationType,flProtect);
}

BOOL WINAPI Proxy_VirtualFreeEx(
        __in HANDLE hProcess,
        __in LPVOID lpAddress,
        __in SIZE_T dwSize,
        __in DWORD  dwFreeType
        )
{
    return gEntryVirtualFreeEx(hProcess,lpAddress,dwSize,dwFreeType);
}

LONG WINAPI Proxy_ZwProtectVirtualMemory(
        IN HANDLE hProcess,
        IN OUT PVOID *BaseAddress,
        IN OUT PULONG RegionSize,
        IN ULONG Protect,
        OUT PULONG OldProtect
        )
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
        s_mm_dbg_virprotect ps_dbg_virprotect;
        ps_dbg_virprotect.lpAddress = *BaseAddress;
        ps_dbg_virprotect.dwSize = *RegionSize;
        ps_dbg_virprotect.flNewProtect = Protect;
        LONG plRet = ERROR_ACCESS_DENIED;
        SIZE_T pszBuf = gPipeSend->fpTransact(&ps_dbg_virprotect,sizeof(ps_dbg_virprotect),&ps_dbg_virprotect,sizeof(ps_dbg_virprotect));
        if (pszBuf == sizeof(ps_dbg_virprotect))
        {
            if (ps_dbg_virprotect.RetCode = TRUE)
            {
                plRet = ERROR_SUCCESS;
                *BaseAddress = ps_dbg_virprotect.lpAddress;
                *RegionSize = ps_dbg_virprotect.dwSize;
                if (OldProtect)
                    *OldProtect =ps_dbg_virprotect.lpflOldProtect;
            }
        }
        return plRet;
    }
    return gEntryZwProtectVirtualMemory(hProcess,BaseAddress,RegionSize,Protect,OldProtect);
}

SIZE_T WINAPI Proxy_VirtualQueryEx(
        __in     HANDLE hProcess,
        __in_opt LPCVOID lpAddress,
        PMEMORY_BASIC_INFORMATION lpBuffer,
        __in     SIZE_T dwLength
        )
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
        s_mm_dbg_virquery ps_dbg_virquery;
        ps_dbg_virquery.lpAddress = lpAddress;
        ps_dbg_virquery.lpBuffer = *lpBuffer;
        ps_dbg_virquery.dwLength = dwLength;
        SIZE_T pszBuf = gPipeSend->fpTransact(&ps_dbg_virquery,sizeof(ps_dbg_virquery),&ps_dbg_virquery,sizeof(ps_dbg_virquery));
        if (pszBuf == sizeof(ps_dbg_virquery))
        {
            *lpBuffer = ps_dbg_virquery.lpBuffer;
            return ps_dbg_virquery.RetCode;
        }
        return 0;
    }
    return gEntryVirtualQueryEx(hProcess,lpAddress,lpBuffer,dwLength);
}



LONG WINAPI Proxy_ZwQueryInformationProcess(
        IN HANDLE hProcess,
        IN int ProcessInfoClass,
        OUT PVOID ProcessInfoBuffer,
        IN ULONG ProcessInfoBufferLength,
        OUT PULONG BytesReturned
        )
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
        if (ProcessInfoClass == 7)//ProcessDebugPort
        {
            *(ULONG*)ProcessInfoBuffer = 0x1000;
            *BytesReturned = ProcessInfoBufferLength;
            return ERROR_SUCCESS;
        }
        s_mm_dbg_queryinfops ps_dbg_qinfops;
        ps_dbg_qinfops.ProcessInfoClass = ProcessInfoClass;
        ps_dbg_qinfops.ProcessInfoBuffer = ProcessInfoBuffer;
        ps_dbg_qinfops.ProcessInfoBufferLength = ProcessInfoBufferLength;
        ps_dbg_qinfops.BytesReturned = BytesReturned;
        LONG plRet = ERROR_ACCESS_DENIED;
        SIZE_T pszBuf = 0;
        gPipeSend->fpTransact(&ps_dbg_qinfops,sizeof(ps_dbg_qinfops),&ps_dbg_qinfops,sizeof(ps_dbg_qinfops));
        if (ps_dbg_qinfops.RetCode == ERROR_SUCCESS)
            pszBuf = gPipeSend->fpRead(ProcessInfoBuffer,ps_dbg_qinfops.ProcessInfoBufferLength);
        if (BytesReturned)
            *BytesReturned = pszBuf;
        return ps_dbg_qinfops.RetCode;
    }
    return gEntryZwQueryInformationProcess(hProcess,ProcessInfoClass,ProcessInfoBuffer,ProcessInfoBufferLength,BytesReturned);
}



LONG WINAPI Proxy_ZwSetInformationProcess(
        IN HANDLE hProcess,
        IN int ProcessInfoClass,
        IN PVOID ProcessInfoBuffer,
        IN ULONG ProcessInfoBufferLength
        )
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
#ifdef _DEBUG
        printf("Remote SetInformationProcess Called!\n");
#endif
    }
    return gEntryZwSetInformationProcess(hProcess,ProcessInfoClass,ProcessInfoBuffer,ProcessInfoBufferLength);
}



LONG WINAPI Proxy_ZwQueryInformationThread(
        IN HANDLE hThread,
        IN int ThreadInfoClass,
        OUT PVOID ThreadInfoBuffer,
        IN ULONG ThreadInfoBufferLength,
        OUT PULONG BytesReturned
        )
{
    if (hThread != GetCurrentThread() && (LONG)hThread & HANDLE_THREAD_MASK)
    {
        s_mm_dbg_queryinfotd ps_dbg_qinfotd;
        ps_dbg_qinfotd.hThread = (HANDLE)((ULONG_PTR)hThread & ~HANDLE_THREAD_MASK);
        ps_dbg_qinfotd.ThreadInfoClass = ThreadInfoClass;
        ps_dbg_qinfotd.ThreadInfoBuffer = ThreadInfoBuffer;
        ps_dbg_qinfotd.ThreadInfoBufferLength = ThreadInfoBufferLength;
        ps_dbg_qinfotd.BytesReturned = 0;
        SIZE_T pszBuf = 0;
        gPipeSend->fpTransact(&ps_dbg_qinfotd,sizeof(ps_dbg_qinfotd),&ps_dbg_qinfotd,sizeof(ps_dbg_qinfotd));
        if (ps_dbg_qinfotd.RetCode == ERROR_SUCCESS)
            pszBuf = gPipeSend->fpRead(ThreadInfoBuffer,ps_dbg_qinfotd.ThreadInfoBufferLength);
        if (BytesReturned)
            *BytesReturned = pszBuf;
        return ps_dbg_qinfotd.RetCode;
    }
    return gEntryZwQueryInformationThread(hThread,ThreadInfoClass,ThreadInfoBuffer,ThreadInfoBufferLength,BytesReturned);
}

LONG WINAPI Proxy_ZwSetInformationThread(
        IN HANDLE hThread,
        IN int ThreadInfoClass,
        IN PVOID ThreadInfoBuffer,
        IN ULONG ThreadInfoBufferLength
        )
{
    if (hThread != GetCurrentThread() && (LONG)hThread & HANDLE_THREAD_MASK)
    {
        s_mm_dbg_setinfotd ps_dbg_setinfotd;
        ps_dbg_setinfotd.hThread = (HANDLE)((ULONG_PTR)hThread & ~HANDLE_THREAD_MASK);
        ps_dbg_setinfotd.ThreadInfoClass = ThreadInfoClass;
        ps_dbg_setinfotd.ThreadInfoBuffer = ThreadInfoBuffer;
        ps_dbg_setinfotd.ThreadInfoBufferLength = ThreadInfoBufferLength;
        gPipeSend->fpWrite(&ps_dbg_setinfotd,sizeof(ps_dbg_setinfotd));
        if (ThreadInfoBufferLength > 0)
            gPipeSend->fpWrite(ThreadInfoBuffer,ThreadInfoBufferLength);
        gPipeSend->fpRead(&ps_dbg_setinfotd,sizeof(ps_dbg_setinfotd));
        return ps_dbg_setinfotd.m_RetCode;
    }
    return gEntryZwSetInformationThread(hThread,ThreadInfoClass,ThreadInfoBuffer,ThreadInfoBufferLength);
}


LONG WINAPI Proxy_ZwTerminateThread(
        __in HANDLE hThread,
        __in DWORD dwExitCode
        )
{
    if (hThread != GetCurrentThread() && (LONG)hThread & HANDLE_THREAD_MASK)
    {
        s_mm_dbg_termthread ps_dbg_termtd;
        ps_dbg_termtd.m_ThreadID = ((ULONG_PTR)hThread & ~HANDLE_THREAD_MASK);
        ps_dbg_termtd.m_dwExitCode = dwExitCode;
        gPipeSend->fpTransact(&ps_dbg_termtd,sizeof(ps_dbg_termtd),&ps_dbg_termtd,sizeof(ps_dbg_termtd));
        return ps_dbg_termtd.m_RetCode;
    }
    return gEntryZwTerminateThread(hThread,dwExitCode);
}

LONG WINAPI Proxy_ZwTerminateProcess(
        __in HANDLE hProcess,
        __in DWORD dwExitCode
        )
{
    if (hProcess != GetCurrentProcess() && (LONG)hProcess & HANDLE_PROCESS_MASK)
    {
#ifdef _DEBUG
        printf("Remote ZwTerminateProcess Called!\n");
#endif
    }
    return gEntryZwTerminateProcess(hProcess,dwExitCode);
}



BOOL WINAPI Proxy_DebugActiveProcess( __in DWORD dwProcessId )
{
    TCHAR pPipeName[64];
    _stprintf(pPipeName,_T("\\\\.\\pipe\\xVMDebug%04X"),dwProcessId);
    if (!gPipeSend->fnOpenInst(pPipeName))
        return FALSE;
    StartRecvPipe();
    s_mm_dbg_connect st_dbg_conn;
    st_dbg_conn.m_cmdid = XVM_VD_DBG_ATTACH;
    _stprintf(st_dbg_conn.m_dbgPipe,_T("\\\\.\\pipe\\xVMDebug%04X"),GetCurrentProcessId());
    st_dbg_conn.m_int3cmd = gxvmcfg.int3cmd;
    st_dbg_conn.m_int3ecode = gxvmcfg.int3ecode;
    st_dbg_conn.m_stepcmd = gxvmcfg.stepcmd;
    st_dbg_conn.m_stepecode = gxvmcfg.stepecode;
    SIZE_T pszBuf = gPipeSend->fpTransact(&st_dbg_conn,sizeof(st_dbg_conn),&st_dbg_conn,sizeof(st_dbg_conn));
    if (pszBuf != sizeof(st_dbg_conn))
        return FALSE;
    return st_dbg_conn.m_RetCode;
}

//??????????????????????????????????????????????????????????????????
BOOL InstallVirtualDebugLayer()
{
    //????????????kernelbase???vista??????????????????HOOK kernelbase??????
    HMODULE hKrl32 = GetModuleHandle(_T("KERNELBASE"));
    if (!hKrl32) hKrl32 = GetModuleHandle(_T("Kernel32"));
    gEntryCreateProcessInternalW=(LPCreateProcessInternalW)HookCode(GetProcAddress(hKrl32,"CreateProcessInternalW"),Proxy_CreateProcessInternalW,HOOKTYPE_PUSH,0);
    gEntryWaitForDebugEvent=(LPWaitForDebugEvent)HookCode(GetProcAddress(hKrl32,"WaitForDebugEvent"),Proxy_WaitForDebugEvent,HOOKTYPE_PUSH,0);
    gEntryContinueDebugEvent=(LPContinueDebugEvent)HookCode(GetProcAddress(hKrl32,"ContinueDebugEvent"),Proxy_ContinueDebugEvent,HOOKTYPE_PUSH,0);
    gEntryDebugActiveProcess=(LPDebugActiveProcess)HookCode(GetProcAddress(hKrl32,"DebugActiveProcess"),Proxy_DebugActiveProcess,HOOKTYPE_PUSH,0);
    gEntryGetThreadContext=(LPGetThreadContext)HookCode(GetProcAddress(hKrl32,"GetThreadContext"),Proxy_GetThreadContext,HOOKTYPE_PUSH,0);
    gEntrySetThreadContext=(LPSetThreadContext)HookCode(GetProcAddress(hKrl32,"SetThreadContext"),Proxy_SetThreadContext,HOOKTYPE_PUSH,0);
    gEntrySuspendThread=(LPSuspendThread)HookCode(GetProcAddress(hKrl32,"SuspendThread"),Proxy_SuspendThread,HOOKTYPE_PUSH,0);
    gEntryResumeThread=(LPResumeThread)HookCode(GetProcAddress(hKrl32,"ResumeThread"),Proxy_ResumeThread,HOOKTYPE_PUSH,0);
    gEntryOpenThread=(LPOpenThread)HookCode(GetProcAddress(hKrl32,"OpenThread"),Proxy_OpenThread,HOOKTYPE_PUSH,0);
    gEntryCloseHandle=(LPCloseHandle)HookCode(GetProcAddress(hKrl32,"CloseHandle"),Proxy_CloseHandle,HOOKTYPE_PUSH,0);
    gEntryDuplicateHandle=(LPDuplicateHandle)HookCode(GetProcAddress(hKrl32,"DuplicateHandle"),Proxy_DuplicateHandle,HOOKTYPE_PUSH,0);
    gEntryVirtualAllocEx=(LPVirtualAllocEx)HookCode(GetProcAddress(hKrl32,"VirtualAllocEx"),Proxy_VirtualAllocEx,HOOKTYPE_PUSH,0);
    gEntryVirtualFreeEx=(LPVirtualFreeEx)HookCode(GetProcAddress(hKrl32,"VirtualFreeEx"),Proxy_VirtualFreeEx,HOOKTYPE_PUSH,0);
    gEntryVirtualQueryEx=(LPVirtualQueryEx)HookCode(GetProcAddress(hKrl32,"VirtualQueryEx"),Proxy_VirtualQueryEx,HOOKTYPE_PUSH,0);
    //??????????????????????????????HOOK?????????????????????????????????
    HMODULE hNtDLL = GetModuleHandle(_T("NTDLL"));
    gEntryZwReadVirtualMemory=(LPZwReadVirtualMemory)HookCode(GetProcAddress(hNtDLL,"ZwReadVirtualMemory"),Proxy_ZwReadVirtualMemory,HOOKTYPE_PUSH,0);
    gEntryZwWriteVirtualMemory=(LPZwWriteVirtualMemory)HookCode(GetProcAddress(hNtDLL,"ZwWriteVirtualMemory"),Proxy_ZwWriteVirtualMemory,HOOKTYPE_PUSH,0);
    gEntryZwQueryInformationProcess =(LPZwQueryInformationProcess)HookCode(GetProcAddress(hNtDLL,"ZwQueryInformationProcess"),Proxy_ZwQueryInformationProcess,HOOKTYPE_PUSH,0);
    gEntryZwSetInformationProcess=(LPZwSetInformationProcess)HookCode(GetProcAddress(hNtDLL,"ZwSetInformationProcess"),Proxy_ZwSetInformationProcess,HOOKTYPE_PUSH,0);
    gEntryZwQueryInformationThread=(LPZwQueryInformationThread)HookCode(GetProcAddress(hNtDLL,"ZwQueryInformationThread"),Proxy_ZwQueryInformationThread,HOOKTYPE_PUSH,0);
    gEntryZwSetInformationThread=(LPZwSetInformationThread)HookCode(GetProcAddress(hNtDLL,"ZwSetInformationThread"),Proxy_ZwSetInformationThread,HOOKTYPE_PUSH,0);
    gEntryZwTerminateThread=(LPZwTerminateThread)HookCode(GetProcAddress(hNtDLL,"ZwTerminateThread"),Proxy_ZwTerminateThread,HOOKTYPE_PUSH,0);
    gEntryZwTerminateProcess=(LPZwTerminateProcess)HookCode(GetProcAddress(hNtDLL,"ZwTerminateProcess"),Proxy_ZwTerminateProcess,HOOKTYPE_PUSH,0);

    void* lpZwProtectVirtualMemory = GetProcAddress(hNtDLL,"ZwProtectVirtualMemory");
    ULONG oldPG = 0;
    if (!VirtualProtect((char*)lpZwProtectVirtualMemory,10,PAGE_EXECUTE_READWRITE,&oldPG))
        return FALSE;
    gEntryZwProtectVirtualMemory = (LPZwProtectVirtualMemory)HookCodeDirect(hNtDLL,lpZwProtectVirtualMemory,Proxy_ZwProtectVirtualMemory,HOOKTYPE_PUSH,0);
    VirtualProtect((char*)lpZwProtectVirtualMemory,10,oldPG,&oldPG);
    return TRUE;
}

//????????????HOOK
BOOL UnInstallVirtualDebugLayer()
{
    if (gEntryZwTerminateProcess == 0)
        return TRUE;
    HMODULE hKrl32 = GetModuleHandle(_T("KERNELBASE"));
    if (!hKrl32) hKrl32 = GetModuleHandle(_T("Kernel32"));
    HMODULE hNtDLL = GetModuleHandle(_T("NTDLL"));
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwProtectVirtualMemory"))) gEntryZwProtectVirtualMemory = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwTerminateProcess"))) gEntryZwTerminateProcess = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwTerminateThread"))) gEntryZwTerminateThread = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwSetInformationThread"))) gEntryZwSetInformationThread = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwQueryInformationThread"))) gEntryZwQueryInformationThread = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwSetInformationProcess"))) gEntryZwSetInformationProcess = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwQueryInformationProcess"))) gEntryZwQueryInformationProcess = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwWriteVirtualMemory"))) gEntryZwWriteVirtualMemory = 0;
    if (UnHookCode(GetProcAddress(hNtDLL,"ZwReadVirtualMemory"))) gEntryZwReadVirtualMemory = 0;

    if (UnHookCode(GetProcAddress(hKrl32,"VirtualQueryEx"))) gEntryVirtualQueryEx = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"VirtualFreeEx"))) gEntryVirtualFreeEx = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"VirtualAllocEx"))) gEntryVirtualAllocEx = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"DuplicateHandle"))) gEntryDuplicateHandle = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"CloseHandle"))) gEntryCloseHandle = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"OpenThread"))) gEntryOpenThread = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"ResumeThread"))) gEntryResumeThread = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"SuspendThread"))) gEntrySuspendThread = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"SetThreadContext"))) gEntrySetThreadContext = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"GetThreadContext"))) gEntryGetThreadContext = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"DebugActiveProcess"))) gEntryDebugActiveProcess = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"ContinueDebugEvent"))) gEntryContinueDebugEvent = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"WaitForDebugEvent"))) gEntryWaitForDebugEvent = 0;
    if (UnHookCode(GetProcAddress(hKrl32,"CreateProcessInternalW"))) gEntryCreateProcessInternalW = 0;
    return TRUE;
}
