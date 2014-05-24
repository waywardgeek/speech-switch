/*----------------------------------------------------------------------------------------
  Module header file for: sw module
----------------------------------------------------------------------------------------*/
#ifndef SWDATABASE_H

#define SWDATABASE_H

#if defined __cplusplus
extern "C" {
#endif

#ifndef DD_UTIL_H
#include "ddutil.h"
#endif

extern uint8 swModuleID;
/* Class reference definitions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
typedef struct _struct_swRoot{char val;} *swRoot;
#define swRootNull ((swRoot)0)
typedef struct _struct_swUser{char val;} *swUser;
#define swUserNull ((swUser)0)
typedef struct _struct_swMengine{char val;} *swMengine;
#define swMengineNull ((swMengine)0)
typedef struct _struct_swEngine{char val;} *swEngine;
#define swEngineNull ((swEngine)0)
typedef struct _struct_swVoice{char val;} *swVoice;
#define swVoiceNull ((swVoice)0)
typedef struct _struct_swQueue{char val;} *swQueue;
#define swQueueNull ((swQueue)0)
typedef struct _struct_swMessage{char val;} *swMessage;
#define swMessageNull ((swMessage)0)
typedef struct _struct_swLanguage{char val;} *swLanguage;
#define swLanguageNull ((swLanguage)0)
#else
typedef uint32 swRoot;
#define swRootNull 0
typedef uint32 swUser;
#define swUserNull 0
typedef uint32 swMengine;
#define swMengineNull 0
typedef uint32 swEngine;
#define swEngineNull 0
typedef uint32 swVoice;
#define swVoiceNull 0
typedef uint32 swQueue;
#define swQueueNull 0
typedef uint32 swMessage;
#define swMessageNull 0
typedef uint32 swLanguage;
#define swLanguageNull 0
#endif

/* MessageType enumerated type */
typedef enum {
    SW_SPEAK = 0,
    SW_MARK = 1,
    SW_SETSPEED = 2,
    SW_SETPITCH = 3,
    SW_SETVOLUME = 4,
    SW_AUDIO = 5
} swMessageType;

/* Constructor/Destructor hooks. */
typedef void (*swRootCallbackType)(swRoot);
extern swRootCallbackType swRootConstructorCallback;
extern swRootCallbackType swRootDestructorCallback;
typedef void (*swUserCallbackType)(swUser);
extern swUserCallbackType swUserConstructorCallback;
extern swUserCallbackType swUserDestructorCallback;
typedef void (*swMengineCallbackType)(swMengine);
extern swMengineCallbackType swMengineConstructorCallback;
extern swMengineCallbackType swMengineDestructorCallback;
typedef void (*swEngineCallbackType)(swEngine);
extern swEngineCallbackType swEngineConstructorCallback;
extern swEngineCallbackType swEngineDestructorCallback;
typedef void (*swVoiceCallbackType)(swVoice);
extern swVoiceCallbackType swVoiceConstructorCallback;
extern swVoiceCallbackType swVoiceDestructorCallback;
typedef void (*swQueueCallbackType)(swQueue);
extern swQueueCallbackType swQueueConstructorCallback;
extern swQueueCallbackType swQueueDestructorCallback;
typedef void (*swMessageCallbackType)(swMessage);
extern swMessageCallbackType swMessageConstructorCallback;
extern swMessageCallbackType swMessageDestructorCallback;
typedef void (*swLanguageCallbackType)(swLanguage);
extern swLanguageCallbackType swLanguageConstructorCallback;
extern swLanguageCallbackType swLanguageDestructorCallback;

/*----------------------------------------------------------------------------------------
  Root structure
----------------------------------------------------------------------------------------*/
struct swRootType_ {
    uint32 hash; /* This depends only on the structure of the database */
    swRoot firstFreeRoot;
    uint32 usedRoot, allocatedRoot;
    uint32 usedRootUserTable, allocatedRootUserTable, freeRootUserTable;
    uint32 usedRootLanguageTable, allocatedRootLanguageTable, freeRootLanguageTable;
    swUser firstFreeUser;
    uint32 usedUser, allocatedUser;
    swMengine firstFreeMengine;
    uint32 usedMengine, allocatedMengine;
    uint32 usedMengineVoiceTable, allocatedMengineVoiceTable, freeMengineVoiceTable;
    swEngine firstFreeEngine;
    uint32 usedEngine, allocatedEngine;
    swVoice firstFreeVoice;
    uint32 usedVoice, allocatedVoice;
    swQueue firstFreeQueue;
    uint32 usedQueue, allocatedQueue;
    swMessage firstFreeMessage;
    uint32 usedMessage, allocatedMessage;
    uint32 usedMessageData, allocatedMessageData, freeMessageData;
    swLanguage firstFreeLanguage;
    uint32 usedLanguage, allocatedLanguage;
};
extern struct swRootType_ swRootData;

utInlineC uint32 swHash(void) {return swRootData.hash;}
utInlineC swRoot swFirstFreeRoot(void) {return swRootData.firstFreeRoot;}
utInlineC void swSetFirstFreeRoot(swRoot value) {swRootData.firstFreeRoot = (value);}
utInlineC uint32 swUsedRoot(void) {return swRootData.usedRoot;}
utInlineC uint32 swAllocatedRoot(void) {return swRootData.allocatedRoot;}
utInlineC void swSetUsedRoot(uint32 value) {swRootData.usedRoot = value;}
utInlineC void swSetAllocatedRoot(uint32 value) {swRootData.allocatedRoot = value;}
utInlineC uint32 swUsedRootUserTable(void) {return swRootData.usedRootUserTable;}
utInlineC uint32 swAllocatedRootUserTable(void) {return swRootData.allocatedRootUserTable;}
utInlineC uint32 swFreeRootUserTable(void) {return swRootData.freeRootUserTable;}
utInlineC void swSetUsedRootUserTable(uint32 value) {swRootData.usedRootUserTable = value;}
utInlineC void swSetAllocatedRootUserTable(uint32 value) {swRootData.allocatedRootUserTable = value;}
utInlineC void swSetFreeRootUserTable(int32 value) {swRootData.freeRootUserTable = value;}
utInlineC uint32 swUsedRootLanguageTable(void) {return swRootData.usedRootLanguageTable;}
utInlineC uint32 swAllocatedRootLanguageTable(void) {return swRootData.allocatedRootLanguageTable;}
utInlineC uint32 swFreeRootLanguageTable(void) {return swRootData.freeRootLanguageTable;}
utInlineC void swSetUsedRootLanguageTable(uint32 value) {swRootData.usedRootLanguageTable = value;}
utInlineC void swSetAllocatedRootLanguageTable(uint32 value) {swRootData.allocatedRootLanguageTable = value;}
utInlineC void swSetFreeRootLanguageTable(int32 value) {swRootData.freeRootLanguageTable = value;}
utInlineC swUser swFirstFreeUser(void) {return swRootData.firstFreeUser;}
utInlineC void swSetFirstFreeUser(swUser value) {swRootData.firstFreeUser = (value);}
utInlineC uint32 swUsedUser(void) {return swRootData.usedUser;}
utInlineC uint32 swAllocatedUser(void) {return swRootData.allocatedUser;}
utInlineC void swSetUsedUser(uint32 value) {swRootData.usedUser = value;}
utInlineC void swSetAllocatedUser(uint32 value) {swRootData.allocatedUser = value;}
utInlineC swMengine swFirstFreeMengine(void) {return swRootData.firstFreeMengine;}
utInlineC void swSetFirstFreeMengine(swMengine value) {swRootData.firstFreeMengine = (value);}
utInlineC uint32 swUsedMengine(void) {return swRootData.usedMengine;}
utInlineC uint32 swAllocatedMengine(void) {return swRootData.allocatedMengine;}
utInlineC void swSetUsedMengine(uint32 value) {swRootData.usedMengine = value;}
utInlineC void swSetAllocatedMengine(uint32 value) {swRootData.allocatedMengine = value;}
utInlineC uint32 swUsedMengineVoiceTable(void) {return swRootData.usedMengineVoiceTable;}
utInlineC uint32 swAllocatedMengineVoiceTable(void) {return swRootData.allocatedMengineVoiceTable;}
utInlineC uint32 swFreeMengineVoiceTable(void) {return swRootData.freeMengineVoiceTable;}
utInlineC void swSetUsedMengineVoiceTable(uint32 value) {swRootData.usedMengineVoiceTable = value;}
utInlineC void swSetAllocatedMengineVoiceTable(uint32 value) {swRootData.allocatedMengineVoiceTable = value;}
utInlineC void swSetFreeMengineVoiceTable(int32 value) {swRootData.freeMengineVoiceTable = value;}
utInlineC swEngine swFirstFreeEngine(void) {return swRootData.firstFreeEngine;}
utInlineC void swSetFirstFreeEngine(swEngine value) {swRootData.firstFreeEngine = (value);}
utInlineC uint32 swUsedEngine(void) {return swRootData.usedEngine;}
utInlineC uint32 swAllocatedEngine(void) {return swRootData.allocatedEngine;}
utInlineC void swSetUsedEngine(uint32 value) {swRootData.usedEngine = value;}
utInlineC void swSetAllocatedEngine(uint32 value) {swRootData.allocatedEngine = value;}
utInlineC swVoice swFirstFreeVoice(void) {return swRootData.firstFreeVoice;}
utInlineC void swSetFirstFreeVoice(swVoice value) {swRootData.firstFreeVoice = (value);}
utInlineC uint32 swUsedVoice(void) {return swRootData.usedVoice;}
utInlineC uint32 swAllocatedVoice(void) {return swRootData.allocatedVoice;}
utInlineC void swSetUsedVoice(uint32 value) {swRootData.usedVoice = value;}
utInlineC void swSetAllocatedVoice(uint32 value) {swRootData.allocatedVoice = value;}
utInlineC swQueue swFirstFreeQueue(void) {return swRootData.firstFreeQueue;}
utInlineC void swSetFirstFreeQueue(swQueue value) {swRootData.firstFreeQueue = (value);}
utInlineC uint32 swUsedQueue(void) {return swRootData.usedQueue;}
utInlineC uint32 swAllocatedQueue(void) {return swRootData.allocatedQueue;}
utInlineC void swSetUsedQueue(uint32 value) {swRootData.usedQueue = value;}
utInlineC void swSetAllocatedQueue(uint32 value) {swRootData.allocatedQueue = value;}
utInlineC swMessage swFirstFreeMessage(void) {return swRootData.firstFreeMessage;}
utInlineC void swSetFirstFreeMessage(swMessage value) {swRootData.firstFreeMessage = (value);}
utInlineC uint32 swUsedMessage(void) {return swRootData.usedMessage;}
utInlineC uint32 swAllocatedMessage(void) {return swRootData.allocatedMessage;}
utInlineC void swSetUsedMessage(uint32 value) {swRootData.usedMessage = value;}
utInlineC void swSetAllocatedMessage(uint32 value) {swRootData.allocatedMessage = value;}
utInlineC uint32 swUsedMessageData(void) {return swRootData.usedMessageData;}
utInlineC uint32 swAllocatedMessageData(void) {return swRootData.allocatedMessageData;}
utInlineC uint32 swFreeMessageData(void) {return swRootData.freeMessageData;}
utInlineC void swSetUsedMessageData(uint32 value) {swRootData.usedMessageData = value;}
utInlineC void swSetAllocatedMessageData(uint32 value) {swRootData.allocatedMessageData = value;}
utInlineC void swSetFreeMessageData(int32 value) {swRootData.freeMessageData = value;}
utInlineC swLanguage swFirstFreeLanguage(void) {return swRootData.firstFreeLanguage;}
utInlineC void swSetFirstFreeLanguage(swLanguage value) {swRootData.firstFreeLanguage = (value);}
utInlineC uint32 swUsedLanguage(void) {return swRootData.usedLanguage;}
utInlineC uint32 swAllocatedLanguage(void) {return swRootData.allocatedLanguage;}
utInlineC void swSetUsedLanguage(uint32 value) {swRootData.usedLanguage = value;}
utInlineC void swSetAllocatedLanguage(uint32 value) {swRootData.allocatedLanguage = value;}

/* Validate macros */
#if defined(DD_DEBUG)
utInlineC swRoot swValidRoot(swRoot Root) {
    utAssert(utLikely(Root != swRootNull && (uint32)(Root - (swRoot)0) < swRootData.usedRoot));
    return Root;}
utInlineC swUser swValidUser(swUser User) {
    utAssert(utLikely(User != swUserNull && (uint32)(User - (swUser)0) < swRootData.usedUser));
    return User;}
utInlineC swMengine swValidMengine(swMengine Mengine) {
    utAssert(utLikely(Mengine != swMengineNull && (uint32)(Mengine - (swMengine)0) < swRootData.usedMengine));
    return Mengine;}
utInlineC swEngine swValidEngine(swEngine Engine) {
    utAssert(utLikely(Engine != swEngineNull && (uint32)(Engine - (swEngine)0) < swRootData.usedEngine));
    return Engine;}
utInlineC swVoice swValidVoice(swVoice Voice) {
    utAssert(utLikely(Voice != swVoiceNull && (uint32)(Voice - (swVoice)0) < swRootData.usedVoice));
    return Voice;}
utInlineC swQueue swValidQueue(swQueue Queue) {
    utAssert(utLikely(Queue != swQueueNull && (uint32)(Queue - (swQueue)0) < swRootData.usedQueue));
    return Queue;}
utInlineC swMessage swValidMessage(swMessage Message) {
    utAssert(utLikely(Message != swMessageNull && (uint32)(Message - (swMessage)0) < swRootData.usedMessage));
    return Message;}
utInlineC swLanguage swValidLanguage(swLanguage Language) {
    utAssert(utLikely(Language != swLanguageNull && (uint32)(Language - (swLanguage)0) < swRootData.usedLanguage));
    return Language;}
#else
utInlineC swRoot swValidRoot(swRoot Root) {return Root;}
utInlineC swUser swValidUser(swUser User) {return User;}
utInlineC swMengine swValidMengine(swMengine Mengine) {return Mengine;}
utInlineC swEngine swValidEngine(swEngine Engine) {return Engine;}
utInlineC swVoice swValidVoice(swVoice Voice) {return Voice;}
utInlineC swQueue swValidQueue(swQueue Queue) {return Queue;}
utInlineC swMessage swValidMessage(swMessage Message) {return Message;}
utInlineC swLanguage swValidLanguage(swLanguage Language) {return Language;}
#endif

/* Object ref to integer conversions */
#if (defined(DD_DEBUG) && !defined(DD_NOSTRICT)) || defined(DD_STRICT)
utInlineC uint32 swRoot2Index(swRoot Root) {return Root - (swRoot)0;}
utInlineC uint32 swRoot2ValidIndex(swRoot Root) {return swValidRoot(Root) - (swRoot)0;}
utInlineC swRoot swIndex2Root(uint32 xRoot) {return (swRoot)(xRoot + (swRoot)(0));}
utInlineC uint32 swUser2Index(swUser User) {return User - (swUser)0;}
utInlineC uint32 swUser2ValidIndex(swUser User) {return swValidUser(User) - (swUser)0;}
utInlineC swUser swIndex2User(uint32 xUser) {return (swUser)(xUser + (swUser)(0));}
utInlineC uint32 swMengine2Index(swMengine Mengine) {return Mengine - (swMengine)0;}
utInlineC uint32 swMengine2ValidIndex(swMengine Mengine) {return swValidMengine(Mengine) - (swMengine)0;}
utInlineC swMengine swIndex2Mengine(uint32 xMengine) {return (swMengine)(xMengine + (swMengine)(0));}
utInlineC uint32 swEngine2Index(swEngine Engine) {return Engine - (swEngine)0;}
utInlineC uint32 swEngine2ValidIndex(swEngine Engine) {return swValidEngine(Engine) - (swEngine)0;}
utInlineC swEngine swIndex2Engine(uint32 xEngine) {return (swEngine)(xEngine + (swEngine)(0));}
utInlineC uint32 swVoice2Index(swVoice Voice) {return Voice - (swVoice)0;}
utInlineC uint32 swVoice2ValidIndex(swVoice Voice) {return swValidVoice(Voice) - (swVoice)0;}
utInlineC swVoice swIndex2Voice(uint32 xVoice) {return (swVoice)(xVoice + (swVoice)(0));}
utInlineC uint32 swQueue2Index(swQueue Queue) {return Queue - (swQueue)0;}
utInlineC uint32 swQueue2ValidIndex(swQueue Queue) {return swValidQueue(Queue) - (swQueue)0;}
utInlineC swQueue swIndex2Queue(uint32 xQueue) {return (swQueue)(xQueue + (swQueue)(0));}
utInlineC uint32 swMessage2Index(swMessage Message) {return Message - (swMessage)0;}
utInlineC uint32 swMessage2ValidIndex(swMessage Message) {return swValidMessage(Message) - (swMessage)0;}
utInlineC swMessage swIndex2Message(uint32 xMessage) {return (swMessage)(xMessage + (swMessage)(0));}
utInlineC uint32 swLanguage2Index(swLanguage Language) {return Language - (swLanguage)0;}
utInlineC uint32 swLanguage2ValidIndex(swLanguage Language) {return swValidLanguage(Language) - (swLanguage)0;}
utInlineC swLanguage swIndex2Language(uint32 xLanguage) {return (swLanguage)(xLanguage + (swLanguage)(0));}
#else
utInlineC uint32 swRoot2Index(swRoot Root) {return Root;}
utInlineC uint32 swRoot2ValidIndex(swRoot Root) {return swValidRoot(Root);}
utInlineC swRoot swIndex2Root(uint32 xRoot) {return xRoot;}
utInlineC uint32 swUser2Index(swUser User) {return User;}
utInlineC uint32 swUser2ValidIndex(swUser User) {return swValidUser(User);}
utInlineC swUser swIndex2User(uint32 xUser) {return xUser;}
utInlineC uint32 swMengine2Index(swMengine Mengine) {return Mengine;}
utInlineC uint32 swMengine2ValidIndex(swMengine Mengine) {return swValidMengine(Mengine);}
utInlineC swMengine swIndex2Mengine(uint32 xMengine) {return xMengine;}
utInlineC uint32 swEngine2Index(swEngine Engine) {return Engine;}
utInlineC uint32 swEngine2ValidIndex(swEngine Engine) {return swValidEngine(Engine);}
utInlineC swEngine swIndex2Engine(uint32 xEngine) {return xEngine;}
utInlineC uint32 swVoice2Index(swVoice Voice) {return Voice;}
utInlineC uint32 swVoice2ValidIndex(swVoice Voice) {return swValidVoice(Voice);}
utInlineC swVoice swIndex2Voice(uint32 xVoice) {return xVoice;}
utInlineC uint32 swQueue2Index(swQueue Queue) {return Queue;}
utInlineC uint32 swQueue2ValidIndex(swQueue Queue) {return swValidQueue(Queue);}
utInlineC swQueue swIndex2Queue(uint32 xQueue) {return xQueue;}
utInlineC uint32 swMessage2Index(swMessage Message) {return Message;}
utInlineC uint32 swMessage2ValidIndex(swMessage Message) {return swValidMessage(Message);}
utInlineC swMessage swIndex2Message(uint32 xMessage) {return xMessage;}
utInlineC uint32 swLanguage2Index(swLanguage Language) {return Language;}
utInlineC uint32 swLanguage2ValidIndex(swLanguage Language) {return swValidLanguage(Language);}
utInlineC swLanguage swIndex2Language(uint32 xLanguage) {return xLanguage;}
#endif

/*----------------------------------------------------------------------------------------
  Fields for class Root.
----------------------------------------------------------------------------------------*/
struct swRootFields {
    swUser *FirstUser;
    swUser *LastUser;
    uint32 *UserTableIndex_;
    uint32 *NumUserTable;
    swUser *UserTable;
    uint32 *NumUser;
    swEngine *FirstEngine;
    swEngine *LastEngine;
    swQueue *InQueue;
    swQueue *OutQueue;
    swQueue *AudioQueue;
    swLanguage *FirstLanguage;
    swLanguage *LastLanguage;
    uint32 *LanguageTableIndex_;
    uint32 *NumLanguageTable;
    swLanguage *LanguageTable;
    uint32 *NumLanguage;
};
extern struct swRootFields swRoots;

void swRootAllocMore(void);
void swRootCopyProps(swRoot swOldRoot, swRoot swNewRoot);
void swRootAllocUserTables(swRoot Root, uint32 numUserTables);
void swRootResizeUserTables(swRoot Root, uint32 numUserTables);
void swRootFreeUserTables(swRoot Root);
void swCompactRootUserTables(void);
void swRootAllocLanguageTables(swRoot Root, uint32 numLanguageTables);
void swRootResizeLanguageTables(swRoot Root, uint32 numLanguageTables);
void swRootFreeLanguageTables(swRoot Root);
void swCompactRootLanguageTables(void);
utInlineC swUser swRootGetFirstUser(swRoot Root) {return swRoots.FirstUser[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetFirstUser(swRoot Root, swUser value) {swRoots.FirstUser[swRoot2ValidIndex(Root)] = value;}
utInlineC swUser swRootGetLastUser(swRoot Root) {return swRoots.LastUser[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetLastUser(swRoot Root, swUser value) {swRoots.LastUser[swRoot2ValidIndex(Root)] = value;}
utInlineC uint32 swRootGetUserTableIndex_(swRoot Root) {return swRoots.UserTableIndex_[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetUserTableIndex_(swRoot Root, uint32 value) {swRoots.UserTableIndex_[swRoot2ValidIndex(Root)] = value;}
utInlineC uint32 swRootGetNumUserTable(swRoot Root) {return swRoots.NumUserTable[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetNumUserTable(swRoot Root, uint32 value) {swRoots.NumUserTable[swRoot2ValidIndex(Root)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 swRootCheckUserTableIndex(swRoot Root, uint32 x) {utAssert(x < swRootGetNumUserTable(Root)); return x;}
#else
utInlineC uint32 swRootCheckUserTableIndex(swRoot Root, uint32 x) {return x;}
#endif
utInlineC swUser swRootGetiUserTable(swRoot Root, uint32 x) {return swRoots.UserTable[
    swRootGetUserTableIndex_(Root) + swRootCheckUserTableIndex(Root, x)];}
utInlineC swUser *swRootGetUserTable(swRoot Root) {return swRoots.UserTable + swRootGetUserTableIndex_(Root);}
#define swRootGetUserTables swRootGetUserTable
utInlineC void swRootSetUserTable(swRoot Root, swUser *valuePtr, uint32 numUserTable) {
    swRootResizeUserTables(Root, numUserTable);
    memcpy(swRootGetUserTables(Root), valuePtr, numUserTable*sizeof(swUser));}
utInlineC void swRootSetiUserTable(swRoot Root, uint32 x, swUser value) {
    swRoots.UserTable[swRootGetUserTableIndex_(Root) + swRootCheckUserTableIndex(Root, (x))] = value;}
utInlineC uint32 swRootGetNumUser(swRoot Root) {return swRoots.NumUser[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetNumUser(swRoot Root, uint32 value) {swRoots.NumUser[swRoot2ValidIndex(Root)] = value;}
utInlineC swEngine swRootGetFirstEngine(swRoot Root) {return swRoots.FirstEngine[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetFirstEngine(swRoot Root, swEngine value) {swRoots.FirstEngine[swRoot2ValidIndex(Root)] = value;}
utInlineC swEngine swRootGetLastEngine(swRoot Root) {return swRoots.LastEngine[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetLastEngine(swRoot Root, swEngine value) {swRoots.LastEngine[swRoot2ValidIndex(Root)] = value;}
utInlineC swQueue swRootGetInQueue(swRoot Root) {return swRoots.InQueue[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetInQueue(swRoot Root, swQueue value) {swRoots.InQueue[swRoot2ValidIndex(Root)] = value;}
utInlineC swQueue swRootGetOutQueue(swRoot Root) {return swRoots.OutQueue[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetOutQueue(swRoot Root, swQueue value) {swRoots.OutQueue[swRoot2ValidIndex(Root)] = value;}
utInlineC swQueue swRootGetAudioQueue(swRoot Root) {return swRoots.AudioQueue[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetAudioQueue(swRoot Root, swQueue value) {swRoots.AudioQueue[swRoot2ValidIndex(Root)] = value;}
utInlineC swLanguage swRootGetFirstLanguage(swRoot Root) {return swRoots.FirstLanguage[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetFirstLanguage(swRoot Root, swLanguage value) {swRoots.FirstLanguage[swRoot2ValidIndex(Root)] = value;}
utInlineC swLanguage swRootGetLastLanguage(swRoot Root) {return swRoots.LastLanguage[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetLastLanguage(swRoot Root, swLanguage value) {swRoots.LastLanguage[swRoot2ValidIndex(Root)] = value;}
utInlineC uint32 swRootGetLanguageTableIndex_(swRoot Root) {return swRoots.LanguageTableIndex_[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetLanguageTableIndex_(swRoot Root, uint32 value) {swRoots.LanguageTableIndex_[swRoot2ValidIndex(Root)] = value;}
utInlineC uint32 swRootGetNumLanguageTable(swRoot Root) {return swRoots.NumLanguageTable[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetNumLanguageTable(swRoot Root, uint32 value) {swRoots.NumLanguageTable[swRoot2ValidIndex(Root)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 swRootCheckLanguageTableIndex(swRoot Root, uint32 x) {utAssert(x < swRootGetNumLanguageTable(Root)); return x;}
#else
utInlineC uint32 swRootCheckLanguageTableIndex(swRoot Root, uint32 x) {return x;}
#endif
utInlineC swLanguage swRootGetiLanguageTable(swRoot Root, uint32 x) {return swRoots.LanguageTable[
    swRootGetLanguageTableIndex_(Root) + swRootCheckLanguageTableIndex(Root, x)];}
utInlineC swLanguage *swRootGetLanguageTable(swRoot Root) {return swRoots.LanguageTable + swRootGetLanguageTableIndex_(Root);}
#define swRootGetLanguageTables swRootGetLanguageTable
utInlineC void swRootSetLanguageTable(swRoot Root, swLanguage *valuePtr, uint32 numLanguageTable) {
    swRootResizeLanguageTables(Root, numLanguageTable);
    memcpy(swRootGetLanguageTables(Root), valuePtr, numLanguageTable*sizeof(swLanguage));}
utInlineC void swRootSetiLanguageTable(swRoot Root, uint32 x, swLanguage value) {
    swRoots.LanguageTable[swRootGetLanguageTableIndex_(Root) + swRootCheckLanguageTableIndex(Root, (x))] = value;}
utInlineC uint32 swRootGetNumLanguage(swRoot Root) {return swRoots.NumLanguage[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetNumLanguage(swRoot Root, uint32 value) {swRoots.NumLanguage[swRoot2ValidIndex(Root)] = value;}
utInlineC void swRootSetConstructorCallback(void(*func)(swRoot)) {swRootConstructorCallback = func;}
utInlineC swRootCallbackType swRootGetConstructorCallback(void) {return swRootConstructorCallback;}
utInlineC void swRootSetDestructorCallback(void(*func)(swRoot)) {swRootDestructorCallback = func;}
utInlineC swRootCallbackType swRootGetDestructorCallback(void) {return swRootDestructorCallback;}
utInlineC swRoot swRootNextFree(swRoot Root) {return ((swRoot *)(void *)(swRoots.FirstUser))[swRoot2ValidIndex(Root)];}
utInlineC void swRootSetNextFree(swRoot Root, swRoot value) {
    ((swRoot *)(void *)(swRoots.FirstUser))[swRoot2ValidIndex(Root)] = value;}
utInlineC void swRootFree(swRoot Root) {
    swRootFreeUserTables(Root);
    swRootFreeLanguageTables(Root);
    swRootSetNextFree(Root, swRootData.firstFreeRoot);
    swSetFirstFreeRoot(Root);}
void swRootDestroy(swRoot Root);
utInlineC swRoot swRootAllocRaw(void) {
    swRoot Root;
    if(swRootData.firstFreeRoot != swRootNull) {
        Root = swRootData.firstFreeRoot;
        swSetFirstFreeRoot(swRootNextFree(Root));
    } else {
        if(swRootData.usedRoot == swRootData.allocatedRoot) {
            swRootAllocMore();
        }
        Root = swIndex2Root(swRootData.usedRoot);
        swSetUsedRoot(swUsedRoot() + 1);
    }
    return Root;}
utInlineC swRoot swRootAlloc(void) {
    swRoot Root = swRootAllocRaw();
    swRootSetFirstUser(Root, swUserNull);
    swRootSetLastUser(Root, swUserNull);
    swRootSetUserTableIndex_(Root, 0);
    swRootSetNumUserTable(Root, 0);
    swRootSetNumUserTable(Root, 0);
    swRootSetNumUser(Root, 0);
    swRootSetFirstEngine(Root, swEngineNull);
    swRootSetLastEngine(Root, swEngineNull);
    swRootSetInQueue(Root, swQueueNull);
    swRootSetOutQueue(Root, swQueueNull);
    swRootSetAudioQueue(Root, swQueueNull);
    swRootSetFirstLanguage(Root, swLanguageNull);
    swRootSetLastLanguage(Root, swLanguageNull);
    swRootSetLanguageTableIndex_(Root, 0);
    swRootSetNumLanguageTable(Root, 0);
    swRootSetNumLanguageTable(Root, 0);
    swRootSetNumLanguage(Root, 0);
    if(swRootConstructorCallback != NULL) {
        swRootConstructorCallback(Root);
    }
    return Root;}

/*----------------------------------------------------------------------------------------
  Fields for class User.
----------------------------------------------------------------------------------------*/
struct swUserFields {
    uint32 *Id;
    swRoot *Root;
    swUser *NextRootUser;
    swUser *PrevRootUser;
    swUser *NextTableRootUser;
    swEngine *Engine;
    swMessage *FirstMessage;
    swMessage *LastMessage;
};
extern struct swUserFields swUsers;

void swUserAllocMore(void);
void swUserCopyProps(swUser swOldUser, swUser swNewUser);
utInlineC uint32 swUserGetId(swUser User) {return swUsers.Id[swUser2ValidIndex(User)];}
utInlineC void swUserSetId(swUser User, uint32 value) {swUsers.Id[swUser2ValidIndex(User)] = value;}
utInlineC swRoot swUserGetRoot(swUser User) {return swUsers.Root[swUser2ValidIndex(User)];}
utInlineC void swUserSetRoot(swUser User, swRoot value) {swUsers.Root[swUser2ValidIndex(User)] = value;}
utInlineC swUser swUserGetNextRootUser(swUser User) {return swUsers.NextRootUser[swUser2ValidIndex(User)];}
utInlineC void swUserSetNextRootUser(swUser User, swUser value) {swUsers.NextRootUser[swUser2ValidIndex(User)] = value;}
utInlineC swUser swUserGetPrevRootUser(swUser User) {return swUsers.PrevRootUser[swUser2ValidIndex(User)];}
utInlineC void swUserSetPrevRootUser(swUser User, swUser value) {swUsers.PrevRootUser[swUser2ValidIndex(User)] = value;}
utInlineC swUser swUserGetNextTableRootUser(swUser User) {return swUsers.NextTableRootUser[swUser2ValidIndex(User)];}
utInlineC void swUserSetNextTableRootUser(swUser User, swUser value) {swUsers.NextTableRootUser[swUser2ValidIndex(User)] = value;}
utInlineC swEngine swUserGetEngine(swUser User) {return swUsers.Engine[swUser2ValidIndex(User)];}
utInlineC void swUserSetEngine(swUser User, swEngine value) {swUsers.Engine[swUser2ValidIndex(User)] = value;}
utInlineC swMessage swUserGetFirstMessage(swUser User) {return swUsers.FirstMessage[swUser2ValidIndex(User)];}
utInlineC void swUserSetFirstMessage(swUser User, swMessage value) {swUsers.FirstMessage[swUser2ValidIndex(User)] = value;}
utInlineC swMessage swUserGetLastMessage(swUser User) {return swUsers.LastMessage[swUser2ValidIndex(User)];}
utInlineC void swUserSetLastMessage(swUser User, swMessage value) {swUsers.LastMessage[swUser2ValidIndex(User)] = value;}
utInlineC void swUserSetConstructorCallback(void(*func)(swUser)) {swUserConstructorCallback = func;}
utInlineC swUserCallbackType swUserGetConstructorCallback(void) {return swUserConstructorCallback;}
utInlineC void swUserSetDestructorCallback(void(*func)(swUser)) {swUserDestructorCallback = func;}
utInlineC swUserCallbackType swUserGetDestructorCallback(void) {return swUserDestructorCallback;}
utInlineC swUser swUserNextFree(swUser User) {return ((swUser *)(void *)(swUsers.Root))[swUser2ValidIndex(User)];}
utInlineC void swUserSetNextFree(swUser User, swUser value) {
    ((swUser *)(void *)(swUsers.Root))[swUser2ValidIndex(User)] = value;}
utInlineC void swUserFree(swUser User) {
    swUserSetNextFree(User, swRootData.firstFreeUser);
    swSetFirstFreeUser(User);}
void swUserDestroy(swUser User);
utInlineC swUser swUserAllocRaw(void) {
    swUser User;
    if(swRootData.firstFreeUser != swUserNull) {
        User = swRootData.firstFreeUser;
        swSetFirstFreeUser(swUserNextFree(User));
    } else {
        if(swRootData.usedUser == swRootData.allocatedUser) {
            swUserAllocMore();
        }
        User = swIndex2User(swRootData.usedUser);
        swSetUsedUser(swUsedUser() + 1);
    }
    return User;}
utInlineC swUser swUserAlloc(void) {
    swUser User = swUserAllocRaw();
    swUserSetId(User, 0);
    swUserSetRoot(User, swRootNull);
    swUserSetNextRootUser(User, swUserNull);
    swUserSetPrevRootUser(User, swUserNull);
    swUserSetNextTableRootUser(User, swUserNull);
    swUserSetEngine(User, swEngineNull);
    swUserSetFirstMessage(User, swMessageNull);
    swUserSetLastMessage(User, swMessageNull);
    if(swUserConstructorCallback != NULL) {
        swUserConstructorCallback(User);
    }
    return User;}

/*----------------------------------------------------------------------------------------
  Fields for class Mengine.
----------------------------------------------------------------------------------------*/
struct swMengineFields {
    swEngine *FirstEngine;
    swEngine *LastEngine;
    swVoice *FirstVoice;
    swVoice *LastVoice;
    uint32 *VoiceTableIndex_;
    uint32 *NumVoiceTable;
    swVoice *VoiceTable;
    uint32 *NumVoice;
};
extern struct swMengineFields swMengines;

void swMengineAllocMore(void);
void swMengineCopyProps(swMengine swOldMengine, swMengine swNewMengine);
void swMengineAllocVoiceTables(swMengine Mengine, uint32 numVoiceTables);
void swMengineResizeVoiceTables(swMengine Mengine, uint32 numVoiceTables);
void swMengineFreeVoiceTables(swMengine Mengine);
void swCompactMengineVoiceTables(void);
utInlineC swEngine swMengineGetFirstEngine(swMengine Mengine) {return swMengines.FirstEngine[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetFirstEngine(swMengine Mengine, swEngine value) {swMengines.FirstEngine[swMengine2ValidIndex(Mengine)] = value;}
utInlineC swEngine swMengineGetLastEngine(swMengine Mengine) {return swMengines.LastEngine[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetLastEngine(swMengine Mengine, swEngine value) {swMengines.LastEngine[swMengine2ValidIndex(Mengine)] = value;}
utInlineC swVoice swMengineGetFirstVoice(swMengine Mengine) {return swMengines.FirstVoice[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetFirstVoice(swMengine Mengine, swVoice value) {swMengines.FirstVoice[swMengine2ValidIndex(Mengine)] = value;}
utInlineC swVoice swMengineGetLastVoice(swMengine Mengine) {return swMengines.LastVoice[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetLastVoice(swMengine Mengine, swVoice value) {swMengines.LastVoice[swMengine2ValidIndex(Mengine)] = value;}
utInlineC uint32 swMengineGetVoiceTableIndex_(swMengine Mengine) {return swMengines.VoiceTableIndex_[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetVoiceTableIndex_(swMengine Mengine, uint32 value) {swMengines.VoiceTableIndex_[swMengine2ValidIndex(Mengine)] = value;}
utInlineC uint32 swMengineGetNumVoiceTable(swMengine Mengine) {return swMengines.NumVoiceTable[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetNumVoiceTable(swMengine Mengine, uint32 value) {swMengines.NumVoiceTable[swMengine2ValidIndex(Mengine)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 swMengineCheckVoiceTableIndex(swMengine Mengine, uint32 x) {utAssert(x < swMengineGetNumVoiceTable(Mengine)); return x;}
#else
utInlineC uint32 swMengineCheckVoiceTableIndex(swMengine Mengine, uint32 x) {return x;}
#endif
utInlineC swVoice swMengineGetiVoiceTable(swMengine Mengine, uint32 x) {return swMengines.VoiceTable[
    swMengineGetVoiceTableIndex_(Mengine) + swMengineCheckVoiceTableIndex(Mengine, x)];}
utInlineC swVoice *swMengineGetVoiceTable(swMengine Mengine) {return swMengines.VoiceTable + swMengineGetVoiceTableIndex_(Mengine);}
#define swMengineGetVoiceTables swMengineGetVoiceTable
utInlineC void swMengineSetVoiceTable(swMengine Mengine, swVoice *valuePtr, uint32 numVoiceTable) {
    swMengineResizeVoiceTables(Mengine, numVoiceTable);
    memcpy(swMengineGetVoiceTables(Mengine), valuePtr, numVoiceTable*sizeof(swVoice));}
utInlineC void swMengineSetiVoiceTable(swMengine Mengine, uint32 x, swVoice value) {
    swMengines.VoiceTable[swMengineGetVoiceTableIndex_(Mengine) + swMengineCheckVoiceTableIndex(Mengine, (x))] = value;}
utInlineC uint32 swMengineGetNumVoice(swMengine Mengine) {return swMengines.NumVoice[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetNumVoice(swMengine Mengine, uint32 value) {swMengines.NumVoice[swMengine2ValidIndex(Mengine)] = value;}
utInlineC void swMengineSetConstructorCallback(void(*func)(swMengine)) {swMengineConstructorCallback = func;}
utInlineC swMengineCallbackType swMengineGetConstructorCallback(void) {return swMengineConstructorCallback;}
utInlineC void swMengineSetDestructorCallback(void(*func)(swMengine)) {swMengineDestructorCallback = func;}
utInlineC swMengineCallbackType swMengineGetDestructorCallback(void) {return swMengineDestructorCallback;}
utInlineC swMengine swMengineNextFree(swMengine Mengine) {return ((swMengine *)(void *)(swMengines.FirstEngine))[swMengine2ValidIndex(Mengine)];}
utInlineC void swMengineSetNextFree(swMengine Mengine, swMengine value) {
    ((swMengine *)(void *)(swMengines.FirstEngine))[swMengine2ValidIndex(Mengine)] = value;}
utInlineC void swMengineFree(swMengine Mengine) {
    swMengineFreeVoiceTables(Mengine);
    swMengineSetNextFree(Mengine, swRootData.firstFreeMengine);
    swSetFirstFreeMengine(Mengine);}
void swMengineDestroy(swMengine Mengine);
utInlineC swMengine swMengineAllocRaw(void) {
    swMengine Mengine;
    if(swRootData.firstFreeMengine != swMengineNull) {
        Mengine = swRootData.firstFreeMengine;
        swSetFirstFreeMengine(swMengineNextFree(Mengine));
    } else {
        if(swRootData.usedMengine == swRootData.allocatedMengine) {
            swMengineAllocMore();
        }
        Mengine = swIndex2Mengine(swRootData.usedMengine);
        swSetUsedMengine(swUsedMengine() + 1);
    }
    return Mengine;}
utInlineC swMengine swMengineAlloc(void) {
    swMengine Mengine = swMengineAllocRaw();
    swMengineSetFirstEngine(Mengine, swEngineNull);
    swMengineSetLastEngine(Mengine, swEngineNull);
    swMengineSetFirstVoice(Mengine, swVoiceNull);
    swMengineSetLastVoice(Mengine, swVoiceNull);
    swMengineSetVoiceTableIndex_(Mengine, 0);
    swMengineSetNumVoiceTable(Mengine, 0);
    swMengineSetNumVoiceTable(Mengine, 0);
    swMengineSetNumVoice(Mengine, 0);
    if(swMengineConstructorCallback != NULL) {
        swMengineConstructorCallback(Mengine);
    }
    return Mengine;}

/*----------------------------------------------------------------------------------------
  Fields for class Engine.
----------------------------------------------------------------------------------------*/
struct swEngineFields {
    swVoice *Voice;
    swLanguage *Langauge;
    float *Pitch;
    float *Speed;
    float *Volume;
    swRoot *Root;
    swEngine *NextRootEngine;
    swEngine *PrevRootEngine;
    swUser *User;
    swMengine *Mengine;
    swEngine *NextMengineEngine;
    swEngine *PrevMengineEngine;
};
extern struct swEngineFields swEngines;

void swEngineAllocMore(void);
void swEngineCopyProps(swEngine swOldEngine, swEngine swNewEngine);
utInlineC swVoice swEngineGetVoice(swEngine Engine) {return swEngines.Voice[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetVoice(swEngine Engine, swVoice value) {swEngines.Voice[swEngine2ValidIndex(Engine)] = value;}
utInlineC swLanguage swEngineGetLangauge(swEngine Engine) {return swEngines.Langauge[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetLangauge(swEngine Engine, swLanguage value) {swEngines.Langauge[swEngine2ValidIndex(Engine)] = value;}
utInlineC float swEngineGetPitch(swEngine Engine) {return swEngines.Pitch[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetPitch(swEngine Engine, float value) {swEngines.Pitch[swEngine2ValidIndex(Engine)] = value;}
utInlineC float swEngineGetSpeed(swEngine Engine) {return swEngines.Speed[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetSpeed(swEngine Engine, float value) {swEngines.Speed[swEngine2ValidIndex(Engine)] = value;}
utInlineC float swEngineGetVolume(swEngine Engine) {return swEngines.Volume[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetVolume(swEngine Engine, float value) {swEngines.Volume[swEngine2ValidIndex(Engine)] = value;}
utInlineC swRoot swEngineGetRoot(swEngine Engine) {return swEngines.Root[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetRoot(swEngine Engine, swRoot value) {swEngines.Root[swEngine2ValidIndex(Engine)] = value;}
utInlineC swEngine swEngineGetNextRootEngine(swEngine Engine) {return swEngines.NextRootEngine[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetNextRootEngine(swEngine Engine, swEngine value) {swEngines.NextRootEngine[swEngine2ValidIndex(Engine)] = value;}
utInlineC swEngine swEngineGetPrevRootEngine(swEngine Engine) {return swEngines.PrevRootEngine[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetPrevRootEngine(swEngine Engine, swEngine value) {swEngines.PrevRootEngine[swEngine2ValidIndex(Engine)] = value;}
utInlineC swUser swEngineGetUser(swEngine Engine) {return swEngines.User[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetUser(swEngine Engine, swUser value) {swEngines.User[swEngine2ValidIndex(Engine)] = value;}
utInlineC swMengine swEngineGetMengine(swEngine Engine) {return swEngines.Mengine[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetMengine(swEngine Engine, swMengine value) {swEngines.Mengine[swEngine2ValidIndex(Engine)] = value;}
utInlineC swEngine swEngineGetNextMengineEngine(swEngine Engine) {return swEngines.NextMengineEngine[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetNextMengineEngine(swEngine Engine, swEngine value) {swEngines.NextMengineEngine[swEngine2ValidIndex(Engine)] = value;}
utInlineC swEngine swEngineGetPrevMengineEngine(swEngine Engine) {return swEngines.PrevMengineEngine[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetPrevMengineEngine(swEngine Engine, swEngine value) {swEngines.PrevMengineEngine[swEngine2ValidIndex(Engine)] = value;}
utInlineC void swEngineSetConstructorCallback(void(*func)(swEngine)) {swEngineConstructorCallback = func;}
utInlineC swEngineCallbackType swEngineGetConstructorCallback(void) {return swEngineConstructorCallback;}
utInlineC void swEngineSetDestructorCallback(void(*func)(swEngine)) {swEngineDestructorCallback = func;}
utInlineC swEngineCallbackType swEngineGetDestructorCallback(void) {return swEngineDestructorCallback;}
utInlineC swEngine swEngineNextFree(swEngine Engine) {return ((swEngine *)(void *)(swEngines.Voice))[swEngine2ValidIndex(Engine)];}
utInlineC void swEngineSetNextFree(swEngine Engine, swEngine value) {
    ((swEngine *)(void *)(swEngines.Voice))[swEngine2ValidIndex(Engine)] = value;}
utInlineC void swEngineFree(swEngine Engine) {
    swEngineSetNextFree(Engine, swRootData.firstFreeEngine);
    swSetFirstFreeEngine(Engine);}
void swEngineDestroy(swEngine Engine);
utInlineC swEngine swEngineAllocRaw(void) {
    swEngine Engine;
    if(swRootData.firstFreeEngine != swEngineNull) {
        Engine = swRootData.firstFreeEngine;
        swSetFirstFreeEngine(swEngineNextFree(Engine));
    } else {
        if(swRootData.usedEngine == swRootData.allocatedEngine) {
            swEngineAllocMore();
        }
        Engine = swIndex2Engine(swRootData.usedEngine);
        swSetUsedEngine(swUsedEngine() + 1);
    }
    return Engine;}
utInlineC swEngine swEngineAlloc(void) {
    swEngine Engine = swEngineAllocRaw();
    swEngineSetVoice(Engine, swVoiceNull);
    swEngineSetLangauge(Engine, swLanguageNull);
    swEngineSetPitch(Engine, 0);
    swEngineSetSpeed(Engine, 0);
    swEngineSetVolume(Engine, 0);
    swEngineSetRoot(Engine, swRootNull);
    swEngineSetNextRootEngine(Engine, swEngineNull);
    swEngineSetPrevRootEngine(Engine, swEngineNull);
    swEngineSetUser(Engine, swUserNull);
    swEngineSetMengine(Engine, swMengineNull);
    swEngineSetNextMengineEngine(Engine, swEngineNull);
    swEngineSetPrevMengineEngine(Engine, swEngineNull);
    if(swEngineConstructorCallback != NULL) {
        swEngineConstructorCallback(Engine);
    }
    return Engine;}

/*----------------------------------------------------------------------------------------
  Fields for class Voice.
----------------------------------------------------------------------------------------*/
struct swVoiceFields {
    utSym *Sym;
    swMengine *Mengine;
    swVoice *NextMengineVoice;
    swVoice *PrevMengineVoice;
    swVoice *NextTableMengineVoice;
    swLanguage *Language;
    swVoice *NextLanguageVoice;
    swVoice *PrevLanguageVoice;
};
extern struct swVoiceFields swVoices;

void swVoiceAllocMore(void);
void swVoiceCopyProps(swVoice swOldVoice, swVoice swNewVoice);
utInlineC utSym swVoiceGetSym(swVoice Voice) {return swVoices.Sym[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetSym(swVoice Voice, utSym value) {swVoices.Sym[swVoice2ValidIndex(Voice)] = value;}
utInlineC swMengine swVoiceGetMengine(swVoice Voice) {return swVoices.Mengine[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetMengine(swVoice Voice, swMengine value) {swVoices.Mengine[swVoice2ValidIndex(Voice)] = value;}
utInlineC swVoice swVoiceGetNextMengineVoice(swVoice Voice) {return swVoices.NextMengineVoice[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetNextMengineVoice(swVoice Voice, swVoice value) {swVoices.NextMengineVoice[swVoice2ValidIndex(Voice)] = value;}
utInlineC swVoice swVoiceGetPrevMengineVoice(swVoice Voice) {return swVoices.PrevMengineVoice[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetPrevMengineVoice(swVoice Voice, swVoice value) {swVoices.PrevMengineVoice[swVoice2ValidIndex(Voice)] = value;}
utInlineC swVoice swVoiceGetNextTableMengineVoice(swVoice Voice) {return swVoices.NextTableMengineVoice[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetNextTableMengineVoice(swVoice Voice, swVoice value) {swVoices.NextTableMengineVoice[swVoice2ValidIndex(Voice)] = value;}
utInlineC swLanguage swVoiceGetLanguage(swVoice Voice) {return swVoices.Language[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetLanguage(swVoice Voice, swLanguage value) {swVoices.Language[swVoice2ValidIndex(Voice)] = value;}
utInlineC swVoice swVoiceGetNextLanguageVoice(swVoice Voice) {return swVoices.NextLanguageVoice[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetNextLanguageVoice(swVoice Voice, swVoice value) {swVoices.NextLanguageVoice[swVoice2ValidIndex(Voice)] = value;}
utInlineC swVoice swVoiceGetPrevLanguageVoice(swVoice Voice) {return swVoices.PrevLanguageVoice[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetPrevLanguageVoice(swVoice Voice, swVoice value) {swVoices.PrevLanguageVoice[swVoice2ValidIndex(Voice)] = value;}
utInlineC void swVoiceSetConstructorCallback(void(*func)(swVoice)) {swVoiceConstructorCallback = func;}
utInlineC swVoiceCallbackType swVoiceGetConstructorCallback(void) {return swVoiceConstructorCallback;}
utInlineC void swVoiceSetDestructorCallback(void(*func)(swVoice)) {swVoiceDestructorCallback = func;}
utInlineC swVoiceCallbackType swVoiceGetDestructorCallback(void) {return swVoiceDestructorCallback;}
utInlineC swVoice swVoiceNextFree(swVoice Voice) {return ((swVoice *)(void *)(swVoices.Sym))[swVoice2ValidIndex(Voice)];}
utInlineC void swVoiceSetNextFree(swVoice Voice, swVoice value) {
    ((swVoice *)(void *)(swVoices.Sym))[swVoice2ValidIndex(Voice)] = value;}
utInlineC void swVoiceFree(swVoice Voice) {
    swVoiceSetNextFree(Voice, swRootData.firstFreeVoice);
    swSetFirstFreeVoice(Voice);}
void swVoiceDestroy(swVoice Voice);
utInlineC swVoice swVoiceAllocRaw(void) {
    swVoice Voice;
    if(swRootData.firstFreeVoice != swVoiceNull) {
        Voice = swRootData.firstFreeVoice;
        swSetFirstFreeVoice(swVoiceNextFree(Voice));
    } else {
        if(swRootData.usedVoice == swRootData.allocatedVoice) {
            swVoiceAllocMore();
        }
        Voice = swIndex2Voice(swRootData.usedVoice);
        swSetUsedVoice(swUsedVoice() + 1);
    }
    return Voice;}
utInlineC swVoice swVoiceAlloc(void) {
    swVoice Voice = swVoiceAllocRaw();
    swVoiceSetSym(Voice, utSymNull);
    swVoiceSetMengine(Voice, swMengineNull);
    swVoiceSetNextMengineVoice(Voice, swVoiceNull);
    swVoiceSetPrevMengineVoice(Voice, swVoiceNull);
    swVoiceSetNextTableMengineVoice(Voice, swVoiceNull);
    swVoiceSetLanguage(Voice, swLanguageNull);
    swVoiceSetNextLanguageVoice(Voice, swVoiceNull);
    swVoiceSetPrevLanguageVoice(Voice, swVoiceNull);
    if(swVoiceConstructorCallback != NULL) {
        swVoiceConstructorCallback(Voice);
    }
    return Voice;}

/*----------------------------------------------------------------------------------------
  Fields for class Queue.
----------------------------------------------------------------------------------------*/
struct swQueueFields {
    swRoot *InRoot;
    swRoot *OutRoot;
    swRoot *AudioRoot;
    swMessage *FirstMessage;
    swMessage *LastMessage;
};
extern struct swQueueFields swQueues;

void swQueueAllocMore(void);
void swQueueCopyProps(swQueue swOldQueue, swQueue swNewQueue);
utInlineC swRoot swQueueGetInRoot(swQueue Queue) {return swQueues.InRoot[swQueue2ValidIndex(Queue)];}
utInlineC void swQueueSetInRoot(swQueue Queue, swRoot value) {swQueues.InRoot[swQueue2ValidIndex(Queue)] = value;}
utInlineC swRoot swQueueGetOutRoot(swQueue Queue) {return swQueues.OutRoot[swQueue2ValidIndex(Queue)];}
utInlineC void swQueueSetOutRoot(swQueue Queue, swRoot value) {swQueues.OutRoot[swQueue2ValidIndex(Queue)] = value;}
utInlineC swRoot swQueueGetAudioRoot(swQueue Queue) {return swQueues.AudioRoot[swQueue2ValidIndex(Queue)];}
utInlineC void swQueueSetAudioRoot(swQueue Queue, swRoot value) {swQueues.AudioRoot[swQueue2ValidIndex(Queue)] = value;}
utInlineC swMessage swQueueGetFirstMessage(swQueue Queue) {return swQueues.FirstMessage[swQueue2ValidIndex(Queue)];}
utInlineC void swQueueSetFirstMessage(swQueue Queue, swMessage value) {swQueues.FirstMessage[swQueue2ValidIndex(Queue)] = value;}
utInlineC swMessage swQueueGetLastMessage(swQueue Queue) {return swQueues.LastMessage[swQueue2ValidIndex(Queue)];}
utInlineC void swQueueSetLastMessage(swQueue Queue, swMessage value) {swQueues.LastMessage[swQueue2ValidIndex(Queue)] = value;}
utInlineC void swQueueSetConstructorCallback(void(*func)(swQueue)) {swQueueConstructorCallback = func;}
utInlineC swQueueCallbackType swQueueGetConstructorCallback(void) {return swQueueConstructorCallback;}
utInlineC void swQueueSetDestructorCallback(void(*func)(swQueue)) {swQueueDestructorCallback = func;}
utInlineC swQueueCallbackType swQueueGetDestructorCallback(void) {return swQueueDestructorCallback;}
utInlineC swQueue swQueueNextFree(swQueue Queue) {return ((swQueue *)(void *)(swQueues.InRoot))[swQueue2ValidIndex(Queue)];}
utInlineC void swQueueSetNextFree(swQueue Queue, swQueue value) {
    ((swQueue *)(void *)(swQueues.InRoot))[swQueue2ValidIndex(Queue)] = value;}
utInlineC void swQueueFree(swQueue Queue) {
    swQueueSetNextFree(Queue, swRootData.firstFreeQueue);
    swSetFirstFreeQueue(Queue);}
void swQueueDestroy(swQueue Queue);
utInlineC swQueue swQueueAllocRaw(void) {
    swQueue Queue;
    if(swRootData.firstFreeQueue != swQueueNull) {
        Queue = swRootData.firstFreeQueue;
        swSetFirstFreeQueue(swQueueNextFree(Queue));
    } else {
        if(swRootData.usedQueue == swRootData.allocatedQueue) {
            swQueueAllocMore();
        }
        Queue = swIndex2Queue(swRootData.usedQueue);
        swSetUsedQueue(swUsedQueue() + 1);
    }
    return Queue;}
utInlineC swQueue swQueueAlloc(void) {
    swQueue Queue = swQueueAllocRaw();
    swQueueSetInRoot(Queue, swRootNull);
    swQueueSetOutRoot(Queue, swRootNull);
    swQueueSetAudioRoot(Queue, swRootNull);
    swQueueSetFirstMessage(Queue, swMessageNull);
    swQueueSetLastMessage(Queue, swMessageNull);
    if(swQueueConstructorCallback != NULL) {
        swQueueConstructorCallback(Queue);
    }
    return Queue;}

/*----------------------------------------------------------------------------------------
  Unions for class Message.
----------------------------------------------------------------------------------------*/
typedef union {
    uint32 MarkNum;
    float Value;
} swMessageUnion1;

/*----------------------------------------------------------------------------------------
  Fields for class Message.
----------------------------------------------------------------------------------------*/
struct swMessageFields {
    swMessageType *Type;
    uint32 *DataIndex_;
    uint32 *NumData;
    uint8 *Data;
    swUser *User;
    swMessage *NextUserMessage;
    swMessage *PrevUserMessage;
    swQueue *Queue;
    swMessage *NextQueueMessage;
    swMessage *PrevQueueMessage;
    swMessageUnion1 *union1;
};
extern struct swMessageFields swMessages;

void swMessageAllocMore(void);
void swMessageCopyProps(swMessage swOldMessage, swMessage swNewMessage);
void swMessageAllocDatas(swMessage Message, uint32 numDatas);
void swMessageResizeDatas(swMessage Message, uint32 numDatas);
void swMessageFreeDatas(swMessage Message);
void swCompactMessageDatas(void);
utInlineC swMessageType swMessageGetType(swMessage Message) {return swMessages.Type[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetType(swMessage Message, swMessageType value) {swMessages.Type[swMessage2ValidIndex(Message)] = value;}
utInlineC uint32 swMessageGetDataIndex_(swMessage Message) {return swMessages.DataIndex_[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetDataIndex_(swMessage Message, uint32 value) {swMessages.DataIndex_[swMessage2ValidIndex(Message)] = value;}
utInlineC uint32 swMessageGetNumData(swMessage Message) {return swMessages.NumData[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetNumData(swMessage Message, uint32 value) {swMessages.NumData[swMessage2ValidIndex(Message)] = value;}
#if defined(DD_DEBUG)
utInlineC uint32 swMessageCheckDataIndex(swMessage Message, uint32 x) {utAssert(x < swMessageGetNumData(Message)); return x;}
#else
utInlineC uint32 swMessageCheckDataIndex(swMessage Message, uint32 x) {return x;}
#endif
utInlineC uint8 swMessageGetiData(swMessage Message, uint32 x) {return swMessages.Data[
    swMessageGetDataIndex_(Message) + swMessageCheckDataIndex(Message, x)];}
utInlineC uint8 *swMessageGetData(swMessage Message) {return swMessages.Data + swMessageGetDataIndex_(Message);}
#define swMessageGetDatas swMessageGetData
utInlineC void swMessageSetData(swMessage Message, uint8 *valuePtr, uint32 numData) {
    swMessageResizeDatas(Message, numData);
    memcpy(swMessageGetDatas(Message), valuePtr, numData*sizeof(uint8));}
utInlineC void swMessageSetiData(swMessage Message, uint32 x, uint8 value) {
    swMessages.Data[swMessageGetDataIndex_(Message) + swMessageCheckDataIndex(Message, (x))] = value;}
utInlineC void swMessageMoveDatas(swMessage Message, uint32 from, uint32 to, uint32 count) {
    utAssert((to+count) <= swMessageGetNumData(Message));
    utAssert((from+count) <= swMessageGetNumData(Message));
    memmove(swMessageGetDatas(Message)+to,swMessageGetDatas(Message)+from,((int32)count)*sizeof(uint8));
}
utInlineC void swMessageCopyDatas(swMessage Message, uint32 x, uint8 * values, uint32 count) {
    utAssert((x+count) <= swMessageGetNumData(Message));
    memcpy(swMessageGetDatas(Message)+x, values, count*sizeof(uint8));
}
utInlineC void swMessageAppendDatas(swMessage Message, uint8 * values, uint32 count) {
    uint32 num = swMessageGetNumData(Message);
    swMessageResizeDatas(Message, num+count);
    swMessageCopyDatas(Message, num, values, count);
}
utInlineC void swMessageAppendData(swMessage Message, uint8 Data) {
    swMessageResizeDatas(Message, swMessageGetNumData(Message)+1);
    swMessageSetiData(Message, swMessageGetNumData(Message)-1, Data);
}
utInlineC void swMessageInsertDatas(swMessage Message, uint32 x, uint8 *Data, uint32 count) {
    utAssert(x <= swMessageGetNumData(Message));
    if(x < swMessageGetNumData(Message)) {
        swMessageResizeDatas(Message, swMessageGetNumData(Message)+count);
        swMessageMoveDatas(Message, x, x+count, swMessageGetNumData(Message)-x-count);
        swMessageCopyDatas(Message, x, Data, count);
    }
    else {
        swMessageAppendDatas(Message, Data, count);
    }
}
utInlineC void swMessageInsertData(swMessage Message, uint32 x, uint8 Data) {
    swMessageInsertDatas(Message, x, &Data, 1);
}
utInlineC void swMessageRemoveDatas(swMessage Message, uint32 x, uint32 count) {
    utAssert((x+count) <= swMessageGetNumData(Message));
    if((x+count) < swMessageGetNumData(Message)) {
        swMessageMoveDatas(Message, x+count,x,swMessageGetNumData(Message)-x-count);
    }
    swMessageResizeDatas(Message, swMessageGetNumData(Message)-(int32)count);
}
utInlineC void swMessageRemoveData(swMessage Message, uint32 x) {
    swMessageRemoveDatas(Message, x, 1);
}
utInlineC void swMessageSwapData(swMessage Message, uint32 from, uint32 to) {
    utAssert(from <= swMessageGetNumData(Message));
    utAssert(to <= swMessageGetNumData(Message));
    uint8 tmp = swMessageGetiData(Message, from);
    swMessageSetiData(Message, from, swMessageGetiData(Message, to));
    swMessageSetiData(Message, to, tmp);
}
utInlineC void swMessageSwapDatas(swMessage Message, uint32 from, uint32 to, uint32 count) {
    utAssert((from+count) < swMessageGetNumData(Message));
    utAssert((to+count) < swMessageGetNumData(Message));
    uint8 tmp[count];
    memcpy(tmp, swMessageGetDatas(Message)+from, count*sizeof(uint8));
    memcpy(swMessageGetDatas(Message)+from, swMessageGetDatas(Message)+to, count*sizeof(uint8));
    memcpy(swMessageGetDatas(Message)+to, tmp, count*sizeof(uint8));
}
#define swForeachMessageData(pVar, cVar) { \
    uint32 _xData; \
    for(_xData = 0; _xData < swMessageGetNumData(pVar); _xData++) { \
        cVar = swMessageGetiData(pVar, _xData);
#define swEndMessageData }}
utInlineC uint32 swMessageGetMarkNum(swMessage Message) {return swMessages.union1[swMessage2ValidIndex(Message)].MarkNum;}
utInlineC void swMessageSetMarkNum(swMessage Message, uint32 value) {
    swMessages.union1[swMessage2ValidIndex(Message)].MarkNum = value;}
utInlineC float swMessageGetValue(swMessage Message) {return swMessages.union1[swMessage2ValidIndex(Message)].Value;}
utInlineC void swMessageSetValue(swMessage Message, float value) {
    swMessages.union1[swMessage2ValidIndex(Message)].Value = value;}
utInlineC swUser swMessageGetUser(swMessage Message) {return swMessages.User[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetUser(swMessage Message, swUser value) {swMessages.User[swMessage2ValidIndex(Message)] = value;}
utInlineC swMessage swMessageGetNextUserMessage(swMessage Message) {return swMessages.NextUserMessage[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetNextUserMessage(swMessage Message, swMessage value) {swMessages.NextUserMessage[swMessage2ValidIndex(Message)] = value;}
utInlineC swMessage swMessageGetPrevUserMessage(swMessage Message) {return swMessages.PrevUserMessage[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetPrevUserMessage(swMessage Message, swMessage value) {swMessages.PrevUserMessage[swMessage2ValidIndex(Message)] = value;}
utInlineC swQueue swMessageGetQueue(swMessage Message) {return swMessages.Queue[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetQueue(swMessage Message, swQueue value) {swMessages.Queue[swMessage2ValidIndex(Message)] = value;}
utInlineC swMessage swMessageGetNextQueueMessage(swMessage Message) {return swMessages.NextQueueMessage[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetNextQueueMessage(swMessage Message, swMessage value) {swMessages.NextQueueMessage[swMessage2ValidIndex(Message)] = value;}
utInlineC swMessage swMessageGetPrevQueueMessage(swMessage Message) {return swMessages.PrevQueueMessage[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetPrevQueueMessage(swMessage Message, swMessage value) {swMessages.PrevQueueMessage[swMessage2ValidIndex(Message)] = value;}
utInlineC void swMessageSetConstructorCallback(void(*func)(swMessage)) {swMessageConstructorCallback = func;}
utInlineC swMessageCallbackType swMessageGetConstructorCallback(void) {return swMessageConstructorCallback;}
utInlineC void swMessageSetDestructorCallback(void(*func)(swMessage)) {swMessageDestructorCallback = func;}
utInlineC swMessageCallbackType swMessageGetDestructorCallback(void) {return swMessageDestructorCallback;}
utInlineC swMessage swMessageNextFree(swMessage Message) {return ((swMessage *)(void *)(swMessages.User))[swMessage2ValidIndex(Message)];}
utInlineC void swMessageSetNextFree(swMessage Message, swMessage value) {
    ((swMessage *)(void *)(swMessages.User))[swMessage2ValidIndex(Message)] = value;}
utInlineC void swMessageFree(swMessage Message) {
    swMessageFreeDatas(Message);
    swMessageSetNextFree(Message, swRootData.firstFreeMessage);
    swSetFirstFreeMessage(Message);}
void swMessageDestroy(swMessage Message);
utInlineC swMessage swMessageAllocRaw(void) {
    swMessage Message;
    if(swRootData.firstFreeMessage != swMessageNull) {
        Message = swRootData.firstFreeMessage;
        swSetFirstFreeMessage(swMessageNextFree(Message));
    } else {
        if(swRootData.usedMessage == swRootData.allocatedMessage) {
            swMessageAllocMore();
        }
        Message = swIndex2Message(swRootData.usedMessage);
        swSetUsedMessage(swUsedMessage() + 1);
    }
    return Message;}
utInlineC swMessage swMessageAlloc(void) {
    swMessage Message = swMessageAllocRaw();
    swMessageSetType(Message, SW_SPEAK);
    swMessageSetDataIndex_(Message, 0);
    swMessageSetNumData(Message, 0);
    swMessageSetNumData(Message, 0);
    swMessageSetMarkNum(Message, 0);
    swMessageSetUser(Message, swUserNull);
    swMessageSetNextUserMessage(Message, swMessageNull);
    swMessageSetPrevUserMessage(Message, swMessageNull);
    swMessageSetQueue(Message, swQueueNull);
    swMessageSetNextQueueMessage(Message, swMessageNull);
    swMessageSetPrevQueueMessage(Message, swMessageNull);
    if(swMessageConstructorCallback != NULL) {
        swMessageConstructorCallback(Message);
    }
    return Message;}

/*----------------------------------------------------------------------------------------
  Fields for class Language.
----------------------------------------------------------------------------------------*/
struct swLanguageFields {
    utSym *Sym;
    swRoot *Root;
    swLanguage *NextRootLanguage;
    swLanguage *PrevRootLanguage;
    swLanguage *NextTableRootLanguage;
    swVoice *FirstVoice;
    swVoice *LastVoice;
};
extern struct swLanguageFields swLanguages;

void swLanguageAllocMore(void);
void swLanguageCopyProps(swLanguage swOldLanguage, swLanguage swNewLanguage);
utInlineC utSym swLanguageGetSym(swLanguage Language) {return swLanguages.Sym[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetSym(swLanguage Language, utSym value) {swLanguages.Sym[swLanguage2ValidIndex(Language)] = value;}
utInlineC swRoot swLanguageGetRoot(swLanguage Language) {return swLanguages.Root[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetRoot(swLanguage Language, swRoot value) {swLanguages.Root[swLanguage2ValidIndex(Language)] = value;}
utInlineC swLanguage swLanguageGetNextRootLanguage(swLanguage Language) {return swLanguages.NextRootLanguage[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetNextRootLanguage(swLanguage Language, swLanguage value) {swLanguages.NextRootLanguage[swLanguage2ValidIndex(Language)] = value;}
utInlineC swLanguage swLanguageGetPrevRootLanguage(swLanguage Language) {return swLanguages.PrevRootLanguage[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetPrevRootLanguage(swLanguage Language, swLanguage value) {swLanguages.PrevRootLanguage[swLanguage2ValidIndex(Language)] = value;}
utInlineC swLanguage swLanguageGetNextTableRootLanguage(swLanguage Language) {return swLanguages.NextTableRootLanguage[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetNextTableRootLanguage(swLanguage Language, swLanguage value) {swLanguages.NextTableRootLanguage[swLanguage2ValidIndex(Language)] = value;}
utInlineC swVoice swLanguageGetFirstVoice(swLanguage Language) {return swLanguages.FirstVoice[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetFirstVoice(swLanguage Language, swVoice value) {swLanguages.FirstVoice[swLanguage2ValidIndex(Language)] = value;}
utInlineC swVoice swLanguageGetLastVoice(swLanguage Language) {return swLanguages.LastVoice[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetLastVoice(swLanguage Language, swVoice value) {swLanguages.LastVoice[swLanguage2ValidIndex(Language)] = value;}
utInlineC void swLanguageSetConstructorCallback(void(*func)(swLanguage)) {swLanguageConstructorCallback = func;}
utInlineC swLanguageCallbackType swLanguageGetConstructorCallback(void) {return swLanguageConstructorCallback;}
utInlineC void swLanguageSetDestructorCallback(void(*func)(swLanguage)) {swLanguageDestructorCallback = func;}
utInlineC swLanguageCallbackType swLanguageGetDestructorCallback(void) {return swLanguageDestructorCallback;}
utInlineC swLanguage swLanguageNextFree(swLanguage Language) {return ((swLanguage *)(void *)(swLanguages.Sym))[swLanguage2ValidIndex(Language)];}
utInlineC void swLanguageSetNextFree(swLanguage Language, swLanguage value) {
    ((swLanguage *)(void *)(swLanguages.Sym))[swLanguage2ValidIndex(Language)] = value;}
utInlineC void swLanguageFree(swLanguage Language) {
    swLanguageSetNextFree(Language, swRootData.firstFreeLanguage);
    swSetFirstFreeLanguage(Language);}
void swLanguageDestroy(swLanguage Language);
utInlineC swLanguage swLanguageAllocRaw(void) {
    swLanguage Language;
    if(swRootData.firstFreeLanguage != swLanguageNull) {
        Language = swRootData.firstFreeLanguage;
        swSetFirstFreeLanguage(swLanguageNextFree(Language));
    } else {
        if(swRootData.usedLanguage == swRootData.allocatedLanguage) {
            swLanguageAllocMore();
        }
        Language = swIndex2Language(swRootData.usedLanguage);
        swSetUsedLanguage(swUsedLanguage() + 1);
    }
    return Language;}
utInlineC swLanguage swLanguageAlloc(void) {
    swLanguage Language = swLanguageAllocRaw();
    swLanguageSetSym(Language, utSymNull);
    swLanguageSetRoot(Language, swRootNull);
    swLanguageSetNextRootLanguage(Language, swLanguageNull);
    swLanguageSetPrevRootLanguage(Language, swLanguageNull);
    swLanguageSetNextTableRootLanguage(Language, swLanguageNull);
    swLanguageSetFirstVoice(Language, swVoiceNull);
    swLanguageSetLastVoice(Language, swVoiceNull);
    if(swLanguageConstructorCallback != NULL) {
        swLanguageConstructorCallback(Language);
    }
    return Language;}

/*----------------------------------------------------------------------------------------
  Relationship macros between classes.
----------------------------------------------------------------------------------------*/
swUser swRootFindUser(swRoot Root, uint32 Id);
#define swForeachRootUser(pVar, cVar) \
    for(cVar = swRootGetFirstUser(pVar); cVar != swUserNull; \
        cVar = swUserGetNextRootUser(cVar))
#define swEndRootUser
#define swSafeForeachRootUser(pVar, cVar) { \
    swUser _nextUser; \
    for(cVar = swRootGetFirstUser(pVar); cVar != swUserNull; cVar = _nextUser) { \
        _nextUser = swUserGetNextRootUser(cVar);
#define swEndSafeRootUser }}
#define swForeachRootEngine(pVar, cVar) \
    for(cVar = swRootGetFirstEngine(pVar); cVar != swEngineNull; \
        cVar = swEngineGetNextRootEngine(cVar))
#define swEndRootEngine
#define swSafeForeachRootEngine(pVar, cVar) { \
    swEngine _nextEngine; \
    for(cVar = swRootGetFirstEngine(pVar); cVar != swEngineNull; cVar = _nextEngine) { \
        _nextEngine = swEngineGetNextRootEngine(cVar);
#define swEndSafeRootEngine }}
swLanguage swRootFindLanguage(swRoot Root, utSym Sym);
void swRootRenameLanguage(swRoot Root, swLanguage _Language, utSym sym);
utInlineC char *swLanguageGetName(swLanguage Language) {return utSymGetName(swLanguageGetSym(Language));}
#define swForeachRootLanguage(pVar, cVar) \
    for(cVar = swRootGetFirstLanguage(pVar); cVar != swLanguageNull; \
        cVar = swLanguageGetNextRootLanguage(cVar))
#define swEndRootLanguage
#define swSafeForeachRootLanguage(pVar, cVar) { \
    swLanguage _nextLanguage; \
    for(cVar = swRootGetFirstLanguage(pVar); cVar != swLanguageNull; cVar = _nextLanguage) { \
        _nextLanguage = swLanguageGetNextRootLanguage(cVar);
#define swEndSafeRootLanguage }}
void swRootInsertUser(swRoot Root, swUser _User);
void swRootRemoveUser(swRoot Root, swUser _User);
void swRootInsertAfterUser(swRoot Root, swUser prevUser, swUser _User);
void swRootAppendUser(swRoot Root, swUser _User);
void swRootInsertEngine(swRoot Root, swEngine _Engine);
void swRootRemoveEngine(swRoot Root, swEngine _Engine);
void swRootInsertAfterEngine(swRoot Root, swEngine prevEngine, swEngine _Engine);
void swRootAppendEngine(swRoot Root, swEngine _Engine);
utInlineC void swRootInsertInQueue(swRoot Root, swQueue _Queue) {swRootSetInQueue(Root, _Queue); swQueueSetInRoot(_Queue, Root);}
utInlineC void swRootRemoveInQueue(swRoot Root, swQueue _Queue) {swRootSetInQueue(Root, swQueueNull); swQueueSetInRoot(_Queue, swRootNull);}
utInlineC void swRootInsertOutQueue(swRoot Root, swQueue _Queue) {swRootSetOutQueue(Root, _Queue); swQueueSetOutRoot(_Queue, Root);}
utInlineC void swRootRemoveOutQueue(swRoot Root, swQueue _Queue) {swRootSetOutQueue(Root, swQueueNull); swQueueSetOutRoot(_Queue, swRootNull);}
utInlineC void swRootInsertAudioQueue(swRoot Root, swQueue _Queue) {swRootSetAudioQueue(Root, _Queue); swQueueSetAudioRoot(_Queue, Root);}
utInlineC void swRootRemoveAudioQueue(swRoot Root, swQueue _Queue) {swRootSetAudioQueue(Root, swQueueNull); swQueueSetAudioRoot(_Queue, swRootNull);}
void swRootInsertLanguage(swRoot Root, swLanguage _Language);
void swRootRemoveLanguage(swRoot Root, swLanguage _Language);
void swRootInsertAfterLanguage(swRoot Root, swLanguage prevLanguage, swLanguage _Language);
void swRootAppendLanguage(swRoot Root, swLanguage _Language);
#define swForeachUserMessage(pVar, cVar) \
    for(cVar = swUserGetFirstMessage(pVar); cVar != swMessageNull; \
        cVar = swMessageGetNextUserMessage(cVar))
#define swEndUserMessage
#define swSafeForeachUserMessage(pVar, cVar) { \
    swMessage _nextMessage; \
    for(cVar = swUserGetFirstMessage(pVar); cVar != swMessageNull; cVar = _nextMessage) { \
        _nextMessage = swMessageGetNextUserMessage(cVar);
#define swEndSafeUserMessage }}
utInlineC void swUserInsertEngine(swUser User, swEngine _Engine) {swUserSetEngine(User, _Engine); swEngineSetUser(_Engine, User);}
utInlineC void swUserRemoveEngine(swUser User, swEngine _Engine) {swUserSetEngine(User, swEngineNull); swEngineSetUser(_Engine, swUserNull);}
void swUserInsertMessage(swUser User, swMessage _Message);
void swUserRemoveMessage(swUser User, swMessage _Message);
void swUserInsertAfterMessage(swUser User, swMessage prevMessage, swMessage _Message);
void swUserAppendMessage(swUser User, swMessage _Message);
#define swForeachMengineEngine(pVar, cVar) \
    for(cVar = swMengineGetFirstEngine(pVar); cVar != swEngineNull; \
        cVar = swEngineGetNextMengineEngine(cVar))
#define swEndMengineEngine
#define swSafeForeachMengineEngine(pVar, cVar) { \
    swEngine _nextEngine; \
    for(cVar = swMengineGetFirstEngine(pVar); cVar != swEngineNull; cVar = _nextEngine) { \
        _nextEngine = swEngineGetNextMengineEngine(cVar);
#define swEndSafeMengineEngine }}
swVoice swMengineFindVoice(swMengine Mengine, utSym Sym);
void swMengineRenameVoice(swMengine Mengine, swVoice _Voice, utSym sym);
utInlineC char *swVoiceGetName(swVoice Voice) {return utSymGetName(swVoiceGetSym(Voice));}
#define swForeachMengineVoice(pVar, cVar) \
    for(cVar = swMengineGetFirstVoice(pVar); cVar != swVoiceNull; \
        cVar = swVoiceGetNextMengineVoice(cVar))
#define swEndMengineVoice
#define swSafeForeachMengineVoice(pVar, cVar) { \
    swVoice _nextVoice; \
    for(cVar = swMengineGetFirstVoice(pVar); cVar != swVoiceNull; cVar = _nextVoice) { \
        _nextVoice = swVoiceGetNextMengineVoice(cVar);
#define swEndSafeMengineVoice }}
void swMengineInsertEngine(swMengine Mengine, swEngine _Engine);
void swMengineRemoveEngine(swMengine Mengine, swEngine _Engine);
void swMengineInsertAfterEngine(swMengine Mengine, swEngine prevEngine, swEngine _Engine);
void swMengineAppendEngine(swMengine Mengine, swEngine _Engine);
void swMengineInsertVoice(swMengine Mengine, swVoice _Voice);
void swMengineRemoveVoice(swMengine Mengine, swVoice _Voice);
void swMengineInsertAfterVoice(swMengine Mengine, swVoice prevVoice, swVoice _Voice);
void swMengineAppendVoice(swMengine Mengine, swVoice _Voice);
#define swForeachQueueMessage(pVar, cVar) \
    for(cVar = swQueueGetFirstMessage(pVar); cVar != swMessageNull; \
        cVar = swMessageGetNextQueueMessage(cVar))
#define swEndQueueMessage
#define swSafeForeachQueueMessage(pVar, cVar) { \
    swMessage _nextMessage; \
    for(cVar = swQueueGetFirstMessage(pVar); cVar != swMessageNull; cVar = _nextMessage) { \
        _nextMessage = swMessageGetNextQueueMessage(cVar);
#define swEndSafeQueueMessage }}
void swQueueInsertMessage(swQueue Queue, swMessage _Message);
void swQueueRemoveMessage(swQueue Queue, swMessage _Message);
void swQueueInsertAfterMessage(swQueue Queue, swMessage prevMessage, swMessage _Message);
void swQueueAppendMessage(swQueue Queue, swMessage _Message);
#define swForeachLanguageVoice(pVar, cVar) \
    for(cVar = swLanguageGetFirstVoice(pVar); cVar != swVoiceNull; \
        cVar = swVoiceGetNextLanguageVoice(cVar))
#define swEndLanguageVoice
#define swSafeForeachLanguageVoice(pVar, cVar) { \
    swVoice _nextVoice; \
    for(cVar = swLanguageGetFirstVoice(pVar); cVar != swVoiceNull; cVar = _nextVoice) { \
        _nextVoice = swVoiceGetNextLanguageVoice(cVar);
#define swEndSafeLanguageVoice }}
void swLanguageInsertVoice(swLanguage Language, swVoice _Voice);
void swLanguageRemoveVoice(swLanguage Language, swVoice _Voice);
void swLanguageInsertAfterVoice(swLanguage Language, swVoice prevVoice, swVoice _Voice);
void swLanguageAppendVoice(swLanguage Language, swVoice _Voice);
void swDatabaseStart(void);
void swDatabaseStop(void);
utInlineC void swDatabaseSetSaved(bool value) {utModuleSetSaved(utModules + swModuleID, value);}
#if defined __cplusplus
}
#endif

#endif
