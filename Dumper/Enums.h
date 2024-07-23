#pragma once

#include <cstdint>
#include <type_traits>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>

#include <cassert>

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;


template<typename T>
constexpr T Align(T Size, T Alignment)
{
	static_assert(std::is_integral_v<T>, L"Align can only hanlde integral types!");
	assert(Alignment != 0 && "Alignment was 0, division by zero exception.");

	const T RequiredAlign = Alignment - (Size % Alignment);

	return Size + (RequiredAlign != Alignment ? RequiredAlign : 0x0);
}


#define ENUM_OPERATORS(EEnumClass)																																		\
																																										\
inline constexpr EEnumClass operator|(EEnumClass Left, EEnumClass Right)																								\
{																																										\
	return (EEnumClass)((std::underlying_type<EEnumClass>::type)(Left) | (std::underlying_type<EEnumClass>::type)(Right));												\
}																																										\
																																										\
inline constexpr EEnumClass& operator|=(EEnumClass& Left, EEnumClass Right)																								\
{																																										\
	return (EEnumClass&)((std::underlying_type<EEnumClass>::type&)(Left) |= (std::underlying_type<EEnumClass>::type)(Right));											\
}																																										\
																																										\
inline bool operator&(EEnumClass Left, EEnumClass Right)																												\
{																																										\
	return (((std::underlying_type<EEnumClass>::type)(Left) & (std::underlying_type<EEnumClass>::type)(Right)) == (std::underlying_type<EEnumClass>::type)(Right));		\
}																																										


enum class EPropertyFlags : uint64
{
	None							= 0x0000000000000000,

	Edit							= 0x0000000000000001,
	ConstParm						= 0x0000000000000002,
	BlueprintVisible				= 0x0000000000000004,
	ExportObject					= 0x0000000000000008,
	BlueprintReadOnly				= 0x0000000000000010,
	Net								= 0x0000000000000020,
	EditFixedSize					= 0x0000000000000040,
	Parm							= 0x0000000000000080,
	OutParm							= 0x0000000000000100,
	ZeroConstructor					= 0x0000000000000200,
	ReturnParm						= 0x0000000000000400,
	DisableEditOnTemplate 			= 0x0000000000000800,

	Transient						= 0x0000000000002000,
	Config							= 0x0000000000004000,

	DisableEditOnInstance			= 0x0000000000010000,
	EditConst						= 0x0000000000020000,
	GlobalConfig					= 0x0000000000040000,
	InstancedReference				= 0x0000000000080000,	

	DuplicateTransient				= 0x0000000000200000,	
	SubobjectReference				= 0x0000000000400000,	

	SaveGame						= 0x0000000001000000,
	NoClear							= 0x0000000002000000,

	ReferenceParm					= 0x0000000008000000,
	BlueprintAssignable				= 0x0000000010000000,
	Deprecated						= 0x0000000020000000,
	IsPlainOldData					= 0x0000000040000000,
	RepSkip							= 0x0000000080000000,
	RepNotify						= 0x0000000100000000, 
	Interp							= 0x0000000200000000,
	NonTransactional				= 0x0000000400000000,
	EditorOnly						= 0x0000000800000000,
	NoDestructor					= 0x0000001000000000,

	AutoWeak						= 0x0000004000000000,
	ContainsInstancedReference		= 0x0000008000000000,	
	AssetRegistrySearchable			= 0x0000010000000000,
	SimpleDisplay					= 0x0000020000000000,
	AdvancedDisplay					= 0x0000040000000000,
	Protected						= 0x0000080000000000,
	BlueprintCallable				= 0x0000100000000000,
	BlueprintAuthorityOnly			= 0x0000200000000000,
	TextExportTransient				= 0x0000400000000000,
	NonPIEDuplicateTransient		= 0x0000800000000000,
	ExposeOnSpawn					= 0x0001000000000000,
	PersistentInstance				= 0x0002000000000000,
	UObjectWrapper					= 0x0004000000000000, 
	HasGetValueTypeHash				= 0x0008000000000000, 
	NativeAccessSpecifierPublic		= 0x0010000000000000,	
	NativeAccessSpecifierProtected	= 0x0020000000000000,
	NativeAccessSpecifierPrivate	= 0x0040000000000000,	
	SkipSerialization				= 0x0080000000000000, 
};

enum class EFunctionFlags : uint32
{
	None					= 0x00000000,

