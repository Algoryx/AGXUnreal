// Copyright 2025, Algoryx Simulation AB.

#include "Plot/AGX_PlotComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_StringUtilities.h"

UAGX_PlotComponent::UAGX_PlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_PlotComponent::CreatePlot(
	FAGX_PlotDataSeries& SeriesX, FAGX_PlotDataSeries& SeriesY, const FString& Name)
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreatePlot was called on Plot '%s' in '%s' but the Plot does not have a AGX "
				 "Native. This function should only be called during Play."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	// The reason we allocate the PlotDataSeries Natives here is that they are of UStruct type,
	// meaning they do not get the BeginPlay callback where AllocateNative is usually called.
	if (!SeriesX.HasNative())
		SeriesX.NativeBarrier.AllocateNative(SeriesX.Label);

	if (!SeriesY.HasNative())
		SeriesY.NativeBarrier.AllocateNative(SeriesY.Label);

	if (!SeriesX.HasNative() || !SeriesY.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("At least one of the Plot Data Series did not have a AGX Native object even "
				 "though AllocateNative was called for it. CreatePlot failed for Plot Component "
				 "'%s' in '%s'."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	NativeBarrier.CreatePlot(Name, SeriesX.NativeBarrier, SeriesY.NativeBarrier);
}

void UAGX_PlotComponent::OpenPlotWindow()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("OpenPlotWindow was called on Plot '%s' in '%s' but the Plot does not have a AGX "
				 "Native. This function should only be called during Play."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	NativeBarrier.OpenWebPlot();
}

FPlotBarrier* UAGX_PlotComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Plot '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	check(HasNative());
	return &NativeBarrier;
}

FPlotBarrier* UAGX_PlotComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FPlotBarrier* UAGX_PlotComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool UAGX_PlotComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

void UAGX_PlotComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateNative();
}

void UAGX_PlotComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_PlotComponent::CreateNative()
{
	/*A Plot Component's Native will "survive" a Blueprint reconstruction thanks to the fact that it
	 * does not explicitly create a new AGX PlotSystem when calling AllocateNative on its barrier,
	 * but instead gets the AGX PlotSystem pointer from the Simulation object.
	 * So on Blueprint reconstruction the Plot Component will get a BeginPlay which calls
	 * CreateNative which resets the pointer to the same underlying AGX Plot System object in
	 * AllocateNative.
	 *
	 * The PlotDataSeries survives a Blueprint reconstruction if it is a variable in the same
	 * Blueprint as the PlotComponent. This is because variables apparently survives Blueprint
	 * reconstruction out-of-the-box without any special handling needed.
	 *
	 * @todo
	 * One situation that will not work when getting a Blueprint reconstruction is if the
	 * PlotDataSeries is part of a Blueprint that is a Component type, and we try to use it in
	 * another Blueprint to create a plot by calling CreatePlot on a Plot Component in that second
	 * Blueprint. If we get a Blueprint reconstruction in that situation, the PlotDataSeries will
	 * lose it's native.
	 * One solution to this would be to make the PlotComponent keep track of each PlotDataSeries it
	 * creates Natives for (in CreatePlot) and then on Blueprint reconstruction, it stores away the
	 * pointer addresses of all of those PlotDataSeries AGX Native objects and restores them right
	 * after. We could leverage IAGX_NativeOwner but that would need to be extended to support
	 * arrays of native addresses. Also, since we cannot have a UPROPERTY
	 * TArray<FAGX_PlotDataSeries*> member (since FAGX_PlotDataSeries is a USTRUCT), we cannot
	 * easily hold those pointers through a Blueprint reconstruction. Instead we would need to get
	 * them from somewhere else. We could get them from the user by setting up a delegate in Plot
	 * Component that the user can bind to that takes an array of PlotDataSeries as input. It would
	 * put the responsibility on the user to then set this up in their Blueprint if using the
	 * PlotComponent with PlotDataSeries from a Blueprint Component.
	 * This has not been done (yet) but can be implemented in the future if we need to. If we do
	 * this, we need to make very clear instructions on how to set this up since the user will be
	 * expected to do some of the heavy lifting themselves.
	 */

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Plot '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	if (!Simulation->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Plot '%s' in '%s' tried to get Simulation Native, but UAGX_Simulation::HasNative "
				 "returned false."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FString* OutFile = bWriteToFile && !FileOutputName.IsEmpty() ? &FileOutputName : nullptr;
	const bool AutoOpenWindow = bAutoOpenPlotWindow && !GIsReconstructingBlueprintInstances;
	NativeBarrier.AllocateNative(*Simulation->GetNative(), OutFile, AutoOpenWindow);
}
