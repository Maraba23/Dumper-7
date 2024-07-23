#pragma once
#include <filesystem>
#include "DependencyManager.h"
#include "StructManager.h"
#include "MemberManager.h"
#include "HashStringTable.h"
#include "StructWrapper.h"
#include "MemberWrappers.h"
#include "EnumWrapper.h"
#include "PackageManager.h"

#include "Generator.h"

#include <fstream>

namespace fs = std::filesystem;


class CppGenerator
{
private:
    friend class CppGeneratorTest;
    friend class Generator;

private:
    struct ParamInfo
    {
        bool bIsOutPtr;
        bool bIsOutRef;
        bool bIsMoveParam;
        bool bIsRetParam;
        bool bIsConst;
        EPropertyFlags PropFlags;

        std::wstring Type;
        std::wstring Name;
    };

    struct FunctionInfo
    {
        bool bIsReturningVoid;
        EFunctionFlags FuncFlags = EFunctionFlags::None;

        std::wstring RetType;
        std::wstring FuncNameWithParams;

        std::vector<ParamInfo> UnrealFuncParams; // for unreal-functions only
    };

    enum class EFileType
    {
        Classes,
        Structs,
        Parameters,
        Functions,

        NameCollisionsInl,

        BasicHpp,
        BasicCpp,

        UnrealContainers,

        PropertyFixup,
        SdkHpp,

        DebugAssertions,
    };

private:
    using StreamType = std::wofstream;

public:
    static inline PredefinedMemberLookupMapType PredefinedMembers;

    static inline std::wstring MainFolderName = L"CppSDK";
    static inline std::wstring SubfolderName = L"SDK";

    static inline fs::path MainFolder;
    static inline fs::path Subfolder;

private:
    static inline std::vector<PredefinedStruct> PredefinedStructs;

private:
    static std::wstring MakeMemberString(const std::wstring& Type, const std::wstring& Name, std::wstring&& Comment);
    static std::wstring MakeMemberStringWithoutName(const std::wstring& Type, std::wstring&& Comment);

    static std::wstring GenerateBytePadding(const int32 Offset, const int32 PadSize, std::wstring&& Reason);
    static std::wstring GenerateBitPadding(uint8 UnderlayingSizeBytes, const uint8 PrevBitPropertyEndBit, const int32 Offset, const int32 PadSize, std::wstring&& Reason);

    static std::wstring GenerateMembers(const StructWrapper& Struct, const MemberManager& Members, int32 SuperSize, int32 SuperLastMemberEnd, int32 SuperAlign, int32 PackageIndex = -1);
    static std::wstring GenerateFunctionInHeader(const MemberManager& Members);
    static FunctionInfo GenerateFunctionInfo(const FunctionWrapper& Func);

    // return: In-header function declarations and inline functions
    static std::wstring GenerateSingleFunction(const FunctionWrapper& Func, const std::wstring& StructName, StreamType& FunctionFile, StreamType& ParamFile);
    static std::wstring GenerateFunctions(const StructWrapper& Struct, const MemberManager& Members, const std::wstring& StructName, StreamType& FunctionFile, StreamType& ParamFile);

    static void GenerateStruct(const StructWrapper& Struct, StreamType& StructFile, StreamType& FunctionFile, StreamType& ParamFile, int32 PackageIndex = -1);

    static void GenerateEnum(const EnumWrapper& Enum, StreamType& StructFile);

private: /* utility functions */
    static std::wstring GetMemberTypeString(const PropertyWrapper& MemberWrapper, int32 PackageIndex = -1, bool bAllowForConstPtrMembers = false /* const USomeClass* Member; */);
    static std::wstring GetMemberTypeString(UEProperty Member, int32 PackageIndex = -1, bool bAllowForConstPtrMembers = false);
    static std::wstring GetMemberTypeStringWithoutConst(UEProperty Member, int32 PackageIndex = -1);

    static std::wstring GetFunctionSignature(UEFunction Func);

    static std::wstring GetStructPrefixedName(const StructWrapper& Struct);
    static std::wstring GetEnumPrefixedName(const EnumWrapper& Enum);
    static std::wstring GetEnumUnderlayingType(const EnumWrapper& Enm);

    static std::wstring GetCycleFixupType(const StructWrapper& Struct, bool bIsForInheritance);

    static std::unordered_map<std::wstring, UEProperty> GetUnknownProperties();

private:
    static void GenerateEnumFwdDeclarations(StreamType& ClassOrStructFile, PackageInfoHandle Package, bool bIsClassFile);

private:
    static void GenerateNameCollisionsInl(StreamType& NameCollisionsFile);
    static void GeneratePropertyFixupFile(StreamType& PropertyFixup);
    static void GenerateDebugAssertions(StreamType& AssertionStream);
    static void WriteFileHead(StreamType& File, PackageInfoHandle Package, EFileType Type, const std::wstring& CustomFileComment = L"", const std::wstring& CustomIncludes = L"");
    static void WriteFileEnd(StreamType& File, EFileType Type);

    static void GenerateSDKHeader(StreamType& SdkHpp);

    static void GenerateBasicFiles(StreamType& BasicH, StreamType& BasicCpp);

    /*
    * Creates the UnrealContainers.hpp file (without allocation code) for the SDK. 
    * File contains the implementation of TArray, FString, TSparseArray, TSet, TMap and iterators for them
    *
    * See https://github.com/Fischsalat/UnrealContainers/blob/master/UnrealContainers/UnrealContainersNoAlloc.h 
    */
    static void GenerateUnrealContainers(StreamType& UEContainersHeader);

public:
    static void Generate();

    static void InitPredefinedMembers();
    static void InitPredefinedFunctions();
};