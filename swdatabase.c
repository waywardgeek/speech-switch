/*----------------------------------------------------------------------------------------
  Database sw
----------------------------------------------------------------------------------------*/

#include "swdatabase.h"

struct swRootType_ swRootData;
uint8 swModuleID;
struct swRootFields swRoots;
struct swUserFields swUsers;
struct swMengineFields swMengines;
struct swEngineFields swEngines;
struct swVoiceFields swVoices;
struct swQueueFields swQueues;
struct swMessageFields swMessages;
struct swLanguageFields swLanguages;

/*----------------------------------------------------------------------------------------
  Constructor/Destructor hooks.
----------------------------------------------------------------------------------------*/
swRootCallbackType swRootConstructorCallback;
swRootCallbackType swRootDestructorCallback;
swUserCallbackType swUserConstructorCallback;
swUserCallbackType swUserDestructorCallback;
swMengineCallbackType swMengineConstructorCallback;
swMengineCallbackType swMengineDestructorCallback;
swEngineCallbackType swEngineConstructorCallback;
swEngineCallbackType swEngineDestructorCallback;
swVoiceCallbackType swVoiceConstructorCallback;
swVoiceCallbackType swVoiceDestructorCallback;
swQueueCallbackType swQueueConstructorCallback;
swQueueCallbackType swQueueDestructorCallback;
swMessageCallbackType swMessageConstructorCallback;
swMessageCallbackType swMessageDestructorCallback;
swLanguageCallbackType swLanguageConstructorCallback;
swLanguageCallbackType swLanguageDestructorCallback;

