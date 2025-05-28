// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/AGX_ShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Contacts/AGX_ShapeContact.h"
#include "Import/AGX_ImportContext.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Shapes/RenderDataBarrier.h"
#include "Shapes/RenderMaterial.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Misc/EngineVersionComparison.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "PhysicsEngine/BodySetup.h"

// Standard library includes.
#include <tuple>

// Sets default values for this component's properties
UAGX_ShapeComponent::UAGX_ShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ShapeComponent::HasNative() const
{
	return GetNative() != nullptr;
}

uint64 UAGX_ShapeComponent::GetNativeAddress() const
{
	return static_cast<uint64>(GetNativeBarrier()->GetNativeAddress());
}

void UAGX_ShapeComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	GetNativeBarrier()->SetNativeAddress(static_cast<uintptr_t>(NativeAddress));

	if (HasNative())
	{
		MergeSplitProperties.BindBarrierToOwner(*GetNative());
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_ShapeComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_ShapeComponent::UpdateVisualMesh()
{
	ClearMeshData();

	TSharedPtr<FAGX_SimpleMeshData> Data(new FAGX_SimpleMeshData());

	CreateVisualMesh(*Data.Get());

	SetMeshData(Data);

	if (SupportsShapeBodySetup() && GetWorld() && GetWorld()->IsGameWorld())
		UpdateBodySetup(); // Used only in runtime.
}

void UAGX_ShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	FShapeBarrier* Barrier = GetNative();
	Barrier->SetName(!ImportName.IsEmpty() ? ImportName : GetName());
	Barrier->SetIsSensor(bIsSensor, SensorType == EAGX_ShapeSensorType::ContactsSensor);
	Barrier->SetSurfaceVelocity(SurfaceVelocity);

	if (!UpdateNativeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeMaterial returned false in AGX_ShapeComponent. "
				 "Ensure the selected Shape Material is valid."));
	}

	GetNative()->SetEnableCollisions(bCanCollide);

	for (const FName& Group : CollisionGroups)
	{
		GetNative()->AddCollisionGroup(Group);
	}
}

bool UAGX_ShapeComponent::UpdateNativeMaterial()
{
	if (!HasNative())
		return false;

	if (!ShapeMaterial)
	{
		if (HasNative())
		{
			GetNative()->ClearMaterial();
		}
		return true;
	}

	UWorld* World = GetWorld();
	UAGX_ShapeMaterial* Instance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(World));
	check(Instance);

	if (ShapeMaterial != Instance)
	{
		ShapeMaterial = Instance;
	}

	FShapeMaterialBarrier* MaterialBarrier = Instance->GetOrCreateShapeMaterialNative(World);
	check(MaterialBarrier);
	GetNative()->SetMaterial(*MaterialBarrier);
	return true;
}

#if WITH_EDITOR
bool UAGX_ShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
#if UE_VERSION_OLDER_THAN(4, 24, 0)
	const FName& VisibleName = GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bVisible);
	const FName& RelativeScale3DName =
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, RelativeScale3D);
#else
	/// \todo bVisible and RelativeScale3D will become private in some Unreal Engine version > 4.25.
	/// Unclear how we should handle PostEditChangeProperty events for those after that, since
	/// GET_MEMBER_NAME_CHECKED can't be used with inherited private properties.
	/// Monitor
	/// https://answers.unrealengine.com/questions/950031/how-to-use-get-member-name-checked-with-upropertie.html
	/// for possible solutions.
	FName VisibleName(TEXT("bVisible"));
	FName RelativeScale3DName(TEXT("RelativeScale3D"));
#endif
	return PropertyName == VisibleName || MemberPropertyName == RelativeScale3DName;
}

void UAGX_ShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = GetFNameSafe(PropertyChangedEvent.Property);
	const FName MemberPropertyName = GetFNameSafe(PropertyChangedEvent.MemberProperty);

	if (DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName))
	{
		UpdateVisualMesh();
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_ShapeComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif

void UAGX_ShapeComponent::PostLoad()
{
	Super::PostLoad();
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateVisualMesh();

#if WITH_EDITOR
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(CanCollide);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(IsSensor);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SurfaceVelocity);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShapeMaterial);

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, MergeSplitProperties),
		[](ThisClass* This) { This->MergeSplitProperties.OnPostEditChangeProperty(*This); });
