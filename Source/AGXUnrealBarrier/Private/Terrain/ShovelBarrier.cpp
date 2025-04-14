// Copyright 2024, Algoryx Simulation AB.

#include "Terrain/ShovelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "agxTerrain/Shovel.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

// Unreal Engine includes.
#include "Math/TwoVectors.h"
#include "Math/Vector.h"

FShovelBarrier::FShovelBarrier()
	: NativeRef {new FShovelRef}
{
}

FShovelBarrier::FShovelBarrier(std::unique_ptr<FShovelRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FShovelBarrier::FShovelBarrier(FShovelBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FShovelBarrier::~FShovelBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the
	// definition, not just the forward declaration, of FShovelRef.
}

void FShovelBarrier::SetTopEdge(const FTwoVectors& TopEdgeUnreal)
{
	check(HasNative());
	const agx::Line TopEdgeAGX = ConvertDisplacement(TopEdgeUnreal);
	NativeRef->Native->setTopEdge(TopEdgeAGX);
}

FTwoVectors FShovelBarrier::GetTopEdge() const
{
	check(HasNative());
	const agx::Line TopEdgeAGX = NativeRef->Native->getTopEdge();
	const FTwoVectors TopEdgeUnreal = ConvertDisplacement(TopEdgeAGX);
	return TopEdgeUnreal;
}

void FShovelBarrier::SetCuttingEdge(const FTwoVectors& CuttingEdge)
{
	check(HasNative());
	const agx::Line CuttingEdgeAGX = ConvertDisplacement(CuttingEdge);
	NativeRef->Native->setCuttingEdge(CuttingEdgeAGX);
}

FTwoVectors FShovelBarrier::GetCuttingEdge() const
{
	check(HasNative());
	const agx::Line CuttingEdgeAGX = NativeRef->Native->getCuttingEdge();
	const FTwoVectors CuttingEdgeUnreal = ConvertDisplacement(CuttingEdgeAGX);
	return CuttingEdgeUnreal;
}

void FShovelBarrier::SetCuttingDirection(const FVector& CuttingDirection)
{
	check(HasNative());
	const agx::Vec3 DirectionAGX = ConvertVector(CuttingDirection);
	NativeRef->Native->setCuttingDirection(DirectionAGX);
}

FVector FShovelBarrier::GetCuttingDirection() const
{
	check(HasNative());
	const agx::Vec3 DirectionAGX = NativeRef->Native->getCuttingDirection();
	const FVector DirectionUnreal = ConvertVector(DirectionAGX);
	return DirectionUnreal;
}

void FShovelBarrier::SetToothLength(double ToothLength)
{
	check(HasNative());
	const agx::Real LengthAGX = ConvertDistanceToAGX(ToothLength);
	NativeRef->Native->getSettings()->setToothLength(LengthAGX);
}

double FShovelBarrier::GetToothLength() const
{
	check(HasNative());
	const agx::Real LengthAGX = NativeRef->Native->getSettings()->getToothLength();
	const double LengthUnreal = ConvertDistanceToUnreal<double>(LengthAGX);
	return LengthUnreal;
}

void FShovelBarrier::SetToothMinimumRadius(double MinimumToothRadius)
{
	check(HasNative());
	const agx::Real RadiusAGX = ConvertDistanceToAGX(MinimumToothRadius);
	NativeRef->Native->getSettings()->setToothMinimumRadius(RadiusAGX);
}

double FShovelBarrier::GetToothMinimumRadius() const
{
	check(HasNative());
	const agx::Real RadiusAGX = NativeRef->Native->getSettings()->getToothMinimumRadius();
	const double RadiusUnreal = ConvertDistanceToUnreal<double>(RadiusAGX);
	return RadiusUnreal;
}

void FShovelBarrier::SetToothMaximumRadius(double MaximumToothRadius)
{
	check(HasNative());
	const agx::Real RadiusAgx = ConvertDistanceToAGX(MaximumToothRadius);
	NativeRef->Native->getSettings()->setToothMaximumRadius(RadiusAgx);
}

double FShovelBarrier::GetToothMaximumRadius() const
{
	check(HasNative());
	const agx::Real RadiusAGX = NativeRef->Native->getSettings()->getToothMaximumRadius();
	const double RadiusUnreal = ConvertDistanceToUnreal<double>(RadiusAGX);
	return RadiusUnreal;
}

void FShovelBarrier::SetNumberOfTeeth(int32 NumberOfTeeth)
{
	check(HasNative());
	NativeRef->Native->getSettings()->setNumberOfTeeth(NumberOfTeeth);
}

int32 FShovelBarrier::GetNumberOfTeeth() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getNumberOfTeeth();
}