	Final					= 0x00000001,
	RequiredAPI				= 0x00000002,
	BlueprintAuthorityOnly	= 0x00000004, 
	BlueprintCosmetic		= 0x00000008, 
	Net						= 0x00000040,  
	NetReliable				= 0x00000080, 
	NetRequest				= 0x00000100,	
	Exec					= 0x00000200,	
	Native					= 0x00000400,	
	Event					= 0x00000800,   
	NetResponse				= 0x00001000,  
	Static					= 0x00002000,   
	NetMulticast			= 0x00004000,	
	UbergraphFunction		= 0x00008000,  
	MulticastDelegate		= 0x00010000,
	Public					= 0x00020000,	
	Private					= 0x00040000,	
	Protected				= 0x00080000,
	Delegate				= 0x00100000,	
	NetServer				= 0x00200000,	
	HasOutParms				= 0x00400000,	
	HasDefaults				= 0x00800000,
	NetClient				= 0x01000000,
	DLLImport				= 0x02000000,
	BlueprintCallable		= 0x04000000,
	BlueprintEvent			= 0x08000000,
	BlueprintPure			= 0x10000000,	
	EditorOnly				= 0x20000000,	
	Const					= 0x40000000,
	NetValidate				= 0x80000000,

	AllFlags = 0xFFFFFFFF,
};

enum class EObjectFlags
{
	NoFlags							= 0x00000000,

	Public							= 0x00000001,
	Standalone						= 0x00000002,
	MarkAsNative					= 0x00000004,
	Transactional					= 0x00000008,
	ClassDefaultObject				= 0x00000010,
	ArchetypeObject					= 0x00000020,
	Transient						= 0x00000040,

	MarkAsRootSet					= 0x00000080,
	TagGarbageTemp					= 0x00000100,

	NeedInitialization				= 0x00000200,
	NeedLoad						= 0x00000400,
	KeepForCooker					= 0x00000800,
	NeedPostLoad					= 0x00001000,
	NeedPostLoadSubobjects			= 0x00002000,
	NewerVersionExists				= 0x00004000,
	BeginDestroyed					= 0x00008000,
	FinishDestroyed					= 0x00010000,

	BeingRegenerated				= 0x00020000,
	DefaultSubObject				= 0x00040000,
	WasLoaded						= 0x00080000,
	TextExportTransient				= 0x00100000,
	LoadCompleted					= 0x00200000,
	InheritableComponentTemplate	= 0x00400000,
	DuplicateTransient				= 0x00800000,
	StrongRefOnFrame				= 0x01000000,
	NonPIEDuplicateTransient		= 0x02000000,
	Dynamic							= 0x04000000,
	WillBeLoaded					= 0x08000000, 
};

enum class EFieldClassID : uint64
{
	Int8					= 1llu << 1,
	Byte					= 1llu << 6,
	Int						= 1llu << 7,
	Float					= 1llu << 8,
	UInt64					= 1llu << 9,
	Class					= 1llu << 10,
	UInt32					= 1llu << 11,
	Interface				= 1llu << 12,
	Name					= 1llu << 13,
	String					= 1llu << 14,
	Object					= 1llu << 16,
	Bool					= 1llu << 17,
	UInt16					= 1llu << 18,
	Struct					= 1llu << 20,
	Array					= 1llu << 21,
	Int64					= 1llu << 22,
	Delegate				= 1llu << 23,
	SoftObject				= 1llu << 27,
	LazyObject				= 1llu << 28,
	WeakObject				= 1llu << 29,
	Text					= 1llu << 30,
	Int16					= 1llu << 31,
	Double					= 1llu << 32,
	SoftClass				= 1llu << 33,
	Map						= 1llu << 46,
	Set						= 1llu << 47,
	Enum					= 1llu << 48,
	MulticastInlineDelegate = 1llu << 50,
	MulticastSparseDelegate = 1llu << 51,
	ObjectPointer			= 1llu << 53
};

enum class EClassCastFlags : uint64
{
	None								= 0x0000000000000000,

