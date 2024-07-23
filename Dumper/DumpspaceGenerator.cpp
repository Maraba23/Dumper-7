#include "DumpspaceGenerator.h"

inline std::string ConvWStrToStr(const std::wstring& WStr)
{
	return std::string(WStr.begin(), WStr.end());
}

inline std::wstring ConvStrToWStr(const std::string& Str)
{
	return std::wstring(Str.begin(), Str.end());
}

std::wstring DumpspaceGenerator::GetStructPrefixedName(const StructWrapper& Struct)
{
	if (Struct.IsFunction())
		return Struct.GetUnrealStruct().GetOuter().GetValidName() + L"_" + Struct.GetName();

	auto [ValidName, bIsUnique] = Struct.GetUniqueName();

	if (bIsUnique) [[likely]]
		return ValidName;

	/* Package::FStructName */
	return PackageManager::GetName(Struct.GetUnrealStruct().GetPackageIndex()) + L"::" + ValidName;
}

std::wstring DumpspaceGenerator::GetEnumPrefixedName(const EnumWrapper& Enum)
{
	auto [ValidName, bIsUnique] = Enum.GetUniqueName();

	if (bIsUnique) [[likely]]
		return ValidName;

	/* Package::ESomeEnum */
	return PackageManager::GetName(Enum.GetUnrealEnum().GetPackageIndex()) + L"::" + ValidName;
}

std::wstring DumpspaceGenerator::EnumSizeToType(const int32 Size)
{
	static constexpr std::array<const wchar_t*, 8> UnderlayingTypesBySize = {
		L"uint8",
		L"uint16",
		L"InvalidEnumSize",
		L"uint32",
		L"InvalidEnumSize",
		L"InvalidEnumSize",
		L"InvalidEnumSize",
		L"uint64"
	};

	return Size <= 0x8 ? UnderlayingTypesBySize[static_cast<size_t>(Size) - 1] : L"uint8";
}

DSGen::EType DumpspaceGenerator::GetMemberEType(const PropertyWrapper& Property)
{
	/* Predefined members are currently not supported by DumpspaceGenerator */
	if (!Property.IsUnrealProperty())
		return DSGen::ET_Default;

	return GetMemberEType(Property.GetUnrealProperty());
}

DSGen::EType DumpspaceGenerator::GetMemberEType(UEProperty Prop)
{
	if (Prop.IsA(EClassCastFlags::EnumProperty))
	{
		return DSGen::ET_Enum;
	}
	else if (Prop.IsA(EClassCastFlags::ByteProperty))
	{
		if (Prop.Cast<UEByteProperty>().GetEnum())
			return DSGen::ET_Enum;
	}
	//else if (Prop.IsA(EClassCastFlags::ClassProperty))
	//{
	//	/* Check if this is a UClass*, not TSubclassof<UObject> */
	//	if (!Prop.Cast<UEClassProperty>().HasPropertyFlags(EPropertyFlags::UObjectWrapper))
	//		return DSGen::ET_Class; 
	//}
	else if (Prop.IsA(EClassCastFlags::ObjectProperty))
	{
		return DSGen::ET_Class;
	}
	else if (Prop.IsA(EClassCastFlags::StructProperty))
	{
		return DSGen::ET_Struct;
	}
	else if (Prop.IsType(EClassCastFlags::ArrayProperty | EClassCastFlags::MapProperty | EClassCastFlags::SetProperty))
	{
		return DSGen::ET_Class;
	}

	return DSGen::ET_Default;
}

