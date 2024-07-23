#pragma once

#include <vector>
#include <unordered_map>
#include "Enums.h"
#include "UnrealTypes.h"

class UEClass;
class UEFField;
class UEObject;
class UEProperty;

class UEFFieldClass
{
protected:
	uint8* Class;

public:

	UEFFieldClass() = default;

	UEFFieldClass(void* NewFieldClass)
		: Class(reinterpret_cast<uint8*>(NewFieldClass))
	{
	}

	UEFFieldClass(const UEFFieldClass& OldFieldClass)
		: Class(reinterpret_cast<uint8*>(OldFieldClass.Class))
	{
	}

	void* GetAddress();

	operator bool() const;

	EFieldClassID GetId() const;

	EClassCastFlags GetCastFlags() const;
	EClassFlags GetClassFlags() const;
	UEFFieldClass GetSuper() const;
	FName GetFName() const;

	bool IsType(EClassCastFlags Flags) const;

	std::wstring GetName() const;
	std::wstring GetValidName() const;
	std::wstring GetCppName() const;
};

class UEFField
{
protected:
	uint8* Field;

public:

	UEFField() = default;

	UEFField(void* NewField)
		: Field(reinterpret_cast<uint8*>(NewField))
	{
	}

	UEFField(const UEFField& OldField)
		: Field(reinterpret_cast<uint8*>(OldField.Field))
	{
	}

	void* GetAddress();

	EObjectFlags GetFlags() const;
	class UEObject GetOwnerAsUObject() const;
	class UEFField GetOwnerAsFField() const;
	class UEObject GetOwnerUObject() const;
	UEFFieldClass GetClass() const;
	FName GetFName() const;
	UEFField GetNext() const;

	template<typename UEType>
	UEType Cast() const;

	bool IsOwnerUObject() const;
	bool IsA(EClassCastFlags Flags) const;

	std::wstring GetName() const;
	std::wstring GetValidName() const;
	std::wstring GetCppName() const;

	explicit operator bool() const;
	bool operator==(const UEFField& Other) const;
	bool operator!=(const UEFField& Other) const;
};

class UEObject
{
private:
	static void(*PE)(void*, void*, void*);

protected:
	uint8* Object;

public:

	UEObject() = default;

	UEObject(void* NewObject)
		: Object(reinterpret_cast<uint8*>(NewObject))
	{
	}

	UEObject(const UEObject&) = default;

	void* GetAddress();

	void* GetVft() const;
	EObjectFlags GetFlags() const;
	int32 GetIndex() const;
	UEClass GetClass() const;
	FName GetFName() const;
	UEObject GetOuter() const;

	int32 GetPackageIndex() const;

	bool HasAnyFlags(EObjectFlags Flags) const;

	bool IsA(EClassCastFlags TypeFlags) const;
	bool IsA(UEClass Class) const;

	UEObject GetOutermost() const;

	std::wstring StringifyObjFlags() const;

	std::wstring GetName() const;
	std::wstring GetNameWithPath() const;
	std::wstring GetValidName() const;
	std::wstring GetCppName() const;
	std::wstring GetFullName(int32& OutNameLength) const;
	std::wstring GetFullName() const;
	std::wstring GetPathName() const;

	explicit operator bool() const;
	explicit operator uint8*();
	bool operator==(const UEObject& Other) const;
	bool operator!=(const UEObject& Other) const;

	void ProcessEvent(class UEFunction Func, void* Params);

public:
	template<typename UEType>
	inline UEType Cast()
	{
		return UEType(Object);
	}

	template<typename UEType>
	inline const UEType Cast() const
	{
		return UEType(Object);
	}
};

class UEField : public UEObject
{
	using UEObject::UEObject;

public:
	UEField GetNext() const;
	bool IsNextValid() const;
};

class UEEnum : public UEField
{
	using UEField::UEField;

public:
	EEnumFlags GetEnumFlags() const;

	std::vector<std::pair<FName, int64>> GetNameValuePairs() const;
	std::wstring GetSingleName(int32 Index) const;
	std::wstring GetEnumPrefixedName() const;
	std::wstring GetEnumTypeAsStr() const;
};

class UEStruct : public UEField
{
	using UEField::UEField;

public:
	UEStruct GetSuper() const;
	UEField GetChild() const;
	UEFField GetChildProperties() const;
	int32 GetMinAlignment() const;
	int32 GetStructSize() const;

	std::vector<UEProperty> GetProperties() const;
	std::vector<UEFunction> GetFunctions() const;


	UEProperty FindMember(const std::wstring& MemberName, EClassCastFlags TypeFlags = EClassCastFlags::None) const;

