#include <cstdio>
#include <stdafx.h>
#include "guest_thread.h"
#include <kernel/memory.h>
#include <kernel/heap.h>
#include <kernel/function.h>
#include "ppc_context.h"
#include <SDL.h>

extern void PumpSdlEventsIfNeeded();

constexpr size_t PCR_SIZE = X360_PCR_SIZE;
constexpr size_t TLS_SIZE = X360_TLS_SIZE;
constexpr size_t TEB_SIZE = X360_TEB_SIZE;
constexpr size_t STACK_SIZE = X360_STACK_SIZE;
constexpr size_t TOTAL_SIZE = X360_THREAD_CONTEXT_TOTAL;
constexpr size_t TEB_OFFSET = X360_TEB_OFFSET;

GuestThreadContext::GuestThreadContext(uint32_t cpuNumber)
{
    assert(thread == nullptr);

    // Block allocation order is PCR, TLS, TEB, Stack
    thread = (uint8_t*)g_userHeap.AllocPhysical(TOTAL_SIZE, 16);
    memset(thread, 0, TOTAL_SIZE);

    X360_PCR* pcr = reinterpret_cast<X360_PCR*>(thread);
    X360_TLS* tls = reinterpret_cast<X360_TLS*>(thread + X360_TLS_OFFSET);
    X360_TEB* teb = reinterpret_cast<X360_TEB*>(thread + X360_TEB_OFFSET);

    // calc virtual addresses (calc is short for calculate btw)
    uint32_t pcrAddr   = g_memory.MapVirtual(thread);
    uint32_t tlsAddr   = g_memory.MapVirtual(thread + X360_TLS_OFFSET);
    uint32_t tebAddr   = g_memory.MapVirtual(thread + X360_TEB_OFFSET);
    uint32_t stackBase = g_memory.MapVirtual(thread + TOTAL_SIZE);
    uint32_t stackLimit = g_memory.MapVirtual(thread + X360_STACK_OFFSET);

    // init processor control region (PCR)
    pcr->tls_ptr = tlsAddr;
    pcr->pcr_ptr = pcrAddr;
    pcr->stack_base = stackBase;
    pcr->stack_limit = stackLimit;
    pcr->current_thread = tebAddr;
    pcr->current_cpu = cpuNumber;
    // Note: dpc_active (0x150) left as 0 - setting it suppresses error reporting

    // init Thread local storage (TLS)
    tls->quirky_slot = 0xFFFFFFFF;

    // init Thread Environment block (TEB)
    teb->thread_id = GuestThread::GetCurrentThreadId();
    teb->start_address = 0;
    teb->last_error = 0;
    teb->fiber_ptr = 0;
    teb->creation_flags = 0;

    // init ppc context
    ppcContext.r1.u64 = stackBase; // Stack pointer
    ppcContext.r13.u64 = pcrAddr; //TLS pointer
    ppcContext.fpscr.loadFromHost();

    assert(GetPPCContext() == nullptr);
    SetPPCContext(ppcContext);
}

GuestThreadContext::~GuestThreadContext()
{
    g_userHeap.Free(thread);
}

#ifdef USE_PTHREAD
static size_t GetStackSize()
{
    // Cache as this should not change.
    static size_t stackSize = 0;
    if (stackSize == 0)
    {
        // 8 MiB is a typical default.
        constexpr auto defaultSize = 8 * 1024 * 1024;
        struct rlimit lim;
        const auto ret = getrlimit(RLIMIT_STACK, &lim);
        if (ret == 0 && lim.rlim_cur < defaultSize)
        {
            // Use what the system allows.
            stackSize = lim.rlim_cur;
        }
        else
        {
            stackSize = defaultSize;
        }
    }
    return stackSize;
}

static void* GuestThreadFunc(void* arg)
{
    GuestThreadHandle* hThread = (GuestThreadHandle*)arg;
#else
static void* GuestThreadFunc(GuestThreadHandle* hThread)
{
#endif
    bool wasSuspended = hThread->suspended.load();
    fprintf(stderr, "[GuestThreadFunc] Thread starting, suspended=%d, waiting...\n", wasSuspended ? 1 : 0);
    if (wasSuspended) {
        hThread->suspended.wait(true);
        fprintf(stderr, "[GuestThreadFunc] Thread RESUMED after wait\n");
    } else {
        fprintf(stderr, "[GuestThreadFunc] Thread NOT suspended, running immediately\n");
    }
    GuestThread::Start(hThread->params);
    // HACK(1)
    hThread->isFinished = true;
    return nullptr;
}

GuestThreadHandle::GuestThreadHandle(const GuestThreadParams& params)
    : params(params), suspended((params.flags & 0x1) != 0)
#ifdef USE_PTHREAD
{
    // Debug: verify flags and suspended state before creating thread
    bool shouldBeSuspended = (params.flags & 0x1) != 0;
    fprintf(stderr, "[GuestThreadHandle] CTOR flags=0x%X shouldSuspend=%d suspended=%d\n",
            params.flags, shouldBeSuspended ? 1 : 0, suspended.load() ? 1 : 0);
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, GetStackSize());
    const auto ret = pthread_create(&thread, &attr, GuestThreadFunc, this);
    if (ret != 0) {
        fprintf(stderr, "pthread_create failed with error code 0x%X.\n", ret);
        return;
    }
}
#else
, thread(GuestThreadFunc, this)
{
}
#endif