	Field								= 0x0000000000000001,
	Int8Property						= 0x0000000000000002,
	Enum								= 0x0000000000000004,
	Struct								= 0x0000000000000008,
	ScriptStruct						= 0x0000000000000010,
	Class								= 0x0000000000000020,
	ByteProperty						= 0x0000000000000040,
	IntProperty							= 0x0000000000000080,
	FloatProperty						= 0x0000000000000100,
	UInt64Property						= 0x0000000000000200,
	ClassProperty						= 0x0000000000000400,
	UInt32Property						= 0x0000000000000800,
	InterfaceProperty					= 0x0000000000001000,
	NameProperty						= 0x0000000000002000,
	StrProperty							= 0x0000000000004000,
	Property							= 0x0000000000008000,
	ObjectProperty						= 0x0000000000010000,
	BoolProperty						= 0x0000000000020000,
	UInt16Property						= 0x0000000000040000,
	Function							= 0x0000000000080000,
	StructProperty						= 0x0000000000100000,
	ArrayProperty						= 0x0000000000200000,
	Int64Property						= 0x0000000000400000,
	DelegateProperty					= 0x0000000000800000,
	NumericProperty						= 0x0000000001000000,
	MulticastDelegateProperty			= 0x0000000002000000,
	ObjectPropertyBase					= 0x0000000004000000,
	WeakObjectProperty					= 0x0000000008000000,
	LazyObjectProperty					= 0x0000000010000000,
	SoftObjectProperty					= 0x0000000020000000,
	TextProperty						= 0x0000000040000000,
	Int16Property						= 0x0000000080000000,
	DoubleProperty						= 0x0000000100000000,
	SoftClassProperty					= 0x0000000200000000,
	Package								= 0x0000000400000000,
	Level								= 0x0000000800000000,
	Actor								= 0x0000001000000000,
	PlayerController					= 0x0000002000000000,
	Pawn								= 0x0000004000000000,
	SceneComponent						= 0x0000008000000000,
	PrimitiveComponent					= 0x0000010000000000,
	SkinnedMeshComponent				= 0x0000020000000000,
	SkeletalMeshComponent				= 0x0000040000000000,
	Blueprint							= 0x0000080000000000,
	DelegateFunction					= 0x0000100000000000,
	StaticMeshComponent					= 0x0000200000000000,
	MapProperty							= 0x0000400000000000,
	SetProperty							= 0x0000800000000000,
	EnumProperty						= 0x0001000000000000,
	SparseDelegateFunction				= 0x0002000000000000,
	MulticastInlineDelegateProperty		= 0x0004000000000000,
	MulticastSparseDelegateProperty		= 0x0008000000000000,
	FieldPathProperty					= 0x0010000000000000,
	LargeWorldCoordinatesRealProperty	= 0x0080000000000000,
	OptionalProperty					= 0x0100000000000000,
	VValueProperty						= 0x0200000000000000,
	VerseVMClass						= 0x0400000000000000,
	VRestValueProperty					= 0x0800000000000000,
};

enum class EClassFlags
{
	None						= 0x00000000u,
	Abstract					= 0x00000001u,
	DefaultConfig				= 0x00000002u,
	Config						= 0x00000004u,
	Transient					= 0x00000008u,
	Parsed						= 0x00000010u,
	MatchedSerializers			= 0x00000020u,
	ProjectUserConfig			= 0x00000040u,
	Native						= 0x00000080u,
	NoExport					= 0x00000100u,
	NotPlaceable				= 0x00000200u,
	PerObjectConfig				= 0x00000400u,
	ReplicationDataIsSetUp		= 0x00000800u,
	EditInlineNew				= 0x00001000u,
	CollapseCategories			= 0x00002000u,
	Interface					= 0x00004000u,
	CustomConstructor			= 0x00008000u,
	Const						= 0x00010000u,
	LayoutChanging				= 0x00020000u,
	CompiledFromBlueprint		= 0x00040000u,
	MinimalAPI					= 0x00080000u,
	RequiredAPI					= 0x00100000u,
	DefaultToInstanced			= 0x00200000u,
	TokenStreamAssembled		= 0x00400000u,
	HasInstancedReference		= 0x00800000u,
	Hidden						= 0x01000000u,
	Deprecated					= 0x02000000u,
	HideDropDown				= 0x04000000u,
	GlobalUserConfig			= 0x08000000u,
	Intrinsic					= 0x10000000u,
	Constructed					= 0x20000000u,
	ConfigDoNotCheckDefaults	= 0x40000000u,
	NewerVersionExists			= 0x80000000u,
};

enum class EMappingsTypeFlags : uint8
{
	ByteProperty,
	BoolProperty,
	IntProperty,
	FloatProperty,
	ObjectProperty,
	NameProperty,
	DelegateProperty,
	DoubleProperty,
	ArrayProperty,
	StructProperty,
	StrProperty,
	TextProperty,
	InterfaceProperty,
	MulticastDelegateProperty,
	WeakObjectProperty, //
	LazyObjectProperty, // When deserialized, these 3 properties will be SoftObjects
	AssetObjectProperty, //
	SoftObjectProperty,
	UInt64Property,
	UInt32Property,
	UInt16Property,
	Int64Property,
	Int16Property,
	Int8Property,
	MapProperty,
	SetProperty,
	EnumProperty,
	FieldPathProperty,
	OptionalProperty,