void FShovelBarrier::SetNoMergeExtensionDistance(double NoMergeExtensionDistance)
{
	check(HasNative());
	const agx::Real DistanceAGX = ConvertDistanceToAGX(NoMergeExtensionDistance);
	NativeRef->Native->getSettings()->getAdvancedSettings()->setNoMergeExtensionDistance(DistanceAGX);
}

double FShovelBarrier::GetNoMergeExtensionDistance() const
{
	check(HasNative());
	const agx::Real DistanceAGX = NativeRef->Native->getSettings()->getAdvancedSettings()->getNoMergeExtensionDistance();
	const double DistanceUnreal = ConvertDistanceToUnreal<double>(DistanceAGX);
	return DistanceUnreal;
}

void FShovelBarrier::SetMinimumSubmergedContactLengthFraction(
	double MinimumSubmergedContactLengthFraction)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setMinimumSubmergedContactLengthFraction(
		MinimumSubmergedContactLengthFraction);
}

double FShovelBarrier::GetMinimumSubmergedContactLengthFraction() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getAdvancedSettings()->getMinimumSubmergedContactLengthFraction();
}

void FShovelBarrier::SetVerticalBladeSoilMergeDistance(double VerticalBladeSoilMergeDistance)
{
	check(HasNative());
	const agx::Real DistanceAGX = ConvertDistanceToAGX(VerticalBladeSoilMergeDistance);
	NativeRef->Native->getSettings()->setVerticalBladeSoilMergeDistance(DistanceAGX);
}

double FShovelBarrier::GetVerticalBladeSoilMergeDistance() const
{
	check(HasNative());
	const agx::Real DistanceAGX = NativeRef->Native->getSettings()->getVerticalBladeSoilMergeDistance();
	const double DistanceUnreal = ConvertDistanceToUnreal<double>(DistanceAGX);
	return DistanceUnreal;
}

void FShovelBarrier::SetSecondarySeparationDeadloadLimit(double SecondarySeparationDeadloadLimit)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setSecondarySeparationDeadloadLimit(SecondarySeparationDeadloadLimit);
}

double FShovelBarrier::GetSecondarySeparationDeadloadLimit() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getAdvancedSettings()->getSecondarySeparationDeadloadLimit();
}

void FShovelBarrier::SetPenetrationDepthThreshold(double PenetrationDepthThreshold)
{
	check(HasNative());
	const agx::Real DepthAGX = ConvertDistanceToAGX(PenetrationDepthThreshold);
	NativeRef->Native->getSettings()->setPenetrationDepthThreshold(DepthAGX);
}

double FShovelBarrier::GetPenetrationDepthThreshold() const
{
	check(HasNative());
	const agx::Real DepthAGX = NativeRef->Native->getSettings()->getPenetrationDepthThreshold();
	const double DepthUnreal = ConvertDistanceToUnreal<double>(DepthAGX);
	return DepthUnreal;
}

void FShovelBarrier::SetPenetrationForceScaling(double PenetrationForceScaling)
{
	check(HasNative());
	NativeRef->Native->getSettings()->setPenetrationForceScaling(PenetrationForceScaling);
}

double FShovelBarrier::GetPenetrationForceScaling() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getPenetrationForceScaling();
}

void FShovelBarrier::SetEnableParticleFreeDeformers(bool Enable)
{
	check(HasNative());
	NativeRef->Native->getSettings()->setEnableParticleFreeDeformers(Enable);
}

