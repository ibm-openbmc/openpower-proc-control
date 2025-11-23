#pragma once

#include <ecmdDataBuffer.H>
#include <ecmdDefines.H>
#include <ecmdReturnCodes.H>
#include <ecmdStructs.H>
#include <inttypes.h>

#include <string>
#include <vector>

extern "C"
{

//--------------------------------------------------------------------
// ecmd_dll_init_dll
//--------------------------------------------------------------------
uint32_t dllLoadDll(const char* version, uint32_t debugLevel);

uint32_t dllUnloadDll();

uint32_t dllInitDll();

uint32_t dllFreeDll();

uint32_t dllQueryDllInfo(ecmdDllInfo&);

uint32_t dllCheckDllVersion(const char* options);

bool dllQueryVersionGreater(const char* version);

uint32_t dllConfigLooperInit(ecmdChipTarget& target,
                             ecmdLoopType_t looptype,
                             ecmdLooperData& state);

uint32_t dllExistLooperInit(ecmdChipTarget& target,
                            ecmdLoopType_t looptype, ecmdLooperData& state);

uint32_t dllLooperInit(ecmdChipTarget& target, ecmdLoopType_t looptype,
                       ecmdLooperData& state, ecmdLoopMode_t mode);

uint32_t dllConfigLooperNext(ecmdChipTarget& target,
                             ecmdLooperData& state);

uint32_t dllExistLooperNext(ecmdChipTarget& target, ecmdLooperData& state);

uint32_t dllLooperNext(ecmdChipTarget& target, ecmdLooperData& state,
                       ecmdLoopMode_t mode);

uint32_t dllQuerySelected(ecmdChipTarget& target, ecmdQueryData& queryData,
                          ecmdLoopType_t looptype);

uint32_t dllQueryConfigSelected(ecmdChipTarget& target,
                                ecmdQueryData& queryData,
                                ecmdLoopType_t looptype);

uint32_t dllQueryExistSelected(ecmdChipTarget& target,
                               ecmdQueryData& queryData,
                               ecmdLoopType_t looptype);

uint32_t queryConfigExistSelected(ecmdChipTarget& target,
                                  ecmdQueryData& queryData,
                                  ecmdLoopType_t looptype, bool existMode);


void dllPushCommandArgs();

void dllPopCommandArgs();

uint32_t dllCommonCommandArgs(int* argc, char** argv[]);

void dllOutputError(const char* message);

void dllOutputWarning(const char* message);

void dllOutput(const char* message);

uint32_t dllDelay(uint32_t simCycles, uint32_t msDelay);

uint32_t dllGetGlobalVar(ecmdGlobalVarType_t type);

uint32_t dllSetGlobalVar(ecmdGlobalVarType_t type, uint32_t value);

std::string dllGetCurrentCmdline();

void dllSetCurrentCmdline(int argc, char* argv[]);

bool dllQueryTargetConfigured(const ecmdChipTarget& target,
                              const ecmdQueryData* queryData);

bool dllQueryTargetExist(const ecmdChipTarget& target,
                         const ecmdQueryData* queryData);

//
///ecmd_dll_istep
//
uint32_t dllIStepsByNumber(const ecmdDataBuffer& steps); 

uint32_t dllIStepsByName(std::string stepName); 

uint32_t dllIStepsByNameMultiple(std::list< std::string > stepNames); 

uint32_t dllIStepsByNameRange(std::string stepNameBegin, std::string stepNameEnd); 

//--------------------------------------------------------------------
// ecmd_dll_cfam
//--------------------------------------------------------------------
uint32_t dllGetCfamRegister(const ecmdChipTarget& target, uint32_t address,
                            ecmdDataBuffer& data);

uint32_t dllPutCfamRegister(const ecmdChipTarget& target, uint32_t address,
                            const ecmdDataBuffer& data);

//--------------------------------------------------------------------
// ecmd_dll_ring
//--------------------------------------------------------------------
bool dllIsRingCacheEnabled(const ecmdChipTarget&);

//--------------------------------------------------------------------
// ecmd_dll_error
//--------------------------------------------------------------------
std::string dllGetErrorMsg(uint32_t returnCode, bool parseReturnCode,
                           bool deleteMessage, bool messageBorder);

uint32_t dllRegisterErrorMsg(uint32_t returnCode, const char* whom,
                             const char* message);

uint32_t dllFlushRegisteredErrorMsgs(uint32_t returnCode);

uint32_t dllFlushRegisteredErrorMsgsString(uint32_t returnCode,
                                           std::string searchString);

uint32_t dllGetErrorTarget(uint32_t returnCode,
                           std::list<ecmdChipTarget>& errorTargets,
                           bool deleteTarget);

uint32_t dllRegisterErrorTarget(uint32_t returnCode,
                                const ecmdChipTarget& errorTarget);

uint32_t dllFlushRegisteredErrorTargets(uint32_t returnCode);

std::string dllParseReturnCode(uint32_t returnCode);

uint32_t dllSpecificCommandArgs([[maybe_unused]] int* argc,
                                [[maybe_unused]] char** argv[]);

///
//ecmd_dll_query
///
uint32_t dllQueryConfig(const ecmdChipTarget& target, ecmdQueryData& queryData,
                        ecmdQueryDetail_t detail);

uint32_t dllQueryExist(const ecmdChipTarget& target, ecmdQueryData& queryData,
                       ecmdQueryDetail_t detail);

uint32_t dllRelatedTargets(const ecmdChipTarget&, const std::string,
                           std::list<ecmdChipTarget>&, const ecmdLoopMode_t);

uint32_t dllQueryFileLocation(const ecmdChipTarget&, ecmdFileType_t,
                              std::list<ecmdFileLocation>&, std::string&);

uint32_t dllQueryConnectedTargets(const ecmdChipTarget&, const char*,
                                  std::list<ecmdConnectionData>&);

uint32_t dllGetChipData(const ecmdChipTarget& ecmdTarget, ecmdChipData& data);

uint32_t dllQueryMode(const ecmdChipTarget&, std::string&, std::string&);

uint32_t dllQueryScom(const ecmdChipTarget&, std::list<ecmdScomData>&, uint64_t,
                      ecmdQueryDetail_t);

}