	Unknown = 0xFF
};

enum class EEnumFlags : uint8
{
	None,

	Flags = 0x00000001,              // Whether the UEnum represents a set of flags
	NewerVersionExists = 0x00000002, // If set, this UEnum has been replaced by a newer version
};

ENUM_OPERATORS(EObjectFlags);
ENUM_OPERATORS(EFunctionFlags);
ENUM_OPERATORS(EPropertyFlags);
ENUM_OPERATORS(EClassCastFlags);
ENUM_OPERATORS(EClassFlags);
ENUM_OPERATORS(EMappingsTypeFlags);
ENUM_OPERATORS(EFieldClassID);
ENUM_OPERATORS(EEnumFlags);

static std::wstring StringifyFunctionFlags(EFunctionFlags FunctionFlags, const wchar_t* Seperator = L", L")
{
	/* Make sure the size of this vector is always greater, or equal, to the number of flags existing */
	std::array<const wchar_t*, 0x30> StringFlags;
	int32 CurrentIdx = 0x0;

	if (FunctionFlags & EFunctionFlags::Final) { StringFlags[CurrentIdx++] = L"Final"; }
	if (FunctionFlags & EFunctionFlags::RequiredAPI) { StringFlags[CurrentIdx++] = L"RequiredAPI"; }
	if (FunctionFlags & EFunctionFlags::BlueprintAuthorityOnly) { StringFlags[CurrentIdx++] = L"BlueprintAuthorityOnly"; }
	if (FunctionFlags & EFunctionFlags::BlueprintCosmetic) { StringFlags[CurrentIdx++] = L"BlueprintCosmetic"; }
	if (FunctionFlags & EFunctionFlags::Net) { StringFlags[CurrentIdx++] = L"Net"; }
	if (FunctionFlags & EFunctionFlags::NetReliable) { StringFlags[CurrentIdx++] = L"NetReliable"; }
	if (FunctionFlags & EFunctionFlags::NetRequest) { StringFlags[CurrentIdx++] = L"NetRequest"; }
	if (FunctionFlags & EFunctionFlags::Exec) { StringFlags[CurrentIdx++] = L"Exec"; }
	if (FunctionFlags & EFunctionFlags::Native) { StringFlags[CurrentIdx++] = L"Native"; }
	if (FunctionFlags & EFunctionFlags::Event) { StringFlags[CurrentIdx++] = L"Event"; }
	if (FunctionFlags & EFunctionFlags::NetResponse) { StringFlags[CurrentIdx++] = L"NetResponse"; }
	if (FunctionFlags & EFunctionFlags::Static) { StringFlags[CurrentIdx++] = L"Static"; }
	if (FunctionFlags & EFunctionFlags::NetMulticast) { StringFlags[CurrentIdx++] = L"NetMulticast"; }
	if (FunctionFlags & EFunctionFlags::UbergraphFunction) { StringFlags[CurrentIdx++] = L"UbergraphFunction"; }
	if (FunctionFlags & EFunctionFlags::MulticastDelegate) { StringFlags[CurrentIdx++] = L"MulticastDelegate"; }
	if (FunctionFlags & EFunctionFlags::Public) { StringFlags[CurrentIdx++] = L"Public"; }
	if (FunctionFlags & EFunctionFlags::Private) { StringFlags[CurrentIdx++] = L"Private"; }
	if (FunctionFlags & EFunctionFlags::Protected) { StringFlags[CurrentIdx++] = L"Protected"; }
	if (FunctionFlags & EFunctionFlags::Delegate) { StringFlags[CurrentIdx++] = L"Delegate"; }
	if (FunctionFlags & EFunctionFlags::NetServer) { StringFlags[CurrentIdx++] = L"NetServer"; }
	if (FunctionFlags & EFunctionFlags::HasOutParms) { StringFlags[CurrentIdx++] = L"HasOutParams"; }
	if (FunctionFlags & EFunctionFlags::HasDefaults) { StringFlags[CurrentIdx++] = L"HasDefaults"; }
	if (FunctionFlags & EFunctionFlags::NetClient) { StringFlags[CurrentIdx++] = L"NetClient"; }
	if (FunctionFlags & EFunctionFlags::DLLImport) { StringFlags[CurrentIdx++] = L"DLLImport"; }
	if (FunctionFlags & EFunctionFlags::BlueprintCallable) { StringFlags[CurrentIdx++] = L"BlueprintCallable"; }
	if (FunctionFlags & EFunctionFlags::BlueprintEvent) { StringFlags[CurrentIdx++] = L"BlueprintEvent"; }
	if (FunctionFlags & EFunctionFlags::BlueprintPure) { StringFlags[CurrentIdx++] = L"BlueprintPure"; }
	if (FunctionFlags & EFunctionFlags::EditorOnly) { StringFlags[CurrentIdx++] = L"EditorOnly"; }
	if (FunctionFlags & EFunctionFlags::Const) { StringFlags[CurrentIdx++] = L"Const"; }
	if (FunctionFlags & EFunctionFlags::NetValidate) { StringFlags[CurrentIdx++] = L"NetValidate"; }

	std::wstring RetFlags;
	RetFlags.reserve(CurrentIdx * 0xF);

	for (int i = 0; i < CurrentIdx; i++)
	{
		RetFlags += StringFlags[i];

		if (i != (CurrentIdx - 1))
			RetFlags += Seperator;
	}

	return RetFlags;
}