#endif
}

void UAGX_ShapeComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GIsReconstructingBlueprintInstances)
	{
		// This Component will soon be given a Native Geometry and Shape from a
		// FAGX_NativeOwnerInstanceData, so don't create a new one here.
		return;
	}

	GetOrCreateNative();
	if (HasNative())
	{
		MergeSplitProperties.OnBeginPlay(*this);
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shape '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);
	UpdateVisualMesh();
}

void UAGX_ShapeComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_ShapeComponent will inherit this one's Native, so don't wreck it.
		// It's still safe to release the native since the Simulation will hold a reference if
		// necessary.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		// @todo Figure out how to handle removal of Shape Materials from the Simulation. They can
		// be shared between many Shape Components, so some kind of reference counting might be
		// needed.
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		ReleaseNative();
	}
}

void UAGX_ShapeComponent::UpdateNativeLocalTransform()
{
	if (!HasNative())
		return;
	UpdateNativeLocalTransform(*GetNative());
}

namespace AGX_ShapeComponent_helpers
{
	UMaterialInterface* GetOrCreateRenderMaterial(
		const FRenderDataBarrier& RenderData, bool IsSensor, FAGX_ImportContext& Context)
	{
		if (Context.RenderMaterials == nullptr || !RenderData.HasNative() ||
			!RenderData.HasMaterial())
			return AGX_MeshUtilities::GetDefaultRenderMaterial(IsSensor);

		const FAGX_RenderMaterial MBarrier = RenderData.GetMaterial();
		if (auto Existing = Context.RenderMaterials->FindRef(MBarrier.Guid))
			return Existing;

		UMaterial* Base = AGX_MeshUtilities::GetDefaultRenderMaterial(IsSensor);
		auto Result = AGX_MeshUtilities::CreateRenderMaterial(MBarrier, Base, *Context.Outer);
		AGX_CHECK(Result != nullptr);
		if (Result == nullptr)
			return nullptr;

		FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*Result, Context.SessionGuid);
		Context.RenderMaterials->Add(MBarrier.Guid, Result);
		return Result;
	}

	UStaticMesh* GetOrCreateStaticMesh(
		const FRenderDataBarrier& RenderData, UMaterialInterface* Material,
		FAGX_ImportContext& Context)
	{
		AGX_CHECK(Context.RenderStaticMeshes != nullptr);
		if (auto Existing = Context.RenderStaticMeshes->FindRef(RenderData.GetGuid()))
			return Existing;

		UStaticMesh* Mesh =
#if WITH_EDITOR
			AGX_MeshUtilities::CreateStaticMeshNoBuild(RenderData, *Context.Outer, Material);
#else
			AGX_MeshUtilities::CreateStaticMesh(RenderData, *Context.Outer, Material);
#endif // WITH_EDITOR
		if (Mesh != nullptr)
			Context.RenderStaticMeshes->Add(RenderData.GetGuid(), Mesh);

		return Mesh;
	}

	UStaticMeshComponent* CreateStaticMeshComponent(
		const FShapeBarrier& Shape, AActor& Owner, UMaterialInterface* Material,
		FAGX_ImportContext& Context)
	{
		AGX_CHECK(AGX_MeshUtilities::HasRenderDataMesh(Shape));
		const FRenderDataBarrier RenderData = Shape.GetRenderData();

		UStaticMesh* StaticMesh = GetOrCreateStaticMesh(RenderData, Material, Context);
		AGX_CHECK(StaticMesh != nullptr);
		if (StaticMesh == nullptr)
			return nullptr;

		FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*StaticMesh, Context.SessionGuid);

		// The triangles in the AGX Dynamics render data are relative to the Geometry, but the
		// Unreal Engine Component we create is placed at the position of the AGX Dynamics
		// Shape. There is no Component for the Geometry. To get the triangles in the right
		// place we need to offset the render data Component by the inverse of the
		// Geometry-to-Shape transformation in the source AGX Dynamics data.
		const FTransform RelTransform = Shape.GetGeometryToShapeTransform().Inverse();

		const FString ComponentName = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			&Owner, FString::Printf(TEXT("RenderMesh_%s"), *Shape.GetShapeGuid().ToString()),
			nullptr);
		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(&Owner, *ComponentName);
		FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, Owner, Context.SessionGuid);
		Component->SetMaterial(0, Material);
		Component->SetRelativeTransform(RelTransform);
		Component->SetStaticMesh(StaticMesh);
		return Component;
	}
}

