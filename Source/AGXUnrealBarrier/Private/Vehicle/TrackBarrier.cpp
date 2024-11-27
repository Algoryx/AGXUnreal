// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/TrackBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "AGX_LogCategory.h"
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Vehicle/TrackPropertiesBarrier.h"
#include "BarrierOnly/Vehicle/TrackPropertiesRef.h"
#include "BarrierOnly/Vehicle/TrackRef.h"
#include "Vehicle/TrackWheelBarrier.h"
#include "BarrierOnly/Vehicle/TrackWheelRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Vec3.h>
#include <agx/Quat.h>
#include <agxCollide/Box.h>
#include <agxVehicle/Track.h>
#include <agxVehicle/TrackNodeOnInitializeCallback.h>
#include "EndAGXIncludes.h"

#if 1
// \todo The functions below are a copy of the ones with same in AgxDynamicsObjectAccess_Helper of
// the AgxUnrealBarrier module, because they are unreachable from this separate module. When merging
// this experimental module with the actual AgxUnrealBarrier module, remove the functions below.
namespace AgxDynamicsVehicleAccess_Helper
{
	template <typename BarrierType>
	bool CheckAgxDynamicsObject(const BarrierType* Barrier)
	{
		if (!Barrier)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Could not get AGX Dynamics native object from barrier. Barrier was nullptr."));
			return false;
		}

		if (!Barrier->HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not get AGX Dynamics native object from barrier since one has not been "
					 "allocated."));
			return false;
		}

		return true;
	}

	template <typename AgxType, typename BarrierType>
	AgxType* GetFrom(const BarrierType* Barrier)
	{
		if (!CheckAgxDynamicsObject<BarrierType>(Barrier))
		{
			return nullptr;
		}

		return Barrier->GetNative()->Native.get();
	}
}
#endif

FTrackBarrier::FTrackBarrier()
	: NativeRef {new FTrackRef}
{
}

FTrackBarrier::FTrackBarrier(std::unique_ptr<FTrackRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

FTrackBarrier::FTrackBarrier(FTrackBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FTrackRef);
}

FTrackBarrier::~FTrackBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrackRef.
}

void FTrackBarrier::AddTrackWheel(
	uint8 Model, double Radius, const FRigidBodyBarrier& RigidBody, const FVector& RelativePosition,
	const FQuat& RelativeRotation, bool bSplitSegments, bool bMoveNodesToRotationPlane,
	bool bMoveNodesToWheel)
{
	check(HasNative());
	check(RigidBody.HasNative()); // \todo More gentle check and return false?

	// Convert to AGX Dynamics types.
	agxVehicle::TrackWheel::Model ModelAGX = static_cast<agxVehicle::TrackWheel::Model>(Model);
	agx::Real RadiusAGX = ConvertDistanceToAGX<agx::Real>(Radius);
	agx::RigidBody* RigidBodyAGX = FAGX_AgxDynamicsObjectsAccess::GetFrom(&RigidBody);
	agx::AffineMatrix4x4 RelTransformAGX = ConvertMatrix(RelativePosition, RelativeRotation);

	// Create AGX Dynamics TrackWheel
	agxVehicle::TrackWheelRef WheelAGX =
		new agxVehicle::TrackWheel(ModelAGX, RadiusAGX, RigidBodyAGX, RelTransformAGX);

	// Set properties.
	// \remark MERGE_NODES seems to be automatically set by AGX Dynamics depending on Model
	//         (set to true for Sprocket and Idler). Not sure if there is any purpose for letting
	//         the user override it, so ignoring it for now..
	// WheelAGX->setEnableProperty(agxVehicle::TrackWheel::Property::MERGE_NODES, ...);
	WheelAGX->setEnableProperty(agxVehicle::TrackWheel::Property::SPLIT_SEGMENTS, bSplitSegments);
	WheelAGX->setEnableProperty(
		agxVehicle::TrackWheel::Property::MOVE_NODES_TO_ROTATION_PLANE, bMoveNodesToRotationPlane);
	WheelAGX->setEnableProperty(
		agxVehicle::TrackWheel::Property::MOVE_NODES_TO_WHEEL, bMoveNodesToWheel);

	// Add to Track
	NativeRef->Native->add(WheelAGX);
}