static std::wstring StringifyPropertyFlags(EPropertyFlags PropertyFlags)
{
	std::wstring RetFlags;

	if (PropertyFlags & EPropertyFlags::Edit) { RetFlags += L"Edit, L"; }
	if (PropertyFlags & EPropertyFlags::ConstParm) { RetFlags += L"ConstParm, L"; }
	if (PropertyFlags & EPropertyFlags::BlueprintVisible) { RetFlags += L"BlueprintVisible, L"; }
	if (PropertyFlags & EPropertyFlags::ExportObject) { RetFlags += L"ExportObject, L"; }
	if (PropertyFlags & EPropertyFlags::BlueprintReadOnly) { RetFlags += L"BlueprintReadOnly, L"; }
	if (PropertyFlags & EPropertyFlags::Net) { RetFlags += L"Net, L"; }
	if (PropertyFlags & EPropertyFlags::EditFixedSize) { RetFlags += L"EditFixedSize, L"; }
	if (PropertyFlags & EPropertyFlags::Parm) { RetFlags += L"Parm, L"; }
	if (PropertyFlags & EPropertyFlags::OutParm) { RetFlags += L"OutParm, L"; }
	if (PropertyFlags & EPropertyFlags::ZeroConstructor) { RetFlags += L"ZeroConstructor, L"; }
	if (PropertyFlags & EPropertyFlags::ReturnParm) { RetFlags += L"ReturnParm, L"; }
	if (PropertyFlags & EPropertyFlags::DisableEditOnTemplate) { RetFlags += L"DisableEditOnTemplate, L"; }
	if (PropertyFlags & EPropertyFlags::Transient) { RetFlags += L"Transient, L"; }
	if (PropertyFlags & EPropertyFlags::Config) { RetFlags += L"Config, L"; }
	if (PropertyFlags & EPropertyFlags::DisableEditOnInstance) { RetFlags += L"DisableEditOnInstance, L"; }
	if (PropertyFlags & EPropertyFlags::EditConst) { RetFlags += L"EditConst, L"; }
	if (PropertyFlags & EPropertyFlags::GlobalConfig) { RetFlags += L"GlobalConfig, L"; }
	if (PropertyFlags & EPropertyFlags::InstancedReference) { RetFlags += L"InstancedReference, L"; }
	if (PropertyFlags & EPropertyFlags::DuplicateTransient) { RetFlags += L"DuplicateTransient, L"; }
	if (PropertyFlags & EPropertyFlags::SubobjectReference) { RetFlags += L"SubobjectReference, L"; }
	if (PropertyFlags & EPropertyFlags::SaveGame) { RetFlags += L"SaveGame, L"; }
	if (PropertyFlags & EPropertyFlags::NoClear) { RetFlags += L"NoClear, L"; }
	if (PropertyFlags & EPropertyFlags::ReferenceParm) { RetFlags += L"ReferenceParm, L"; }
	if (PropertyFlags & EPropertyFlags::BlueprintAssignable) { RetFlags += L"BlueprintAssignable, L"; }
	if (PropertyFlags & EPropertyFlags::Deprecated) { RetFlags += L"Deprecated, L"; }
	if (PropertyFlags & EPropertyFlags::IsPlainOldData) { RetFlags += L"IsPlainOldData, L"; }
	if (PropertyFlags & EPropertyFlags::RepSkip) { RetFlags += L"RepSkip, L"; }
	if (PropertyFlags & EPropertyFlags::RepNotify) { RetFlags += L"RepNotify, L"; }
	if (PropertyFlags & EPropertyFlags::Interp) { RetFlags += L"Interp, L"; }
	if (PropertyFlags & EPropertyFlags::NonTransactional) { RetFlags += L"NonTransactional, L"; }
	if (PropertyFlags & EPropertyFlags::EditorOnly) { RetFlags += L"EditorOnly, L"; }
	if (PropertyFlags & EPropertyFlags::NoDestructor) { RetFlags += L"NoDestructor, L"; }
	if (PropertyFlags & EPropertyFlags::AutoWeak) { RetFlags += L"AutoWeak, L"; }
	if (PropertyFlags & EPropertyFlags::ContainsInstancedReference) { RetFlags += L"ContainsInstancedReference, L"; }
	if (PropertyFlags & EPropertyFlags::AssetRegistrySearchable) { RetFlags += L"AssetRegistrySearchable, L"; }
	if (PropertyFlags & EPropertyFlags::SimpleDisplay) { RetFlags += L"SimpleDisplay, L"; }
	if (PropertyFlags & EPropertyFlags::AdvancedDisplay) { RetFlags += L"AdvancedDisplay, L"; }
	if (PropertyFlags & EPropertyFlags::Protected) { RetFlags += L"Protected, L"; }
	if (PropertyFlags & EPropertyFlags::BlueprintCallable) { RetFlags += L"BlueprintCallable, L"; }
	if (PropertyFlags & EPropertyFlags::BlueprintAuthorityOnly) { RetFlags += L"BlueprintAuthorityOnly, L"; }
	if (PropertyFlags & EPropertyFlags::TextExportTransient) { RetFlags += L"TextExportTransient, L"; }
	if (PropertyFlags & EPropertyFlags::NonPIEDuplicateTransient) { RetFlags += L"NonPIEDuplicateTransient, L"; }
	if (PropertyFlags & EPropertyFlags::ExposeOnSpawn) { RetFlags += L"ExposeOnSpawn, L"; }
	if (PropertyFlags & EPropertyFlags::PersistentInstance) { RetFlags += L"PersistentInstance, L"; }
	if (PropertyFlags & EPropertyFlags::UObjectWrapper) { RetFlags += L"UObjectWrapper, L"; }
	if (PropertyFlags & EPropertyFlags::HasGetValueTypeHash) { RetFlags += L"HasGetValueTypeHash, L"; }
	if (PropertyFlags & EPropertyFlags::NativeAccessSpecifierPublic) { RetFlags += L"NativeAccessSpecifierPublic, L"; }
	if (PropertyFlags & EPropertyFlags::NativeAccessSpecifierProtected) { RetFlags += L"NativeAccessSpecifierProtected, L"; }
	if (PropertyFlags & EPropertyFlags::NativeAccessSpecifierPrivate) { RetFlags += L"NativeAccessSpecifierPrivate, L"; }

	return RetFlags.size() > 2 ? RetFlags.erase(RetFlags.size() - 2) : RetFlags;
}