void UAGX_ShapeComponent::CopyFrom(const FShapeBarrier& Barrier, FAGX_ImportContext* Context)
{
	using namespace AGX_ShapeComponent_helpers;

	bCanCollide = Barrier.GetEnableCollisions();
	bIsSensor = Barrier.GetIsSensor();
	ImportGuid = Barrier.GetShapeGuid();
	ImportName = Barrier.GetName();
	SurfaceVelocity = Barrier.GetSurfaceVelocity();
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOwner(), Barrier.GetName(), UAGX_ShapeComponent::StaticClass());
	Rename(*Name);

	const EAGX_ShapeSensorType BarrierSensorType = Barrier.GetIsSensorGeneratingContactData()
													   ? EAGX_ShapeSensorType::ContactsSensor
													   : EAGX_ShapeSensorType::BooleanSensor;
	SensorType = BarrierSensorType;

	auto [BarrierPosition, BarrierRotation] = Barrier.GetLocalPositionAndRotation();
	SetRelativeLocationAndRotation(BarrierPosition, BarrierRotation);

	for (const FName& Group : Barrier.GetCollisionGroups())
		AddCollisionGroup(Group);

	const FMergeSplitPropertiesBarrier Msp =
		FMergeSplitPropertiesBarrier::CreateFrom(*const_cast<FShapeBarrier*>(&Barrier));
	if (Msp.HasNative())
		MergeSplitProperties.CopyFrom(Msp, Context);

	if (Context == nullptr || Context->Shapes == nullptr || Context->Outer == nullptr)
		return; // We are done.

	AGX_CHECK(!Context->Shapes->Contains(ImportGuid));
	Context->Shapes->Add(ImportGuid, this);

	////// Shape Material ///////
	const FShapeMaterialBarrier SMB = Barrier.GetMaterial();
	if (SMB.HasNative())
	{
		UAGX_ShapeMaterial* Sm =
			FAGX_ImportRuntimeUtilities::GetOrCreateShapeMaterial(SMB, Context);
		ShapeMaterial = Sm;
	}

	////// Visibility ///////
	// The reason we let GetEnableCollisions and GetEnable determine whether or not this Shape
	// should be visible or not has to do with the behavior of agxViewer which we want to mimic. If
	// a shape in a agxCollide::Geometry which has canCollide == false is written to a AGX archive
	// and then read by agxViewer, the shape will not be visible (unless it has RenderData).
	const bool Visible =
		Barrier.GetEnableCollisions() && Barrier.GetEnabled() && !Barrier.HasRenderData();
	SetVisibility(Visible);

	////// Render Material ///////
	UMaterialInterface* Material =
		GetOrCreateRenderMaterial(Barrier.GetRenderData(), Barrier.GetIsSensor(), *Context);
	SetMaterial(0, Material);

	////// Render Mesh ///////
	if (Context->RenderStaticMeshes != nullptr && GetOwner() != nullptr &&
		AGX_MeshUtilities::HasRenderDataMesh(Barrier))
	{
		UStaticMeshComponent* Mesh =
			CreateStaticMeshComponent(Barrier, *GetOwner(), Material, *Context);
		AGX_CHECK(Mesh != nullptr);
		if (Mesh != nullptr)
		{
			Mesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			Context->RenderStaticMeshCom->Add(Barrier.GetShapeGuid(), Mesh);
		}
	}
}

void UAGX_ShapeComponent::CreateMergeSplitProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_ShapeComponent::CreateMergeSplitProperties was called "
				 "on Shape Component '%s' that does not have a Native AGX Dynamics object. Only "
				 "call "
				 "this function "
				 "during play."),
			*GetName());
		return;
	}

	if (!MergeSplitProperties.HasNative())
	{
		MergeSplitProperties.CreateNative(*this);
	}
}

void UAGX_ShapeComponent::UpdateNativeGlobalTransform()
{
	if (!HasNative())
	{
		return;
	}

	FShapeBarrier* Shape = GetNative();
	Shape->SetWorldPosition(GetComponentLocation());
	Shape->SetWorldRotation(GetComponentQuat());
}