GuestThreadHandle::~GuestThreadHandle()
{
#ifdef USE_PTHREAD
    pthread_join(thread, nullptr);
#else
    if (thread.joinable())
        thread.join();
#endif
}

template <typename ThreadType>
static uint32_t CalcThreadId(const ThreadType& id)
{
    if constexpr (sizeof(id) == 4)
        return *reinterpret_cast<const uint32_t*>(&id);
    else
        return XXH32(&id, sizeof(id), 0);
}

uint32_t GuestThreadHandle::GetThreadId() const
{
#ifdef USE_PTHREAD
    return CalcThreadId(thread);
#else
    return CalcThreadId(thread.get_id());
#endif
}

uint32_t GuestThreadHandle::Wait(uint32_t timeout)
{
    if (timeout == INFINITE || isFinished.load()) // HACK(1): isFinished
    {
#ifdef USE_PTHREAD
        pthread_join(thread, nullptr);
#else
        if (thread.joinable())
            thread.join();
#endif

        return STATUS_WAIT_0;
    }
    else if (timeout == 0)
    {
#ifndef USE_PTHREAD
        if (thread.joinable())
            return STATUS_TIMEOUT;
#endif

        return STATUS_WAIT_0;
    }
    else
    {
#ifdef USE_PTHREAD
        pthread_join(thread, nullptr);
#else
        auto start = std::chrono::steady_clock::now();
        while (thread.joinable())
        {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() >= timeout)
                return STATUS_TIMEOUT;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
#endif

        return STATUS_WAIT_0;
    }
}

uint32_t GuestThread::Start(const GuestThreadParams& params)
{
    PumpSdlEventsIfNeeded();
    
    const auto procMask = (uint8_t)(params.flags >> 24);
    const auto cpuNumber = procMask == 0 ? 0 : 7 - std::countl_zero(procMask);

    GuestThreadContext ctx(cpuNumber);
    ctx.ppcContext.r3.u64 = params.value;
    ctx.ppcContext.r4.u64 = params.value2;

    printf("GuestThread: Starting guest code at 0x%08X\n", params.function);
    fflush(stdout);
    
    auto func = g_memory.FindFunction(params.function);
    if (func == nullptr) {
        printf("GuestThread: ERROR: Function not found at 0x%08X\n", params.function);
        fflush(stdout);
        return 0;
    }
    
    printf("GuestThread: Calling function...\n");
    fflush(stdout);
    
    func(ctx.ppcContext, g_memory.base);

    printf("GuestThread: Guest code returned\n");
    fflush(stdout);
    
    return ctx.ppcContext.r3.u32;
}

GuestThreadHandle* GuestThread::Start(const GuestThreadParams& params, uint32_t* threadId)
{
    auto hThread = CreateKernelObject<GuestThreadHandle>(params);

    if (threadId != nullptr)
    {
        *threadId = hThread->GetThreadId();
    }

    return hThread;
}

uint32_t GuestThread::GetCurrentThreadId()
{
#ifdef USE_PTHREAD
    return CalcThreadId(pthread_self());
#else
    return CalcThreadId(std::this_thread::get_id());
#endif
}

void GuestThread::SetLastError(uint32_t error)
{
    auto* thread = (uint8_t*)g_memory.Translate(GetPPCContext()->r13.u32);
    X360_PCR* pcr = reinterpret_cast<X360_PCR*>(thread);
    X360_TEB* teb = reinterpret_cast<X360_TEB*>(thread + X360_TEB_OFFSET);
    
    // Check PCR dpc_active field
    if (pcr->dpc_active != 0)
    {
        // Program doesn't want errors
        return;
    }

    // Set Win32 as the last error in TEB
    teb->last_error = error;
}

#ifdef _WIN32
void GuestThread::SetThreadName(uint32_t threadId, const char* name)
{
#pragma pack(push,8)
    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
#endif

void SetThreadNameImpl(uint32_t a1, uint32_t threadId, uint32_t* name)
{
#ifdef _WIN32
    GuestThread::SetThreadName(threadId, (const char*)g_memory.Translate(ByteSwap(*name)));
#endif
}

int GetThreadPriorityImpl(GuestThreadHandle* hThread)
{
#ifdef _WIN32
    return GetThreadPriority(hThread == GetKernelObject(CURRENT_THREAD_HANDLE) ? GetCurrentThread() : hThread->thread.native_handle());
#else 
    return 0;
#endif
}

uint32_t SetThreadIdealProcessorImpl(GuestThreadHandle* hThread, uint32_t dwIdealProcessor)
{
    return 0;
}

// GUEST_FUNCTION_HOOK(sub_82DFA2E8, SetThreadNameImpl);
// GUEST_FUNCTION_HOOK(sub_82BD57A8, GetThreadPriorityImpl);
GUEST_FUNCTION_HOOK(sub_82537F80, SetThreadIdealProcessorImpl);

// GUEST_FUNCTION_STUB(sub_82BD58F8); // Some function that updates the TEB, don't really care since the field is not set