#pragma once


#include "ShapeBarrier.h"

#include <Math/Vector.h>

#include <memory>

struct FShapeRef;
struct FBoxShapeRef;

class AGXUNREALBARRIER_API FBoxShapeBarrier : public FShapeBarrier
{
public:
	FBoxShapeBarrier();
	~FBoxShapeBarrier() override;

	void SetHalfExtents(FVector NewHalfExtents);
	FVector GetHalfExtents() const;

	FBoxShapeRef* GetNative();
	const FBoxShapeRef* GetNative() const;

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FBoxShapeBarrier(const FBoxShapeBarrier&) = delete;
	void operator=(const FBoxShapeBarrier&) = delete;

private:
	std::unique_ptr<FBoxShapeRef> NativeRef;
};