void UAGX_ShapeComponent::AddCollisionGroup(FName GroupName)
{
	if (GroupName.IsNone())
		return;

	if (CollisionGroups.Contains(GroupName))
		return;

	CollisionGroups.Add(GroupName);
	if (HasNative())
		GetNative()->AddCollisionGroup(GroupName);
}

void UAGX_ShapeComponent::RemoveCollisionGroupIfExists(FName GroupName)
{
	if (GroupName.IsNone())
		return;

	auto Index = CollisionGroups.IndexOfByKey(GroupName);
	if (Index == INDEX_NONE)
		return;

	CollisionGroups.RemoveAt(Index);
	if (HasNative())
		GetNative()->RemoveCollisionGroup(GroupName);
}

bool UAGX_ShapeComponent::SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial)
{
	UAGX_ShapeMaterial* ShapeMaterialOrig = ShapeMaterial;
	ShapeMaterial = InShapeMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeMaterial())
	{
		// Something went wrong, restore original ShapeMaterial.
		ShapeMaterial = ShapeMaterialOrig;
		return false;
	}

	return true;
}

void UAGX_ShapeComponent::SetCanCollide(bool CanCollide)
{
	if (HasNative())
	{
		GetNative()->SetEnableCollisions(CanCollide);
	}

	bCanCollide = CanCollide;
}

bool UAGX_ShapeComponent::GetCanCollide() const
{
	if (HasNative())
	{
		return GetNative()->GetEnableCollisions();
	}

	return bCanCollide;
}

void UAGX_ShapeComponent::SetIsSensor(bool IsSensor)
{
	if (HasNative())
	{
		GetNative()->SetIsSensor(IsSensor, SensorType == EAGX_ShapeSensorType::ContactsSensor);
	}

	bIsSensor = IsSensor;

	IsSensor ? ApplySensorMaterial(*this) : RemoveSensorMaterial(*this);
}

bool UAGX_ShapeComponent::GetIsSensor() const
{
	if (HasNative())
	{
		return GetNative()->GetIsSensor();
	}

	return bIsSensor;
}

void UAGX_ShapeComponent::SetSurfaceVelocity(FVector InSurfaceVelocity)
{
	if (HasNative())
	{
		GetNative()->SetSurfaceVelocity(InSurfaceVelocity);
	}
	SurfaceVelocity = InSurfaceVelocity;
}

FVector UAGX_ShapeComponent::GetSurfaceVelocity() const
{
	if (HasNative())
	{
		return GetNative()->GetSurfaceVelocity();
	}
	return SurfaceVelocity;
}

TArray<FAGX_ShapeContact> UAGX_ShapeComponent::GetShapeContacts() const
{
	if (!HasNative())
	{
		return TArray<FAGX_ShapeContact>();
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (!Simulation)
	{
		return TArray<FAGX_ShapeContact>();
	}

	TArray<FShapeContactBarrier> Barriers = Simulation->GetShapeContacts(*GetNative());
	TArray<FAGX_ShapeContact> ShapeContacts;
	ShapeContacts.Reserve(Barriers.Num());
	for (FShapeContactBarrier& Barrier : Barriers)
	{
		ShapeContacts.Emplace(std::move(Barrier));
	}
	return ShapeContacts;
}

void UAGX_ShapeComponent::ApplySensorMaterial(UMeshComponent& Mesh)
{
	static const TCHAR* AssetPath =
		TEXT("Material'/AGXUnreal/Runtime/Materials/M_SensorMaterial.M_SensorMaterial'");
	static UMaterial* SensorMaterial = FAGX_ObjectUtilities::GetAssetFromPath<UMaterial>(AssetPath);
	if (SensorMaterial == nullptr)
	{
		return;
	}

	for (int32 I = 0; I < Mesh.GetNumMaterials(); ++I)
	{
		// Don't want to ruin the material setup of any mesh that has one so only setting the
		// sensor material to empty material slots. The intention is that sensors usually aren't
		// rendered at all in-game so their material slots should be empty.
		if (Mesh.GetMaterial(I) != nullptr)
		{
			continue;
		}
		Mesh.SetMaterial(I, SensorMaterial);
	}

	// Update any archetype instance.
	if (FAGX_ObjectUtilities::IsTemplateComponent(Mesh))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Mesh))
		{
			ApplySensorMaterial(*Instance);
		}
	}
}

