#include "bindconfig.h"

#include <stdio.h>

#include <netcfgx.h>
#include <devguid.h>
#include <shlwapi.h>

int doPrint = 0;
static INetCfg *netCfg;

static int configBindings(INetCfgComponent *netCfgComp, LPCWSTR pathStr, 
        int status)
{
    int count = 0;
    HRESULT ret;
    LPWSTR idStr, nameStr;
    INetCfgComponentBindings *bindings;
    IEnumNetCfgBindingPath *bindingIter;
    INetCfgBindingPath *bindingPath;

    netCfgComp->GetBindName(&idStr);
    netCfgComp->GetDisplayName(&nameStr);

    printf("%ws (%ws)\n", idStr, nameStr);

    CoTaskMemFree(idStr);
    CoTaskMemFree(nameStr);

    ret = netCfgComp->QueryInterface(IID_INetCfgComponentBindings, 
            (void**)&bindings);
    if (ret != S_OK)
    {
        printf("netCfgComp->QueryInterface() returned %x\n", ret);
        goto _error_exit;
    }

    ret = bindings->EnumBindingPaths(EBP_ABOVE, &bindingIter);
    if (ret != S_OK)
    {
        printf("bindings->EnumBindingPaths() returned %x\n", ret);
        goto _error_exit;
    }

    bindingIter->Reset();

    while ((ret = bindingIter->Next(1, &bindingPath, NULL)) == S_OK)
    {
        LPWSTR str;

        bindingPath->GetPathToken(&str);

        //printf("@%ws\n@%ws\n", pathStr, str);

        if (pathStr == NULL || (StrCmpLogicalW(pathStr, str) == 0))
        {
            if (status < 0)
            {
                int ena = bindingPath->IsEnabled() == S_OK ? 1 : 0;

                count += ena;
                if (doPrint)
                    printf("    %ws: %d\n", str, ena);
            }
            else
            {
                int ena = bindingPath->IsEnabled() == S_OK ? 1 : 0;

                if (doPrint)
                    printf("    %ws: %d->%d\n", str, ena, (status > 0));

                if (status > 0) 
                    count += ena;
                ret = bindingPath->Enable(status > 0);
                if (ret != S_OK)
                    printf("bindingPath->Enable(%d) returned %x\n", status,ret);
            }

            CoTaskMemFree(str);

            if (pathStr)
                break;
        }
    }

    if (status >= 0)
    {
        if (doPrint)
            printf("Applying changes ... ");

        ret = netCfg->Apply();

        if (doPrint)
        {
            if (ret == S_OK)
                printf("success\n");
            else
                printf("failed - error code %x\n", ret);
        }
    }

    return count;

_error_exit:
    return -1;
}

// bindconfig() returns the count of "enabled" bindings; in case of error,
// it returns a negative number
int bindconfig(CompClass compClass, char *comp, char *path, int status)
{
    HRESULT ret;
    int count = 0;
    const GUID *clsId;
    WCHAR compWStr[64];
    WCHAR pathWStr[64];
    LPCWSTR compStr = NULL;
    LPCWSTR pathStr = NULL;
    INetCfgLock *lock;
    INetCfgClass *netCfgClass;
    IEnumNetCfgComponent *iter;
    INetCfgComponent *netCfgComp;

    switch(compClass)
    {
    case kCompClassDevice:
        clsId = &GUID_DEVCLASS_NET;
        break;
    case kCompClassClient:
        clsId = &GUID_DEVCLASS_NETCLIENT;
        break;
    case kCompClassProtocol:
        clsId = &GUID_DEVCLASS_NETTRANS;
        break;
    case kCompClassService:
        clsId = &GUID_DEVCLASS_NETSERVICE;
        break;
    default:
        printf("invalid compClass %d\n", compClass);
        goto _invalid_param;
    }

    if (comp)
    {
        mbstowcs(compWStr, comp, strlen(comp)+1);
        compStr = (LPWSTR) &compWStr;
    }

    if (path)
    {
        mbstowcs(pathWStr, path, strlen(path)+1);
        pathStr = (LPWSTR) &pathWStr;
    }

    ret = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (ret != S_OK)
    {
        printf("CoInitializeEx() returned %x\n", ret);
        goto _co_initialize_ex_failed;
    }

    ret = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
            IID_INetCfg, (void**)&netCfg);
    if (ret != S_OK)
    {
        printf("CoCreateInstance(CLSID_CNetCfg) returned %x\n", ret);
        goto _co_create_instance_failed;
    }

    ret = netCfg->QueryInterface(IID_INetCfgLock, (void**) &lock);
    if (ret != S_OK)
    {
        printf("netCfg->QueryInterface(IID_INetCfgLock) returned %x\n", ret);
        goto _query_interface_lock_failed;
    }

    ret = lock->AcquireWriteLock(5000, L"bindconfig", NULL);
    if (ret != S_OK)
    {
        printf("lock->AcquireWriteLock() returned %x\n", ret);
        goto _lock_acquire_failed;
    }

    ret = netCfg->Initialize(NULL);
    if (ret != S_OK)
    {
        printf("netCfg->Initialize() returned %x\n", ret);
        goto _initialize_failed;
    }

    ret = netCfg->QueryNetCfgClass(clsId, IID_INetCfgClass, 
        (void**)&netCfgClass);
    if (ret != S_OK)
    {
        printf("QueryNetCfgClass() returned %x\n", ret);
        goto _query_class_failed;
    }

    ret = netCfgClass->EnumComponents(&iter);
    if (ret != S_OK)
    {
        printf("EnumComponents() returned %x\n", ret);
        goto _enum_comp_failed;
    }

    iter->Reset();

    while ((ret = iter->Next(1, &netCfgComp, NULL)) == S_OK)
    {
        LPWSTR idStr, nameStr;
        GUID guid;

        netCfgComp->GetBindName(&idStr);
        netCfgComp->GetDisplayName(&nameStr);

        if (compStr == NULL || (StrCmpLogicalW(idStr, compStr) == 0))
        {
            count += configBindings(netCfgComp, pathStr, status);

            CoTaskMemFree(idStr);
            CoTaskMemFree(nameStr);
        }
    }


_query_class_failed:
_enum_comp_failed:
    ret = netCfg->Uninitialize();
    if (ret != S_OK)
    {
        printf("netCfg->Uninitialize() returned %x\n", ret);
    }

_initialize_failed:
    lock->ReleaseWriteLock();

_query_interface_lock_failed:
_lock_acquire_failed:
    netCfg->Release();

_co_create_instance_failed:
    CoUninitialize();

_co_initialize_ex_failed:
    return count;
_invalid_param:
    return 255;
}

// bindingStatus() returns "enabled" if one or more bindings are enabled, 
// else returns "disabled"
BindingStatus bindingStatus(CompClass compClass, char *comp, char *path)
{
    if (bindconfig(compClass, comp, path, -1) > 0)
        return kBindingEnabled;
    else
        return kBindingDisabled;
}

// setBindingStatus() returns true if the new 'status' has been successfully
// set otherwise returns false
bool setBindingStatus(CompClass compClass, char *comp, char *path, 
        BindingStatus status)
{
    if (status == kBindingEnabled)
    {
        if (bindconfig(compClass, comp, path, 1) > 0)
            return true;
        else
            return false;
    }
    else if (status == kBindingDisabled)
    {
        if (bindconfig(compClass, comp, path, 0) == 0)
            return true;
        else
            return false;
    }

    return false;
}