std::wstring DumpspaceGenerator::GetMemberTypeStr(UEProperty Property, std::string& OutExtendedType, std::vector<DSGen::MemberType>& OutSubtypes)
{
	UEProperty Member = Property;

	auto [Class, FieldClass] = Member.GetClass();

	EClassCastFlags Flags = Class ? Class.GetCastFlags() : FieldClass.GetCastFlags();

	if (Flags & EClassCastFlags::ByteProperty)
	{
		if (UEEnum Enum = Member.Cast<UEByteProperty>().GetEnum())
			return GetEnumPrefixedName(Enum);

		return L"uint8";
	}
	else if (Flags & EClassCastFlags::UInt16Property)
	{
		return L"uint16";
	}
	else if (Flags & EClassCastFlags::UInt32Property)
	{
		return L"uint32";
	}
	else if (Flags & EClassCastFlags::UInt64Property)
	{
		return L"uint64";
	}
	else if (Flags & EClassCastFlags::Int8Property)
	{
		return L"int8";
	}
	else if (Flags & EClassCastFlags::Int16Property)
	{
		return L"int16";
	}
	else if (Flags & EClassCastFlags::IntProperty)
	{
		return L"int32";
	}
	else if (Flags & EClassCastFlags::Int64Property)
	{
		return L"int64";
	}
	else if (Flags & EClassCastFlags::FloatProperty)
	{
		return L"float";
	}
	else if (Flags & EClassCastFlags::DoubleProperty)
	{
		return L"double";
	}
	else if (Flags & EClassCastFlags::ClassProperty)
	{
		if (Member.HasPropertyFlags(EPropertyFlags::UObjectWrapper))
		{
			OutSubtypes.emplace_back(GetMemberType(Member.Cast<UEClassProperty>().GetMetaClass()));

			return L"TSubclassOf";
		}

		OutExtendedType = "*";

		return L"UClass";
	}
	else if (Flags & EClassCastFlags::NameProperty)
	{
		return L"FName";
	}
	else if (Flags & EClassCastFlags::StrProperty)
	{
		return L"FString";
	}
	else if (Flags & EClassCastFlags::TextProperty)
	{
		return L"FText";
	}
	else if (Flags & EClassCastFlags::BoolProperty)
	{
		return Member.Cast<UEBoolProperty>().IsNativeBool() ? L"bool" : L"uint8";
	}
	else if (Flags & EClassCastFlags::StructProperty)
	{
		const StructWrapper& UnderlayingStruct = Member.Cast<UEStructProperty>().GetUnderlayingStruct();

		return GetStructPrefixedName(UnderlayingStruct);
	}
	else if (Flags & EClassCastFlags::ArrayProperty)
	{
		OutSubtypes.push_back(GetMemberType(Member.Cast<UEArrayProperty>().GetInnerProperty()));

		return L"TArray";
	}
	else if (Flags & EClassCastFlags::WeakObjectProperty)
	{
		if (UEClass PropertyClass = Member.Cast<UEWeakObjectProperty>().GetPropertyClass()) 
		{
			OutSubtypes.push_back(GetMemberType(PropertyClass));
		}
		else
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Class, L"UObject"));
		}

		return L"TWeakObjectPtr";
	}
	else if (Flags & EClassCastFlags::LazyObjectProperty)
	{
		if (UEClass PropertyClass = Member.Cast<UELazyObjectProperty>().GetPropertyClass())
		{
			OutSubtypes.push_back(GetMemberType(PropertyClass));
		}
		else
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Class, L"UObject"));
		}

		return L"TLazyObjectPtr";
	}
	else if (Flags & EClassCastFlags::SoftClassProperty)
	{
		if (UEClass PropertyClass = Member.Cast<UESoftClassProperty>().GetPropertyClass())
		{
			OutSubtypes.push_back(GetMemberType(PropertyClass));
		}
		else
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Class, L"UClass"));
		}

		return L"TSoftClassPtr";
	}
	else if (Flags & EClassCastFlags::SoftObjectProperty)
	{
		if (UEClass PropertyClass = Member.Cast<UESoftObjectProperty>().GetPropertyClass())
		{
			OutSubtypes.push_back(GetMemberType(PropertyClass));
		}
		else
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Class, L"UObject"));
		}

		return L"TSoftObjectPtr";
	}
	else if (Flags & EClassCastFlags::ObjectProperty)
	{
		OutExtendedType = "*";

		if (UEClass PropertyClass = Member.Cast<UEObjectProperty>().GetPropertyClass())
			return GetStructPrefixedName(PropertyClass);
		
		return L"UObject";
	}
	else if (Flags & EClassCastFlags::MapProperty)
	{
		UEMapProperty MemberAsMapProperty = Member.Cast<UEMapProperty>();

		OutSubtypes.emplace_back(GetMemberType(Member.Cast<UEMapProperty>().GetKeyProperty()));
		OutSubtypes.emplace_back(GetMemberType(Member.Cast<UEMapProperty>().GetValueProperty()));

		return L"TMap";
	}
	else if (Flags & EClassCastFlags::SetProperty)
	{
		OutSubtypes.emplace_back(GetMemberType(Member.Cast<UESetProperty>().GetElementProperty()));

		return L"TSet";
	}
	else if (Flags & EClassCastFlags::EnumProperty)
	{
		if (UEEnum Enum = Member.Cast<UEEnumProperty>().GetEnum())
			return GetEnumPrefixedName(Enum);

		return L"NamelessEnumIGuessIdkWhatToPutHereWithRegardsTheGuyFromDumper7";
	}
	else if (Flags & EClassCastFlags::InterfaceProperty)
	{
		if (UEClass PropertyClass = Member.Cast<UEInterfaceProperty>().GetPropertyClass())
		{
			OutSubtypes.push_back(GetMemberType(PropertyClass));
		}
		else
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Class, L"IInterface"));
		}

		return L"TScriptInterface";
	}
	else if (Flags & EClassCastFlags::FieldPathProperty)
	{
		if (UEFFieldClass PropertyClass = Member.Cast<UEFieldPathProperty>().GetFielClass())
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Struct, PropertyClass.GetCppName()));
		}
		else
		{
			OutSubtypes.push_back(ManualCreateMemberType(DSGen::ET_Struct, L"FField"));
		}

		return L"TFieldPath";
	}
	else if (Flags & EClassCastFlags::OptionalProperty)
	{
		UEProperty ValueProperty = Member.Cast<UEOptionalProperty>().GetValueProperty();

		OutSubtypes.push_back(GetMemberType(ValueProperty));

		return L"TOptional";
	}
	else
	{
		/* When changing this also change 'GetUnknownProperties()' */
		return (Class ? Class.GetCppName() : FieldClass.GetCppName()) + L"_";
	}
}