static std::wstring StringifyObjectFlags(EObjectFlags ObjFlags)
{
	std::wstring RetFlags;

	if (ObjFlags & EObjectFlags::Public) { RetFlags += L"Public, L"; }
	if (ObjFlags & EObjectFlags::Standalone) { RetFlags += L"Standalone, L"; }
	if (ObjFlags & EObjectFlags::MarkAsNative) { RetFlags += L"MarkAsNative, L"; }
	if (ObjFlags & EObjectFlags::Transactional) { RetFlags += L"Transactional, L"; }
	if (ObjFlags & EObjectFlags::ClassDefaultObject) { RetFlags += L"ClassDefaultObject, L"; }
	if (ObjFlags & EObjectFlags::ArchetypeObject) { RetFlags += L"ArchetypeObject, L"; }
	if (ObjFlags & EObjectFlags::Transient) { RetFlags += L"Transient, L"; }
	if (ObjFlags & EObjectFlags::MarkAsRootSet) { RetFlags += L"MarkAsRootSet, L"; }
	if (ObjFlags & EObjectFlags::TagGarbageTemp) { RetFlags += L"TagGarbageTemp, L"; }
	if (ObjFlags & EObjectFlags::NeedInitialization) { RetFlags += L"NeedInitialization, L"; }
	if (ObjFlags & EObjectFlags::NeedLoad) { RetFlags += L"NeedLoad, L"; }
	if (ObjFlags & EObjectFlags::KeepForCooker) { RetFlags += L"KeepForCooker, L"; }
	if (ObjFlags & EObjectFlags::NeedPostLoad) { RetFlags += L"NeedPostLoad, L"; }
	if (ObjFlags & EObjectFlags::NeedPostLoadSubobjects) { RetFlags += L"NeedPostLoadSubobjects, L"; }
	if (ObjFlags & EObjectFlags::NewerVersionExists) { RetFlags += L"NewerVersionExists, L"; }
	if (ObjFlags & EObjectFlags::BeginDestroyed) { RetFlags += L"BeginDestroyed, L"; }
	if (ObjFlags & EObjectFlags::FinishDestroyed) { RetFlags += L"FinishDestroyed, L"; }
	if (ObjFlags & EObjectFlags::BeingRegenerated) { RetFlags += L"BeingRegenerated, L"; }
	if (ObjFlags & EObjectFlags::DefaultSubObject) { RetFlags += L"DefaultSubObject, L"; }
	if (ObjFlags & EObjectFlags::WasLoaded) { RetFlags += L"WasLoaded, L"; }
	if (ObjFlags & EObjectFlags::TextExportTransient) { RetFlags += L"TextExportTransient, L"; }
	if (ObjFlags & EObjectFlags::LoadCompleted) { RetFlags += L"LoadCompleted, L"; }
	if (ObjFlags & EObjectFlags::InheritableComponentTemplate) { RetFlags += L"InheritableComponentTemplate, L"; }
	if (ObjFlags & EObjectFlags::DuplicateTransient) { RetFlags += L"DuplicateTransient, L"; }
	if (ObjFlags & EObjectFlags::StrongRefOnFrame) { RetFlags += L"StrongRefOnFrame, L"; }
	if (ObjFlags & EObjectFlags::NonPIEDuplicateTransient) { RetFlags += L"NonPIEDuplicateTransient, L"; }
	if (ObjFlags & EObjectFlags::Dynamic) { RetFlags += L"Dynamic, L"; }
	if (ObjFlags & EObjectFlags::WillBeLoaded) { RetFlags += L"WillBeLoaded, L"; }

	return RetFlags.size() > 2 ? RetFlags.erase(RetFlags.size() - 2) : RetFlags;
}

