#include "PskFactory.h"

#include "IMeshBuilderModule.h"
#include "PskPsaUtils.h"
#include "PskReader.h"
#include "UnrealPSKPSA.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"

UObject* UPskFactory::Import(const FString& Filename, UObject* Parent, const FName Name, const EObjectFlags Flags, TMap<FString, FString> MaterialNameToPathMap)
{
	auto Data = FPskReader(Filename);
	if (!Data.bIsValid) return nullptr;

	// This file has no skeleton/no vertex weights - it isn't actually a skinned mesh, and building
	// a USkeletalMesh with zero bones can crash the editor later (thumbnail rendering, GPU skinning
	// setup, etc. all assume at least one bone exists). Fail cleanly instead and point the user at
	// the correct importer for this data.
	if (Data.Bones.Num() == 0 || Data.Influences.Num() == 0)
	{
		UE_LOG(LogUnrealPSKPSA, Error, TEXT("'%s' has no skeleton/vertex weight data (%d bones, %d influences) - it isn't a skinned mesh. Rename it to .pskx and import it as a static mesh instead."), *Filename, Data.Bones.Num(), Data.Influences.Num());
		return nullptr;
	}

	TArray<FColor> VertexColorsByPoint;
	VertexColorsByPoint.Init(FColor::Black, Data.VertexColors.Num());
	if (Data.bHasVertexColors)
	{
		for (auto i = 0; i < Data.Wedges.Num(); i++)
		{
			auto FixedColor = Data.VertexColors[i];
			Swap(FixedColor.R, FixedColor.B);
			VertexColorsByPoint[Data.Wedges[i].PointIndex] = FixedColor;
		}
	}
	
	FSkeletalMeshImportData SkeletalMeshImportData;

	for (auto i = 0; i < Data.Normals.Num(); i++)
	{
		Data.Normals[i].Y = -Data.Normals[i].Y; // MIRROR_MESH
	}

	for (auto Vertex : Data.Vertices)
	{
		auto FixedVertex = Vertex;
		FixedVertex.Y = -FixedVertex.Y; // MIRROR_MESH
		SkeletalMeshImportData.Points.Add(FixedVertex);
		SkeletalMeshImportData.PointToRawMap.Add(SkeletalMeshImportData.Points.Num()-1);
	}
	
	auto WindingOrder = {2, 1, 0};
	for (const auto PskFace : Data.Faces)
	{
		SkeletalMeshImportData::FTriangle Face;
		Face.MatIndex = PskFace.MatIndex;
		Face.SmoothingGroups = 1;
		Face.AuxMatIndex = 0;

		for (auto VertexIndex : WindingOrder)
		{
			const auto WedgeIndex = PskFace.WedgeIndex[VertexIndex];
			const auto PskWedge = Data.Wedges[WedgeIndex];
			
			SkeletalMeshImportData::FVertex Wedge;
			Wedge.MatIndex = PskWedge.MatIndex;
			Wedge.VertexIndex = PskWedge.PointIndex;
			Wedge.Color = Data.bHasVertexColors ? VertexColorsByPoint[PskWedge.PointIndex] : FColor::Black;
			Wedge.UVs[0] = FVector2D(PskWedge.U, PskWedge.V);
			for (auto UVIdx = 0; UVIdx < Data.ExtraUVs.Num(); UVIdx++)
			{
				auto UV =  Data.ExtraUVs[UVIdx][PskFace.WedgeIndex[VertexIndex]];
				Wedge.UVs[UVIdx+1] = UV;
			}
			
			Face.WedgeIndex[VertexIndex] = SkeletalMeshImportData.Wedges.Add(Wedge);
			Face.TangentZ[VertexIndex] = Data.bHasVertexNormals ? Data.Normals[PskWedge.PointIndex] : FVector::ZeroVector;
			Face.TangentY[VertexIndex] = FVector::ZeroVector;
			Face.TangentX[VertexIndex] = FVector::ZeroVector;
		}
		Swap(Face.WedgeIndex[0], Face.WedgeIndex[2]);
		Swap(Face.TangentZ[0], Face.TangentZ[2]);

		SkeletalMeshImportData.Faces.Add(Face);
	}

	TArray<FString> AddedBoneNames;
	for (auto PskBone : Data.Bones)
	{
		SkeletalMeshImportData::FBone Bone;
		Bone.Name = PskBone.Name;
		if (AddedBoneNames.Contains(Bone.Name)) continue;
		
		Bone.NumChildren = PskBone.NumChildren;
		Bone.ParentIndex = PskBone.ParentIndex == -1 ? INDEX_NONE : PskBone.ParentIndex;
		
		auto PskBonePos = PskBone.BonePos;
		FTransform PskTransform;
		PskTransform.SetLocation(FVector(PskBonePos.Position.X, -PskBonePos.Position.Y, PskBonePos.Position.Z));
		PskTransform.SetRotation(FQuat(PskBonePos.Orientation.X, -PskBonePos.Orientation.Y, PskBonePos.Orientation.Z, PskBonePos.Orientation.W).GetNormalized());

		SkeletalMeshImportData::FJointPos BonePos;
		BonePos.Transform = PskTransform;
		BonePos.Length = PskBonePos.Length;
		BonePos.XSize = PskBonePos.XSize;
		BonePos.YSize = PskBonePos.YSize;
		BonePos.ZSize = PskBonePos.ZSize;

		Bone.BonePos = BonePos;
		SkeletalMeshImportData.RefBonesBinary.Add(Bone);
		AddedBoneNames.Add(Bone.Name);
	}

	for (auto PskInfluence : Data.Influences)
	{
		SkeletalMeshImportData::FRawBoneInfluence Influence;
		Influence.BoneIndex = PskInfluence.BoneIdx;
		Influence.VertexIndex = PskInfluence.PointIdx;
		Influence.Weight = PskInfluence.Weight;
		SkeletalMeshImportData.Influences.Add(Influence);
	}

	for (auto PskMaterial : Data.Materials)
	{
		SkeletalMeshImportData::FMaterial Material;
		Material.MaterialImportName = PskMaterial.MaterialName;

		UObject* MatParent;
		auto FoundMaterialPath = MaterialNameToPathMap.Find(*Material.MaterialImportName);
		if (FoundMaterialPath != nullptr)
		{
			MatParent = CreatePackage(**FoundMaterialPath);
		}
		else
		{
			MatParent = Parent;
		}
		
		auto MaterialAdd = FPskPsaUtils::LocalFindOrCreate<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), MatParent, PskMaterial.MaterialName, Flags);
		Material.Material = MaterialAdd;
		SkeletalMeshImportData.Materials.Add(Material);
	}
	
	SkeletalMeshImportData.MaxMaterialIndex = SkeletalMeshImportData.Materials.Num()-1;

	SkeletalMeshImportData.bDiffPose = false;
	SkeletalMeshImportData.bHasNormals = Data.bHasVertexNormals;
	SkeletalMeshImportData.bHasTangents = false;
	SkeletalMeshImportData.bHasVertexColors = true;
	SkeletalMeshImportData.NumTexCoords = 1 + Data.ExtraUVs.Num(); 
	SkeletalMeshImportData.bUseT0AsRefPose = false;
	
	const auto Skeleton = FPskPsaUtils::LocalCreate<USkeleton>(USkeleton::StaticClass(), Parent,  Name.ToString().Append("_Skeleton"), Flags);

	FReferenceSkeleton RefSkeleton;
	auto SkeletalDepth = 0;
	ProcessSkeleton(SkeletalMeshImportData, Skeleton, RefSkeleton, SkeletalDepth);

	TArray<FVector> LODPoints;
	TArray<SkeletalMeshImportData::FMeshWedge> LODWedges;
	TArray<SkeletalMeshImportData::FMeshFace> LODFaces;
	TArray<SkeletalMeshImportData::FVertInfluence> LODInfluences;
	TArray<int32> LODPointToRawMap;
	SkeletalMeshImportData.CopyLODImportData(LODPoints, LODWedges, LODFaces, LODInfluences, LODPointToRawMap);

	FSkeletalMeshLODModel LODModel;
	LODModel.NumTexCoords = FMath::Max<uint32>(1, SkeletalMeshImportData.NumTexCoords);
	
	const auto SkeletalMesh = FPskPsaUtils::LocalCreate<USkeletalMesh>(USkeletalMesh::StaticClass(), Parent, Name.ToString(), Flags);
	SkeletalMesh->PreEditChange(nullptr);
	SkeletalMesh->InvalidateDeriveDataCacheGUID();
	SkeletalMesh->UnregisterAllMorphTarget();

	SkeletalMesh->GetRefBasesInvMatrix().Empty();
	SkeletalMesh->GetMaterials().Empty();
	SkeletalMesh->SetHasVertexColors(true);

	FSkeletalMeshModel* ImportedResource = SkeletalMesh->GetImportedModel();
	auto& SkeletalMeshLODInfos = SkeletalMesh->GetLODInfoArray();
	SkeletalMeshLODInfos.Empty();
	SkeletalMeshLODInfos.Add(FSkeletalMeshLODInfo());
	SkeletalMeshLODInfos[0].ReductionSettings.NumOfTrianglesPercentage = 1.0f;
	SkeletalMeshLODInfos[0].ReductionSettings.NumOfVertPercentage = 1.0f;
	SkeletalMeshLODInfos[0].ReductionSettings.MaxDeviationPercentage = 0.0f;
	SkeletalMeshLODInfos[0].LODHysteresis = 0.02f;

	ImportedResource->LODModels.Empty();
	ImportedResource->LODModels.Add(new FSkeletalMeshLODModel);
	SkeletalMesh->SetRefSkeleton(RefSkeleton);
	SkeletalMesh->CalculateInvRefMatrices();

	SkeletalMesh->SaveLODImportedData(0, SkeletalMeshImportData);
	FSkeletalMeshBuildSettings BuildOptions;
	BuildOptions.bRemoveDegenerates = true;
	BuildOptions.bRecomputeNormals = !Data.bHasVertexNormals;
	BuildOptions.bRecomputeTangents = true;
	BuildOptions.bUseMikkTSpace = true;
	SkeletalMesh->GetLODInfo(0)->BuildSettings = BuildOptions;
	SkeletalMesh->SetImportedBounds(FBoxSphereBounds(FBox(SkeletalMeshImportData.Points)));

	auto& MeshBuilderModule = IMeshBuilderModule::GetForRunningPlatform();
	const FSkeletalMeshBuildParameters SkeletalMeshBuildParameters(SkeletalMesh, GetTargetPlatformManagerRef().GetRunningTargetPlatform(), 0, false);
	if (!MeshBuilderModule.BuildSkeletalMesh(SkeletalMeshBuildParameters))
	{
		SkeletalMesh->MarkPendingKill(); // UE4.27: MarkAsGarbage() is the UE5 rename of MarkPendingKill()
		return nullptr;
	}

	for (auto Material : SkeletalMeshImportData.Materials)
	{
		SkeletalMesh->GetMaterials().Add(FSkeletalMaterial(Material.Material.Get()));
	}

	// currently not working
	if (Data.bHasMorphData)
	{
		auto DataPosition = 0;
		
		for (const auto& MorphInfo : Data.MorphInfos)
		{
			// UE4.27: no C++17 structured bindings without CppStandard = Cpp17 in Build.cs (now set).
			// MorphInfo.Name is a raw char[64] from the PSK format - go through FName explicitly,
			// otherwise it's ambiguous which NewObject<> overload (FName vs UClass*) should catch it.
			const FName MorphName(MorphInfo.Name);
			const int32 VertexCount = MorphInfo.VertexCount;
			auto MorphTarget = NewObject<UMorphTarget>(SkeletalMesh, MorphName);
			
			TArray<FMorphTargetDelta> Deltas;
			for (auto i = DataPosition; i < DataPosition + VertexCount; i++)
			{
				const auto& MorphData = Data.MorphDatas[i];
				
				FMorphTargetDelta Delta;
				Delta.PositionDelta = MorphData.PositionDelta;
				Delta.TangentZDelta = MorphData.TangentZDelta;
				Delta.SourceIdx = MorphData.PointIdx;
				Deltas.Add(Delta);
			}
			MorphTarget->PopulateDeltas(Deltas, 0, LODModel.Sections);
			MorphTarget->BaseSkelMesh = SkeletalMesh;
			SkeletalMesh->GetMorphTargets().Add(MorphTarget);
			DataPosition += VertexCount;
		}
		
		SkeletalMesh->InitMorphTargetsAndRebuildRenderData();
	}
	
	SkeletalMesh->PostEditChange();
	
	SkeletalMesh->SetSkeleton(Skeleton);
	Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
	
	FAssetRegistryModule::AssetCreated(SkeletalMesh);
	SkeletalMesh->MarkPackageDirty();

	Skeleton->PostEditChange();
	FAssetRegistryModule::AssetCreated(Skeleton);
	Skeleton->MarkPackageDirty();

	return SkeletalMesh;
}

