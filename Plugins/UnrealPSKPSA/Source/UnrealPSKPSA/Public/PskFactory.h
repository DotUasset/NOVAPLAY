#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "PskFactory.generated.h"

UCLASS()
class UNREALPSKPSA_API UPskFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()
public:
	UPskFactory()
	{
		bEditorImport = true;
		bText = false;

		Formats.Add(FactoryExtension + ";" + FactoryDescription);

		SupportedClass = FactoryClass;
	}
	
	// ExistingObject: when set (reimport), the mesh is rebuilt in place instead of creating a new asset.
	static UObject* Import(const FString& Filename, UObject* Parent, const FName Name, const EObjectFlags Flags, TMap<FString, FString>
	                       MaterialNameToPathMap, UObject* ExistingObject = nullptr);
	static void ProcessSkeleton(const FSkeletalMeshImportData&    ImportData,
								const USkeleton*                  Skeleton,
								FReferenceSkeleton&               OutRefSkeleton,
								int32&                            OutSkeletalDepth);

	// FReimportHandler
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;

protected:
	UClass* FactoryClass = USkeletalMesh::StaticClass();
	FString FactoryExtension = "psk";
	FString FactoryDescription = "Unreal Skeletal Mesh";

	virtual bool FactoryCanImport(const FString& Filename) override
	{
		const auto Extension = FPaths::GetExtension(Filename);
		return Extension.Equals(FactoryExtension);
	}
	
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Params, FFeedbackContext* Warn, bool& bOutOperationCanceled) override
	{
		return Import(Filename, InParent, FName(*InName.ToString().Replace(TEXT("_LOD0"), TEXT(""))), Flags, TMap<FString, FString>());
	}
	
};