bool FShovelBarrier::GetEnableParticleFreeDeformers() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getEnableParticleFreeDeformers();
}

void FShovelBarrier::SetAlwaysRemoveShovelContacts(bool Enable)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setAlwaysRemoveShovelContacts(Enable);
}

bool FShovelBarrier::GetAlwaysRemoveShovelContacts() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getAdvancedSettings()->getAlwaysRemoveShovelContacts();
}

void FShovelBarrier::SetMaximumPenetrationForce(double MaximumPenetrationForce)
{
	check(HasNative());
	NativeRef->Native->getSettings()->setMaxPenetrationForce(MaximumPenetrationForce);
}

double FShovelBarrier::GetMaximumPenetrationForce() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getMaxPenetrationForce();
}

void FShovelBarrier::SetContactRegionThreshold(double ContactRegionThreshold)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setContactRegionThreshold(ConvertDistanceToAGX(ContactRegionThreshold));
}

double FShovelBarrier::GetContactRegionThreshold() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSettings()->getAdvancedSettings()->getContactRegionThreshold());
}

void FShovelBarrier::SetContactRegionVerticalLimit(double ContactRegionVerticalLimit)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setContactRegionVerticalLimit(ContactRegionVerticalLimit);
}

double FShovelBarrier::GetContactRegionVerticalLimit() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSettings()->getAdvancedSettings()->getContactRegionVerticalLimit());
}

void FShovelBarrier::SetEnableInnerShapeCreateDynamicMass(bool Enable)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setEnableInnerShapeCreateDynamicMass(Enable);
}

bool FShovelBarrier::GetEnableInnerShapeCreateDynamicMass() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getAdvancedSettings()->getEnableInnerShapeCreateDynamicMass();
}

void FShovelBarrier::SetEnableParticleForceFeedback(bool Enable)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setEnableParticleForceFeedback(Enable);
}

bool FShovelBarrier::GetEnableParticleForceFeedback() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getAdvancedSettings()->getEnableParticleForceFeedback();
}

void FShovelBarrier::SetEnable(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enable);
}

bool FShovelBarrier::GetEnable() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FShovelBarrier::SetParticleInclusionMultiplier(double Multiplier)
{
	check(HasNative());
	NativeRef->Native->getSettings()->getAdvancedSettings()->setParticleInclusionMultiplier(Multiplier);
}

double FShovelBarrier::GetParticleInclusionMultiplier() const
{
	check(HasNative());
	return NativeRef->Native->getSettings()->getAdvancedSettings()->getParticleInclusionMultiplier();
}

void FShovelBarrier::SetExcavationSettingsEnabled(EAGX_ExcavationMode Mode, bool Enable)
{
	check(HasNative());
	NativeRef->Native->getExcavationSettings(Convert(Mode)).setEnable(Enable);
}

bool FShovelBarrier::GetExcavationSettingsEnabled(EAGX_ExcavationMode Mode) const
{
	check(HasNative());
	return NativeRef->Native->getExcavationSettings(Convert(Mode)).getEnable();
}

void FShovelBarrier::SetExcavationSettingsEnableCreateDynamicMass(
	EAGX_ExcavationMode Mode, bool Enable)
{
	check(HasNative());
	NativeRef->Native->getExcavationSettings(Convert(Mode)).setEnableCreateDynamicMass(Enable);
}

bool FShovelBarrier::GetExcavationSettingsEnableCreateDynamicMass(EAGX_ExcavationMode Mode) const
{
	check(HasNative());
	return NativeRef->Native->getExcavationSettings(Convert(Mode)).getEnableCreateDynamicMass();
}

void FShovelBarrier::SetExcavationSettingsEnableForceFeedback(EAGX_ExcavationMode Mode, bool Enable)
{
	check(HasNative());
	NativeRef->Native->getExcavationSettings(Convert(Mode)).setEnableForceFeedback(Enable);
}