void UPskFactory::ProcessSkeleton(const FSkeletalMeshImportData& ImportData, const USkeleton* Skeleton, FReferenceSkeleton& OutRefSkeleton, int& OutSkeletalDepth)
{
	const auto RefBonesBinary = ImportData.RefBonesBinary;
	OutRefSkeleton.Empty();
	
	FReferenceSkeletonModifier RefSkeletonModifier(OutRefSkeleton, Skeleton);
	
	for (const auto Bone : RefBonesBinary)
	{
		const FMeshBoneInfo BoneInfo(FName(*Bone.Name), Bone.Name, Bone.ParentIndex);
		RefSkeletonModifier.Add(BoneInfo, FTransform(Bone.BonePos.Transform));
	}

    OutSkeletalDepth = 0;

    TArray<int> SkeletalDepths;
    SkeletalDepths.Empty(ImportData.RefBonesBinary.Num());
    SkeletalDepths.AddZeroed(ImportData.RefBonesBinary.Num());
    for (auto b = 0; b < OutRefSkeleton.GetNum(); b++)
    {
        const auto Parent = OutRefSkeleton.GetParentIndex(b);
        auto Depth  = 1.0f;

        SkeletalDepths[b] = 1.0f;
        if (Parent != INDEX_NONE)
        {
            Depth += SkeletalDepths[Parent];
        }
        if (OutSkeletalDepth < Depth)
        {
            OutSkeletalDepth = Depth;
        }
        SkeletalDepths[b] = Depth;
    }
}