bool FTrackBarrier::AddToSimulation(FSimulationBarrier& Sim) const
{
	check(HasNative());
	check(Sim.HasNative()); // \todo More gentle check and return false?
	agxSDK::Simulation* SimAGX = FAGX_AgxDynamicsObjectsAccess::GetFrom(&Sim);
	return SimAGX->add(NativeRef->Native);
}

bool FTrackBarrier::RemoveFromSimulation(FSimulationBarrier& Sim) const
{
	check(HasNative());
	check(Sim.HasNative()); // \todo More gentle check and return false?
	agxSDK::Simulation* SimAGX = FAGX_AgxDynamicsObjectsAccess::GetFrom(&Sim);
	return SimAGX->remove(NativeRef->Native);
}

void FTrackBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->Native->setName(Convert(Name));
}

FString FTrackBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

void FTrackBarrier::ClearMaterial()
{
	// Set to default material (just setting to nullptr doesn't appear to change anything).
	check(HasNative());
	NativeRef->Native->setMaterial(agx::Material::getDefaultMaterial());
}

void FTrackBarrier::SetMaterial(const FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	NativeRef->Native->setMaterial(FAGX_AgxDynamicsObjectsAccess::GetFrom(&Material));
}

FShapeMaterialBarrier FTrackBarrier::GetMaterial() const
{
	check(HasNative());
	agx::Material* Material = NativeRef->Native->getMaterial();
	return FShapeMaterialBarrier(std::make_unique<FMaterialRef>(Material));
}

void FTrackBarrier::ClearProperties()
{
	check(HasNative());
	NativeRef->Native->setProperties(nullptr);
}

void FTrackBarrier::SetProperties(const FTrackPropertiesBarrier& Properties)
{
	check(HasNative());
	check(Properties.HasNative());
	agxVehicle::TrackProperties* PropertiesAGX =
		AgxDynamicsVehicleAccess_Helper::GetFrom<agxVehicle::TrackProperties>(
			&Properties); // \todo Replace
	NativeRef->Native->setProperties(PropertiesAGX);
}

FTrackPropertiesBarrier FTrackBarrier::GetProperties() const
{
	check(HasNative());
	agxVehicle::TrackProperties* PropertiesAGX = NativeRef->Native->getProperties();
	return {std::make_unique<FTrackPropertiesRef>(PropertiesAGX)}; // \ todo Replace
}

void FTrackBarrier::AddCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Add collision group as (hashed) unsigned int.
	NativeRef->Native->addGroup(StringTo32BitFnvHash(GroupName.ToString()));
}

void FTrackBarrier::AddCollisionGroups(const TArray<FName>& GroupNames)
{
	check(HasNative());
	for (auto& GroupName : GroupNames)
	{
		AddCollisionGroup(GroupName);
	}
}

TArray<FName> FTrackBarrier::GetCollisionGroups() const
{
	check(HasNative());
	TArray<FName> Result;
	agxCollide::GroupIdCollection Groups = NativeRef->Native->findGroupIdCollection();
	for (const agx::Name& Name : Groups.getNames())
	{
		Result.Add(FName(*Convert(Name)));
	}
	for (const agx::UInt32 Id : Groups.getIds())
	{
		Result.Add(FName(*FString::FromInt(Id)));
	}
	return Result;
}

TArray<FTrackWheelBarrier> FTrackBarrier::GetWheels() const
{
	check(HasNative());
	const auto& WheelsAGX = NativeRef->Native->getWheels();
	TArray<FTrackWheelBarrier> Wheels;
	Wheels.Reserve(WheelsAGX.size());

	for (const agxVehicle::TrackWheelRef& Wheel : WheelsAGX)
	{
		if (Wheel == nullptr)
		{
			continue;
		}
		Wheels.Add(FTrackWheelBarrier(std::make_unique<FTrackWheelRef>(Wheel.get())));
	}

	return Wheels;
}