void UAGX_ShapeComponent::RemoveSensorMaterial(UMeshComponent& Mesh)
{
	for (int32 I = 0; I < Mesh.GetNumMaterials(); ++I)
	{
		const UMaterialInterface* const Material = Mesh.GetMaterial(I);
		if (Material == nullptr)
		{
			continue;
		}
		if (Material->GetName() != TEXT("M_SensorMaterial"))
		{
			continue;
		}
		Mesh.SetMaterial(I, nullptr);
	}

	// Update any archetype instance.
	if (FAGX_ObjectUtilities::IsTemplateComponent(Mesh))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Mesh))
		{
			RemoveSensorMaterial(*Instance);
		}
	}
}

void UAGX_ShapeComponent::OnUpdateTransform(
	EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	if (UpdateTransformFlags == EUpdateTransformFlags::PropagateFromParent)
	{
		return;
	}

	if (!HasNative())
	{
		return;
	}

	// This Shape's transform was updated and it was not because of a parent moving. This should
	// be propagated to the native to keep the AGX Dynamics state in line with the Unreal state.
	// This can for example be due to the user manually changing the transform of the Shape
	// during play by dragging it around in the world, or setting new values via the Details
	// Panel or Blueprint functions. This also covers cases where the Shape is detached from a
	// parent Component and its transform is changed. One exception to this is when the
	// detachment is done from a Blueprint using the DetachFromComponent function with Keep
	// World settings. In that case his function is not triggered with EUpdateTransformFlags !=
	// PropagateFromParent until this Shape Component is selected (highlighted) in the Details
	// Panel. Unclear why. This corner-case is instead directly handled in
	// UAGX_ShapeComponent::OnAttachmentChanged.
	GetNative()->SetWorldPosition(GetComponentLocation());
	GetNative()->SetWorldRotation(GetComponentQuat());
}

void UAGX_ShapeComponent::OnAttachmentChanged()
{
	if (!HasNative())
	{
		return;
	}
	// This is somewhat of a hack, but it works. This Shape's parent has changed, so this
	// Unreal object might get a new world transform depending on how it was detached.  Ideally,
	// this would be fully handled by UAGX_ShapeComponent::OnUpdateTransform, but that function
	// is not triggered if the detachment was made using the Blueprint function
	// DetachFromComponent with Keep World settings. Therefore we handle that here. Note that
	// this will mean writing to the native twice when detaching in other ways (both this
	// function and OnUpdateTransform is triggered). However, setting the world transform of
	// this Component to the natives world transform is generally considered safe since they
	// should always be in sync.
	GetNative()->SetWorldPosition(GetComponentLocation());
	GetNative()->SetWorldRotation(GetComponentQuat());
}

void UAGX_ShapeComponent::OnRegister()
{
	Super::OnRegister();
	UpdateVisualMesh();
}

UBodySetup* UAGX_ShapeComponent::GetBodySetup()
{
	return ShapeBodySetup;
}

void UAGX_ShapeComponent::CreateShapeBodySetupIfNeeded()
{
	AGX_CHECK(SupportsShapeBodySetup());
	if (!IsValid(ShapeBodySetup))
	{
		ShapeBodySetup = NewObject<UBodySetup>(this, NAME_None, RF_Transient);

		ShapeBodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		AddShapeBodySetupGeometry();
		ShapeBodySetup->bNeverNeedsCookedCollisionData = true;
		BodyInstance.BodySetup = ShapeBodySetup;

		ECollisionEnabled::Type UnrealCollision = AdditionalUnrealCollision;
		const UAGX_Simulation* Simulation = GetDefault<UAGX_Simulation>();
		if (Simulation->bOverrideAdditionalUnrealCollision)
			UnrealCollision = Simulation->AdditionalUnrealCollision;

		BodyInstance.SetCollisionEnabled(UnrealCollision);
	}
}

void UAGX_ShapeComponent::UpdateBodySetup()
{
	// Subclass must implement this to support LineTrace collisions.
	check(false);
}

void UAGX_ShapeComponent::AddShapeBodySetupGeometry()
{
	// Subclass must implement this to support LineTrace collisions.
	check(false);
}

bool UAGX_ShapeComponent::SupportsShapeBodySetup()
{
	return false; // Default behavior.
}
