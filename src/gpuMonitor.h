#ifndef _GPU_MONITOR_H
#define _GPU_MONITOR_H

#include <Windows.h>
#include <vector>
#include <string>
#include <stdint.h>
#include "../d3dkmt/d3dkmthk.h"
#include "../config/dltmgr.h"
#include "../config/ntifs.h"

#ifndef GPU_DATA_LIST_SIZE
#define GPU_DATA_LIST_SIZE 5
#endif // !GPU_DATA_LIST_SIZE

// Macros

#define BYTES_NEEDED_FOR_BITS(Bits) ((((Bits) + sizeof(ULONG) * 8 - 1) / 8) & ~(SIZE_T)(sizeof(ULONG) - 1)) // divide round up

// Structures

typedef struct _D3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1
{
    ULONG CommitLimit;
    ULONG BytesCommitted;
    ULONG BytesResident;
    D3DKMT_QUERYSTATISTICS_MEMORY Memory;
    ULONG Aperture; // boolean
    ULONGLONG TotalBytesEvictedByPriority[D3DKMT_MaxAllocationPriorityClass];
    ULONG64 SystemMemoryEndAddress;
    struct
    {
        ULONG64 PreservedDuringStandby : 1;
        ULONG64 PreservedDuringHibernate : 1;
        ULONG64 PartiallyPreservedDuringHibernate : 1;
        ULONG64 Reserved : 61;
    } PowerFlags;
    ULONG64 Reserved[7];
} D3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1, * PD3DKMT_QUERYSTATISTICS_SEGMENT_INFORMATION_V1;

enum GpuMonitorDataIndex
{
    GPU_UTILIZATION = 0,
    GPU_DEDICATED_USAGE = 1,
    GPU_DEDICATED_LIMIT = 2,
    GPU_SHARED_USAGE = 3,
    GPU_SHARED_LIMIT = 4
};

union FLOAT_ULONG64
{
    FLOAT float_;
    ULONG64 ulong64_;
};


class GpuMonitor {
public:
    GpuMonitor();
    ~GpuMonitor();

    bool start();
    std::vector<FLOAT_ULONG64> collect();
    bool stop();

private:
    bool initializeD3DStatistics();
    bool PhHeapInitialization(SIZE_T HeapReserveSize, SIZE_T HeapCommitSize);
    bool PhInitializeWindowsVersion();
    bool EtpIsGpuSoftwareDevice(_In_ D3DKMT_HANDLE AdapterHandle);
    NTSTATUS EtQueryAdapterInformation(
        _In_ D3DKMT_HANDLE AdapterHandle,
        _In_ KMTQUERYADAPTERINFOTYPE InformationClass,
        _Out_writes_bytes_opt_(InformationLength) PVOID Information,
        _In_ UINT32 InformationLength
    );
    BOOLEAN EtCloseAdapterHandle(
        _In_ D3DKMT_HANDLE AdapterHandle
    );

private:
    BOOLEAN EtGpuEnabled_ = FALSE;
    BOOLEAN EtGpuSupported_ = FALSE;
    BOOLEAN EtD3DEnabled_ = FALSE;

    PVOID PhInstanceHandle_ = NULL;
    PVOID PhHeapHandle_ = NULL;
    ULONG windowsVersion_ = { 0 };

    ULONG EtGpuTotalNodeCount_ = 0;
    ULONG EtGpuTotalSegmentCount_ = 0;

    PH_UINT64_DELTA EtClockTotalRunningTimeDelta_ = { 0, 0 };
    LARGE_INTEGER EtClockTotalRunningTimeFrequency_ = { 0 };

    ULONG64 EtGpuDedicatedLimit_ = { 0 };
    ULONG64 EtGpuSharedLimit_ = { 0 };

    std::vector<FLOAT_ULONG64> dataList_;
};


#endif // !_GPU_MONITOR_H
