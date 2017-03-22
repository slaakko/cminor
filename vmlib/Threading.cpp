// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cminor/vmlib/Threading.hpp>
#include <mutex>
#include <condition_variable>

namespace cminor { namespace vmlib {

class LockTable
{
public:
    static void Init();
    static void Done();
    static LockTable& Instance() { Assert(instance, "lock table instance not set"); return *instance; }
    int32_t CreateLock();
    void DestroyLock(int32_t lockId);
    std::recursive_mutex& GetLock(int32_t lockId);
    void AcquireLock(int32_t lockId);
    void ReleaseLock(int32_t lockId);
private:
    const int32_t maxNoLockingMtxSize = 256;
    static std::unique_ptr<LockTable> instance;
    std::atomic_int32_t nextLockId;
    std::mutex mtx;
    std::vector<std::unique_ptr<std::recursive_mutex>> mutexes;
    std::unordered_map<uint32_t, std::unique_ptr<std::recursive_mutex>> mutexMap;
    LockTable();
};

std::unique_ptr<LockTable> LockTable::instance;

void LockTable::Init()
{
    instance.reset(new LockTable());
}

void LockTable::Done()
{
    instance.reset();
}

LockTable::LockTable() : nextLockId(0)
{
    mutexes.resize(maxNoLockingMtxSize);
}

int32_t LockTable::CreateLock()
{
    int32_t lockId = nextLockId++;
    Assert(lockId >= 0, "invalid lock id");
    if (lockId < maxNoLockingMtxSize)
    {
        mutexes[lockId].reset(new std::recursive_mutex());
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        mutexMap.insert(std::make_pair(lockId, std::unique_ptr<std::recursive_mutex>(new std::recursive_mutex())));
    }
    return lockId;
}

void LockTable::DestroyLock(int32_t lockId)
{
    Assert(lockId >= 0, "invalid lock id");
    if (lockId < maxNoLockingMtxSize)
    {
        mutexes[lockId].reset();
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        mutexMap.erase(lockId);
    }
}

std::recursive_mutex& LockTable::GetLock(int32_t lockId)
{
    if (lockId < 0)
    {
        throw std::runtime_error("invalid lock id " + std::to_string(lockId));
    }
    else if (lockId < maxNoLockingMtxSize)
    {
        std::recursive_mutex* m = mutexes[lockId].get();
        if (m)
        {
            return *m;
        }
        else
        {
            throw std::runtime_error("invalid lock id " + std::to_string(lockId));
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = mutexMap.find(lockId);
        if (it != mutexMap.cend())
        {
            std::recursive_mutex* m = it->second.get();
            if (m)
            {
                return *m;
            }
            else
            {
                throw std::runtime_error("invalid lock id " + std::to_string(lockId));
            }
        }
        else
        {
            throw std::runtime_error("invalid lock id " + std::to_string(lockId));
        }
    }
}

void LockTable::AcquireLock(int32_t lockId)
{
    if (lockId < 0)
    {
        throw std::runtime_error("invalid lock id " + std::to_string(lockId));
    }
    else if (lockId < maxNoLockingMtxSize)
    {
        std::recursive_mutex* m = mutexes[lockId].get();
        if (m)
        {
            m->lock();
        }
        else
        {
            throw std::runtime_error("invalid lock id " + std::to_string(lockId));
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = mutexMap.find(lockId);
        if (it != mutexMap.cend())
        {
            std::recursive_mutex* m = it->second.get();
            if (m)
            {
                m->lock();
            }
            else
            {
                throw std::runtime_error("invalid lock id " + std::to_string(lockId));
            }
        }
        else
        {
            throw std::runtime_error("invalid lock id " + std::to_string(lockId));
        }
    }
}

void LockTable::ReleaseLock(int32_t lockId)
{
    if (lockId < 0)
    {
        throw std::runtime_error("invalid lock id " + std::to_string(lockId));
    }
    else if (lockId < maxNoLockingMtxSize)
    {
        std::recursive_mutex* m = mutexes[lockId].get();
        if (m)
        {
            m->unlock();
        }
        else
        {
            throw std::runtime_error("invalid lock id " + std::to_string(lockId));
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = mutexMap.find(lockId);
        if (it != mutexMap.cend())
        {
            std::recursive_mutex* m = it->second.get();
            if (m)
            {
                m->unlock();
            }
            else
            {
                throw std::runtime_error("invalid lock id " + std::to_string(lockId));
            }
        }
        else
        {
            throw std::runtime_error("invalid lock id " + std::to_string(lockId));
        }
    }
}

std::mutex createLockMutex;

void EnterMonitor(ManagedAllocationHeader* header)
{
    if (header->LockId() == lockNotAllocated)
    {
        std::lock_guard<std::mutex> lock(createLockMutex);
        if (header->LockId() == lockNotAllocated)
        {
            header->SetLockId(LockTable::Instance().CreateLock());
        }
    }
    LockTable::Instance().AcquireLock(header->LockId());
}

void ExitMonitor(ManagedAllocationHeader* header)
{
    LockTable::Instance().ReleaseLock(header->LockId());
}

void DestroyLock(uint32_t lockId)
{
    LockTable::Instance().DestroyLock(lockId);
}

class ConditionVariableTable
{
public:
    static void Init();
    static void Done();
    static ConditionVariableTable& Instance() { Assert(instance, "condition variable table not initialized"); return *instance; }
    int32_t CreateConditionVariable();
    void DestroyConditionVariable(int32_t conditionVariableId);
    std::condition_variable_any& GetConditionVariable(int32_t conditionVariableId);
private:
    const int32_t maxNoLockConditionVariables = 256;
    static std::unique_ptr<ConditionVariableTable> instance;
    std::atomic_int32_t nextConditionVariableId;
    std::vector<std::unique_ptr<std::condition_variable_any>> conditionVariables;
    std::unordered_map<int32_t, std::unique_ptr<std::condition_variable_any>> conditionVariableMap;
    std::mutex mtx;
    ConditionVariableTable();
};

std::unique_ptr<ConditionVariableTable> ConditionVariableTable::instance;

void ConditionVariableTable::Init()
{
    instance.reset(new ConditionVariableTable());
}

void ConditionVariableTable::Done()
{
    instance.reset();
}

ConditionVariableTable::ConditionVariableTable() : nextConditionVariableId(0)
{
    conditionVariables.resize(maxNoLockConditionVariables);
}

int32_t ConditionVariableTable::CreateConditionVariable()
{
    int32_t conditionVariableId = nextConditionVariableId++;
    Assert(conditionVariableId >= 0, "invalid condition variable id");
    if (conditionVariableId < maxNoLockConditionVariables)
    {
        conditionVariables[conditionVariableId].reset(new std::condition_variable_any());
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        conditionVariableMap[conditionVariableId].reset(new std::condition_variable_any());
    }
    return conditionVariableId;
}

void ConditionVariableTable::DestroyConditionVariable(int32_t conditionVariableId)
{
    Assert(conditionVariableId >= 0, "invalid condition variable id");
    if (conditionVariableId < maxNoLockConditionVariables)
    {
        conditionVariables[conditionVariableId].reset();
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        conditionVariableMap.erase(conditionVariableId);
    }
}

std::condition_variable_any& ConditionVariableTable::GetConditionVariable(int32_t conditionVariableId)
{
    if (conditionVariableId < 0)
    {
        throw std::runtime_error("invalid condition variable id " + std::to_string(conditionVariableId));
    }
    else if (conditionVariableId < maxNoLockConditionVariables)
    {
        std::condition_variable_any* c = conditionVariables[conditionVariableId].get();
        if (c)
        {
            return *c;
        }
        else
        {
            throw std::runtime_error("invalid condition variable id " + std::to_string(conditionVariableId));
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = conditionVariableMap.find(conditionVariableId);
        if (it != conditionVariableMap.cend())
        {
            std::condition_variable_any* c = it->second.get();
            if (c)
            {
                return *c;
            }
            else
            {
                throw std::runtime_error("invalid condition variable id " + std::to_string(conditionVariableId));
            }
        }
        else
        {
            throw std::runtime_error("invalid condition variable id " + std::to_string(conditionVariableId));
        }
    }
}

int32_t CreateConditionVariable()
{
    return ConditionVariableTable::Instance().CreateConditionVariable();
}

void DestroyConditionVariable(int32_t conditionVariableId)
{
    ConditionVariableTable::Instance().DestroyConditionVariable(conditionVariableId);
}

void NotifyOne(int32_t conditionVariableId)
{
    std::condition_variable_any& conditionVariable = ConditionVariableTable::Instance().GetConditionVariable(conditionVariableId);
    conditionVariable.notify_one();
}

void NotifyAll(int32_t conditionVariableId)
{
    std::condition_variable_any& conditionVariable = ConditionVariableTable::Instance().GetConditionVariable(conditionVariableId);
    conditionVariable.notify_all();
}

void WaitConditionVariable(int32_t conditionVariableId, int32_t lockId)
{
    std::recursive_mutex& mtx = LockTable::Instance().GetLock(lockId);
    std::condition_variable_any& conditionVariable = ConditionVariableTable::Instance().GetConditionVariable(conditionVariableId);
    conditionVariable.wait(mtx);
}

CondVarStatus WaitConditionVariable(int32_t conditionVariableId, int32_t lockId, std::chrono::nanoseconds duration)
{
    std::recursive_mutex& mtx = LockTable::Instance().GetLock(lockId);
    std::condition_variable_any& conditionVariable = ConditionVariableTable::Instance().GetConditionVariable(conditionVariableId);
    CondVarStatus status = CondVarStatus::timeout;
    if (conditionVariable.wait_for(mtx, duration) == std::cv_status::no_timeout)
    {
        status = CondVarStatus::no_timeout;
    }
    return status;
}

void ThreadingInit()
{
    LockTable::Init();
    SetDestroyLockFn(DestroyLock);
    ConditionVariableTable::Init();
    SetDestroyConditionVariableFn(DestroyConditionVariable);
}

void ThreadingDone()
{
    LockTable::Done();
    ConditionVariableTable::Done();
}

} } // namespace cminor::vmlib