DSGen::MemberType DumpspaceGenerator::GetMemberType(const StructWrapper& Struct)
{
	DSGen::MemberType Type;
	Type.type = Struct.IsClass() ? DSGen::ET_Class : DSGen::ET_Struct;
	Type.typeName = ConvWStrToStr(GetStructPrefixedName(Struct));
	Type.extendedType = Struct.IsClass() ? "*" : "";
	Type.reference = false;

	return Type;
}

DSGen::MemberType DumpspaceGenerator::GetMemberType(const PropertyWrapper& Property, bool bIsReference)
{
	DSGen::MemberType Type;

	if (!Property.IsUnrealProperty())
	{
		Type.typeName = "Unsupported_Predefined_Member";
		return Type;
	}

	Type.reference = bIsReference;
	Type.type = GetMemberEType(Property);
	Type.typeName = ConvWStrToStr(GetMemberTypeStr(Property.GetUnrealProperty(), Type.extendedType, Type.subTypes));

	return Type;
}

DSGen::MemberType DumpspaceGenerator::GetMemberType(UEProperty Property, bool bIsReference)
{
	DSGen::MemberType Type;

	Type.reference = bIsReference;
	Type.type = GetMemberEType(Property);
	Type.typeName = ConvWStrToStr(GetMemberTypeStr(Property, Type.extendedType, Type.subTypes));

	return Type;
}

DSGen::MemberType DumpspaceGenerator::ManualCreateMemberType(DSGen::EType Type, const std::wstring& TypeName, const std::wstring& ExtendedType)
{
	return DSGen::createMemberType(Type, ConvWStrToStr(TypeName), ConvWStrToStr(ExtendedType));
}

void DumpspaceGenerator::AddMemberToStruct(DSGen::ClassHolder& Struct, const PropertyWrapper& Property)
{
	DSGen::MemberDefinition Member;
	Member.memberType = GetMemberType(Property);
	Member.bitOffset = Property.IsBitField() ? Property.GetBitIndex() : -1;
	Member.offset = Property.GetOffset();
	Member.size = Property.GetSize();
	Member.memberName = ConvWStrToStr(Property.GetName());

	Struct.members.push_back(std::move(Member));
}

void DumpspaceGenerator::RecursiveGetSuperClasses(const StructWrapper& Struct, std::vector<std::string>& OutSupers)
{
	const StructWrapper& Super = Struct.GetSuper();

	OutSupers.push_back(ConvWStrToStr(Struct.GetUniqueName().first));

	if (Super.IsValid())
		RecursiveGetSuperClasses(Super, OutSupers);
}

std::vector<std::string> DumpspaceGenerator::GetSuperClasses(const StructWrapper& Struct)
{
	std::vector<std::string> RetSuperNames;

	const StructWrapper& Super = Struct.GetSuper();

	if (Super.IsValid())
		RecursiveGetSuperClasses(Super, RetSuperNames);

	return RetSuperNames;
}

DSGen::ClassHolder DumpspaceGenerator::GenerateStruct(const StructWrapper& Struct)
{
	DSGen::ClassHolder StructOrClass;
	StructOrClass.className = ConvWStrToStr(GetStructPrefixedName(Struct));
	StructOrClass.classSize = Struct.GetSize();
	StructOrClass.classType = Struct.IsClass() ? DSGen::ET_Class : DSGen::ET_Struct;
	StructOrClass.interitedTypes = GetSuperClasses(Struct);

	MemberManager Members = Struct.GetMembers();

	for (const PropertyWrapper& Wrapper : Members.IterateMembers())
		AddMemberToStruct(StructOrClass, Wrapper);

	if (!Struct.IsClass())
		return StructOrClass;

	for (const FunctionWrapper& Wrapper : Members.IterateFunctions())
		StructOrClass.functions.push_back(GenearateFunction(Wrapper));

	return StructOrClass;
}