	bool HasMembers() const;
};

class UEFunction : public UEStruct
{
	using UEStruct::UEStruct;

public:
	EFunctionFlags GetFunctionFlags() const;
	bool HasFlags(EFunctionFlags Flags) const;

	void* GetExecFunction() const;

	UEProperty GetReturnProperty() const;

	std::wstring StringifyFlags(const wchar_t* Seperator = L", L") const;
	std::wstring GetParamStructName() const;
};

class UEClass : public UEStruct
{
	using UEStruct::UEStruct;

public:
	EClassCastFlags GetCastFlags() const;
	std::wstring StringifyCastFlags() const;
	bool IsType(EClassCastFlags TypeFlag) const;
	bool HasType(UEClass TypeClass) const;
	UEObject GetDefaultObject() const;

	UEFunction GetFunction(const std::wstring& ClassName, const std::wstring& FuncName) const;
};

class UEProperty
{
protected:
	uint8* Base;

public:
	UEProperty() = default;
	UEProperty(const UEProperty&) = default;

	UEProperty(void* NewProperty)
		: Base(reinterpret_cast<uint8*>(NewProperty))
	{
	}

public:
	void* GetAddress();

	std::pair<UEClass, UEFFieldClass> GetClass() const;
	EClassCastFlags GetCastFlags() const;

	operator bool() const;

	bool IsA(EClassCastFlags TypeFlags) const;

	FName GetFName() const;
	int32 GetArrayDim() const;
	int32 GetSize() const;
	int32 GetOffset() const;
	EPropertyFlags GetPropertyFlags() const;
	bool HasPropertyFlags(EPropertyFlags PropertyFlag) const;
	bool IsType(EClassCastFlags PossibleTypes) const;

	int32 GetAlignment() const;

	std::wstring GetName() const;
	std::wstring GetValidName() const;

	std::wstring GetCppType() const;

	std::wstring StringifyFlags() const;

public:
	template<typename UEType>
	UEType Cast()
	{
		return UEType(Base);
	}

	template<typename UEType>
	const UEType Cast() const
	{
		return UEType(Base);
	}
};

class UEByteProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEEnum GetEnum() const;

	std::wstring GetCppType() const;
};

class UEBoolProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	uint8 GetFieldMask() const;
	uint8 GetBitIndex() const;
	bool IsNativeBool() const;

	std::wstring GetCppType() const;
};

class UEObjectProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEClass GetPropertyClass() const;

	std::wstring GetCppType() const;
};

class UEClassProperty : public UEObjectProperty
{
	using UEObjectProperty::UEObjectProperty;

public:
	UEClass GetMetaClass() const;

	std::wstring GetCppType() const;
};

class UEWeakObjectProperty : public UEObjectProperty
{
	using UEObjectProperty::UEObjectProperty;

public:
	std::wstring GetCppType() const;
};

class UELazyObjectProperty : public UEObjectProperty
{
	using UEObjectProperty::UEObjectProperty;

public:
	std::wstring GetCppType() const;
};

class UESoftObjectProperty : public UEObjectProperty
{
	using UEObjectProperty::UEObjectProperty;

public:
	std::wstring GetCppType() const;
};

class UESoftClassProperty : public UEClassProperty
{
	using UEClassProperty::UEClassProperty;

public:
	std::wstring GetCppType() const;
};

class UEInterfaceProperty : public UEObjectProperty
{
	using UEObjectProperty::UEObjectProperty;

public:
	std::wstring GetCppType() const;
};

class UEStructProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEStruct GetUnderlayingStruct() const;

	std::wstring GetCppType() const;
};

class UEArrayProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEProperty GetInnerProperty() const;

	std::wstring GetCppType() const;
};

class UEDelegateProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEFunction GetSignatureFunction() const;

	std::wstring GetCppType() const;
};

class UEMapProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEProperty GetKeyProperty() const;
	UEProperty GetValueProperty() const;

	std::wstring GetCppType() const;
};

class UESetProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEProperty GetElementProperty() const;

	std::wstring GetCppType() const;
};

class UEEnumProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEProperty GetUnderlayingProperty() const;
	UEEnum GetEnum() const;

	std::wstring GetCppType() const;
};

class UEFieldPathProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEFFieldClass GetFielClass() const;

	std::wstring GetCppType() const;
};

class UEOptionalProperty : public UEProperty
{
	using UEProperty::UEProperty;

public:
	UEProperty GetValueProperty() const;

	std::wstring GetCppType() const;
};