int32 FTrackBarrier::GetNumNodes() const
{
	check(HasNative());
	return NativeRef->Native->getNumNodes();
}

double FTrackBarrier::GetWidth() const
{
	check(HasNative());
	if (NativeRef->Native->getRoute() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetWidth was called on Track: '%s' that does not have a TrackRoute. The value "
				 "returned will not be valid."),
			*GetName());
		return -1.0;
	}

	return ConvertDistanceToUnreal<double>(NativeRef->Native->getRoute()->getNodeWidth());
}

double FTrackBarrier::GetThickness() const
{
	check(HasNative());
	if (NativeRef->Native->getRoute() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"GetThickness was called on Track: '%s' that does not have a TrackRoute. The value "
				"returned will not be valid."),
			*GetName());
		return -1.0;
	}

	return ConvertDistanceToUnreal<double>(NativeRef->Native->getRoute()->getNodeThickness());
}

double FTrackBarrier::GetInitialDistanceTension() const
{
	check(HasNative());
	if (NativeRef->Native->getRoute() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetInitialDistanceTension was called on Track: '%s' that does not have a "
				 "TrackRoute. The value returned will not be valid."),
			*GetName());
		return -1.0;
	}

	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getRoute()->getInitialDistanceTension());
}

FRigidBodyBarrier FTrackBarrier::GetNodeBody(int index) const
{
	check(HasNative());

	agxVehicle::TrackNode* Node = NativeRef->Native->getNode(index);
	if (Node == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Failed to get track node with index %i."), index);
		return FRigidBodyBarrier();
	}

	agx::RigidBody* Body = Node->getRigidBody();
	check(Body);
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

void FTrackBarrier::GetNodeSizes(TArray<FVector>& OutNodeSizes) const
{
	check(HasNative());
	const agx::UInt NumNodes = NativeRef->Native->getNumNodes();
	if (OutNodeSizes.Num() != NumNodes)
	{
		OutNodeSizes.SetNum(NumNodes);
	}

	agxVehicle::TrackNodeRange Nodes = NativeRef->Native->nodes();
	int32 I = 0;
	for (agxVehicle::TrackNode* Node : Nodes)
	{
		const agx::Vec3& HalfExtentAGX = Node->getHalfExtents();
		const FVector SizeUnreal = ConvertDistance(2.0 * HalfExtentAGX);
		OutNodeSizes[I] = SizeUnreal;
	}
}

FGuid FTrackBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

FVector FTrackBarrier::GetNodeSize(uint64 index) const
{
	check(HasNative());
	agx::UInt NumNodes = NativeRef->Native->getNumNodes();
	if (NumNodes > 0)
		return ConvertDistance(2.0 * NativeRef->Native->getNode(index)->getHalfExtents());
	else
		return FVector::ZeroVector;
}

void FTrackBarrier::GetNodeTransforms(
	TArray<FTransform>& OutTransforms, const FVector& LocalScale, const FVector& LocalOffset,
	const FQuat& LocalRotation) const
{
	check(HasNative());

	// Resize output array if necessary.
	agx::UInt NumNodes = NativeRef->Native->getNumNodes();
	if (OutTransforms.Num() != NumNodes)
	{
		// Retain the container buffer so that the same transform cache can be reused for multiple
		// tracks without reallocation every time.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutTransforms.SetNum(NumNodes, false);
#else
		OutTransforms.SetNum(NumNodes, EAllowShrinking::No);
#endif
	}

	agxVehicle::TrackNodeRange Nodes = NativeRef->Native->nodes();
	int32 i = 0;
	for (agxVehicle::TrackNodeIterator It = Nodes.begin(); It != Nodes.end(); ++It)
	{
		// \todo Could optimize this since we currently set the same passed-in scale on all nodes.
		OutTransforms[i].SetScale3D(LocalScale);

		const FQuat RigidBodyRotation = Convert(It->getRigidBody()->getRotation());
		OutTransforms[i].SetRotation(RigidBodyRotation * LocalRotation);

		const FVector WorldOffset = RigidBodyRotation.Rotator().RotateVector(LocalOffset);
		OutTransforms[i].SetLocation(ConvertDisplacement(It->getCenterPosition()) + WorldOffset);

		++i;
	}
}

void FTrackBarrier::GetDebugData(
	TArray<FVectorAndRotator>* BodyTransforms, TArray<FVectorAndRotator>* HingeTransforms,
	TArray<FVector>* MassCenters, TArray<FVectorRotatorRadii>* CollisionBoxes,
	TArray<FLinearColor>* BodyColors, TArray<FVectorQuatRadius>* WheelTransforms,
	TArray<FLinearColor>* WheelColors) const
{
	check(HasNative());

	const agx::UInt NumNodes = NativeRef->Native->getNumNodes();

	// Resize output arrays if necessary. Disallow shrinking so that the same
	// cache can be used both for large and small Tracks without reallocation.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
	constexpr bool bAllowShrinking = false;
#else
	constexpr EAllowShrinking bAllowShrinking = EAllowShrinking::No;
#endif
	if (BodyTransforms != nullptr)
	{
		BodyTransforms->SetNum(NumNodes, bAllowShrinking);
	}
	if (HingeTransforms != nullptr)
	{
		HingeTransforms->SetNum(NumNodes, bAllowShrinking);
	}
	if (MassCenters != nullptr)
	{
		MassCenters->SetNum(NumNodes, bAllowShrinking);
	}
	if (CollisionBoxes != nullptr)
	{
		CollisionBoxes->SetNum(NumNodes, bAllowShrinking);
	}
	if (BodyColors != nullptr)
	{
		BodyColors->SetNum(NumNodes, bAllowShrinking);
	}
	if (WheelTransforms != nullptr)
	{
		WheelTransforms->SetNum(NumNodes, bAllowShrinking);
	}
	if (WheelColors != nullptr)
	{
		WheelColors->SetNum(NumNodes, bAllowShrinking);
	}

	const agxVehicle::TrackNodeRange Nodes = NativeRef->Native->nodes();
	int32 i = 0;
	for (agxVehicle::TrackNodeIterator It = Nodes.begin(); It != Nodes.end(); ++It)
	{
		const agx::RigidBody* Body = It->getRigidBody();
		const agx::Constraint* Constraint = It->getConstraint();

		if (BodyTransforms != nullptr)
		{
			std::get<0>((*BodyTransforms)[i]) = ConvertDisplacement(Body->getPosition());
			std::get<1>((*BodyTransforms)[i]) = Convert(Body->getRotation()).Rotator();
		}

		if (HingeTransforms != nullptr)
		{
			agx::AffineMatrix4x4 HingeTransform =
				Constraint->getAttachment(1)->getFrame()->getLocalMatrix() *
				Body->getFrame()->getMatrix();

			std::get<0>((*HingeTransforms)[i]) = ConvertDisplacement(HingeTransform.getTranslate());
			std::get<1>((*HingeTransforms)[i]) = Convert(HingeTransform.getRotate()).Rotator();
		}

		if (MassCenters != nullptr)
		{
			(*MassCenters)[i] = ConvertDisplacement(Body->getCmPosition());
		}

		if (CollisionBoxes != nullptr)
		{
			agxCollide::GeometryRefVector Geom = Body->getGeometries();
			check(Geom.size() > 0);
			if (auto* Box = dynamic_cast<agxCollide::Box*>(Geom[0].get()->getShape()))
			{
				std::get<0>((*CollisionBoxes)[i]) =
					ConvertDisplacement(Box->getTransform().getTranslate());
				std::get<1>((*CollisionBoxes)[i]) =
					Convert(Box->getTransform().getRotate()).Rotator();
				std::get<2>((*CollisionBoxes)[i]) = ConvertDistance(Box->getHalfExtents());
			}
		}

		if (BodyColors != nullptr)
		{
			agx::Vec3 Color(0, 0, 0);
			if (agx::MergedBody* MergedBody = It->getMergedBody())
			{
				MergedBody->getDebugRenderColor(Color);
			}
			FLinearColor ColorUnreal(Color.x(), Color.y(), Color.z(), 0.4f);
			(*BodyColors)[i] = ColorUnreal;
		}
		++i;
	}

	const auto& Wheels = NativeRef->Native->getWheels();
	i = 0;
	for (const auto& Wheel : Wheels)
	{
		if (WheelTransforms != nullptr)
		{
			std::get<0>((*WheelTransforms)[i]) =
				ConvertDisplacement(Wheel->getTransform().getTranslate());
			std::get<1>((*WheelTransforms)[i]) = Convert(Wheel->getTransform().getRotate());
			std::get<2>((*WheelTransforms)[i]) = ConvertDistanceToUnreal<float>(Wheel->getRadius());
		}

		if (WheelColors != nullptr)
		{
			switch (Wheel->getModel())
			{
				case agxVehicle::TrackWheel::SPROCKET:
					(*WheelColors)[i] = FLinearColor::Red;
					break;
				case agxVehicle::TrackWheel::IDLER:
					(*WheelColors)[i] = FLinearColor::Blue;
					break;
				case agxVehicle::TrackWheel::ROLLER:
					(*WheelColors)[i] = FLinearColor::Green;
					break;
				default:
					(*WheelColors)[i] = FLinearColor::White;
			}
		}

		++i;
	}
}

void FTrackBarrier::GetPreviewData(
	TArray<FTransform>& OutNodeTransforms, TArray<FVector>& OutNodeHalfExtents, uint64 NumNodes,
	double Width, double Thickness, double InitialTensionDistance,
	const TArray<FTrackBarrier::FTrackWheelDescription>& Wheels)
{
	using namespace agxVehicle;

	// Create AGX TrackDesc.
	TrackDesc Desc(
		NumNodes, ConvertDistanceToAGX<agx::Real>(Width),
		ConvertDistanceToAGX<agx::Real>(Thickness),
		ConvertDistanceToAGX<agx::Real>(InitialTensionDistance));

	// Create list of AGX TrackWheelDesc.
	TrackWheelDescVector WheelsVector;
	WheelsVector.reserve(Wheels.Num());
	for (const auto& Wheel : Wheels)
	{
		TrackWheelDesc WheelDesc(
			static_cast<agxVehicle::TrackWheel::Model>(Wheel.Model),
			ConvertDistanceToAGX<agx::Real>(Wheel.Radius),
			agx::AffineMatrix4x4(
				Convert(Wheel.RigidBodyTransform.GetRotation()),
				ConvertDisplacement(Wheel.RigidBodyTransform.GetTranslation())),
			agx::AffineMatrix4x4(
				Convert(Wheel.RelativeRotation), ConvertDisplacement(Wheel.RelativePosition)));

		WheelsVector.push_back(WheelDesc);
	}

	// Generate the AGX track nodes preview.
	TrackNodeDescVector NodesVector = agxVehicle::findTrackNodeConfiguration(Desc, WheelsVector);

	// Convert and write to the output Unreal data structure.
	OutNodeTransforms.SetNum(NodesVector.size());
	OutNodeHalfExtents.SetNum(NodesVector.size());
	int i = 0;
	for (auto It = NodesVector.begin(); It != NodesVector.end(); ++It, ++i)
	{
		OutNodeTransforms[i].SetRotation(Convert(It->transform.getRotate()));
		OutNodeTransforms[i].SetTranslation(ConvertDisplacement(It->transform.getTranslate()));
		OutNodeHalfExtents[i] = ConvertDistance(It->halfExtents);
	}
}

void FTrackBarrier::InternalMergeProperties_SetEnableMerge(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->getInternalMergeProperties()->setEnableMerge(bEnable);
}

bool FTrackBarrier::InternalMergeProperties_GetEnableMerge() const
{
	check(HasNative());
	return NativeRef->Native->getInternalMergeProperties()->getEnableMerge();
}

void FTrackBarrier::InternalMergeProperties_SetNumNodesPerMergeSegment(
	uint32 NumNodesPerMergeSegment)
{
	check(HasNative());
	NativeRef->Native->getInternalMergeProperties()->setNumNodesPerMergeSegment(
		NumNodesPerMergeSegment);
}

uint32 FTrackBarrier::InternalMergeProperties_GetNumNodesPerMergeSegment() const
{
	check(HasNative());
	return NativeRef->Native->getInternalMergeProperties()->getNumNodesPerMergeSegment();
}

void FTrackBarrier::InternalMergeProperties_SetContactReduction(
	EAGX_MergedTrackNodeContactReduction ContactReductionLevel)
{
	check(HasNative());
	agxVehicle::TrackInternalMergeProperties::ContactReduction LevelAGX =
		Convert(ContactReductionLevel);
	NativeRef->Native->getInternalMergeProperties()->setContactReduction(LevelAGX);
}

EAGX_MergedTrackNodeContactReduction FTrackBarrier::InternalMergeProperties_GetContactReduction()
	const
{
	check(HasNative());
	return Convert(NativeRef->Native->getInternalMergeProperties()->getContactReduction());
}

void FTrackBarrier::InternalMergeProperties_SetEnableLockToReachMergeCondition(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->getInternalMergeProperties()->setEnableLockToReachMergeCondition(bEnable);
}

bool FTrackBarrier::InternalMergeProperties_GetEnableLockToReachMergeCondition() const
{
	check(HasNative());
	return NativeRef->Native->getInternalMergeProperties()->getEnableLockToReachMergeCondition();
}

void FTrackBarrier::InternalMergeProperties_SetLockToReachMergeConditionCompliance(
	double Compliance)
{
	check(HasNative());
	NativeRef->Native->getInternalMergeProperties()->setLockToReachMergeConditionCompliance(
		Compliance);
}

double FTrackBarrier::InternalMergeProperties_GetLockToReachMergeConditionCompliance() const
{
	check(HasNative());
	return NativeRef->Native->getInternalMergeProperties()
		->getLockToReachMergeConditionCompliance();
}

void FTrackBarrier::InternalMergeProperties_SetLockToReachMergeConditionSpookDamping(double Damping)
{
	check(HasNative());
	NativeRef->Native->getInternalMergeProperties()->setLockToReachMergeConditionDamping(Damping);
}

double FTrackBarrier::InternalMergeProperties_GetLockToReachMergeConditionSpookDamping() const
{
	check(HasNative());
	return NativeRef->Native->getInternalMergeProperties()->getLockToReachMergeConditionDamping();
}

void FTrackBarrier::InternalMergeProperties_SetMaxAngleMergeCondition(double MaxAngleToMerge)
{
	check(HasNative());
	agx::Real RadiansAGX = ConvertAngleToAGX<agx::Real>(MaxAngleToMerge);
	NativeRef->Native->getInternalMergeProperties()->setMaxAngleMergeCondition(RadiansAGX);
}

double FTrackBarrier::InternalMergeProperties_GetMaxAngleMergeCondition() const
{
	check(HasNative());
	agx::Real RadiansAGX =
		NativeRef->Native->getInternalMergeProperties()->getMaxAngleMergeCondition();
	return ConvertAngleToUnreal<double>(RadiansAGX);
}

bool FTrackBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FTrackRef* FTrackBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTrackRef* FTrackBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FTrackBarrier::AllocateNative(
	int32 NumberOfNodes, float Width, float Thickness, float InitialDistanceTension)
{
	check(!HasNative());
	agx::Real WidthAGX = ConvertDistanceToAGX<agx::Real>(Width);
	agx::Real ThicknessAGX = ConvertDistanceToAGX<agx::Real>(Thickness);
	agx::Real InitialDistanceTensionAGX = ConvertDistanceToAGX<agx::Real>(InitialDistanceTension);
	NativeRef->Native =
		new agxVehicle::Track(NumberOfNodes, WidthAGX, ThicknessAGX, InitialDistanceTensionAGX);
}

void FTrackBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

uintptr_t FTrackBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FTrackBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		this->ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		return;
	}

	NativeRef->Native = reinterpret_cast<agxVehicle::Track*>(NativeAddress);
}