/*----------------------------------------------------------------------------------------
  Destroy Root including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swRootDestroy(
    swRoot Root)
{
    swUser User_;
    swEngine Engine_;
    swQueue InQueue_;
    swQueue OutQueue_;
    swQueue AudioQueue_;
    swLanguage Language_;

    if(swRootDestructorCallback != NULL) {
        swRootDestructorCallback(Root);
    }
    swSafeForeachRootUser(Root, User_) {
        swUserDestroy(User_);
    } swEndSafeRootUser;
    swSafeForeachRootEngine(Root, Engine_) {
        swEngineDestroy(Engine_);
    } swEndSafeRootEngine;
    InQueue_ = swRootGetInQueue(Root);
    if(InQueue_ != swQueueNull) {
        swQueueDestroy(InQueue_);
    }
    OutQueue_ = swRootGetOutQueue(Root);
    if(OutQueue_ != swQueueNull) {
        swQueueDestroy(OutQueue_);
    }
    AudioQueue_ = swRootGetAudioQueue(Root);
    if(AudioQueue_ != swQueueNull) {
        swQueueDestroy(AudioQueue_);
    }
    swSafeForeachRootLanguage(Root, Language_) {
        swLanguageDestroy(Language_);
    } swEndSafeRootLanguage;
    swRootFree(Root);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocRoot(void)
{
    swRoot Root = swRootAlloc();

    return swRoot2Index(Root);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyRoot(
    uint64 objectIndex)
{
    swRootDestroy(swIndex2Root((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Root.
----------------------------------------------------------------------------------------*/
static void allocRoots(void)
{
    swSetAllocatedRoot(2);
    swSetUsedRoot(1);
    swSetFirstFreeRoot(swRootNull);
    swRoots.FirstUser = utNewAInitFirst(swUser, (swAllocatedRoot()));
    swRoots.LastUser = utNewAInitFirst(swUser, (swAllocatedRoot()));
    swRoots.UserTableIndex_ = utNewAInitFirst(uint32, (swAllocatedRoot()));
    swRoots.NumUserTable = utNewAInitFirst(uint32, (swAllocatedRoot()));
    swSetUsedRootUserTable(0);
    swSetAllocatedRootUserTable(2);
    swSetFreeRootUserTable(0);
    swRoots.UserTable = utNewAInitFirst(swUser, swAllocatedRootUserTable());
    swRoots.NumUser = utNewAInitFirst(uint32, (swAllocatedRoot()));
    swRoots.FirstEngine = utNewAInitFirst(swEngine, (swAllocatedRoot()));
    swRoots.LastEngine = utNewAInitFirst(swEngine, (swAllocatedRoot()));
    swRoots.InQueue = utNewAInitFirst(swQueue, (swAllocatedRoot()));
    swRoots.OutQueue = utNewAInitFirst(swQueue, (swAllocatedRoot()));
    swRoots.AudioQueue = utNewAInitFirst(swQueue, (swAllocatedRoot()));
    swRoots.FirstLanguage = utNewAInitFirst(swLanguage, (swAllocatedRoot()));
    swRoots.LastLanguage = utNewAInitFirst(swLanguage, (swAllocatedRoot()));
    swRoots.LanguageTableIndex_ = utNewAInitFirst(uint32, (swAllocatedRoot()));
    swRoots.NumLanguageTable = utNewAInitFirst(uint32, (swAllocatedRoot()));
    swSetUsedRootLanguageTable(0);
    swSetAllocatedRootLanguageTable(2);
    swSetFreeRootLanguageTable(0);
    swRoots.LanguageTable = utNewAInitFirst(swLanguage, swAllocatedRootLanguageTable());
    swRoots.NumLanguage = utNewAInitFirst(uint32, (swAllocatedRoot()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Root.
----------------------------------------------------------------------------------------*/
static void reallocRoots(
    uint32 newSize)
{
    utResizeArray(swRoots.FirstUser, (newSize));
    utResizeArray(swRoots.LastUser, (newSize));
    utResizeArray(swRoots.UserTableIndex_, (newSize));
    utResizeArray(swRoots.NumUserTable, (newSize));
    utResizeArray(swRoots.NumUser, (newSize));
    utResizeArray(swRoots.FirstEngine, (newSize));
    utResizeArray(swRoots.LastEngine, (newSize));
    utResizeArray(swRoots.InQueue, (newSize));
    utResizeArray(swRoots.OutQueue, (newSize));
    utResizeArray(swRoots.AudioQueue, (newSize));
    utResizeArray(swRoots.FirstLanguage, (newSize));
    utResizeArray(swRoots.LastLanguage, (newSize));
    utResizeArray(swRoots.LanguageTableIndex_, (newSize));
    utResizeArray(swRoots.NumLanguageTable, (newSize));
    utResizeArray(swRoots.NumLanguage, (newSize));
    swSetAllocatedRoot(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Roots.
----------------------------------------------------------------------------------------*/
void swRootAllocMore(void)
{
    reallocRoots((uint32)(swAllocatedRoot() + (swAllocatedRoot() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Root.UserTable heap to free memory.
----------------------------------------------------------------------------------------*/
void swCompactRootUserTables(void)
{
    uint32 elementSize = sizeof(swUser);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    swUser *toPtr = swRoots.UserTable;
    swUser *fromPtr = toPtr;
    swRoot Root;
    uint32 size;

    while(fromPtr < swRoots.UserTable + swUsedRootUserTable()) {
        Root = *(swRoot *)(void *)fromPtr;
        if(Root != swRootNull) {
            /* Need to move it to toPtr */
            size = utMax(swRootGetNumUserTable(Root) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            swRootSetUserTableIndex_(Root, toPtr - swRoots.UserTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((swRoot *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    swSetUsedRootUserTable(toPtr - swRoots.UserTable);
    swSetFreeRootUserTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Root.UserTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreRootUserTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = swAllocatedRootUserTable() - swUsedRootUserTable();
    uint32 elementSize = sizeof(swUser);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    swUser *ptr = swRoots.UserTable;
    swRoot Root;
    uint32 size;

    while(ptr < swRoots.UserTable + swUsedRootUserTable()) {
        Root = *(swRoot*)(void*)ptr;
        if(Root != swRootNull) {
            swValidRoot(Root);
            size = utMax(swRootGetNumUserTable(Root) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((swRoot *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((swFreeRootUserTable() << 2) > swUsedRootUserTable()) {
        swCompactRootUserTables();
        freeSpace = swAllocatedRootUserTable() - swUsedRootUserTable();
    }
    if(freeSpace < spaceNeeded) {
        swSetAllocatedRootUserTable(swAllocatedRootUserTable() + spaceNeeded - freeSpace +
            (swAllocatedRootUserTable() >> 1));
        utResizeArray(swRoots.UserTable, swAllocatedRootUserTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Root.UserTable array.
----------------------------------------------------------------------------------------*/
void swRootAllocUserTables(
    swRoot Root,
    uint32 numUserTables)
{
    uint32 freeSpace = swAllocatedRootUserTable() - swUsedRootUserTable();
    uint32 elementSize = sizeof(swUser);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numUserTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(swRootGetNumUserTable(Root) == 0);
#endif
    if(numUserTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreRootUserTables(spaceNeeded);
    }
    swRootSetUserTableIndex_(Root, swUsedRootUserTable() + usedHeaderSize);
    swRootSetNumUserTable(Root, numUserTables);
    *(swRoot *)(void *)(swRoots.UserTable + swUsedRootUserTable()) = Root;
    {
        uint32 xValue;
        for(xValue = (uint32)(swRootGetUserTableIndex_(Root)); xValue < swRootGetUserTableIndex_(Root) + numUserTables; xValue++) {
            swRoots.UserTable[xValue] = swUserNull;
        }
    }
    swSetUsedRootUserTable(swUsedRootUserTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swRootGetUserTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getRootUserTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    swRoot Root = swIndex2Root((uint32)objectNumber);

    *numValues = swRootGetNumUserTable(Root);
    return swRootGetUserTables(Root);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swRootAllocUserTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocRootUserTables(
    uint64 objectNumber,
    uint32 numValues)
{
    swRoot Root = swIndex2Root((uint32)objectNumber);

    swRootSetUserTableIndex_(Root, 0);
    swRootSetNumUserTable(Root, 0);
    if(numValues == 0) {
        return NULL;
    }
    swRootAllocUserTables(Root, numValues);
    return swRootGetUserTables(Root);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Root.UserTable array.
----------------------------------------------------------------------------------------*/
void swRootFreeUserTables(
    swRoot Root)
{
    uint32 elementSize = sizeof(swUser);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(swRootGetNumUserTable(Root) + usedHeaderSize, freeHeaderSize);
    swUser *dataPtr = swRootGetUserTables(Root) - usedHeaderSize;

    if(swRootGetNumUserTable(Root) == 0) {
        return;
    }
    *(swRoot *)(void *)(dataPtr) = swRootNull;
    *(uint32 *)(void *)(((swRoot *)(void *)dataPtr) + 1) = size;
    swRootSetNumUserTable(Root, 0);
    swSetFreeRootUserTable(swFreeRootUserTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Root.UserTable array.
----------------------------------------------------------------------------------------*/
void swRootResizeUserTables(
    swRoot Root,
    uint32 numUserTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(swUser);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numUserTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(swRootGetNumUserTable(Root) + usedHeaderSize, freeHeaderSize);
    swUser *dataPtr;

    if(numUserTables == 0) {
        if(swRootGetNumUserTable(Root) != 0) {
            swRootFreeUserTables(Root);
        }
        return;
    }
    if(swRootGetNumUserTable(Root) == 0) {
        swRootAllocUserTables(Root, numUserTables);
        return;
    }
    freeSpace = swAllocatedRootUserTable() - swUsedRootUserTable();
    if(freeSpace < newSize) {
        allocMoreRootUserTables(newSize);
    }
    dataPtr = swRootGetUserTables(Root) - usedHeaderSize;
    memcpy((void *)(swRoots.UserTable + swUsedRootUserTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(swUsedRootUserTable() + oldSize); xValue < swUsedRootUserTable() + oldSize + newSize - oldSize; xValue++) {
                swRoots.UserTable[xValue] = swUserNull;
            }
        }
    }
    *(swRoot *)(void *)dataPtr = swRootNull;
    *(uint32 *)(void *)(((swRoot *)(void *)dataPtr) + 1) = oldSize;
    swSetFreeRootUserTable(swFreeRootUserTable() + oldSize);
    swRootSetUserTableIndex_(Root, swUsedRootUserTable() + usedHeaderSize);
    swRootSetNumUserTable(Root, numUserTables);
    swSetUsedRootUserTable(swUsedRootUserTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Compact the Root.LanguageTable heap to free memory.
----------------------------------------------------------------------------------------*/
void swCompactRootLanguageTables(void)
{
    uint32 elementSize = sizeof(swLanguage);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    swLanguage *toPtr = swRoots.LanguageTable;
    swLanguage *fromPtr = toPtr;
    swRoot Root;
    uint32 size;

    while(fromPtr < swRoots.LanguageTable + swUsedRootLanguageTable()) {
        Root = *(swRoot *)(void *)fromPtr;
        if(Root != swRootNull) {
            /* Need to move it to toPtr */
            size = utMax(swRootGetNumLanguageTable(Root) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            swRootSetLanguageTableIndex_(Root, toPtr - swRoots.LanguageTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((swRoot *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    swSetUsedRootLanguageTable(toPtr - swRoots.LanguageTable);
    swSetFreeRootLanguageTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Root.LanguageTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreRootLanguageTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = swAllocatedRootLanguageTable() - swUsedRootLanguageTable();
    uint32 elementSize = sizeof(swLanguage);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    swLanguage *ptr = swRoots.LanguageTable;
    swRoot Root;
    uint32 size;

    while(ptr < swRoots.LanguageTable + swUsedRootLanguageTable()) {
        Root = *(swRoot*)(void*)ptr;
        if(Root != swRootNull) {
            swValidRoot(Root);
            size = utMax(swRootGetNumLanguageTable(Root) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((swRoot *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((swFreeRootLanguageTable() << 2) > swUsedRootLanguageTable()) {
        swCompactRootLanguageTables();
        freeSpace = swAllocatedRootLanguageTable() - swUsedRootLanguageTable();
    }
    if(freeSpace < spaceNeeded) {
        swSetAllocatedRootLanguageTable(swAllocatedRootLanguageTable() + spaceNeeded - freeSpace +
            (swAllocatedRootLanguageTable() >> 1));
        utResizeArray(swRoots.LanguageTable, swAllocatedRootLanguageTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Root.LanguageTable array.
----------------------------------------------------------------------------------------*/
void swRootAllocLanguageTables(
    swRoot Root,
    uint32 numLanguageTables)
{
    uint32 freeSpace = swAllocatedRootLanguageTable() - swUsedRootLanguageTable();
    uint32 elementSize = sizeof(swLanguage);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numLanguageTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(swRootGetNumLanguageTable(Root) == 0);
#endif
    if(numLanguageTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreRootLanguageTables(spaceNeeded);
    }
    swRootSetLanguageTableIndex_(Root, swUsedRootLanguageTable() + usedHeaderSize);
    swRootSetNumLanguageTable(Root, numLanguageTables);
    *(swRoot *)(void *)(swRoots.LanguageTable + swUsedRootLanguageTable()) = Root;
    {
        uint32 xValue;
        for(xValue = (uint32)(swRootGetLanguageTableIndex_(Root)); xValue < swRootGetLanguageTableIndex_(Root) + numLanguageTables; xValue++) {
            swRoots.LanguageTable[xValue] = swLanguageNull;
        }
    }
    swSetUsedRootLanguageTable(swUsedRootLanguageTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swRootGetLanguageTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getRootLanguageTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    swRoot Root = swIndex2Root((uint32)objectNumber);

    *numValues = swRootGetNumLanguageTable(Root);
    return swRootGetLanguageTables(Root);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swRootAllocLanguageTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocRootLanguageTables(
    uint64 objectNumber,
    uint32 numValues)
{
    swRoot Root = swIndex2Root((uint32)objectNumber);

    swRootSetLanguageTableIndex_(Root, 0);
    swRootSetNumLanguageTable(Root, 0);
    if(numValues == 0) {
        return NULL;
    }
    swRootAllocLanguageTables(Root, numValues);
    return swRootGetLanguageTables(Root);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Root.LanguageTable array.
----------------------------------------------------------------------------------------*/
void swRootFreeLanguageTables(
    swRoot Root)
{
    uint32 elementSize = sizeof(swLanguage);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(swRootGetNumLanguageTable(Root) + usedHeaderSize, freeHeaderSize);
    swLanguage *dataPtr = swRootGetLanguageTables(Root) - usedHeaderSize;

    if(swRootGetNumLanguageTable(Root) == 0) {
        return;
    }
    *(swRoot *)(void *)(dataPtr) = swRootNull;
    *(uint32 *)(void *)(((swRoot *)(void *)dataPtr) + 1) = size;
    swRootSetNumLanguageTable(Root, 0);
    swSetFreeRootLanguageTable(swFreeRootLanguageTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Root.LanguageTable array.
----------------------------------------------------------------------------------------*/
void swRootResizeLanguageTables(
    swRoot Root,
    uint32 numLanguageTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(swLanguage);
    uint32 usedHeaderSize = (sizeof(swRoot) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swRoot) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numLanguageTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(swRootGetNumLanguageTable(Root) + usedHeaderSize, freeHeaderSize);
    swLanguage *dataPtr;

    if(numLanguageTables == 0) {
        if(swRootGetNumLanguageTable(Root) != 0) {
            swRootFreeLanguageTables(Root);
        }
        return;
    }
    if(swRootGetNumLanguageTable(Root) == 0) {
        swRootAllocLanguageTables(Root, numLanguageTables);
        return;
    }
    freeSpace = swAllocatedRootLanguageTable() - swUsedRootLanguageTable();
    if(freeSpace < newSize) {
        allocMoreRootLanguageTables(newSize);
    }
    dataPtr = swRootGetLanguageTables(Root) - usedHeaderSize;
    memcpy((void *)(swRoots.LanguageTable + swUsedRootLanguageTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(swUsedRootLanguageTable() + oldSize); xValue < swUsedRootLanguageTable() + oldSize + newSize - oldSize; xValue++) {
                swRoots.LanguageTable[xValue] = swLanguageNull;
            }
        }
    }
    *(swRoot *)(void *)dataPtr = swRootNull;
    *(uint32 *)(void *)(((swRoot *)(void *)dataPtr) + 1) = oldSize;
    swSetFreeRootLanguageTable(swFreeRootLanguageTable() + oldSize);
    swRootSetLanguageTableIndex_(Root, swUsedRootLanguageTable() + usedHeaderSize);
    swRootSetNumLanguageTable(Root, numLanguageTables);
    swSetUsedRootLanguageTable(swUsedRootLanguageTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Root.
----------------------------------------------------------------------------------------*/
void swRootCopyProps(
    swRoot oldRoot,
    swRoot newRoot)
{
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeRootUserHashTable(
    swRoot Root)
{
    swUser _User, prevUser, nextUser;
    uint32 oldNumUsers = swRootGetNumUserTable(Root);
    uint32 newNumUsers = oldNumUsers << 1;
    uint32 xUser, index;

    if(newNumUsers == 0) {
        newNumUsers = 2;
        swRootAllocUserTables(Root, 2);
    } else {
        swRootResizeUserTables(Root, newNumUsers);
    }
    for(xUser = 0; xUser < oldNumUsers; xUser++) {
        _User = swRootGetiUserTable(Root, xUser);
        prevUser = swUserNull;
        while(_User != swUserNull) {
            nextUser = swUserGetNextTableRootUser(_User);
            index = (newNumUsers - 1) & (uint32)swUserGetId(_User);
            if(index != xUser) {
                if(prevUser == swUserNull) {
                    swRootSetiUserTable(Root, xUser, nextUser);
                } else {
                    swUserSetNextTableRootUser(prevUser, nextUser);
                }
                swUserSetNextTableRootUser(_User, swRootGetiUserTable(Root, index));
                swRootSetiUserTable(Root, index, _User);
            } else {
                prevUser = _User;
            }
            _User = nextUser;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the User to the Root.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addRootUserToHashTable(
    swRoot Root,
    swUser _User)
{
    swUser nextUser;
    uint32 index;

    if(swRootGetNumUser(Root) >> 1 >= swRootGetNumUserTable(Root)) {
        resizeRootUserHashTable(Root);
    }
    index = (swRootGetNumUserTable(Root) - 1) & (uint32)swUserGetId(_User);
    nextUser = swRootGetiUserTable(Root, index);
    swUserSetNextTableRootUser(_User, nextUser);
    swRootSetiUserTable(Root, index, _User);
    swRootSetNumUser(Root, swRootGetNumUser(Root) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the User from the hash table.
----------------------------------------------------------------------------------------*/
static void removeRootUserFromHashTable(
    swRoot Root,
    swUser _User)
{
    uint32 index = (swRootGetNumUserTable(Root) - 1) & (uint32)swUserGetId(_User);
    swUser prevUser, nextUser;
    
    nextUser = swRootGetiUserTable(Root, index);
    if(nextUser == _User) {
        swRootSetiUserTable(Root, index, swUserGetNextTableRootUser(nextUser));
    } else {
        do {
            prevUser = nextUser;
            nextUser = swUserGetNextTableRootUser(nextUser);
        } while(nextUser != _User);
        swUserSetNextTableRootUser(prevUser, swUserGetNextTableRootUser(_User));
    }
    swRootSetNumUser(Root, swRootGetNumUser(Root) - 1);
    swUserSetNextTableRootUser(_User, swUserNull);
}

/*----------------------------------------------------------------------------------------
  Find the User from the Root and its hash key.
----------------------------------------------------------------------------------------*/
swUser swRootFindUser(
    swRoot Root,
    uint32 Id)
{
    uint32 mask = swRootGetNumUserTable(Root) - 1;
    swUser _User;

    if(mask + 1 != 0) {
        _User = swRootGetiUserTable(Root, (uint32)Id & mask);
        while(_User != swUserNull) {
            if(swUserGetId(_User) == Id) {
                return _User;
            }
            _User = swUserGetNextTableRootUser(_User);
        }
    }
    return swUserNull;
}

/*----------------------------------------------------------------------------------------
  Add the User to the head of the list on the Root.
----------------------------------------------------------------------------------------*/
void swRootInsertUser(
    swRoot Root,
    swUser _User)
{
#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_User == swUserNull) {
        utExit("Non-existent User");
    }
    if(swUserGetRoot(_User) != swRootNull) {
        utExit("Attempting to add User to Root twice");
    }
#endif
    swUserSetNextRootUser(_User, swRootGetFirstUser(Root));
    if(swRootGetFirstUser(Root) != swUserNull) {
        swUserSetPrevRootUser(swRootGetFirstUser(Root), _User);
    }
    swRootSetFirstUser(Root, _User);
    swUserSetPrevRootUser(_User, swUserNull);
    if(swRootGetLastUser(Root) == swUserNull) {
        swRootSetLastUser(Root, _User);
    }
    swUserSetRoot(_User, Root);
    addRootUserToHashTable(Root, _User);
}

/*----------------------------------------------------------------------------------------
  Add the User to the end of the list on the Root.
----------------------------------------------------------------------------------------*/
void swRootAppendUser(
    swRoot Root,
    swUser _User)
{
#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_User == swUserNull) {
        utExit("Non-existent User");
    }
    if(swUserGetRoot(_User) != swRootNull) {
        utExit("Attempting to add User to Root twice");
    }
#endif
    swUserSetPrevRootUser(_User, swRootGetLastUser(Root));
    if(swRootGetLastUser(Root) != swUserNull) {
        swUserSetNextRootUser(swRootGetLastUser(Root), _User);
    }
    swRootSetLastUser(Root, _User);
    swUserSetNextRootUser(_User, swUserNull);
    if(swRootGetFirstUser(Root) == swUserNull) {
        swRootSetFirstUser(Root, _User);
    }
    swUserSetRoot(_User, Root);
    addRootUserToHashTable(Root, _User);
}

/*----------------------------------------------------------------------------------------
  Insert the User to the Root after the previous User.
----------------------------------------------------------------------------------------*/
void swRootInsertAfterUser(
    swRoot Root,
    swUser prevUser,
    swUser _User)
{
    swUser nextUser = swUserGetNextRootUser(prevUser);

#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_User == swUserNull) {
        utExit("Non-existent User");
    }
    if(swUserGetRoot(_User) != swRootNull) {
        utExit("Attempting to add User to Root twice");
    }
#endif
    swUserSetNextRootUser(_User, nextUser);
    swUserSetNextRootUser(prevUser, _User);
    swUserSetPrevRootUser(_User, prevUser);
    if(nextUser != swUserNull) {
        swUserSetPrevRootUser(nextUser, _User);
    }
    if(swRootGetLastUser(Root) == prevUser) {
        swRootSetLastUser(Root, _User);
    }
    swUserSetRoot(_User, Root);
    addRootUserToHashTable(Root, _User);
}

/*----------------------------------------------------------------------------------------
 Remove the User from the Root.
----------------------------------------------------------------------------------------*/
void swRootRemoveUser(
    swRoot Root,
    swUser _User)
{
    swUser pUser, nUser;

#if defined(DD_DEBUG)
    if(_User == swUserNull) {
        utExit("Non-existent User");
    }
    if(swUserGetRoot(_User) != swRootNull && swUserGetRoot(_User) != Root) {
        utExit("Delete User from non-owning Root");
    }
#endif
    nUser = swUserGetNextRootUser(_User);
    pUser = swUserGetPrevRootUser(_User);
    if(pUser != swUserNull) {
        swUserSetNextRootUser(pUser, nUser);
    } else if(swRootGetFirstUser(Root) == _User) {
        swRootSetFirstUser(Root, nUser);
    }
    if(nUser != swUserNull) {
        swUserSetPrevRootUser(nUser, pUser);
    } else if(swRootGetLastUser(Root) == _User) {
        swRootSetLastUser(Root, pUser);
    }
    swUserSetNextRootUser(_User, swUserNull);
    swUserSetPrevRootUser(_User, swUserNull);
    swUserSetRoot(_User, swRootNull);
    removeRootUserFromHashTable(Root, _User);
}

/*----------------------------------------------------------------------------------------
  Add the Engine to the head of the list on the Root.
----------------------------------------------------------------------------------------*/
void swRootInsertEngine(
    swRoot Root,
    swEngine _Engine)
{
#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetRoot(_Engine) != swRootNull) {
        utExit("Attempting to add Engine to Root twice");
    }
#endif
    swEngineSetNextRootEngine(_Engine, swRootGetFirstEngine(Root));
    if(swRootGetFirstEngine(Root) != swEngineNull) {
        swEngineSetPrevRootEngine(swRootGetFirstEngine(Root), _Engine);
    }
    swRootSetFirstEngine(Root, _Engine);
    swEngineSetPrevRootEngine(_Engine, swEngineNull);
    if(swRootGetLastEngine(Root) == swEngineNull) {
        swRootSetLastEngine(Root, _Engine);
    }
    swEngineSetRoot(_Engine, Root);
}

/*----------------------------------------------------------------------------------------
  Add the Engine to the end of the list on the Root.
----------------------------------------------------------------------------------------*/
void swRootAppendEngine(
    swRoot Root,
    swEngine _Engine)
{
#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetRoot(_Engine) != swRootNull) {
        utExit("Attempting to add Engine to Root twice");
    }
#endif
    swEngineSetPrevRootEngine(_Engine, swRootGetLastEngine(Root));
    if(swRootGetLastEngine(Root) != swEngineNull) {
        swEngineSetNextRootEngine(swRootGetLastEngine(Root), _Engine);
    }
    swRootSetLastEngine(Root, _Engine);
    swEngineSetNextRootEngine(_Engine, swEngineNull);
    if(swRootGetFirstEngine(Root) == swEngineNull) {
        swRootSetFirstEngine(Root, _Engine);
    }
    swEngineSetRoot(_Engine, Root);
}

/*----------------------------------------------------------------------------------------
  Insert the Engine to the Root after the previous Engine.
----------------------------------------------------------------------------------------*/
void swRootInsertAfterEngine(
    swRoot Root,
    swEngine prevEngine,
    swEngine _Engine)
{
    swEngine nextEngine = swEngineGetNextRootEngine(prevEngine);

#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetRoot(_Engine) != swRootNull) {
        utExit("Attempting to add Engine to Root twice");
    }
#endif
    swEngineSetNextRootEngine(_Engine, nextEngine);
    swEngineSetNextRootEngine(prevEngine, _Engine);
    swEngineSetPrevRootEngine(_Engine, prevEngine);
    if(nextEngine != swEngineNull) {
        swEngineSetPrevRootEngine(nextEngine, _Engine);
    }
    if(swRootGetLastEngine(Root) == prevEngine) {
        swRootSetLastEngine(Root, _Engine);
    }
    swEngineSetRoot(_Engine, Root);
}

/*----------------------------------------------------------------------------------------
 Remove the Engine from the Root.
----------------------------------------------------------------------------------------*/
void swRootRemoveEngine(
    swRoot Root,
    swEngine _Engine)
{
    swEngine pEngine, nEngine;

#if defined(DD_DEBUG)
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetRoot(_Engine) != swRootNull && swEngineGetRoot(_Engine) != Root) {
        utExit("Delete Engine from non-owning Root");
    }
#endif
    nEngine = swEngineGetNextRootEngine(_Engine);
    pEngine = swEngineGetPrevRootEngine(_Engine);
    if(pEngine != swEngineNull) {
        swEngineSetNextRootEngine(pEngine, nEngine);
    } else if(swRootGetFirstEngine(Root) == _Engine) {
        swRootSetFirstEngine(Root, nEngine);
    }
    if(nEngine != swEngineNull) {
        swEngineSetPrevRootEngine(nEngine, pEngine);
    } else if(swRootGetLastEngine(Root) == _Engine) {
        swRootSetLastEngine(Root, pEngine);
    }
    swEngineSetNextRootEngine(_Engine, swEngineNull);
    swEngineSetPrevRootEngine(_Engine, swEngineNull);
    swEngineSetRoot(_Engine, swRootNull);
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeRootLanguageHashTable(
    swRoot Root)
{
    swLanguage _Language, prevLanguage, nextLanguage;
    uint32 oldNumLanguages = swRootGetNumLanguageTable(Root);
    uint32 newNumLanguages = oldNumLanguages << 1;
    uint32 xLanguage, index;

    if(newNumLanguages == 0) {
        newNumLanguages = 2;
        swRootAllocLanguageTables(Root, 2);
    } else {
        swRootResizeLanguageTables(Root, newNumLanguages);
    }
    for(xLanguage = 0; xLanguage < oldNumLanguages; xLanguage++) {
        _Language = swRootGetiLanguageTable(Root, xLanguage);
        prevLanguage = swLanguageNull;
        while(_Language != swLanguageNull) {
            nextLanguage = swLanguageGetNextTableRootLanguage(_Language);
            index = (newNumLanguages - 1) & (swLanguageGetSym(_Language) == utSymNull? 0 : utSymGetHashValue(swLanguageGetSym(_Language)));
            if(index != xLanguage) {
                if(prevLanguage == swLanguageNull) {
                    swRootSetiLanguageTable(Root, xLanguage, nextLanguage);
                } else {
                    swLanguageSetNextTableRootLanguage(prevLanguage, nextLanguage);
                }
                swLanguageSetNextTableRootLanguage(_Language, swRootGetiLanguageTable(Root, index));
                swRootSetiLanguageTable(Root, index, _Language);
            } else {
                prevLanguage = _Language;
            }
            _Language = nextLanguage;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Language to the Root.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addRootLanguageToHashTable(
    swRoot Root,
    swLanguage _Language)
{
    swLanguage nextLanguage;
    uint32 index;

    if(swRootGetNumLanguage(Root) >> 1 >= swRootGetNumLanguageTable(Root)) {
        resizeRootLanguageHashTable(Root);
    }
    index = (swRootGetNumLanguageTable(Root) - 1) & (swLanguageGetSym(_Language) == utSymNull? 0 : utSymGetHashValue(swLanguageGetSym(_Language)));
    nextLanguage = swRootGetiLanguageTable(Root, index);
    swLanguageSetNextTableRootLanguage(_Language, nextLanguage);
    swRootSetiLanguageTable(Root, index, _Language);
    swRootSetNumLanguage(Root, swRootGetNumLanguage(Root) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Language from the hash table.
----------------------------------------------------------------------------------------*/
static void removeRootLanguageFromHashTable(
    swRoot Root,
    swLanguage _Language)
{
    uint32 index = (swRootGetNumLanguageTable(Root) - 1) & (swLanguageGetSym(_Language) == utSymNull? 0 : utSymGetHashValue(swLanguageGetSym(_Language)));
    swLanguage prevLanguage, nextLanguage;
    
    nextLanguage = swRootGetiLanguageTable(Root, index);
    if(nextLanguage == _Language) {
        swRootSetiLanguageTable(Root, index, swLanguageGetNextTableRootLanguage(nextLanguage));
    } else {
        do {
            prevLanguage = nextLanguage;
            nextLanguage = swLanguageGetNextTableRootLanguage(nextLanguage);
        } while(nextLanguage != _Language);
        swLanguageSetNextTableRootLanguage(prevLanguage, swLanguageGetNextTableRootLanguage(_Language));
    }
    swRootSetNumLanguage(Root, swRootGetNumLanguage(Root) - 1);
    swLanguageSetNextTableRootLanguage(_Language, swLanguageNull);
}

/*----------------------------------------------------------------------------------------
  Find the Language from the Root and its hash key.
----------------------------------------------------------------------------------------*/
swLanguage swRootFindLanguage(
    swRoot Root,
    utSym Sym)
{
    uint32 mask = swRootGetNumLanguageTable(Root) - 1;
    swLanguage _Language;

    if(mask + 1 != 0) {
        _Language = swRootGetiLanguageTable(Root, (Sym == utSymNull? 0 : utSymGetHashValue(Sym)) & mask);
        while(_Language != swLanguageNull) {
            if(swLanguageGetSym(_Language) == Sym) {
                return _Language;
            }
            _Language = swLanguageGetNextTableRootLanguage(_Language);
        }
    }
    return swLanguageNull;
}

/*----------------------------------------------------------------------------------------
  Find the Language from the Root and its name.
----------------------------------------------------------------------------------------*/
void swRootRenameLanguage(
    swRoot Root,
    swLanguage _Language,
    utSym sym)
{
    if(swLanguageGetSym(_Language) != utSymNull) {
        removeRootLanguageFromHashTable(Root, _Language);
    }
    swLanguageSetSym(_Language, sym);
    if(sym != utSymNull) {
        addRootLanguageToHashTable(Root, _Language);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Language to the head of the list on the Root.
----------------------------------------------------------------------------------------*/
void swRootInsertLanguage(
    swRoot Root,
    swLanguage _Language)
{
#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(swLanguageGetRoot(_Language) != swRootNull) {
        utExit("Attempting to add Language to Root twice");
    }
#endif
    swLanguageSetNextRootLanguage(_Language, swRootGetFirstLanguage(Root));
    if(swRootGetFirstLanguage(Root) != swLanguageNull) {
        swLanguageSetPrevRootLanguage(swRootGetFirstLanguage(Root), _Language);
    }
    swRootSetFirstLanguage(Root, _Language);
    swLanguageSetPrevRootLanguage(_Language, swLanguageNull);
    if(swRootGetLastLanguage(Root) == swLanguageNull) {
        swRootSetLastLanguage(Root, _Language);
    }
    swLanguageSetRoot(_Language, Root);
    if(swLanguageGetSym(_Language) != utSymNull) {
        addRootLanguageToHashTable(Root, _Language);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Language to the end of the list on the Root.
----------------------------------------------------------------------------------------*/
void swRootAppendLanguage(
    swRoot Root,
    swLanguage _Language)
{
#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(swLanguageGetRoot(_Language) != swRootNull) {
        utExit("Attempting to add Language to Root twice");
    }
#endif
    swLanguageSetPrevRootLanguage(_Language, swRootGetLastLanguage(Root));
    if(swRootGetLastLanguage(Root) != swLanguageNull) {
        swLanguageSetNextRootLanguage(swRootGetLastLanguage(Root), _Language);
    }
    swRootSetLastLanguage(Root, _Language);
    swLanguageSetNextRootLanguage(_Language, swLanguageNull);
    if(swRootGetFirstLanguage(Root) == swLanguageNull) {
        swRootSetFirstLanguage(Root, _Language);
    }
    swLanguageSetRoot(_Language, Root);
    if(swLanguageGetSym(_Language) != utSymNull) {
        addRootLanguageToHashTable(Root, _Language);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Language to the Root after the previous Language.
----------------------------------------------------------------------------------------*/
void swRootInsertAfterLanguage(
    swRoot Root,
    swLanguage prevLanguage,
    swLanguage _Language)
{
    swLanguage nextLanguage = swLanguageGetNextRootLanguage(prevLanguage);

#if defined(DD_DEBUG)
    if(Root == swRootNull) {
        utExit("Non-existent Root");
    }
    if(_Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(swLanguageGetRoot(_Language) != swRootNull) {
        utExit("Attempting to add Language to Root twice");
    }
#endif
    swLanguageSetNextRootLanguage(_Language, nextLanguage);
    swLanguageSetNextRootLanguage(prevLanguage, _Language);
    swLanguageSetPrevRootLanguage(_Language, prevLanguage);
    if(nextLanguage != swLanguageNull) {
        swLanguageSetPrevRootLanguage(nextLanguage, _Language);
    }
    if(swRootGetLastLanguage(Root) == prevLanguage) {
        swRootSetLastLanguage(Root, _Language);
    }
    swLanguageSetRoot(_Language, Root);
    if(swLanguageGetSym(_Language) != utSymNull) {
        addRootLanguageToHashTable(Root, _Language);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Language from the Root.
----------------------------------------------------------------------------------------*/
void swRootRemoveLanguage(
    swRoot Root,
    swLanguage _Language)
{
    swLanguage pLanguage, nLanguage;

#if defined(DD_DEBUG)
    if(_Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(swLanguageGetRoot(_Language) != swRootNull && swLanguageGetRoot(_Language) != Root) {
        utExit("Delete Language from non-owning Root");
    }
#endif
    nLanguage = swLanguageGetNextRootLanguage(_Language);
    pLanguage = swLanguageGetPrevRootLanguage(_Language);
    if(pLanguage != swLanguageNull) {
        swLanguageSetNextRootLanguage(pLanguage, nLanguage);
    } else if(swRootGetFirstLanguage(Root) == _Language) {
        swRootSetFirstLanguage(Root, nLanguage);
    }
    if(nLanguage != swLanguageNull) {
        swLanguageSetPrevRootLanguage(nLanguage, pLanguage);
    } else if(swRootGetLastLanguage(Root) == _Language) {
        swRootSetLastLanguage(Root, pLanguage);
    }
    swLanguageSetNextRootLanguage(_Language, swLanguageNull);
    swLanguageSetPrevRootLanguage(_Language, swLanguageNull);
    swLanguageSetRoot(_Language, swRootNull);
    if(swLanguageGetSym(_Language) != utSymNull) {
        removeRootLanguageFromHashTable(Root, _Language);
    }
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowRoot(
    swRoot Root)
{
    utDatabaseShowObject("sw", "Root", swRoot2Index(Root));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy User including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swUserDestroy(
    swUser User)
{
    swEngine Engine_;
    swMessage Message_;
    swRoot owningRoot = swUserGetRoot(User);

    if(swUserDestructorCallback != NULL) {
        swUserDestructorCallback(User);
    }
    Engine_ = swUserGetEngine(User);
    if(Engine_ != swEngineNull) {
        swEngineSetUser(Engine_, swUserNull);
    }
    swSafeForeachUserMessage(User, Message_) {
        swMessageDestroy(Message_);
    } swEndSafeUserMessage;
    if(owningRoot != swRootNull) {
        swRootRemoveUser(owningRoot, User);
#if defined(DD_DEBUG)
    } else {
        utExit("User without owning Root");
#endif
    }
    swUserFree(User);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocUser(void)
{
    swUser User = swUserAlloc();

    return swUser2Index(User);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyUser(
    uint64 objectIndex)
{
    swUserDestroy(swIndex2User((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of User.
----------------------------------------------------------------------------------------*/
static void allocUsers(void)
{
    swSetAllocatedUser(2);
    swSetUsedUser(1);
    swSetFirstFreeUser(swUserNull);
    swUsers.Id = utNewAInitFirst(uint32, (swAllocatedUser()));
    swUsers.Root = utNewAInitFirst(swRoot, (swAllocatedUser()));
    swUsers.NextRootUser = utNewAInitFirst(swUser, (swAllocatedUser()));
    swUsers.PrevRootUser = utNewAInitFirst(swUser, (swAllocatedUser()));
    swUsers.NextTableRootUser = utNewAInitFirst(swUser, (swAllocatedUser()));
    swUsers.Engine = utNewAInitFirst(swEngine, (swAllocatedUser()));
    swUsers.FirstMessage = utNewAInitFirst(swMessage, (swAllocatedUser()));
    swUsers.LastMessage = utNewAInitFirst(swMessage, (swAllocatedUser()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class User.
----------------------------------------------------------------------------------------*/
static void reallocUsers(
    uint32 newSize)
{
    utResizeArray(swUsers.Id, (newSize));
    utResizeArray(swUsers.Root, (newSize));
    utResizeArray(swUsers.NextRootUser, (newSize));
    utResizeArray(swUsers.PrevRootUser, (newSize));
    utResizeArray(swUsers.NextTableRootUser, (newSize));
    utResizeArray(swUsers.Engine, (newSize));
    utResizeArray(swUsers.FirstMessage, (newSize));
    utResizeArray(swUsers.LastMessage, (newSize));
    swSetAllocatedUser(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Users.
----------------------------------------------------------------------------------------*/
void swUserAllocMore(void)
{
    reallocUsers((uint32)(swAllocatedUser() + (swAllocatedUser() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of User.
----------------------------------------------------------------------------------------*/
void swUserCopyProps(
    swUser oldUser,
    swUser newUser)
{
    swUserSetId(newUser, swUserGetId(oldUser));
}

/*----------------------------------------------------------------------------------------
  Add the Message to the head of the list on the User.
----------------------------------------------------------------------------------------*/
void swUserInsertMessage(
    swUser User,
    swMessage _Message)
{
#if defined(DD_DEBUG)
    if(User == swUserNull) {
        utExit("Non-existent User");
    }
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetUser(_Message) != swUserNull) {
        utExit("Attempting to add Message to User twice");
    }
#endif
    swMessageSetNextUserMessage(_Message, swUserGetFirstMessage(User));
    if(swUserGetFirstMessage(User) != swMessageNull) {
        swMessageSetPrevUserMessage(swUserGetFirstMessage(User), _Message);
    }
    swUserSetFirstMessage(User, _Message);
    swMessageSetPrevUserMessage(_Message, swMessageNull);
    if(swUserGetLastMessage(User) == swMessageNull) {
        swUserSetLastMessage(User, _Message);
    }
    swMessageSetUser(_Message, User);
}

/*----------------------------------------------------------------------------------------
  Add the Message to the end of the list on the User.
----------------------------------------------------------------------------------------*/
void swUserAppendMessage(
    swUser User,
    swMessage _Message)
{
#if defined(DD_DEBUG)
    if(User == swUserNull) {
        utExit("Non-existent User");
    }
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetUser(_Message) != swUserNull) {
        utExit("Attempting to add Message to User twice");
    }
#endif
    swMessageSetPrevUserMessage(_Message, swUserGetLastMessage(User));
    if(swUserGetLastMessage(User) != swMessageNull) {
        swMessageSetNextUserMessage(swUserGetLastMessage(User), _Message);
    }
    swUserSetLastMessage(User, _Message);
    swMessageSetNextUserMessage(_Message, swMessageNull);
    if(swUserGetFirstMessage(User) == swMessageNull) {
        swUserSetFirstMessage(User, _Message);
    }
    swMessageSetUser(_Message, User);
}

/*----------------------------------------------------------------------------------------
  Insert the Message to the User after the previous Message.
----------------------------------------------------------------------------------------*/
void swUserInsertAfterMessage(
    swUser User,
    swMessage prevMessage,
    swMessage _Message)
{
    swMessage nextMessage = swMessageGetNextUserMessage(prevMessage);

#if defined(DD_DEBUG)
    if(User == swUserNull) {
        utExit("Non-existent User");
    }
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetUser(_Message) != swUserNull) {
        utExit("Attempting to add Message to User twice");
    }
#endif
    swMessageSetNextUserMessage(_Message, nextMessage);
    swMessageSetNextUserMessage(prevMessage, _Message);
    swMessageSetPrevUserMessage(_Message, prevMessage);
    if(nextMessage != swMessageNull) {
        swMessageSetPrevUserMessage(nextMessage, _Message);
    }
    if(swUserGetLastMessage(User) == prevMessage) {
        swUserSetLastMessage(User, _Message);
    }
    swMessageSetUser(_Message, User);
}

/*----------------------------------------------------------------------------------------
 Remove the Message from the User.
----------------------------------------------------------------------------------------*/
void swUserRemoveMessage(
    swUser User,
    swMessage _Message)
{
    swMessage pMessage, nMessage;

#if defined(DD_DEBUG)
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetUser(_Message) != swUserNull && swMessageGetUser(_Message) != User) {
        utExit("Delete Message from non-owning User");
    }
#endif
    nMessage = swMessageGetNextUserMessage(_Message);
    pMessage = swMessageGetPrevUserMessage(_Message);
    if(pMessage != swMessageNull) {
        swMessageSetNextUserMessage(pMessage, nMessage);
    } else if(swUserGetFirstMessage(User) == _Message) {
        swUserSetFirstMessage(User, nMessage);
    }
    if(nMessage != swMessageNull) {
        swMessageSetPrevUserMessage(nMessage, pMessage);
    } else if(swUserGetLastMessage(User) == _Message) {
        swUserSetLastMessage(User, pMessage);
    }
    swMessageSetNextUserMessage(_Message, swMessageNull);
    swMessageSetPrevUserMessage(_Message, swMessageNull);
    swMessageSetUser(_Message, swUserNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowUser(
    swUser User)
{
    utDatabaseShowObject("sw", "User", swUser2Index(User));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Mengine including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swMengineDestroy(
    swMengine Mengine)
{
    swEngine Engine_;
    swVoice Voice_;

    if(swMengineDestructorCallback != NULL) {
        swMengineDestructorCallback(Mengine);
    }
    swSafeForeachMengineEngine(Mengine, Engine_) {
        swEngineDestroy(Engine_);
    } swEndSafeMengineEngine;
    swSafeForeachMengineVoice(Mengine, Voice_) {
        swVoiceDestroy(Voice_);
    } swEndSafeMengineVoice;
    swMengineFree(Mengine);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocMengine(void)
{
    swMengine Mengine = swMengineAlloc();

    return swMengine2Index(Mengine);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyMengine(
    uint64 objectIndex)
{
    swMengineDestroy(swIndex2Mengine((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Mengine.
----------------------------------------------------------------------------------------*/
static void allocMengines(void)
{
    swSetAllocatedMengine(2);
    swSetUsedMengine(1);
    swSetFirstFreeMengine(swMengineNull);
    swMengines.FirstEngine = utNewAInitFirst(swEngine, (swAllocatedMengine()));
    swMengines.LastEngine = utNewAInitFirst(swEngine, (swAllocatedMengine()));
    swMengines.FirstVoice = utNewAInitFirst(swVoice, (swAllocatedMengine()));
    swMengines.LastVoice = utNewAInitFirst(swVoice, (swAllocatedMengine()));
    swMengines.VoiceTableIndex_ = utNewAInitFirst(uint32, (swAllocatedMengine()));
    swMengines.NumVoiceTable = utNewAInitFirst(uint32, (swAllocatedMengine()));
    swSetUsedMengineVoiceTable(0);
    swSetAllocatedMengineVoiceTable(2);
    swSetFreeMengineVoiceTable(0);
    swMengines.VoiceTable = utNewAInitFirst(swVoice, swAllocatedMengineVoiceTable());
    swMengines.NumVoice = utNewAInitFirst(uint32, (swAllocatedMengine()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Mengine.
----------------------------------------------------------------------------------------*/
static void reallocMengines(
    uint32 newSize)
{
    utResizeArray(swMengines.FirstEngine, (newSize));
    utResizeArray(swMengines.LastEngine, (newSize));
    utResizeArray(swMengines.FirstVoice, (newSize));
    utResizeArray(swMengines.LastVoice, (newSize));
    utResizeArray(swMengines.VoiceTableIndex_, (newSize));
    utResizeArray(swMengines.NumVoiceTable, (newSize));
    utResizeArray(swMengines.NumVoice, (newSize));
    swSetAllocatedMengine(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Mengines.
----------------------------------------------------------------------------------------*/
void swMengineAllocMore(void)
{
    reallocMengines((uint32)(swAllocatedMengine() + (swAllocatedMengine() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Mengine.VoiceTable heap to free memory.
----------------------------------------------------------------------------------------*/
void swCompactMengineVoiceTables(void)
{
    uint32 elementSize = sizeof(swVoice);
    uint32 usedHeaderSize = (sizeof(swMengine) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMengine) + sizeof(uint32) + elementSize - 1)/elementSize;
    swVoice *toPtr = swMengines.VoiceTable;
    swVoice *fromPtr = toPtr;
    swMengine Mengine;
    uint32 size;

    while(fromPtr < swMengines.VoiceTable + swUsedMengineVoiceTable()) {
        Mengine = *(swMengine *)(void *)fromPtr;
        if(Mengine != swMengineNull) {
            /* Need to move it to toPtr */
            size = utMax(swMengineGetNumVoiceTable(Mengine) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            swMengineSetVoiceTableIndex_(Mengine, toPtr - swMengines.VoiceTable + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((swMengine *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    swSetUsedMengineVoiceTable(toPtr - swMengines.VoiceTable);
    swSetFreeMengineVoiceTable(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Mengine.VoiceTable heap.
----------------------------------------------------------------------------------------*/
static void allocMoreMengineVoiceTables(
    uint32 spaceNeeded)
{
    uint32 freeSpace = swAllocatedMengineVoiceTable() - swUsedMengineVoiceTable();
    uint32 elementSize = sizeof(swVoice);
    uint32 usedHeaderSize = (sizeof(swMengine) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMengine) + sizeof(uint32) + elementSize - 1)/elementSize;
    swVoice *ptr = swMengines.VoiceTable;
    swMengine Mengine;
    uint32 size;

    while(ptr < swMengines.VoiceTable + swUsedMengineVoiceTable()) {
        Mengine = *(swMengine*)(void*)ptr;
        if(Mengine != swMengineNull) {
            swValidMengine(Mengine);
            size = utMax(swMengineGetNumVoiceTable(Mengine) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((swMengine *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((swFreeMengineVoiceTable() << 2) > swUsedMengineVoiceTable()) {
        swCompactMengineVoiceTables();
        freeSpace = swAllocatedMengineVoiceTable() - swUsedMengineVoiceTable();
    }
    if(freeSpace < spaceNeeded) {
        swSetAllocatedMengineVoiceTable(swAllocatedMengineVoiceTable() + spaceNeeded - freeSpace +
            (swAllocatedMengineVoiceTable() >> 1));
        utResizeArray(swMengines.VoiceTable, swAllocatedMengineVoiceTable());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Mengine.VoiceTable array.
----------------------------------------------------------------------------------------*/
void swMengineAllocVoiceTables(
    swMengine Mengine,
    uint32 numVoiceTables)
{
    uint32 freeSpace = swAllocatedMengineVoiceTable() - swUsedMengineVoiceTable();
    uint32 elementSize = sizeof(swVoice);
    uint32 usedHeaderSize = (sizeof(swMengine) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMengine) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numVoiceTables + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(swMengineGetNumVoiceTable(Mengine) == 0);
#endif
    if(numVoiceTables == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreMengineVoiceTables(spaceNeeded);
    }
    swMengineSetVoiceTableIndex_(Mengine, swUsedMengineVoiceTable() + usedHeaderSize);
    swMengineSetNumVoiceTable(Mengine, numVoiceTables);
    *(swMengine *)(void *)(swMengines.VoiceTable + swUsedMengineVoiceTable()) = Mengine;
    {
        uint32 xValue;
        for(xValue = (uint32)(swMengineGetVoiceTableIndex_(Mengine)); xValue < swMengineGetVoiceTableIndex_(Mengine) + numVoiceTables; xValue++) {
            swMengines.VoiceTable[xValue] = swVoiceNull;
        }
    }
    swSetUsedMengineVoiceTable(swUsedMengineVoiceTable() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swMengineGetVoiceTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *getMengineVoiceTables(
    uint64 objectNumber,
    uint32 *numValues)
{
    swMengine Mengine = swIndex2Mengine((uint32)objectNumber);

    *numValues = swMengineGetNumVoiceTable(Mengine);
    return swMengineGetVoiceTables(Mengine);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swMengineAllocVoiceTables for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocMengineVoiceTables(
    uint64 objectNumber,
    uint32 numValues)
{
    swMengine Mengine = swIndex2Mengine((uint32)objectNumber);

    swMengineSetVoiceTableIndex_(Mengine, 0);
    swMengineSetNumVoiceTable(Mengine, 0);
    if(numValues == 0) {
        return NULL;
    }
    swMengineAllocVoiceTables(Mengine, numValues);
    return swMengineGetVoiceTables(Mengine);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Mengine.VoiceTable array.
----------------------------------------------------------------------------------------*/
void swMengineFreeVoiceTables(
    swMengine Mengine)
{
    uint32 elementSize = sizeof(swVoice);
    uint32 usedHeaderSize = (sizeof(swMengine) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMengine) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(swMengineGetNumVoiceTable(Mengine) + usedHeaderSize, freeHeaderSize);
    swVoice *dataPtr = swMengineGetVoiceTables(Mengine) - usedHeaderSize;

    if(swMengineGetNumVoiceTable(Mengine) == 0) {
        return;
    }
    *(swMengine *)(void *)(dataPtr) = swMengineNull;
    *(uint32 *)(void *)(((swMengine *)(void *)dataPtr) + 1) = size;
    swMengineSetNumVoiceTable(Mengine, 0);
    swSetFreeMengineVoiceTable(swFreeMengineVoiceTable() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Mengine.VoiceTable array.
----------------------------------------------------------------------------------------*/
void swMengineResizeVoiceTables(
    swMengine Mengine,
    uint32 numVoiceTables)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(swVoice);
    uint32 usedHeaderSize = (sizeof(swMengine) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMengine) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numVoiceTables + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(swMengineGetNumVoiceTable(Mengine) + usedHeaderSize, freeHeaderSize);
    swVoice *dataPtr;

    if(numVoiceTables == 0) {
        if(swMengineGetNumVoiceTable(Mengine) != 0) {
            swMengineFreeVoiceTables(Mengine);
        }
        return;
    }
    if(swMengineGetNumVoiceTable(Mengine) == 0) {
        swMengineAllocVoiceTables(Mengine, numVoiceTables);
        return;
    }
    freeSpace = swAllocatedMengineVoiceTable() - swUsedMengineVoiceTable();
    if(freeSpace < newSize) {
        allocMoreMengineVoiceTables(newSize);
    }
    dataPtr = swMengineGetVoiceTables(Mengine) - usedHeaderSize;
    memcpy((void *)(swMengines.VoiceTable + swUsedMengineVoiceTable()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        {
            uint32 xValue;
            for(xValue = (uint32)(swUsedMengineVoiceTable() + oldSize); xValue < swUsedMengineVoiceTable() + oldSize + newSize - oldSize; xValue++) {
                swMengines.VoiceTable[xValue] = swVoiceNull;
            }
        }
    }
    *(swMengine *)(void *)dataPtr = swMengineNull;
    *(uint32 *)(void *)(((swMengine *)(void *)dataPtr) + 1) = oldSize;
    swSetFreeMengineVoiceTable(swFreeMengineVoiceTable() + oldSize);
    swMengineSetVoiceTableIndex_(Mengine, swUsedMengineVoiceTable() + usedHeaderSize);
    swMengineSetNumVoiceTable(Mengine, numVoiceTables);
    swSetUsedMengineVoiceTable(swUsedMengineVoiceTable() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Mengine.
----------------------------------------------------------------------------------------*/
void swMengineCopyProps(
    swMengine oldMengine,
    swMengine newMengine)
{
}

/*----------------------------------------------------------------------------------------
  Add the Engine to the head of the list on the Mengine.
----------------------------------------------------------------------------------------*/
void swMengineInsertEngine(
    swMengine Mengine,
    swEngine _Engine)
{
#if defined(DD_DEBUG)
    if(Mengine == swMengineNull) {
        utExit("Non-existent Mengine");
    }
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetMengine(_Engine) != swMengineNull) {
        utExit("Attempting to add Engine to Mengine twice");
    }
#endif
    swEngineSetNextMengineEngine(_Engine, swMengineGetFirstEngine(Mengine));
    if(swMengineGetFirstEngine(Mengine) != swEngineNull) {
        swEngineSetPrevMengineEngine(swMengineGetFirstEngine(Mengine), _Engine);
    }
    swMengineSetFirstEngine(Mengine, _Engine);
    swEngineSetPrevMengineEngine(_Engine, swEngineNull);
    if(swMengineGetLastEngine(Mengine) == swEngineNull) {
        swMengineSetLastEngine(Mengine, _Engine);
    }
    swEngineSetMengine(_Engine, Mengine);
}

/*----------------------------------------------------------------------------------------
  Add the Engine to the end of the list on the Mengine.
----------------------------------------------------------------------------------------*/
void swMengineAppendEngine(
    swMengine Mengine,
    swEngine _Engine)
{
#if defined(DD_DEBUG)
    if(Mengine == swMengineNull) {
        utExit("Non-existent Mengine");
    }
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetMengine(_Engine) != swMengineNull) {
        utExit("Attempting to add Engine to Mengine twice");
    }
#endif
    swEngineSetPrevMengineEngine(_Engine, swMengineGetLastEngine(Mengine));
    if(swMengineGetLastEngine(Mengine) != swEngineNull) {
        swEngineSetNextMengineEngine(swMengineGetLastEngine(Mengine), _Engine);
    }
    swMengineSetLastEngine(Mengine, _Engine);
    swEngineSetNextMengineEngine(_Engine, swEngineNull);
    if(swMengineGetFirstEngine(Mengine) == swEngineNull) {
        swMengineSetFirstEngine(Mengine, _Engine);
    }
    swEngineSetMengine(_Engine, Mengine);
}

/*----------------------------------------------------------------------------------------
  Insert the Engine to the Mengine after the previous Engine.
----------------------------------------------------------------------------------------*/
void swMengineInsertAfterEngine(
    swMengine Mengine,
    swEngine prevEngine,
    swEngine _Engine)
{
    swEngine nextEngine = swEngineGetNextMengineEngine(prevEngine);

#if defined(DD_DEBUG)
    if(Mengine == swMengineNull) {
        utExit("Non-existent Mengine");
    }
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetMengine(_Engine) != swMengineNull) {
        utExit("Attempting to add Engine to Mengine twice");
    }
#endif
    swEngineSetNextMengineEngine(_Engine, nextEngine);
    swEngineSetNextMengineEngine(prevEngine, _Engine);
    swEngineSetPrevMengineEngine(_Engine, prevEngine);
    if(nextEngine != swEngineNull) {
        swEngineSetPrevMengineEngine(nextEngine, _Engine);
    }
    if(swMengineGetLastEngine(Mengine) == prevEngine) {
        swMengineSetLastEngine(Mengine, _Engine);
    }
    swEngineSetMengine(_Engine, Mengine);
}

/*----------------------------------------------------------------------------------------
 Remove the Engine from the Mengine.
----------------------------------------------------------------------------------------*/
void swMengineRemoveEngine(
    swMengine Mengine,
    swEngine _Engine)
{
    swEngine pEngine, nEngine;

#if defined(DD_DEBUG)
    if(_Engine == swEngineNull) {
        utExit("Non-existent Engine");
    }
    if(swEngineGetMengine(_Engine) != swMengineNull && swEngineGetMengine(_Engine) != Mengine) {
        utExit("Delete Engine from non-owning Mengine");
    }
#endif
    nEngine = swEngineGetNextMengineEngine(_Engine);
    pEngine = swEngineGetPrevMengineEngine(_Engine);
    if(pEngine != swEngineNull) {
        swEngineSetNextMengineEngine(pEngine, nEngine);
    } else if(swMengineGetFirstEngine(Mengine) == _Engine) {
        swMengineSetFirstEngine(Mengine, nEngine);
    }
    if(nEngine != swEngineNull) {
        swEngineSetPrevMengineEngine(nEngine, pEngine);
    } else if(swMengineGetLastEngine(Mengine) == _Engine) {
        swMengineSetLastEngine(Mengine, pEngine);
    }
    swEngineSetNextMengineEngine(_Engine, swEngineNull);
    swEngineSetPrevMengineEngine(_Engine, swEngineNull);
    swEngineSetMengine(_Engine, swMengineNull);
}

/*----------------------------------------------------------------------------------------
  Increase the size of the hash table.
----------------------------------------------------------------------------------------*/
static void resizeMengineVoiceHashTable(
    swMengine Mengine)
{
    swVoice _Voice, prevVoice, nextVoice;
    uint32 oldNumVoices = swMengineGetNumVoiceTable(Mengine);
    uint32 newNumVoices = oldNumVoices << 1;
    uint32 xVoice, index;

    if(newNumVoices == 0) {
        newNumVoices = 2;
        swMengineAllocVoiceTables(Mengine, 2);
    } else {
        swMengineResizeVoiceTables(Mengine, newNumVoices);
    }
    for(xVoice = 0; xVoice < oldNumVoices; xVoice++) {
        _Voice = swMengineGetiVoiceTable(Mengine, xVoice);
        prevVoice = swVoiceNull;
        while(_Voice != swVoiceNull) {
            nextVoice = swVoiceGetNextTableMengineVoice(_Voice);
            index = (newNumVoices - 1) & (swVoiceGetSym(_Voice) == utSymNull? 0 : utSymGetHashValue(swVoiceGetSym(_Voice)));
            if(index != xVoice) {
                if(prevVoice == swVoiceNull) {
                    swMengineSetiVoiceTable(Mengine, xVoice, nextVoice);
                } else {
                    swVoiceSetNextTableMengineVoice(prevVoice, nextVoice);
                }
                swVoiceSetNextTableMengineVoice(_Voice, swMengineGetiVoiceTable(Mengine, index));
                swMengineSetiVoiceTable(Mengine, index, _Voice);
            } else {
                prevVoice = _Voice;
            }
            _Voice = nextVoice;
        }
    }
}

/*----------------------------------------------------------------------------------------
  Add the Voice to the Mengine.  If the table is near full, build a new one twice
  as big, delete the old one, and return the new one.
----------------------------------------------------------------------------------------*/
static void addMengineVoiceToHashTable(
    swMengine Mengine,
    swVoice _Voice)
{
    swVoice nextVoice;
    uint32 index;

    if(swMengineGetNumVoice(Mengine) >> 1 >= swMengineGetNumVoiceTable(Mengine)) {
        resizeMengineVoiceHashTable(Mengine);
    }
    index = (swMengineGetNumVoiceTable(Mengine) - 1) & (swVoiceGetSym(_Voice) == utSymNull? 0 : utSymGetHashValue(swVoiceGetSym(_Voice)));
    nextVoice = swMengineGetiVoiceTable(Mengine, index);
    swVoiceSetNextTableMengineVoice(_Voice, nextVoice);
    swMengineSetiVoiceTable(Mengine, index, _Voice);
    swMengineSetNumVoice(Mengine, swMengineGetNumVoice(Mengine) + 1);
}

/*----------------------------------------------------------------------------------------
  Remove the Voice from the hash table.
----------------------------------------------------------------------------------------*/
static void removeMengineVoiceFromHashTable(
    swMengine Mengine,
    swVoice _Voice)
{
    uint32 index = (swMengineGetNumVoiceTable(Mengine) - 1) & (swVoiceGetSym(_Voice) == utSymNull? 0 : utSymGetHashValue(swVoiceGetSym(_Voice)));
    swVoice prevVoice, nextVoice;
    
    nextVoice = swMengineGetiVoiceTable(Mengine, index);
    if(nextVoice == _Voice) {
        swMengineSetiVoiceTable(Mengine, index, swVoiceGetNextTableMengineVoice(nextVoice));
    } else {
        do {
            prevVoice = nextVoice;
            nextVoice = swVoiceGetNextTableMengineVoice(nextVoice);
        } while(nextVoice != _Voice);
        swVoiceSetNextTableMengineVoice(prevVoice, swVoiceGetNextTableMengineVoice(_Voice));
    }
    swMengineSetNumVoice(Mengine, swMengineGetNumVoice(Mengine) - 1);
    swVoiceSetNextTableMengineVoice(_Voice, swVoiceNull);
}

/*----------------------------------------------------------------------------------------
  Find the Voice from the Mengine and its hash key.
----------------------------------------------------------------------------------------*/
swVoice swMengineFindVoice(
    swMengine Mengine,
    utSym Sym)
{
    uint32 mask = swMengineGetNumVoiceTable(Mengine) - 1;
    swVoice _Voice;

    if(mask + 1 != 0) {
        _Voice = swMengineGetiVoiceTable(Mengine, (Sym == utSymNull? 0 : utSymGetHashValue(Sym)) & mask);
        while(_Voice != swVoiceNull) {
            if(swVoiceGetSym(_Voice) == Sym) {
                return _Voice;
            }
            _Voice = swVoiceGetNextTableMengineVoice(_Voice);
        }
    }
    return swVoiceNull;
}

/*----------------------------------------------------------------------------------------
  Find the Voice from the Mengine and its name.
----------------------------------------------------------------------------------------*/
void swMengineRenameVoice(
    swMengine Mengine,
    swVoice _Voice,
    utSym sym)
{
    if(swVoiceGetSym(_Voice) != utSymNull) {
        removeMengineVoiceFromHashTable(Mengine, _Voice);
    }
    swVoiceSetSym(_Voice, sym);
    if(sym != utSymNull) {
        addMengineVoiceToHashTable(Mengine, _Voice);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Voice to the head of the list on the Mengine.
----------------------------------------------------------------------------------------*/
void swMengineInsertVoice(
    swMengine Mengine,
    swVoice _Voice)
{
#if defined(DD_DEBUG)
    if(Mengine == swMengineNull) {
        utExit("Non-existent Mengine");
    }
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetMengine(_Voice) != swMengineNull) {
        utExit("Attempting to add Voice to Mengine twice");
    }
#endif
    swVoiceSetNextMengineVoice(_Voice, swMengineGetFirstVoice(Mengine));
    if(swMengineGetFirstVoice(Mengine) != swVoiceNull) {
        swVoiceSetPrevMengineVoice(swMengineGetFirstVoice(Mengine), _Voice);
    }
    swMengineSetFirstVoice(Mengine, _Voice);
    swVoiceSetPrevMengineVoice(_Voice, swVoiceNull);
    if(swMengineGetLastVoice(Mengine) == swVoiceNull) {
        swMengineSetLastVoice(Mengine, _Voice);
    }
    swVoiceSetMengine(_Voice, Mengine);
    if(swVoiceGetSym(_Voice) != utSymNull) {
        addMengineVoiceToHashTable(Mengine, _Voice);
    }
}

/*----------------------------------------------------------------------------------------
  Add the Voice to the end of the list on the Mengine.
----------------------------------------------------------------------------------------*/
void swMengineAppendVoice(
    swMengine Mengine,
    swVoice _Voice)
{
#if defined(DD_DEBUG)
    if(Mengine == swMengineNull) {
        utExit("Non-existent Mengine");
    }
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetMengine(_Voice) != swMengineNull) {
        utExit("Attempting to add Voice to Mengine twice");
    }
#endif
    swVoiceSetPrevMengineVoice(_Voice, swMengineGetLastVoice(Mengine));
    if(swMengineGetLastVoice(Mengine) != swVoiceNull) {
        swVoiceSetNextMengineVoice(swMengineGetLastVoice(Mengine), _Voice);
    }
    swMengineSetLastVoice(Mengine, _Voice);
    swVoiceSetNextMengineVoice(_Voice, swVoiceNull);
    if(swMengineGetFirstVoice(Mengine) == swVoiceNull) {
        swMengineSetFirstVoice(Mengine, _Voice);
    }
    swVoiceSetMengine(_Voice, Mengine);
    if(swVoiceGetSym(_Voice) != utSymNull) {
        addMengineVoiceToHashTable(Mengine, _Voice);
    }
}

/*----------------------------------------------------------------------------------------
  Insert the Voice to the Mengine after the previous Voice.
----------------------------------------------------------------------------------------*/
void swMengineInsertAfterVoice(
    swMengine Mengine,
    swVoice prevVoice,
    swVoice _Voice)
{
    swVoice nextVoice = swVoiceGetNextMengineVoice(prevVoice);

#if defined(DD_DEBUG)
    if(Mengine == swMengineNull) {
        utExit("Non-existent Mengine");
    }
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetMengine(_Voice) != swMengineNull) {
        utExit("Attempting to add Voice to Mengine twice");
    }
#endif
    swVoiceSetNextMengineVoice(_Voice, nextVoice);
    swVoiceSetNextMengineVoice(prevVoice, _Voice);
    swVoiceSetPrevMengineVoice(_Voice, prevVoice);
    if(nextVoice != swVoiceNull) {
        swVoiceSetPrevMengineVoice(nextVoice, _Voice);
    }
    if(swMengineGetLastVoice(Mengine) == prevVoice) {
        swMengineSetLastVoice(Mengine, _Voice);
    }
    swVoiceSetMengine(_Voice, Mengine);
    if(swVoiceGetSym(_Voice) != utSymNull) {
        addMengineVoiceToHashTable(Mengine, _Voice);
    }
}

/*----------------------------------------------------------------------------------------
 Remove the Voice from the Mengine.
----------------------------------------------------------------------------------------*/
void swMengineRemoveVoice(
    swMengine Mengine,
    swVoice _Voice)
{
    swVoice pVoice, nVoice;

#if defined(DD_DEBUG)
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetMengine(_Voice) != swMengineNull && swVoiceGetMengine(_Voice) != Mengine) {
        utExit("Delete Voice from non-owning Mengine");
    }
#endif
    nVoice = swVoiceGetNextMengineVoice(_Voice);
    pVoice = swVoiceGetPrevMengineVoice(_Voice);
    if(pVoice != swVoiceNull) {
        swVoiceSetNextMengineVoice(pVoice, nVoice);
    } else if(swMengineGetFirstVoice(Mengine) == _Voice) {
        swMengineSetFirstVoice(Mengine, nVoice);
    }
    if(nVoice != swVoiceNull) {
        swVoiceSetPrevMengineVoice(nVoice, pVoice);
    } else if(swMengineGetLastVoice(Mengine) == _Voice) {
        swMengineSetLastVoice(Mengine, pVoice);
    }
    swVoiceSetNextMengineVoice(_Voice, swVoiceNull);
    swVoiceSetPrevMengineVoice(_Voice, swVoiceNull);
    swVoiceSetMengine(_Voice, swMengineNull);
    if(swVoiceGetSym(_Voice) != utSymNull) {
        removeMengineVoiceFromHashTable(Mengine, _Voice);
    }
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowMengine(
    swMengine Mengine)
{
    utDatabaseShowObject("sw", "Mengine", swMengine2Index(Mengine));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Engine including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swEngineDestroy(
    swEngine Engine)
{
    swRoot owningRoot = swEngineGetRoot(Engine);
    swMengine owningMengine = swEngineGetMengine(Engine);
    swUser owningUser = swEngineGetUser(Engine);

    if(swEngineDestructorCallback != NULL) {
        swEngineDestructorCallback(Engine);
    }
    if(owningRoot != swRootNull) {
        swRootRemoveEngine(owningRoot, Engine);
#if defined(DD_DEBUG)
    } else {
        utExit("Engine without owning Root");
#endif
    }
    if(owningMengine != swMengineNull) {
        swMengineRemoveEngine(owningMengine, Engine);
#if defined(DD_DEBUG)
    } else {
        utExit("Engine without owning Mengine");
#endif
    }
    if(owningUser != swUserNull) {
        swUserSetEngine(owningUser, swEngineNull);
    }
    swEngineFree(Engine);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocEngine(void)
{
    swEngine Engine = swEngineAlloc();

    return swEngine2Index(Engine);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyEngine(
    uint64 objectIndex)
{
    swEngineDestroy(swIndex2Engine((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Engine.
----------------------------------------------------------------------------------------*/
static void allocEngines(void)
{
    swSetAllocatedEngine(2);
    swSetUsedEngine(1);
    swSetFirstFreeEngine(swEngineNull);
    swEngines.Voice = utNewAInitFirst(swVoice, (swAllocatedEngine()));
    swEngines.Langauge = utNewAInitFirst(swLanguage, (swAllocatedEngine()));
    swEngines.Pitch = utNewAInitFirst(float, (swAllocatedEngine()));
    swEngines.Speed = utNewAInitFirst(float, (swAllocatedEngine()));
    swEngines.Volume = utNewAInitFirst(float, (swAllocatedEngine()));
    swEngines.Root = utNewAInitFirst(swRoot, (swAllocatedEngine()));
    swEngines.NextRootEngine = utNewAInitFirst(swEngine, (swAllocatedEngine()));
    swEngines.PrevRootEngine = utNewAInitFirst(swEngine, (swAllocatedEngine()));
    swEngines.User = utNewAInitFirst(swUser, (swAllocatedEngine()));
    swEngines.Mengine = utNewAInitFirst(swMengine, (swAllocatedEngine()));
    swEngines.NextMengineEngine = utNewAInitFirst(swEngine, (swAllocatedEngine()));
    swEngines.PrevMengineEngine = utNewAInitFirst(swEngine, (swAllocatedEngine()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Engine.
----------------------------------------------------------------------------------------*/
static void reallocEngines(
    uint32 newSize)
{
    utResizeArray(swEngines.Voice, (newSize));
    utResizeArray(swEngines.Langauge, (newSize));
    utResizeArray(swEngines.Pitch, (newSize));
    utResizeArray(swEngines.Speed, (newSize));
    utResizeArray(swEngines.Volume, (newSize));
    utResizeArray(swEngines.Root, (newSize));
    utResizeArray(swEngines.NextRootEngine, (newSize));
    utResizeArray(swEngines.PrevRootEngine, (newSize));
    utResizeArray(swEngines.User, (newSize));
    utResizeArray(swEngines.Mengine, (newSize));
    utResizeArray(swEngines.NextMengineEngine, (newSize));
    utResizeArray(swEngines.PrevMengineEngine, (newSize));
    swSetAllocatedEngine(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Engines.
----------------------------------------------------------------------------------------*/
void swEngineAllocMore(void)
{
    reallocEngines((uint32)(swAllocatedEngine() + (swAllocatedEngine() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Engine.
----------------------------------------------------------------------------------------*/
void swEngineCopyProps(
    swEngine oldEngine,
    swEngine newEngine)
{
    swEngineSetPitch(newEngine, swEngineGetPitch(oldEngine));
    swEngineSetSpeed(newEngine, swEngineGetSpeed(oldEngine));
    swEngineSetVolume(newEngine, swEngineGetVolume(oldEngine));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowEngine(
    swEngine Engine)
{
    utDatabaseShowObject("sw", "Engine", swEngine2Index(Engine));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Voice including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swVoiceDestroy(
    swVoice Voice)
{
    swMengine owningMengine = swVoiceGetMengine(Voice);
    swLanguage owningLanguage = swVoiceGetLanguage(Voice);

    if(swVoiceDestructorCallback != NULL) {
        swVoiceDestructorCallback(Voice);
    }
    if(owningMengine != swMengineNull) {
        swMengineRemoveVoice(owningMengine, Voice);
#if defined(DD_DEBUG)
    } else {
        utExit("Voice without owning Mengine");
#endif
    }
    if(owningLanguage != swLanguageNull) {
        swLanguageRemoveVoice(owningLanguage, Voice);
#if defined(DD_DEBUG)
    } else {
        utExit("Voice without owning Language");
#endif
    }
    swVoiceFree(Voice);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocVoice(void)
{
    swVoice Voice = swVoiceAlloc();

    return swVoice2Index(Voice);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyVoice(
    uint64 objectIndex)
{
    swVoiceDestroy(swIndex2Voice((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Voice.
----------------------------------------------------------------------------------------*/
static void allocVoices(void)
{
    swSetAllocatedVoice(2);
    swSetUsedVoice(1);
    swSetFirstFreeVoice(swVoiceNull);
    swVoices.Sym = utNewAInitFirst(utSym, (swAllocatedVoice()));
    swVoices.Mengine = utNewAInitFirst(swMengine, (swAllocatedVoice()));
    swVoices.NextMengineVoice = utNewAInitFirst(swVoice, (swAllocatedVoice()));
    swVoices.PrevMengineVoice = utNewAInitFirst(swVoice, (swAllocatedVoice()));
    swVoices.NextTableMengineVoice = utNewAInitFirst(swVoice, (swAllocatedVoice()));
    swVoices.Language = utNewAInitFirst(swLanguage, (swAllocatedVoice()));
    swVoices.NextLanguageVoice = utNewAInitFirst(swVoice, (swAllocatedVoice()));
    swVoices.PrevLanguageVoice = utNewAInitFirst(swVoice, (swAllocatedVoice()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Voice.
----------------------------------------------------------------------------------------*/
static void reallocVoices(
    uint32 newSize)
{
    utResizeArray(swVoices.Sym, (newSize));
    utResizeArray(swVoices.Mengine, (newSize));
    utResizeArray(swVoices.NextMengineVoice, (newSize));
    utResizeArray(swVoices.PrevMengineVoice, (newSize));
    utResizeArray(swVoices.NextTableMengineVoice, (newSize));
    utResizeArray(swVoices.Language, (newSize));
    utResizeArray(swVoices.NextLanguageVoice, (newSize));
    utResizeArray(swVoices.PrevLanguageVoice, (newSize));
    swSetAllocatedVoice(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Voices.
----------------------------------------------------------------------------------------*/
void swVoiceAllocMore(void)
{
    reallocVoices((uint32)(swAllocatedVoice() + (swAllocatedVoice() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Voice.
----------------------------------------------------------------------------------------*/
void swVoiceCopyProps(
    swVoice oldVoice,
    swVoice newVoice)
{
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowVoice(
    swVoice Voice)
{
    utDatabaseShowObject("sw", "Voice", swVoice2Index(Voice));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Queue including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swQueueDestroy(
    swQueue Queue)
{
    swMessage Message_;
    swRoot owningInRoot = swQueueGetInRoot(Queue);
    swRoot owningOutRoot = swQueueGetOutRoot(Queue);
    swRoot owningAudioRoot = swQueueGetAudioRoot(Queue);

    if(swQueueDestructorCallback != NULL) {
        swQueueDestructorCallback(Queue);
    }
    swSafeForeachQueueMessage(Queue, Message_) {
        swMessageDestroy(Message_);
    } swEndSafeQueueMessage;
    if(owningInRoot != swRootNull) {
        swRootSetInQueue(owningInRoot, swQueueNull);
    }
    if(owningOutRoot != swRootNull) {
        swRootSetOutQueue(owningOutRoot, swQueueNull);
    }
    if(owningAudioRoot != swRootNull) {
        swRootSetAudioQueue(owningAudioRoot, swQueueNull);
    }
    swQueueFree(Queue);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocQueue(void)
{
    swQueue Queue = swQueueAlloc();

    return swQueue2Index(Queue);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyQueue(
    uint64 objectIndex)
{
    swQueueDestroy(swIndex2Queue((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Queue.
----------------------------------------------------------------------------------------*/
static void allocQueues(void)
{
    swSetAllocatedQueue(2);
    swSetUsedQueue(1);
    swSetFirstFreeQueue(swQueueNull);
    swQueues.InRoot = utNewAInitFirst(swRoot, (swAllocatedQueue()));
    swQueues.OutRoot = utNewAInitFirst(swRoot, (swAllocatedQueue()));
    swQueues.AudioRoot = utNewAInitFirst(swRoot, (swAllocatedQueue()));
    swQueues.FirstMessage = utNewAInitFirst(swMessage, (swAllocatedQueue()));
    swQueues.LastMessage = utNewAInitFirst(swMessage, (swAllocatedQueue()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Queue.
----------------------------------------------------------------------------------------*/
static void reallocQueues(
    uint32 newSize)
{
    utResizeArray(swQueues.InRoot, (newSize));
    utResizeArray(swQueues.OutRoot, (newSize));
    utResizeArray(swQueues.AudioRoot, (newSize));
    utResizeArray(swQueues.FirstMessage, (newSize));
    utResizeArray(swQueues.LastMessage, (newSize));
    swSetAllocatedQueue(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Queues.
----------------------------------------------------------------------------------------*/
void swQueueAllocMore(void)
{
    reallocQueues((uint32)(swAllocatedQueue() + (swAllocatedQueue() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Queue.
----------------------------------------------------------------------------------------*/
void swQueueCopyProps(
    swQueue oldQueue,
    swQueue newQueue)
{
}

/*----------------------------------------------------------------------------------------
  Add the Message to the head of the list on the Queue.
----------------------------------------------------------------------------------------*/
void swQueueInsertMessage(
    swQueue Queue,
    swMessage _Message)
{
#if defined(DD_DEBUG)
    if(Queue == swQueueNull) {
        utExit("Non-existent Queue");
    }
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetQueue(_Message) != swQueueNull) {
        utExit("Attempting to add Message to Queue twice");
    }
#endif
    swMessageSetNextQueueMessage(_Message, swQueueGetFirstMessage(Queue));
    if(swQueueGetFirstMessage(Queue) != swMessageNull) {
        swMessageSetPrevQueueMessage(swQueueGetFirstMessage(Queue), _Message);
    }
    swQueueSetFirstMessage(Queue, _Message);
    swMessageSetPrevQueueMessage(_Message, swMessageNull);
    if(swQueueGetLastMessage(Queue) == swMessageNull) {
        swQueueSetLastMessage(Queue, _Message);
    }
    swMessageSetQueue(_Message, Queue);
}

/*----------------------------------------------------------------------------------------
  Add the Message to the end of the list on the Queue.
----------------------------------------------------------------------------------------*/
void swQueueAppendMessage(
    swQueue Queue,
    swMessage _Message)
{
#if defined(DD_DEBUG)
    if(Queue == swQueueNull) {
        utExit("Non-existent Queue");
    }
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetQueue(_Message) != swQueueNull) {
        utExit("Attempting to add Message to Queue twice");
    }
#endif
    swMessageSetPrevQueueMessage(_Message, swQueueGetLastMessage(Queue));
    if(swQueueGetLastMessage(Queue) != swMessageNull) {
        swMessageSetNextQueueMessage(swQueueGetLastMessage(Queue), _Message);
    }
    swQueueSetLastMessage(Queue, _Message);
    swMessageSetNextQueueMessage(_Message, swMessageNull);
    if(swQueueGetFirstMessage(Queue) == swMessageNull) {
        swQueueSetFirstMessage(Queue, _Message);
    }
    swMessageSetQueue(_Message, Queue);
}

/*----------------------------------------------------------------------------------------
  Insert the Message to the Queue after the previous Message.
----------------------------------------------------------------------------------------*/
void swQueueInsertAfterMessage(
    swQueue Queue,
    swMessage prevMessage,
    swMessage _Message)
{
    swMessage nextMessage = swMessageGetNextQueueMessage(prevMessage);

#if defined(DD_DEBUG)
    if(Queue == swQueueNull) {
        utExit("Non-existent Queue");
    }
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetQueue(_Message) != swQueueNull) {
        utExit("Attempting to add Message to Queue twice");
    }
#endif
    swMessageSetNextQueueMessage(_Message, nextMessage);
    swMessageSetNextQueueMessage(prevMessage, _Message);
    swMessageSetPrevQueueMessage(_Message, prevMessage);
    if(nextMessage != swMessageNull) {
        swMessageSetPrevQueueMessage(nextMessage, _Message);
    }
    if(swQueueGetLastMessage(Queue) == prevMessage) {
        swQueueSetLastMessage(Queue, _Message);
    }
    swMessageSetQueue(_Message, Queue);
}

/*----------------------------------------------------------------------------------------
 Remove the Message from the Queue.
----------------------------------------------------------------------------------------*/
void swQueueRemoveMessage(
    swQueue Queue,
    swMessage _Message)
{
    swMessage pMessage, nMessage;

#if defined(DD_DEBUG)
    if(_Message == swMessageNull) {
        utExit("Non-existent Message");
    }
    if(swMessageGetQueue(_Message) != swQueueNull && swMessageGetQueue(_Message) != Queue) {
        utExit("Delete Message from non-owning Queue");
    }
#endif
    nMessage = swMessageGetNextQueueMessage(_Message);
    pMessage = swMessageGetPrevQueueMessage(_Message);
    if(pMessage != swMessageNull) {
        swMessageSetNextQueueMessage(pMessage, nMessage);
    } else if(swQueueGetFirstMessage(Queue) == _Message) {
        swQueueSetFirstMessage(Queue, nMessage);
    }
    if(nMessage != swMessageNull) {
        swMessageSetPrevQueueMessage(nMessage, pMessage);
    } else if(swQueueGetLastMessage(Queue) == _Message) {
        swQueueSetLastMessage(Queue, pMessage);
    }
    swMessageSetNextQueueMessage(_Message, swMessageNull);
    swMessageSetPrevQueueMessage(_Message, swMessageNull);
    swMessageSetQueue(_Message, swQueueNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowQueue(
    swQueue Queue)
{
    utDatabaseShowObject("sw", "Queue", swQueue2Index(Queue));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Message including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swMessageDestroy(
    swMessage Message)
{
    swQueue owningQueue = swMessageGetQueue(Message);
    swUser owningUser = swMessageGetUser(Message);

    if(swMessageDestructorCallback != NULL) {
        swMessageDestructorCallback(Message);
    }
    if(owningQueue != swQueueNull) {
        swQueueRemoveMessage(owningQueue, Message);
#if defined(DD_DEBUG)
    } else {
        utExit("Message without owning Queue");
#endif
    }
    if(owningUser != swUserNull) {
        swUserRemoveMessage(owningUser, Message);
#if defined(DD_DEBUG)
    } else {
        utExit("Message without owning User");
#endif
    }
    swMessageFree(Message);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocMessage(void)
{
    swMessage Message = swMessageAlloc();

    return swMessage2Index(Message);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyMessage(
    uint64 objectIndex)
{
    swMessageDestroy(swIndex2Message((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Message.
----------------------------------------------------------------------------------------*/
static void allocMessages(void)
{
    swSetAllocatedMessage(2);
    swSetUsedMessage(1);
    swSetFirstFreeMessage(swMessageNull);
    swMessages.Type = utNewAInitFirst(swMessageType, (swAllocatedMessage()));
    swMessages.DataIndex_ = utNewAInitFirst(uint32, (swAllocatedMessage()));
    swMessages.NumData = utNewAInitFirst(uint32, (swAllocatedMessage()));
    swSetUsedMessageData(0);
    swSetAllocatedMessageData(2);
    swSetFreeMessageData(0);
    swMessages.Data = utNewAInitFirst(uint8, swAllocatedMessageData());
    swMessages.User = utNewAInitFirst(swUser, (swAllocatedMessage()));
    swMessages.NextUserMessage = utNewAInitFirst(swMessage, (swAllocatedMessage()));
    swMessages.PrevUserMessage = utNewAInitFirst(swMessage, (swAllocatedMessage()));
    swMessages.Queue = utNewAInitFirst(swQueue, (swAllocatedMessage()));
    swMessages.NextQueueMessage = utNewAInitFirst(swMessage, (swAllocatedMessage()));
    swMessages.PrevQueueMessage = utNewAInitFirst(swMessage, (swAllocatedMessage()));
    swMessages.union1 = utNewAInitFirst(swMessageUnion1, swAllocatedMessage());
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Message.
----------------------------------------------------------------------------------------*/
static void reallocMessages(
    uint32 newSize)
{
    utResizeArray(swMessages.Type, (newSize));
    utResizeArray(swMessages.DataIndex_, (newSize));
    utResizeArray(swMessages.NumData, (newSize));
    utResizeArray(swMessages.User, (newSize));
    utResizeArray(swMessages.NextUserMessage, (newSize));
    utResizeArray(swMessages.PrevUserMessage, (newSize));
    utResizeArray(swMessages.Queue, (newSize));
    utResizeArray(swMessages.NextQueueMessage, (newSize));
    utResizeArray(swMessages.PrevQueueMessage, (newSize));
    utResizeArray(swMessages.union1, newSize);
    swSetAllocatedMessage(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Messages.
----------------------------------------------------------------------------------------*/
void swMessageAllocMore(void)
{
    reallocMessages((uint32)(swAllocatedMessage() + (swAllocatedMessage() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Compact the Message.Data heap to free memory.
----------------------------------------------------------------------------------------*/
void swCompactMessageDatas(void)
{
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(swMessage) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMessage) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint8 *toPtr = swMessages.Data;
    uint8 *fromPtr = toPtr;
    swMessage Message;
    uint32 size;

    while(fromPtr < swMessages.Data + swUsedMessageData()) {
        Message = *(swMessage *)(void *)fromPtr;
        if(Message != swMessageNull) {
            /* Need to move it to toPtr */
            size = utMax(swMessageGetNumData(Message) + usedHeaderSize, freeHeaderSize);
            memmove((void *)toPtr, (void *)fromPtr, size*elementSize);
            swMessageSetDataIndex_(Message, toPtr - swMessages.Data + usedHeaderSize);
            toPtr += size;
        } else {
            /* Just skip it */
            size = utMax(*(uint32 *)(void *)(((swMessage *)(void *)fromPtr) + 1), freeHeaderSize);
        }
        fromPtr += size;
    }
    swSetUsedMessageData(toPtr - swMessages.Data);
    swSetFreeMessageData(0);
}

/*----------------------------------------------------------------------------------------
  Allocate more memory for the Message.Data heap.
----------------------------------------------------------------------------------------*/
static void allocMoreMessageDatas(
    uint32 spaceNeeded)
{
    uint32 freeSpace = swAllocatedMessageData() - swUsedMessageData();
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(swMessage) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMessage) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint8 *ptr = swMessages.Data;
    swMessage Message;
    uint32 size;

    while(ptr < swMessages.Data + swUsedMessageData()) {
        Message = *(swMessage*)(void*)ptr;
        if(Message != swMessageNull) {
            swValidMessage(Message);
            size = utMax(swMessageGetNumData(Message) + usedHeaderSize, freeHeaderSize);
        } else {
            size = utMax(*(uint32 *)(void *)(((swMessage *)(void *)ptr) + 1), freeHeaderSize);
        }
        ptr += size;
    }
    if((swFreeMessageData() << 2) > swUsedMessageData()) {
        swCompactMessageDatas();
        freeSpace = swAllocatedMessageData() - swUsedMessageData();
    }
    if(freeSpace < spaceNeeded) {
        swSetAllocatedMessageData(swAllocatedMessageData() + spaceNeeded - freeSpace +
            (swAllocatedMessageData() >> 1));
        utResizeArray(swMessages.Data, swAllocatedMessageData());
    }
}

/*----------------------------------------------------------------------------------------
  Allocate memory for a new Message.Data array.
----------------------------------------------------------------------------------------*/
void swMessageAllocDatas(
    swMessage Message,
    uint32 numDatas)
{
    uint32 freeSpace = swAllocatedMessageData() - swUsedMessageData();
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(swMessage) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMessage) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 spaceNeeded = utMax(numDatas + usedHeaderSize, freeHeaderSize);

#if defined(DD_DEBUG)
    utAssert(swMessageGetNumData(Message) == 0);
#endif
    if(numDatas == 0) {
        return;
    }
    if(freeSpace < spaceNeeded) {
        allocMoreMessageDatas(spaceNeeded);
    }
    swMessageSetDataIndex_(Message, swUsedMessageData() + usedHeaderSize);
    swMessageSetNumData(Message, numDatas);
    *(swMessage *)(void *)(swMessages.Data + swUsedMessageData()) = Message;
    memset(swMessages.Data + swMessageGetDataIndex_(Message), 0, ((numDatas))*sizeof(uint8));
    swSetUsedMessageData(swUsedMessageData() + spaceNeeded);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swMessageGetDatas for the database manager.
----------------------------------------------------------------------------------------*/
static void *getMessageDatas(
    uint64 objectNumber,
    uint32 *numValues)
{
    swMessage Message = swIndex2Message((uint32)objectNumber);

    *numValues = swMessageGetNumData(Message);
    return swMessageGetDatas(Message);
}

/*----------------------------------------------------------------------------------------
  Wrapper around swMessageAllocDatas for the database manager.
----------------------------------------------------------------------------------------*/
static void *allocMessageDatas(
    uint64 objectNumber,
    uint32 numValues)
{
    swMessage Message = swIndex2Message((uint32)objectNumber);

    swMessageSetDataIndex_(Message, 0);
    swMessageSetNumData(Message, 0);
    if(numValues == 0) {
        return NULL;
    }
    swMessageAllocDatas(Message, numValues);
    return swMessageGetDatas(Message);
}

/*----------------------------------------------------------------------------------------
  Free memory used by the Message.Data array.
----------------------------------------------------------------------------------------*/
void swMessageFreeDatas(
    swMessage Message)
{
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(swMessage) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMessage) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 size = utMax(swMessageGetNumData(Message) + usedHeaderSize, freeHeaderSize);
    uint8 *dataPtr = swMessageGetDatas(Message) - usedHeaderSize;

    if(swMessageGetNumData(Message) == 0) {
        return;
    }
    *(swMessage *)(void *)(dataPtr) = swMessageNull;
    *(uint32 *)(void *)(((swMessage *)(void *)dataPtr) + 1) = size;
    swMessageSetNumData(Message, 0);
    swSetFreeMessageData(swFreeMessageData() + size);
}

/*----------------------------------------------------------------------------------------
  Resize the Message.Data array.
----------------------------------------------------------------------------------------*/
void swMessageResizeDatas(
    swMessage Message,
    uint32 numDatas)
{
    uint32 freeSpace;
    uint32 elementSize = sizeof(uint8);
    uint32 usedHeaderSize = (sizeof(swMessage) + elementSize - 1)/elementSize;
    uint32 freeHeaderSize = (sizeof(swMessage) + sizeof(uint32) + elementSize - 1)/elementSize;
    uint32 newSize = utMax(numDatas + usedHeaderSize, freeHeaderSize);
    uint32 oldSize = utMax(swMessageGetNumData(Message) + usedHeaderSize, freeHeaderSize);
    uint8 *dataPtr;

    if(numDatas == 0) {
        if(swMessageGetNumData(Message) != 0) {
            swMessageFreeDatas(Message);
        }
        return;
    }
    if(swMessageGetNumData(Message) == 0) {
        swMessageAllocDatas(Message, numDatas);
        return;
    }
    freeSpace = swAllocatedMessageData() - swUsedMessageData();
    if(freeSpace < newSize) {
        allocMoreMessageDatas(newSize);
    }
    dataPtr = swMessageGetDatas(Message) - usedHeaderSize;
    memcpy((void *)(swMessages.Data + swUsedMessageData()), dataPtr,
        elementSize*utMin(oldSize, newSize));
    if(newSize > oldSize) {
        memset(swMessages.Data + swUsedMessageData() + oldSize, 0, ((newSize - oldSize))*sizeof(uint8));
    }
    *(swMessage *)(void *)dataPtr = swMessageNull;
    *(uint32 *)(void *)(((swMessage *)(void *)dataPtr) + 1) = oldSize;
    swSetFreeMessageData(swFreeMessageData() + oldSize);
    swMessageSetDataIndex_(Message, swUsedMessageData() + usedHeaderSize);
    swMessageSetNumData(Message, numDatas);
    swSetUsedMessageData(swUsedMessageData() + newSize);
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Message.
----------------------------------------------------------------------------------------*/
void swMessageCopyProps(
    swMessage oldMessage,
    swMessage newMessage)
{
    swMessageSetType(newMessage, swMessageGetType(oldMessage));
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowMessage(
    swMessage Message)
{
    utDatabaseShowObject("sw", "Message", swMessage2Index(Message));
}
#endif

/*----------------------------------------------------------------------------------------
  Destroy Language including everything in it. Remove from parents.
----------------------------------------------------------------------------------------*/
void swLanguageDestroy(
    swLanguage Language)
{
    swVoice Voice_;
    swRoot owningRoot = swLanguageGetRoot(Language);

    if(swLanguageDestructorCallback != NULL) {
        swLanguageDestructorCallback(Language);
    }
    swSafeForeachLanguageVoice(Language, Voice_) {
        swVoiceDestroy(Voice_);
    } swEndSafeLanguageVoice;
    if(owningRoot != swRootNull) {
        swRootRemoveLanguage(owningRoot, Language);
#if defined(DD_DEBUG)
    } else {
        utExit("Language without owning Root");
#endif
    }
    swLanguageFree(Language);
}

/*----------------------------------------------------------------------------------------
  Default constructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static uint64 allocLanguage(void)
{
    swLanguage Language = swLanguageAlloc();

    return swLanguage2Index(Language);
}

/*----------------------------------------------------------------------------------------
  Destructor wrapper for the database manager.
----------------------------------------------------------------------------------------*/
static void destroyLanguage(
    uint64 objectIndex)
{
    swLanguageDestroy(swIndex2Language((uint32)objectIndex));
}

/*----------------------------------------------------------------------------------------
  Allocate the field arrays of Language.
----------------------------------------------------------------------------------------*/
static void allocLanguages(void)
{
    swSetAllocatedLanguage(2);
    swSetUsedLanguage(1);
    swSetFirstFreeLanguage(swLanguageNull);
    swLanguages.Sym = utNewAInitFirst(utSym, (swAllocatedLanguage()));
    swLanguages.Root = utNewAInitFirst(swRoot, (swAllocatedLanguage()));
    swLanguages.NextRootLanguage = utNewAInitFirst(swLanguage, (swAllocatedLanguage()));
    swLanguages.PrevRootLanguage = utNewAInitFirst(swLanguage, (swAllocatedLanguage()));
    swLanguages.NextTableRootLanguage = utNewAInitFirst(swLanguage, (swAllocatedLanguage()));
    swLanguages.FirstVoice = utNewAInitFirst(swVoice, (swAllocatedLanguage()));
    swLanguages.LastVoice = utNewAInitFirst(swVoice, (swAllocatedLanguage()));
}

/*----------------------------------------------------------------------------------------
  Realloc the arrays of properties for class Language.
----------------------------------------------------------------------------------------*/
static void reallocLanguages(
    uint32 newSize)
{
    utResizeArray(swLanguages.Sym, (newSize));
    utResizeArray(swLanguages.Root, (newSize));
    utResizeArray(swLanguages.NextRootLanguage, (newSize));
    utResizeArray(swLanguages.PrevRootLanguage, (newSize));
    utResizeArray(swLanguages.NextTableRootLanguage, (newSize));
    utResizeArray(swLanguages.FirstVoice, (newSize));
    utResizeArray(swLanguages.LastVoice, (newSize));
    swSetAllocatedLanguage(newSize);
}

/*----------------------------------------------------------------------------------------
  Allocate more Languages.
----------------------------------------------------------------------------------------*/
void swLanguageAllocMore(void)
{
    reallocLanguages((uint32)(swAllocatedLanguage() + (swAllocatedLanguage() >> 1)));
}

/*----------------------------------------------------------------------------------------
  Copy the properties of Language.
----------------------------------------------------------------------------------------*/
void swLanguageCopyProps(
    swLanguage oldLanguage,
    swLanguage newLanguage)
{
}

/*----------------------------------------------------------------------------------------
  Add the Voice to the head of the list on the Language.
----------------------------------------------------------------------------------------*/
void swLanguageInsertVoice(
    swLanguage Language,
    swVoice _Voice)
{
#if defined(DD_DEBUG)
    if(Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetLanguage(_Voice) != swLanguageNull) {
        utExit("Attempting to add Voice to Language twice");
    }
#endif
    swVoiceSetNextLanguageVoice(_Voice, swLanguageGetFirstVoice(Language));
    if(swLanguageGetFirstVoice(Language) != swVoiceNull) {
        swVoiceSetPrevLanguageVoice(swLanguageGetFirstVoice(Language), _Voice);
    }
    swLanguageSetFirstVoice(Language, _Voice);
    swVoiceSetPrevLanguageVoice(_Voice, swVoiceNull);
    if(swLanguageGetLastVoice(Language) == swVoiceNull) {
        swLanguageSetLastVoice(Language, _Voice);
    }
    swVoiceSetLanguage(_Voice, Language);
}

/*----------------------------------------------------------------------------------------
  Add the Voice to the end of the list on the Language.
----------------------------------------------------------------------------------------*/
void swLanguageAppendVoice(
    swLanguage Language,
    swVoice _Voice)
{
#if defined(DD_DEBUG)
    if(Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetLanguage(_Voice) != swLanguageNull) {
        utExit("Attempting to add Voice to Language twice");
    }
#endif
    swVoiceSetPrevLanguageVoice(_Voice, swLanguageGetLastVoice(Language));
    if(swLanguageGetLastVoice(Language) != swVoiceNull) {
        swVoiceSetNextLanguageVoice(swLanguageGetLastVoice(Language), _Voice);
    }
    swLanguageSetLastVoice(Language, _Voice);
    swVoiceSetNextLanguageVoice(_Voice, swVoiceNull);
    if(swLanguageGetFirstVoice(Language) == swVoiceNull) {
        swLanguageSetFirstVoice(Language, _Voice);
    }
    swVoiceSetLanguage(_Voice, Language);
}

/*----------------------------------------------------------------------------------------
  Insert the Voice to the Language after the previous Voice.
----------------------------------------------------------------------------------------*/
void swLanguageInsertAfterVoice(
    swLanguage Language,
    swVoice prevVoice,
    swVoice _Voice)
{
    swVoice nextVoice = swVoiceGetNextLanguageVoice(prevVoice);

#if defined(DD_DEBUG)
    if(Language == swLanguageNull) {
        utExit("Non-existent Language");
    }
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetLanguage(_Voice) != swLanguageNull) {
        utExit("Attempting to add Voice to Language twice");
    }
#endif
    swVoiceSetNextLanguageVoice(_Voice, nextVoice);
    swVoiceSetNextLanguageVoice(prevVoice, _Voice);
    swVoiceSetPrevLanguageVoice(_Voice, prevVoice);
    if(nextVoice != swVoiceNull) {
        swVoiceSetPrevLanguageVoice(nextVoice, _Voice);
    }
    if(swLanguageGetLastVoice(Language) == prevVoice) {
        swLanguageSetLastVoice(Language, _Voice);
    }
    swVoiceSetLanguage(_Voice, Language);
}

/*----------------------------------------------------------------------------------------
 Remove the Voice from the Language.
----------------------------------------------------------------------------------------*/
void swLanguageRemoveVoice(
    swLanguage Language,
    swVoice _Voice)
{
    swVoice pVoice, nVoice;

#if defined(DD_DEBUG)
    if(_Voice == swVoiceNull) {
        utExit("Non-existent Voice");
    }
    if(swVoiceGetLanguage(_Voice) != swLanguageNull && swVoiceGetLanguage(_Voice) != Language) {
        utExit("Delete Voice from non-owning Language");
    }
#endif
    nVoice = swVoiceGetNextLanguageVoice(_Voice);
    pVoice = swVoiceGetPrevLanguageVoice(_Voice);
    if(pVoice != swVoiceNull) {
        swVoiceSetNextLanguageVoice(pVoice, nVoice);
    } else if(swLanguageGetFirstVoice(Language) == _Voice) {
        swLanguageSetFirstVoice(Language, nVoice);
    }
    if(nVoice != swVoiceNull) {
        swVoiceSetPrevLanguageVoice(nVoice, pVoice);
    } else if(swLanguageGetLastVoice(Language) == _Voice) {
        swLanguageSetLastVoice(Language, pVoice);
    }
    swVoiceSetNextLanguageVoice(_Voice, swVoiceNull);
    swVoiceSetPrevLanguageVoice(_Voice, swVoiceNull);
    swVoiceSetLanguage(_Voice, swLanguageNull);
}

#if defined(DD_DEBUG)
/*----------------------------------------------------------------------------------------
  Write out all the fields of an object.
----------------------------------------------------------------------------------------*/
void swShowLanguage(
    swLanguage Language)
{
    utDatabaseShowObject("sw", "Language", swLanguage2Index(Language));
}
#endif

/*----------------------------------------------------------------------------------------
  Free memory used by the sw database.
----------------------------------------------------------------------------------------*/
void swDatabaseStop(void)
{
    utFree(swRoots.FirstUser);
    utFree(swRoots.LastUser);
    utFree(swRoots.UserTableIndex_);
    utFree(swRoots.NumUserTable);
    utFree(swRoots.UserTable);
    utFree(swRoots.NumUser);
    utFree(swRoots.FirstEngine);
    utFree(swRoots.LastEngine);
    utFree(swRoots.InQueue);
    utFree(swRoots.OutQueue);
    utFree(swRoots.AudioQueue);
    utFree(swRoots.FirstLanguage);
    utFree(swRoots.LastLanguage);
    utFree(swRoots.LanguageTableIndex_);
    utFree(swRoots.NumLanguageTable);
    utFree(swRoots.LanguageTable);
    utFree(swRoots.NumLanguage);
    utFree(swUsers.Id);
    utFree(swUsers.Root);
    utFree(swUsers.NextRootUser);
    utFree(swUsers.PrevRootUser);
    utFree(swUsers.NextTableRootUser);
    utFree(swUsers.Engine);
    utFree(swUsers.FirstMessage);
    utFree(swUsers.LastMessage);
    utFree(swMengines.FirstEngine);
    utFree(swMengines.LastEngine);
    utFree(swMengines.FirstVoice);
    utFree(swMengines.LastVoice);
    utFree(swMengines.VoiceTableIndex_);
    utFree(swMengines.NumVoiceTable);
    utFree(swMengines.VoiceTable);
    utFree(swMengines.NumVoice);
    utFree(swEngines.Voice);
    utFree(swEngines.Langauge);
    utFree(swEngines.Pitch);
    utFree(swEngines.Speed);
    utFree(swEngines.Volume);
    utFree(swEngines.Root);
    utFree(swEngines.NextRootEngine);
    utFree(swEngines.PrevRootEngine);
    utFree(swEngines.User);
    utFree(swEngines.Mengine);
    utFree(swEngines.NextMengineEngine);
    utFree(swEngines.PrevMengineEngine);
    utFree(swVoices.Sym);
    utFree(swVoices.Mengine);
    utFree(swVoices.NextMengineVoice);
    utFree(swVoices.PrevMengineVoice);
    utFree(swVoices.NextTableMengineVoice);
    utFree(swVoices.Language);
    utFree(swVoices.NextLanguageVoice);
    utFree(swVoices.PrevLanguageVoice);
    utFree(swQueues.InRoot);
    utFree(swQueues.OutRoot);
    utFree(swQueues.AudioRoot);
    utFree(swQueues.FirstMessage);
    utFree(swQueues.LastMessage);
    utFree(swMessages.Type);
    utFree(swMessages.DataIndex_);
    utFree(swMessages.NumData);
    utFree(swMessages.Data);
    utFree(swMessages.User);
    utFree(swMessages.NextUserMessage);
    utFree(swMessages.PrevUserMessage);
    utFree(swMessages.Queue);
    utFree(swMessages.NextQueueMessage);
    utFree(swMessages.PrevQueueMessage);
    utFree(swMessages.union1);
    utFree(swLanguages.Sym);
    utFree(swLanguages.Root);
    utFree(swLanguages.NextRootLanguage);
    utFree(swLanguages.PrevRootLanguage);
    utFree(swLanguages.NextTableRootLanguage);
    utFree(swLanguages.FirstVoice);
    utFree(swLanguages.LastVoice);
    utUnregisterModule(swModuleID);
}

/*----------------------------------------------------------------------------------------
  Allocate memory used by the sw database.
----------------------------------------------------------------------------------------*/
void swDatabaseStart(void)
{
    if(!utInitialized()) {
        utStart();
    }
    swRootData.hash = 0x9ffa1e63;
    swModuleID = utRegisterModule("sw", false, swHash(), 8, 76, 1, sizeof(struct swRootType_),
        &swRootData, swDatabaseStart, swDatabaseStop);
    utRegisterEnum("MessageType", 6);
    utRegisterEntry("SW_SPEAK", 0);
    utRegisterEntry("SW_MARK", 1);
    utRegisterEntry("SW_SETSPEED", 2);
    utRegisterEntry("SW_SETPITCH", 3);
    utRegisterEntry("SW_SETVOLUME", 4);
    utRegisterEntry("SW_AUDIO", 5);
    utRegisterClass("Root", 17, &swRootData.usedRoot, &swRootData.allocatedRoot,
        &swRootData.firstFreeRoot, 0, 4, allocRoot, destroyRoot);
    utRegisterField("FirstUser", &swRoots.FirstUser, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("LastUser", &swRoots.LastUser, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("UserTableIndex_", &swRoots.UserTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumUserTable", &swRoots.NumUserTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("UserTable", &swRoots.UserTable, sizeof(swUser), UT_POINTER, "User");
    utRegisterArray(&swRootData.usedRootUserTable, &swRootData.allocatedRootUserTable,
        getRootUserTables, allocRootUserTables, swCompactRootUserTables);
    utRegisterField("NumUser", &swRoots.NumUser, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("FirstEngine", &swRoots.FirstEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("LastEngine", &swRoots.LastEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("InQueue", &swRoots.InQueue, sizeof(swQueue), UT_POINTER, "Queue");
    utRegisterField("OutQueue", &swRoots.OutQueue, sizeof(swQueue), UT_POINTER, "Queue");
    utRegisterField("AudioQueue", &swRoots.AudioQueue, sizeof(swQueue), UT_POINTER, "Queue");
    utRegisterField("FirstLanguage", &swRoots.FirstLanguage, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("LastLanguage", &swRoots.LastLanguage, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("LanguageTableIndex_", &swRoots.LanguageTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumLanguageTable", &swRoots.NumLanguageTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("LanguageTable", &swRoots.LanguageTable, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterArray(&swRootData.usedRootLanguageTable, &swRootData.allocatedRootLanguageTable,
        getRootLanguageTables, allocRootLanguageTables, swCompactRootLanguageTables);
    utRegisterField("NumLanguage", &swRoots.NumLanguage, sizeof(uint32), UT_UINT, NULL);
    utRegisterClass("User", 8, &swRootData.usedUser, &swRootData.allocatedUser,
        &swRootData.firstFreeUser, 18, 4, allocUser, destroyUser);
    utRegisterField("Id", &swUsers.Id, sizeof(uint32), UT_UINT, NULL);
    utRegisterField("Root", &swUsers.Root, sizeof(swRoot), UT_POINTER, "Root");
    utRegisterField("NextRootUser", &swUsers.NextRootUser, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("PrevRootUser", &swUsers.PrevRootUser, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("NextTableRootUser", &swUsers.NextTableRootUser, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("Engine", &swUsers.Engine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("FirstMessage", &swUsers.FirstMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterField("LastMessage", &swUsers.LastMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterClass("Mengine", 8, &swRootData.usedMengine, &swRootData.allocatedMengine,
        &swRootData.firstFreeMengine, 25, 4, allocMengine, destroyMengine);
    utRegisterField("FirstEngine", &swMengines.FirstEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("LastEngine", &swMengines.LastEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("FirstVoice", &swMengines.FirstVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("LastVoice", &swMengines.LastVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("VoiceTableIndex_", &swMengines.VoiceTableIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumVoiceTable", &swMengines.NumVoiceTable, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("VoiceTable", &swMengines.VoiceTable, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterArray(&swRootData.usedMengineVoiceTable, &swRootData.allocatedMengineVoiceTable,
        getMengineVoiceTables, allocMengineVoiceTables, swCompactMengineVoiceTables);
    utRegisterField("NumVoice", &swMengines.NumVoice, sizeof(uint32), UT_UINT, NULL);
    utRegisterClass("Engine", 12, &swRootData.usedEngine, &swRootData.allocatedEngine,
        &swRootData.firstFreeEngine, 33, 4, allocEngine, destroyEngine);
    utRegisterField("Voice", &swEngines.Voice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("Langauge", &swEngines.Langauge, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("Pitch", &swEngines.Pitch, sizeof(float), UT_FLOAT, NULL);
    utRegisterField("Speed", &swEngines.Speed, sizeof(float), UT_FLOAT, NULL);
    utRegisterField("Volume", &swEngines.Volume, sizeof(float), UT_FLOAT, NULL);
    utRegisterField("Root", &swEngines.Root, sizeof(swRoot), UT_POINTER, "Root");
    utRegisterField("NextRootEngine", &swEngines.NextRootEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("PrevRootEngine", &swEngines.PrevRootEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("User", &swEngines.User, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("Mengine", &swEngines.Mengine, sizeof(swMengine), UT_POINTER, "Mengine");
    utRegisterField("NextMengineEngine", &swEngines.NextMengineEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterField("PrevMengineEngine", &swEngines.PrevMengineEngine, sizeof(swEngine), UT_POINTER, "Engine");
    utRegisterClass("Voice", 8, &swRootData.usedVoice, &swRootData.allocatedVoice,
        &swRootData.firstFreeVoice, 45, 4, allocVoice, destroyVoice);
    utRegisterField("Sym", &swVoices.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Mengine", &swVoices.Mengine, sizeof(swMengine), UT_POINTER, "Mengine");
    utRegisterField("NextMengineVoice", &swVoices.NextMengineVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("PrevMengineVoice", &swVoices.PrevMengineVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("NextTableMengineVoice", &swVoices.NextTableMengineVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("Language", &swVoices.Language, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("NextLanguageVoice", &swVoices.NextLanguageVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("PrevLanguageVoice", &swVoices.PrevLanguageVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterClass("Queue", 5, &swRootData.usedQueue, &swRootData.allocatedQueue,
        &swRootData.firstFreeQueue, 53, 4, allocQueue, destroyQueue);
    utRegisterField("InRoot", &swQueues.InRoot, sizeof(swRoot), UT_POINTER, "Root");
    utRegisterField("OutRoot", &swQueues.OutRoot, sizeof(swRoot), UT_POINTER, "Root");
    utRegisterField("AudioRoot", &swQueues.AudioRoot, sizeof(swRoot), UT_POINTER, "Root");
    utRegisterField("FirstMessage", &swQueues.FirstMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterField("LastMessage", &swQueues.LastMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterClass("Message", 11, &swRootData.usedMessage, &swRootData.allocatedMessage,
        &swRootData.firstFreeMessage, 62, 4, allocMessage, destroyMessage);
    utRegisterField("Type", &swMessages.Type, sizeof(swMessageType), UT_ENUM, "MessageType");
    utRegisterField("DataIndex_", &swMessages.DataIndex_, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("NumData", &swMessages.NumData, sizeof(uint32), UT_UINT, NULL);
    utSetFieldHidden();
    utRegisterField("Data", &swMessages.Data, sizeof(uint8), UT_UINT, NULL);
    utRegisterArray(&swRootData.usedMessageData, &swRootData.allocatedMessageData,
        getMessageDatas, allocMessageDatas, swCompactMessageDatas);
    utRegisterField("User", &swMessages.User, sizeof(swUser), UT_POINTER, "User");
    utRegisterField("NextUserMessage", &swMessages.NextUserMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterField("PrevUserMessage", &swMessages.PrevUserMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterField("Queue", &swMessages.Queue, sizeof(swQueue), UT_POINTER, "Queue");
    utRegisterField("NextQueueMessage", &swMessages.NextQueueMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterField("PrevQueueMessage", &swMessages.PrevQueueMessage, sizeof(swMessage), UT_POINTER, "Message");
    utRegisterField("union1", &swMessages.union1, sizeof(swMessageUnion1), UT_UNION, "Type");
    utRegisterUnion("Type", 4);
    utRegisterUnionCase(1, UT_UINT, sizeof(uint32));
    utRegisterUnionCase(2, UT_FLOAT, sizeof(float));
    utRegisterUnionCase(3, UT_FLOAT, sizeof(float));
    utRegisterUnionCase(4, UT_FLOAT, sizeof(float));
    utRegisterClass("Language", 7, &swRootData.usedLanguage, &swRootData.allocatedLanguage,
        &swRootData.firstFreeLanguage, 69, 4, allocLanguage, destroyLanguage);
    utRegisterField("Sym", &swLanguages.Sym, sizeof(utSym), UT_SYM, NULL);
    utRegisterField("Root", &swLanguages.Root, sizeof(swRoot), UT_POINTER, "Root");
    utRegisterField("NextRootLanguage", &swLanguages.NextRootLanguage, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("PrevRootLanguage", &swLanguages.PrevRootLanguage, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("NextTableRootLanguage", &swLanguages.NextTableRootLanguage, sizeof(swLanguage), UT_POINTER, "Language");
    utRegisterField("FirstVoice", &swLanguages.FirstVoice, sizeof(swVoice), UT_POINTER, "Voice");
    utRegisterField("LastVoice", &swLanguages.LastVoice, sizeof(swVoice), UT_POINTER, "Voice");
    allocRoots();
    allocUsers();
    allocMengines();
    allocEngines();
    allocVoices();
    allocQueues();
    allocMessages();
    allocLanguages();
}