bool FShovelBarrier::GetExcavationSettingsEnableForceFeedback(EAGX_ExcavationMode Mode) const
{
	check(HasNative());
	return NativeRef->Native->getExcavationSettings(Convert(Mode)).getEnableForceFeedback();
}

double FShovelBarrier::GetInnerContactArea() const
{
	check(HasNative());
	const agx::Real AreaAGX = NativeRef->Native->getInnerContactArea();
	const double AreaUnreal = ConvertAreaToUnreal<double>(AreaAGX);
	return AreaUnreal;
}

FRigidBodyBarrier FShovelBarrier::GetRigidBody() const
{
	check(HasNative());
	agx::RigidBody* Body = NativeRef->Native->getRigidBody();
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

FGuid FShovelBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

bool FShovelBarrier::HasNative() const
{
	AGX_CHECK(NativeRef.get() != nullptr); // TEXT("Found an FShovelBarrier that does not have a
										   // NativeRef. This should never happen"));
	ensureMsgf(
		NativeRef.get() != nullptr, TEXT("AGXUnreal: Found an FShovelBarrier that does not have a "
										 "NativeRef. This should never happen."));
	if (NativeRef.get() == nullptr)
	{
		// We somehow ended up in a bad state where there is a Barrier that does not have a
		// NativeRef. This should never happen and in local builds we will already have aborted
		// after the failed AGX_CHECK above. In user applications we instead try to fix the
		// situation so that the application can keep running and not crash. The user will hopefully
		// see the ensureMsgf message printed above and let us know about the problem.
		//
		// HasNative is a const member function in that it never alters the salient value, but
		// here we need to do some under-the-hood cleanup. We assume that a FShoveBarrier will never
		// be an actual const value.
		FShovelBarrier* MutableThis = const_cast<FShovelBarrier*>(this);
		MutableThis->NativeRef.reset(new FShovelRef());
	}

	return NativeRef->Native != nullptr;
}

void FShovelBarrier::AllocateNative(
	FRigidBodyBarrier& Body, const FTwoVectors& TopEdge, const FTwoVectors& CuttingEdge,
	const FVector& CuttingDirection)
{
	check(!HasNative());
	check(Body.HasNative());
	agx::RigidBody* BodyAGX = Body.GetNative()->Native;
	const agx::Line TopEdgeAGX = ConvertDisplacement(TopEdge);
	const agx::Line CuttingEdgeAGX = ConvertDisplacement(CuttingEdge);
	agx::Vec3 CuttingDirectionAGX = ConvertVector(CuttingDirection);

	// This is a fix for the error printed from AGX Dynamics where the tolerance of the length of
	// the Cutting Direction is so small that floating point precision is not enough, thus
	// triggering the error message even if set to length 100cm in Unreal.
	CuttingDirectionAGX.normalize();

	NativeRef->Native =
		new agxTerrain::Shovel(BodyAGX, TopEdgeAGX, CuttingEdgeAGX, CuttingDirectionAGX);
}

FShovelRef* FShovelBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FShovelRef* FShovelBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uint64 FShovelBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FShovelBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxTerrain::Shovel*>(Address);
}

void FShovelBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FShovelBarrier::IncrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->reference();
}

void FShovelBarrier::DecrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->unreference();
}

void FShovelBarrier::SetbAlwaysRemoveShovelContacts(bool InbAlwaysRemoveShovelContacts)
{
	SetAlwaysRemoveShovelContacts(InbAlwaysRemoveShovelContacts);
}

void FShovelBarrier::SetbEnableParticleFreeDeformers(bool InbEnableParticleFreeDeformers)
{
	SetEnableParticleFreeDeformers(InbEnableParticleFreeDeformers);
}

void FShovelBarrier::SetbEnableInnerShapeCreateDynamicMass(
	bool InbEnableInnerShapeCreateDynamicMass)
{
	SetEnableInnerShapeCreateDynamicMass(InbEnableInnerShapeCreateDynamicMass);
}

void FShovelBarrier::SetbEnableParticleForceFeedback(bool InbEnableParticleForceFeedback)
{
	SetEnableParticleForceFeedback(InbEnableParticleForceFeedback);
}
