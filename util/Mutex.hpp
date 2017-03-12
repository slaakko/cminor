// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMINOR_UTIL_MUTEX_INCLUDED
#define CMINOR_UTIL_MUTEX_INCLUDED
#include <mutex>

namespace cminor {namespace util {

class MutexOwner
{
public:
    MutexOwner(char id_);
    void Push(char mutexId);
    void Pop();
private:
    char id;
    std::string ownedMutexes;
};

class Mutex;

class OwnerGuard
{
public:
    OwnerGuard(Mutex& mtx_, MutexOwner& owner_);
    ~OwnerGuard();
private:
    Mutex& mtx;
    MutexOwner& owner;
};

class LockGuard
{
public:
    LockGuard(Mutex& mtx_, MutexOwner& owner_);
    ~LockGuard();
    void Lock();
    void Unlock();
private:
    Mutex& mtx;
    MutexOwner& owner;
    bool locked;
};

class Mutex
{
public:
    Mutex(char id_);
    char Id() const { return id; }
    void Lock() { mtx.lock(); }
    void Unlock() { mtx.unlock(); }
    std::mutex& Mtx() { return mtx; }
private:
    char id;
    std::mutex mtx;
};

} } // namespace cminor::util

#endif // CMINOR_UTIL_MUTEX_INCLUDED