DSGen::EnumHolder DumpspaceGenerator::GenerateEnum(const EnumWrapper& Enum)
{
	DSGen::EnumHolder Enumerator;
	Enumerator.enumName = ConvWStrToStr(GetEnumPrefixedName(Enum));
	Enumerator.enumType = ConvWStrToStr(EnumSizeToType(Enum.GetUnderlyingTypeSize()));

	Enumerator.enumMembers.reserve(Enum.GetNumMembers());
		
	for (const EnumCollisionInfo& Info : Enum.GetMembers())
		Enumerator.enumMembers.emplace_back(ConvWStrToStr(Info.GetUniqueName()), Info.GetValue());

	return Enumerator;
}

DSGen::FunctionHolder DumpspaceGenerator::GenearateFunction(const FunctionWrapper& Function)
{
	DSGen::FunctionHolder RetFunc;

	StructWrapper FuncAsStruct = Function.AsStruct();
	MemberManager FuncParams = FuncAsStruct.GetMembers();

	RetFunc.functionName = ConvWStrToStr(Function.GetName());
	RetFunc.functionOffset = Function.GetExecFuncOffset();
	RetFunc.functionFlags = ConvWStrToStr(Function.StringifyFlags(L"|"));
	RetFunc.returnType = ManualCreateMemberType(DSGen::ET_Default, L"void");

	for (const PropertyWrapper& Param : FuncParams.IterateMembers())
	{
		if (!Param.HasPropertyFlags(EPropertyFlags::Parm))
			continue;

		if (Param.HasPropertyFlags(EPropertyFlags::ReturnParm))
		{
			RetFunc.returnType = GetMemberType(Param);
			continue;
		}

		RetFunc.functionParams.emplace_back(GetMemberType(Param), ConvWStrToStr(Param.GetName()));
	}

	return RetFunc;
}

void DumpspaceGenerator::GeneratedStaticOffsets()
{
	DSGen::addOffset("OFFSET_GOBJECTS", Off::InSDK::ObjArray::GObjects);
	DSGen::addOffset(Off::InSDK::Name::bIsUsingAppendStringOverToString ? "OFFSET_APPENDSTRING" : "OFFSET_TOSTRING", Off::InSDK::Name::AppendNameToString);
	DSGen::addOffset("OFFSET_GNAMES", Off::InSDK::NameArray::GNames);
	DSGen::addOffset("OFFSET_GWORLD", Off::InSDK::World::GWorld);
	DSGen::addOffset("OFFSET_PROCESSEVENT", Off::InSDK::ProcessEvent::PEOffset);
	DSGen::addOffset("INDEX_PROCESSEVENT", Off::InSDK::ProcessEvent::PEIndex);
}

void DumpspaceGenerator::Generate()
{
	/* Set the output directory of DSGen to "...GenerationPath/GameVersion-GameName/Dumespace" */
	DSGen::setDirectory(MainFolder);

	/* Add offsets for GObjects, GNames, GWorld, AppendString, PrcessEvent and ProcessEventIndex*/
	GeneratedStaticOffsets();

	// Generates all packages and writes them to files
	for (PackageInfoHandle Package : PackageManager::IterateOverPackageInfos())
	{
		if (Package.IsEmpty())
			continue;

		/*
		* Generate classes/structs/enums/functions directly into the respective files
		*
		* Note: Some filestreams aren't opened but passed as parameters anyway because the function demands it, they are not used if they are closed
		*/
		for (int32 EnumIdx : Package.GetEnums())
		{
			DSGen::EnumHolder Enum = GenerateEnum(ObjectArray::GetByIndex<UEEnum>(EnumIdx));
			DSGen::bakeEnum(Enum);
		}

		DependencyManager::OnVisitCallbackType GenerateClassOrStructCallback = [&](int32 Index) -> void
		{
			DSGen::ClassHolder StructOrClass = GenerateStruct(ObjectArray::GetByIndex<UEStruct>(Index));
			DSGen::bakeStructOrClass(StructOrClass);
		};

		if (Package.HasStructs())
		{
			const DependencyManager& Structs = Package.GetSortedStructs();

			Structs.VisitAllNodesWithCallback(GenerateClassOrStructCallback);
		}

		if (Package.HasClasses())
		{
			const DependencyManager& Classes = Package.GetSortedClasses();

			Classes.VisitAllNodesWithCallback(GenerateClassOrStructCallback);
		}
	}

	DSGen::dump();
}