static std::wstring StringifyClassCastFlags(EClassCastFlags CastFlags)
{
	std::wstring RetFlags;

	if (CastFlags & EClassCastFlags::Field) { RetFlags += L"Field, L"; }
	if (CastFlags & EClassCastFlags::Int8Property) { RetFlags += L"Int8Property, L"; }
	if (CastFlags & EClassCastFlags::Enum) { RetFlags += L"Enum, L"; }
	if (CastFlags & EClassCastFlags::Struct) { RetFlags += L"Struct, L"; }
	if (CastFlags & EClassCastFlags::ScriptStruct) { RetFlags += L"ScriptStruct, L"; }
	if (CastFlags & EClassCastFlags::Class) { RetFlags += L"Class, L"; }
	if (CastFlags & EClassCastFlags::ByteProperty) { RetFlags += L"ByteProperty, L"; }
	if (CastFlags & EClassCastFlags::IntProperty) { RetFlags += L"IntProperty, L"; }
	if (CastFlags & EClassCastFlags::FloatProperty) { RetFlags += L"FloatProperty, L"; }
	if (CastFlags & EClassCastFlags::UInt64Property) { RetFlags += L"UInt64Property, L"; }
	if (CastFlags & EClassCastFlags::ClassProperty) { RetFlags += L"ClassProperty, L"; }
	if (CastFlags & EClassCastFlags::UInt32Property) { RetFlags += L"UInt32Property, L"; }
	if (CastFlags & EClassCastFlags::InterfaceProperty) { RetFlags += L"InterfaceProperty, L"; }
	if (CastFlags & EClassCastFlags::NameProperty) { RetFlags += L"NameProperty, L"; }
	if (CastFlags & EClassCastFlags::StrProperty) { RetFlags += L"StrProperty, L"; }
	if (CastFlags & EClassCastFlags::Property) { RetFlags += L"Property, L"; }
	if (CastFlags & EClassCastFlags::ObjectProperty) { RetFlags += L"ObjectProperty, L"; }
	if (CastFlags & EClassCastFlags::BoolProperty) { RetFlags += L"BoolProperty, L"; }
	if (CastFlags & EClassCastFlags::UInt16Property) { RetFlags += L"UInt16Property, L"; }
	if (CastFlags & EClassCastFlags::Function) { RetFlags += L"Function, L"; }
	if (CastFlags & EClassCastFlags::StructProperty) { RetFlags += L"StructProperty, L"; }
	if (CastFlags & EClassCastFlags::ArrayProperty) { RetFlags += L"ArrayProperty, L"; }
	if (CastFlags & EClassCastFlags::Int64Property) { RetFlags += L"Int64Property, L"; }
	if (CastFlags & EClassCastFlags::DelegateProperty) { RetFlags += L"DelegateProperty, L"; }
	if (CastFlags & EClassCastFlags::NumericProperty) { RetFlags += L"NumericProperty, L"; }
	if (CastFlags & EClassCastFlags::MulticastDelegateProperty) { RetFlags += L"MulticastDelegateProperty, L"; }
	if (CastFlags & EClassCastFlags::ObjectPropertyBase) { RetFlags += L"ObjectPropertyBase, L"; }
	if (CastFlags & EClassCastFlags::WeakObjectProperty) { RetFlags += L"WeakObjectProperty, L"; }
	if (CastFlags & EClassCastFlags::LazyObjectProperty) { RetFlags += L"LazyObjectProperty, L"; }
	if (CastFlags & EClassCastFlags::SoftObjectProperty) { RetFlags += L"SoftObjectProperty, L"; }
	if (CastFlags & EClassCastFlags::TextProperty) { RetFlags += L"TextProperty, L"; }
	if (CastFlags & EClassCastFlags::Int16Property) { RetFlags += L"Int16Property, L"; }
	if (CastFlags & EClassCastFlags::DoubleProperty) { RetFlags += L"DoubleProperty, L"; }
	if (CastFlags & EClassCastFlags::SoftClassProperty) { RetFlags += L"SoftClassProperty, L"; }
	if (CastFlags & EClassCastFlags::Package) { RetFlags += L"Package, L"; }
	if (CastFlags & EClassCastFlags::Level) { RetFlags += L"Level, L"; }
	if (CastFlags & EClassCastFlags::Actor) { RetFlags += L"Actor, L"; }
	if (CastFlags & EClassCastFlags::PlayerController) { RetFlags += L"PlayerController, L"; }
	if (CastFlags & EClassCastFlags::Pawn) { RetFlags += L"Pawn, L"; }
	if (CastFlags & EClassCastFlags::SceneComponent) { RetFlags += L"SceneComponent, L"; }
	if (CastFlags & EClassCastFlags::PrimitiveComponent) { RetFlags += L"PrimitiveComponent, L"; }
	if (CastFlags & EClassCastFlags::SkinnedMeshComponent) { RetFlags += L"SkinnedMeshComponent, L"; }
	if (CastFlags & EClassCastFlags::SkeletalMeshComponent) { RetFlags += L"SkeletalMeshComponent, L"; }
	if (CastFlags & EClassCastFlags::Blueprint) { RetFlags += L"Blueprint, L"; }
	if (CastFlags & EClassCastFlags::DelegateFunction) { RetFlags += L"DelegateFunction, L"; }
	if (CastFlags & EClassCastFlags::StaticMeshComponent) { RetFlags += L"StaticMeshComponent, L"; }
	if (CastFlags & EClassCastFlags::MapProperty) { RetFlags += L"MapProperty, L"; }
	if (CastFlags & EClassCastFlags::SetProperty) { RetFlags += L"SetProperty, L"; }
	if (CastFlags & EClassCastFlags::EnumProperty) { RetFlags += L"EnumProperty, L"; }
	if (CastFlags & EClassCastFlags::SparseDelegateFunction) { RetFlags += L"SparseDelegateFunction, L"; }
	if (CastFlags & EClassCastFlags::MulticastInlineDelegateProperty) { RetFlags += L"MulticastInlineDelegateProperty, L"; }
	if (CastFlags & EClassCastFlags::MulticastSparseDelegateProperty) { RetFlags += L"MulticastSparseDelegateProperty, L"; }
	if (CastFlags & EClassCastFlags::FieldPathProperty) { RetFlags += L"MarkAsFieldPathPropertyRootSet, L"; }
	if (CastFlags & EClassCastFlags::LargeWorldCoordinatesRealProperty) { RetFlags += L"LargeWorldCoordinatesRealProperty, L"; }
	if (CastFlags & EClassCastFlags::OptionalProperty) { RetFlags += L"OptionalProperty, L"; }
	if (CastFlags & EClassCastFlags::VValueProperty) { RetFlags += L"VValueProperty, L"; }
	if (CastFlags & EClassCastFlags::VerseVMClass) { RetFlags += L"VerseVMClass, L"; }
	if (CastFlags & EClassCastFlags::VRestValueProperty) { RetFlags += L"VRestValueProperty, L"; }

	return RetFlags.size() > 2 ? RetFlags.erase(RetFlags.size() - 2) : RetFlags;
}
