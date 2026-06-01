// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/AnyMessageBuilder.h>
#include <agxROS2/AnyMessageParser.h>
#include <agxROS2/Publisher.h>
#include <agxROS2/Subscriber.h>
#include "EndAGXIncludes.h"

struct FAnyMessageBuilder
{
	FAnyMessageBuilder(agxROS2::AnyMessageBuilder* InNative)
		: Native(InNative)
	{
	}

	std::unique_ptr<agxROS2::AnyMessageBuilder> Native;
};

struct FAnyMessageParser
{
	FAnyMessageParser(agxROS2::AnyMessageParser* InNative)
		: Native(InNative)
	{
	}

	std::unique_ptr<agxROS2::AnyMessageParser> Native;
};

struct FAgxAny
{
	FAgxAny(agxROS2::agxMsgs::Any* InNative)
		: Native(InNative)
	{
	}

	std::unique_ptr<agxROS2::agxMsgs::Any> Native;
};

struct FROS2Publisher
{
	virtual ~FROS2Publisher() = default;
};

struct FROS2Subscriber
{
	virtual ~FROS2Subscriber() = default;
};

//
// AgxMsgs
//

struct FPublisherAny : public FROS2Publisher
{
	FPublisherAny(agxROS2::agxMsgs::PublisherAny* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::agxMsgs::PublisherAny> Native;
};

struct FPublisherAnySequence : public FROS2Publisher
{
	FPublisherAnySequence(agxROS2::agxMsgs::PublisherAnySequence* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::agxMsgs::PublisherAnySequence> Native;
};

struct FSubscriberAny : public FROS2Subscriber
{
	FSubscriberAny(agxROS2::agxMsgs::SubscriberAny* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::agxMsgs::SubscriberAny> Native;
};

struct FSubscriberAnySequence : public FROS2Subscriber
{
	FSubscriberAnySequence(agxROS2::agxMsgs::SubscriberAnySequence* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::agxMsgs::SubscriberAnySequence> Native;
};

//
// BuiltinInterfaces
//

struct FPublisherTime : public FROS2Publisher
{
	FPublisherTime(agxROS2::builtinInterfaces::PublisherTime* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::builtinInterfaces::PublisherTime> Native;
};

struct FPublisherDuration : public FROS2Publisher
{
	FPublisherDuration(agxROS2::builtinInterfaces::PublisherDuration* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::builtinInterfaces::PublisherDuration> Native;
};

struct FSubscriberTime : public FROS2Subscriber
{
	FSubscriberTime(agxROS2::builtinInterfaces::SubscriberTime* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::builtinInterfaces::SubscriberTime> Native;
};

struct FSubscriberDuration : public FROS2Subscriber
{
	FSubscriberDuration(agxROS2::builtinInterfaces::SubscriberDuration* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::builtinInterfaces::SubscriberDuration> Native;
};

//
// RosgraphMsgs
//

struct FPublisherClock : public FROS2Publisher
{
	FPublisherClock(agxROS2::rosgraphMsgs::PublisherClock* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::rosgraphMsgs::PublisherClock> Native;
};

struct FSubscriberClock : public FROS2Subscriber
{
	FSubscriberClock(agxROS2::rosgraphMsgs::SubscriberClock* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::rosgraphMsgs::SubscriberClock> Native;
};

//
// StdMsgs
//

struct FPublisherBool : public FROS2Publisher
{
	FPublisherBool(agxROS2::stdMsgs::PublisherBool* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherBool> Native;
};

struct FPublisherByte : public FROS2Publisher
{
	FPublisherByte(agxROS2::stdMsgs::PublisherByte* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherByte> Native;
};

struct FPublisherByteMultiArray : public FROS2Publisher
{
	FPublisherByteMultiArray(agxROS2::stdMsgs::PublisherByteMultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherByteMultiArray> Native;
};

struct FPublisherChar : public FROS2Publisher
{
	FPublisherChar(agxROS2::stdMsgs::PublisherChar* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherChar> Native;
};

struct FPublisherColorRGBA : public FROS2Publisher
{
	FPublisherColorRGBA(agxROS2::stdMsgs::PublisherColorRGBA* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherColorRGBA> Native;
};

struct FPublisherEmpty : public FROS2Publisher
{
	FPublisherEmpty(agxROS2::stdMsgs::PublisherEmpty* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherEmpty> Native;
};

struct FPublisherFloat32 : public FROS2Publisher
{
	FPublisherFloat32(agxROS2::stdMsgs::PublisherFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherFloat32> Native;
};

struct FPublisherFloat32MultiArray : public FROS2Publisher
{
	FPublisherFloat32MultiArray(agxROS2::stdMsgs::PublisherFloat32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherFloat32MultiArray> Native;
};

struct FPublisherFloat64 : public FROS2Publisher
{
	FPublisherFloat64(agxROS2::stdMsgs::PublisherFloat64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherFloat64> Native;
};

struct FPublisherFloat64MultiArray : public FROS2Publisher
{
	FPublisherFloat64MultiArray(agxROS2::stdMsgs::PublisherFloat64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherFloat64MultiArray> Native;
};

struct FPublisherInt16 : public FROS2Publisher
{
	FPublisherInt16(agxROS2::stdMsgs::PublisherInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt16> Native;
};

struct FPublisherInt16MultiArray : public FROS2Publisher
{
	FPublisherInt16MultiArray(agxROS2::stdMsgs::PublisherInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt16MultiArray> Native;
};

struct FPublisherInt32 : public FROS2Publisher
{
	FPublisherInt32(agxROS2::stdMsgs::PublisherInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt32> Native;
};

struct FPublisherInt32MultiArray : public FROS2Publisher
{
	FPublisherInt32MultiArray(agxROS2::stdMsgs::PublisherInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt32MultiArray> Native;
};

struct FPublisherInt64 : public FROS2Publisher
{
	FPublisherInt64(agxROS2::stdMsgs::PublisherInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt64> Native;
};

struct FPublisherInt64MultiArray : public FROS2Publisher
{
	FPublisherInt64MultiArray(agxROS2::stdMsgs::PublisherInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt64MultiArray> Native;
};

struct FPublisherInt8 : public FROS2Publisher
{
	FPublisherInt8(agxROS2::stdMsgs::PublisherInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt8> Native;
};

struct FPublisherInt8MultiArray : public FROS2Publisher
{
	FPublisherInt8MultiArray(agxROS2::stdMsgs::PublisherInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherInt8MultiArray> Native;
};

struct FPublisherString : public FROS2Publisher
{
	FPublisherString(agxROS2::stdMsgs::PublisherString* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherString> Native;
};

struct FPublisherUInt16 : public FROS2Publisher
{
	FPublisherUInt16(agxROS2::stdMsgs::PublisherUInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt16> Native;
};

struct FPublisherUInt16MultiArray : public FROS2Publisher
{
	FPublisherUInt16MultiArray(agxROS2::stdMsgs::PublisherUInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt16MultiArray> Native;
};

struct FPublisherUInt32 : public FROS2Publisher
{
	FPublisherUInt32(agxROS2::stdMsgs::PublisherUInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt32> Native;
};

struct FPublisherUInt32MultiArray : public FROS2Publisher
{
	FPublisherUInt32MultiArray(agxROS2::stdMsgs::PublisherUInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt32MultiArray> Native;
};

struct FPublisherUInt64 : public FROS2Publisher
{
	FPublisherUInt64(agxROS2::stdMsgs::PublisherUInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt64> Native;
};

struct FPublisherUInt64MultiArray : public FROS2Publisher
{
	FPublisherUInt64MultiArray(agxROS2::stdMsgs::PublisherUInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt64MultiArray> Native;
};

struct FPublisherUInt8 : public FROS2Publisher
{
	FPublisherUInt8(agxROS2::stdMsgs::PublisherUInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt8> Native;
};

struct FPublisherUInt8MultiArray : public FROS2Publisher
{
	FPublisherUInt8MultiArray(agxROS2::stdMsgs::PublisherUInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherUInt8MultiArray> Native;
};

struct FPublisherHeader : public FROS2Publisher
{
	FPublisherHeader(agxROS2::stdMsgs::PublisherHeader* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::PublisherHeader> Native;
};

struct FSubscriberBool : public FROS2Subscriber
{
	FSubscriberBool(agxROS2::stdMsgs::SubscriberBool* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberBool> Native;
};

struct FSubscriberByte : public FROS2Subscriber
{
	FSubscriberByte(agxROS2::stdMsgs::SubscriberByte* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberByte> Native;
};

struct FSubscriberByteMultiArray : public FROS2Subscriber
{
	FSubscriberByteMultiArray(agxROS2::stdMsgs::SubscriberByteMultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberByteMultiArray> Native;
};

struct FSubscriberChar : public FROS2Subscriber
{
	FSubscriberChar(agxROS2::stdMsgs::SubscriberChar* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberChar> Native;
};

struct FSubscriberColorRGBA : public FROS2Subscriber
{
	FSubscriberColorRGBA(agxROS2::stdMsgs::SubscriberColorRGBA* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberColorRGBA> Native;
};

struct FSubscriberEmpty : public FROS2Subscriber
{
	FSubscriberEmpty(agxROS2::stdMsgs::SubscriberEmpty* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberEmpty> Native;
};

struct FSubscriberFloat32 : public FROS2Subscriber
{
	FSubscriberFloat32(agxROS2::stdMsgs::SubscriberFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberFloat32> Native;
};

struct FSubscriberFloat32MultiArray : public FROS2Subscriber
{
	FSubscriberFloat32MultiArray(agxROS2::stdMsgs::SubscriberFloat32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberFloat32MultiArray> Native;
};

struct FSubscriberFloat64 : public FROS2Subscriber
{
	FSubscriberFloat64(agxROS2::stdMsgs::SubscriberFloat64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberFloat64> Native;
};

struct FSubscriberFloat64MultiArray : public FROS2Subscriber
{
	FSubscriberFloat64MultiArray(agxROS2::stdMsgs::SubscriberFloat64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberFloat64MultiArray> Native;
};

struct FSubscriberInt16 : public FROS2Subscriber
{
	FSubscriberInt16(agxROS2::stdMsgs::SubscriberInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt16> Native;
};

struct FSubscriberInt16MultiArray : public FROS2Subscriber
{
	FSubscriberInt16MultiArray(agxROS2::stdMsgs::SubscriberInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt16MultiArray> Native;
};

struct FSubscriberInt32 : public FROS2Subscriber
{
	FSubscriberInt32(agxROS2::stdMsgs::SubscriberInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt32> Native;
};

struct FSubscriberInt32MultiArray : public FROS2Subscriber
{
	FSubscriberInt32MultiArray(agxROS2::stdMsgs::SubscriberInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt32MultiArray> Native;
};

struct FSubscriberInt64 : public FROS2Subscriber
{
	FSubscriberInt64(agxROS2::stdMsgs::SubscriberInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt64> Native;
};

struct FSubscriberInt64MultiArray : public FROS2Subscriber
{
	FSubscriberInt64MultiArray(agxROS2::stdMsgs::SubscriberInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt64MultiArray> Native;
};

struct FSubscriberInt8 : public FROS2Subscriber
{
	FSubscriberInt8(agxROS2::stdMsgs::SubscriberInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt8> Native;
};

struct FSubscriberInt8MultiArray : public FROS2Subscriber
{
	FSubscriberInt8MultiArray(agxROS2::stdMsgs::SubscriberInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberInt8MultiArray> Native;
};

struct FSubscriberString : public FROS2Subscriber
{
	FSubscriberString(agxROS2::stdMsgs::SubscriberString* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberString> Native;
};

struct FSubscriberUInt16 : public FROS2Subscriber
{
	FSubscriberUInt16(agxROS2::stdMsgs::SubscriberUInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt16> Native;
};

struct FSubscriberUInt16MultiArray : public FROS2Subscriber
{
	FSubscriberUInt16MultiArray(agxROS2::stdMsgs::SubscriberUInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt16MultiArray> Native;
};

struct FSubscriberUInt32 : public FROS2Subscriber
{
	FSubscriberUInt32(agxROS2::stdMsgs::SubscriberUInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt32> Native;
};

struct FSubscriberUInt32MultiArray : public FROS2Subscriber
{
	FSubscriberUInt32MultiArray(agxROS2::stdMsgs::SubscriberUInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt32MultiArray> Native;
};

struct FSubscriberUInt64 : public FROS2Subscriber
{
	FSubscriberUInt64(agxROS2::stdMsgs::SubscriberUInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt64> Native;
};

struct FSubscriberUInt64MultiArray : public FROS2Subscriber
{
	FSubscriberUInt64MultiArray(agxROS2::stdMsgs::SubscriberUInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt64MultiArray> Native;
};

struct FSubscriberUInt8 : public FROS2Subscriber
{
	FSubscriberUInt8(agxROS2::stdMsgs::SubscriberUInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt8> Native;
};

struct FSubscriberUInt8MultiArray : public FROS2Subscriber
{
	FSubscriberUInt8MultiArray(agxROS2::stdMsgs::SubscriberUInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberUInt8MultiArray> Native;
};

struct FSubscriberHeader : public FROS2Subscriber
{
	FSubscriberHeader(agxROS2::stdMsgs::SubscriberHeader* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::stdMsgs::SubscriberHeader> Native;
};

//
// GeometryMsgs
//

struct FPublisherVector3 : public FROS2Publisher
{
	FPublisherVector3(agxROS2::geometryMsgs::PublisherVector3* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherVector3> Native;
};

struct FPublisherQuaternion : public FROS2Publisher
{
	FPublisherQuaternion(agxROS2::geometryMsgs::PublisherQuaternion* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherQuaternion> Native;
};

struct FPublisherAccel : public FROS2Publisher
{
	FPublisherAccel(agxROS2::geometryMsgs::PublisherAccel* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherAccel> Native;
};

struct FPublisherAccelStamped : public FROS2Publisher
{
	FPublisherAccelStamped(agxROS2::geometryMsgs::PublisherAccelStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherAccelStamped> Native;
};

struct FPublisherAccelWithCovariance : public FROS2Publisher
{
	FPublisherAccelWithCovariance(agxROS2::geometryMsgs::PublisherAccelWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherAccelWithCovariance> Native;
};

struct FPublisherAccelWithCovarianceStamped : public FROS2Publisher
{
	FPublisherAccelWithCovarianceStamped(
		agxROS2::geometryMsgs::PublisherAccelWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherAccelWithCovarianceStamped> Native;
};

struct FPublisherInertia : public FROS2Publisher
{
	FPublisherInertia(agxROS2::geometryMsgs::PublisherInertia* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherInertia> Native;
};

struct FPublisherInertiaStamped : public FROS2Publisher
{
	FPublisherInertiaStamped(agxROS2::geometryMsgs::PublisherInertiaStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherInertiaStamped> Native;
};

struct FPublisherPoint : public FROS2Publisher
{
	FPublisherPoint(agxROS2::geometryMsgs::PublisherPoint* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPoint> Native;
};

struct FPublisherPoint32 : public FROS2Publisher
{
	FPublisherPoint32(agxROS2::geometryMsgs::PublisherPoint32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPoint32> Native;
};

struct FPublisherPointStamped : public FROS2Publisher
{
	FPublisherPointStamped(agxROS2::geometryMsgs::PublisherPointStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPointStamped> Native;
};

struct FPublisherPolygon : public FROS2Publisher
{
	FPublisherPolygon(agxROS2::geometryMsgs::PublisherPolygon* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPolygon> Native;
};

struct FPublisherPolygonStamped : public FROS2Publisher
{
	FPublisherPolygonStamped(agxROS2::geometryMsgs::PublisherPolygonStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPolygonStamped> Native;
};

struct FPublisherPose : public FROS2Publisher
{
	FPublisherPose(agxROS2::geometryMsgs::PublisherPose* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPose> Native;
};

struct FPublisherPose2D : public FROS2Publisher
{
	FPublisherPose2D(agxROS2::geometryMsgs::PublisherPose2D* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPose2D> Native;
};

struct FPublisherPoseArray : public FROS2Publisher
{
	FPublisherPoseArray(agxROS2::geometryMsgs::PublisherPoseArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPoseArray> Native;
};

struct FPublisherPoseStamped : public FROS2Publisher
{
	FPublisherPoseStamped(agxROS2::geometryMsgs::PublisherPoseStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPoseStamped> Native;
};

struct FPublisherPoseWithCovariance : public FROS2Publisher
{
	FPublisherPoseWithCovariance(agxROS2::geometryMsgs::PublisherPoseWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPoseWithCovariance> Native;
};

struct FPublisherPoseWithCovarianceStamped : public FROS2Publisher
{
	FPublisherPoseWithCovarianceStamped(
		agxROS2::geometryMsgs::PublisherPoseWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherPoseWithCovarianceStamped> Native;
};

struct FPublisherQuaternionStamped : public FROS2Publisher
{
	FPublisherQuaternionStamped(agxROS2::geometryMsgs::PublisherQuaternionStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherQuaternionStamped> Native;
};

struct FPublisherTransform : public FROS2Publisher
{
	FPublisherTransform(agxROS2::geometryMsgs::PublisherTransform* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherTransform> Native;
};

struct FPublisherTransformStamped : public FROS2Publisher
{
	FPublisherTransformStamped(agxROS2::geometryMsgs::PublisherTransformStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherTransformStamped> Native;
};

struct FPublisherTwist : public FROS2Publisher
{
	FPublisherTwist(agxROS2::geometryMsgs::PublisherTwist* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherTwist> Native;
};

struct FPublisherTwistStamped : public FROS2Publisher
{
	FPublisherTwistStamped(agxROS2::geometryMsgs::PublisherTwistStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherTwistStamped> Native;
};

struct FPublisherTwistWithCovariance : public FROS2Publisher
{
	FPublisherTwistWithCovariance(agxROS2::geometryMsgs::PublisherTwistWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherTwistWithCovariance> Native;
};

struct FPublisherTwistWithCovarianceStamped : public FROS2Publisher
{
	FPublisherTwistWithCovarianceStamped(
		agxROS2::geometryMsgs::PublisherTwistWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherTwistWithCovarianceStamped> Native;
};

struct FPublisherVector3Stamped : public FROS2Publisher
{
	FPublisherVector3Stamped(agxROS2::geometryMsgs::PublisherVector3Stamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherVector3Stamped> Native;
};

struct FPublisherWrench : public FROS2Publisher
{
	FPublisherWrench(agxROS2::geometryMsgs::PublisherWrench* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherWrench> Native;
};

struct FPublisherWrenchStamped : public FROS2Publisher
{
	FPublisherWrenchStamped(agxROS2::geometryMsgs::PublisherWrenchStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::PublisherWrenchStamped> Native;
};

struct FSubscriberVector3 : public FROS2Subscriber
{
	FSubscriberVector3(agxROS2::geometryMsgs::SubscriberVector3* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberVector3> Native;
};

struct FSubscriberQuaternion : public FROS2Subscriber
{
	FSubscriberQuaternion(agxROS2::geometryMsgs::SubscriberQuaternion* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberQuaternion> Native;
};

struct FSubscriberAccel : public FROS2Subscriber
{
	FSubscriberAccel(agxROS2::geometryMsgs::SubscriberAccel* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberAccel> Native;
};

struct FSubscriberAccelStamped : public FROS2Subscriber
{
	FSubscriberAccelStamped(agxROS2::geometryMsgs::SubscriberAccelStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberAccelStamped> Native;
};

struct FSubscriberAccelWithCovariance : public FROS2Subscriber
{
	FSubscriberAccelWithCovariance(agxROS2::geometryMsgs::SubscriberAccelWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberAccelWithCovariance> Native;
};

struct FSubscriberAccelWithCovarianceStamped : public FROS2Subscriber
{
	FSubscriberAccelWithCovarianceStamped(
		agxROS2::geometryMsgs::SubscriberAccelWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberAccelWithCovarianceStamped> Native;
};

struct FSubscriberInertia : public FROS2Subscriber
{
	FSubscriberInertia(agxROS2::geometryMsgs::SubscriberInertia* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberInertia> Native;
};

struct FSubscriberInertiaStamped : public FROS2Subscriber
{
	FSubscriberInertiaStamped(agxROS2::geometryMsgs::SubscriberInertiaStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberInertiaStamped> Native;
};

struct FSubscriberPoint : public FROS2Subscriber
{
	FSubscriberPoint(agxROS2::geometryMsgs::SubscriberPoint* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPoint> Native;
};

struct FSubscriberPoint32 : public FROS2Subscriber
{
	FSubscriberPoint32(agxROS2::geometryMsgs::SubscriberPoint32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPoint32> Native;
};

struct FSubscriberPointStamped : public FROS2Subscriber
{
	FSubscriberPointStamped(agxROS2::geometryMsgs::SubscriberPointStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPointStamped> Native;
};

struct FSubscriberPolygon : public FROS2Subscriber
{
	FSubscriberPolygon(agxROS2::geometryMsgs::SubscriberPolygon* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPolygon> Native;
};

struct FSubscriberPolygonStamped : public FROS2Subscriber
{
	FSubscriberPolygonStamped(agxROS2::geometryMsgs::SubscriberPolygonStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPolygonStamped> Native;
};

struct FSubscriberPose : public FROS2Subscriber
{
	FSubscriberPose(agxROS2::geometryMsgs::SubscriberPose* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPose> Native;
};

struct FSubscriberPose2D : public FROS2Subscriber
{
	FSubscriberPose2D(agxROS2::geometryMsgs::SubscriberPose2D* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPose2D> Native;
};

struct FSubscriberPoseArray : public FROS2Subscriber
{
	FSubscriberPoseArray(agxROS2::geometryMsgs::SubscriberPoseArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPoseArray> Native;
};

struct FSubscriberPoseStamped : public FROS2Subscriber
{
	FSubscriberPoseStamped(agxROS2::geometryMsgs::SubscriberPoseStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPoseStamped> Native;
};

struct FSubscriberPoseWithCovariance : public FROS2Subscriber
{
	FSubscriberPoseWithCovariance(agxROS2::geometryMsgs::SubscriberPoseWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPoseWithCovariance> Native;
};

struct FSubscriberPoseWithCovarianceStamped : public FROS2Subscriber
{
	FSubscriberPoseWithCovarianceStamped(
		agxROS2::geometryMsgs::SubscriberPoseWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberPoseWithCovarianceStamped> Native;
};

struct FSubscriberQuaternionStamped : public FROS2Subscriber
{
	FSubscriberQuaternionStamped(agxROS2::geometryMsgs::SubscriberQuaternionStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberQuaternionStamped> Native;
};

struct FSubscriberTransform : public FROS2Subscriber
{
	FSubscriberTransform(agxROS2::geometryMsgs::SubscriberTransform* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberTransform> Native;
};

struct FSubscriberTransformStamped : public FROS2Subscriber
{
	FSubscriberTransformStamped(agxROS2::geometryMsgs::SubscriberTransformStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberTransformStamped> Native;
};

struct FSubscriberTwist : public FROS2Subscriber
{
	FSubscriberTwist(agxROS2::geometryMsgs::SubscriberTwist* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberTwist> Native;
};

struct FSubscriberTwistStamped : public FROS2Subscriber
{
	FSubscriberTwistStamped(agxROS2::geometryMsgs::SubscriberTwistStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberTwistStamped> Native;
};

struct FSubscriberTwistWithCovariance : public FROS2Subscriber
{
	FSubscriberTwistWithCovariance(agxROS2::geometryMsgs::SubscriberTwistWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberTwistWithCovariance> Native;
};

struct FSubscriberTwistWithCovarianceStamped : public FROS2Subscriber
{
	FSubscriberTwistWithCovarianceStamped(
		agxROS2::geometryMsgs::SubscriberTwistWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberTwistWithCovarianceStamped> Native;
};

struct FSubscriberVector3Stamped : public FROS2Subscriber
{
	FSubscriberVector3Stamped(agxROS2::geometryMsgs::SubscriberVector3Stamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberVector3Stamped> Native;
};

struct FSubscriberWrench : public FROS2Subscriber
{
	FSubscriberWrench(agxROS2::geometryMsgs::SubscriberWrench* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberWrench> Native;
};

struct FSubscriberWrenchStamped : public FROS2Subscriber
{
	FSubscriberWrenchStamped(agxROS2::geometryMsgs::SubscriberWrenchStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::geometryMsgs::SubscriberWrenchStamped> Native;
};

//
// SensorMsgs
//

struct FPublisherBatteryState : public FROS2Publisher
{
	FPublisherBatteryState(agxROS2::sensorMsgs::PublisherBatteryState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherBatteryState> Native;
};

struct FPublisherChannelFloat32 : public FROS2Publisher
{
	FPublisherChannelFloat32(agxROS2::sensorMsgs::PublisherChannelFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherChannelFloat32> Native;
};

struct FPublisherCompressedImage : public FROS2Publisher
{
	FPublisherCompressedImage(agxROS2::sensorMsgs::PublisherCompressedImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherCompressedImage> Native;
};

struct FPublisherFluidPressure : public FROS2Publisher
{
	FPublisherFluidPressure(agxROS2::sensorMsgs::PublisherFluidPressure* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherFluidPressure> Native;
};

struct FPublisherIlluminance : public FROS2Publisher
{
	FPublisherIlluminance(agxROS2::sensorMsgs::PublisherIlluminance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherIlluminance> Native;
};

struct FPublisherImage : public FROS2Publisher
{
	FPublisherImage(agxROS2::sensorMsgs::PublisherImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherImage> Native;
};

struct FPublisherImu : public FROS2Publisher
{
	FPublisherImu(agxROS2::sensorMsgs::PublisherImu* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherImu> Native;
};

struct FPublisherJointState : public FROS2Publisher
{
	FPublisherJointState(agxROS2::sensorMsgs::PublisherJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherJointState> Native;
};

struct FPublisherJoy : public FROS2Publisher
{
	FPublisherJoy(agxROS2::sensorMsgs::PublisherJoy* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherJoy> Native;
};

struct FPublisherJoyFeedback : public FROS2Publisher
{
	FPublisherJoyFeedback(agxROS2::sensorMsgs::PublisherJoyFeedback* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherJoyFeedback> Native;
};

struct FPublisherJoyFeedbackArray : public FROS2Publisher
{
	FPublisherJoyFeedbackArray(agxROS2::sensorMsgs::PublisherJoyFeedbackArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherJoyFeedbackArray> Native;
};

struct FPublisherLaserEcho : public FROS2Publisher
{
	FPublisherLaserEcho(agxROS2::sensorMsgs::PublisherLaserEcho* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherLaserEcho> Native;
};

struct FPublisherLaserScan : public FROS2Publisher
{
	FPublisherLaserScan(agxROS2::sensorMsgs::PublisherLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherLaserScan> Native;
};

struct FPublisherMagneticField : public FROS2Publisher
{
	FPublisherMagneticField(agxROS2::sensorMsgs::PublisherMagneticField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherMagneticField> Native;
};

struct FPublisherMultiDOFJointState : public FROS2Publisher
{
	FPublisherMultiDOFJointState(agxROS2::sensorMsgs::PublisherMultiDOFJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherMultiDOFJointState> Native;
};

struct FPublisherMultiEchoLaserScan : public FROS2Publisher
{
	FPublisherMultiEchoLaserScan(agxROS2::sensorMsgs::PublisherMultiEchoLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherMultiEchoLaserScan> Native;
};

struct FPublisherNavSatStatus : public FROS2Publisher
{
	FPublisherNavSatStatus(agxROS2::sensorMsgs::PublisherNavSatStatus* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherNavSatStatus> Native;
};

struct FPublisherNavSatFix : public FROS2Publisher
{
	FPublisherNavSatFix(agxROS2::sensorMsgs::PublisherNavSatFix* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherNavSatFix> Native;
};

struct FPublisherPointCloud : public FROS2Publisher
{
	FPublisherPointCloud(agxROS2::sensorMsgs::PublisherPointCloud* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherPointCloud> Native;
};

struct FPublisherPointField : public FROS2Publisher
{
	FPublisherPointField(agxROS2::sensorMsgs::PublisherPointField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherPointField> Native;
};

struct FPublisherPointCloud2 : public FROS2Publisher
{
	FPublisherPointCloud2(agxROS2::sensorMsgs::PublisherPointCloud2* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherPointCloud2> Native;
};

struct FPublisherRange : public FROS2Publisher
{
	FPublisherRange(agxROS2::sensorMsgs::PublisherRange* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherRange> Native;
};

struct FPublisherRegionOfInterest : public FROS2Publisher
{
	FPublisherRegionOfInterest(agxROS2::sensorMsgs::PublisherRegionOfInterest* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherRegionOfInterest> Native;
};

struct FPublisherCameraInfo : public FROS2Publisher
{
	FPublisherCameraInfo(agxROS2::sensorMsgs::PublisherCameraInfo* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherCameraInfo> Native;
};

struct FPublisherRelativeHumidity : public FROS2Publisher
{
	FPublisherRelativeHumidity(agxROS2::sensorMsgs::PublisherRelativeHumidity* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherRelativeHumidity> Native;
};

struct FPublisherTemperature : public FROS2Publisher
{
	FPublisherTemperature(agxROS2::sensorMsgs::PublisherTemperature* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherTemperature> Native;
};

struct FPublisherTimeReference : public FROS2Publisher
{
	FPublisherTimeReference(agxROS2::sensorMsgs::PublisherTimeReference* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::PublisherTimeReference> Native;
};

struct FSubscriberBatteryState : public FROS2Subscriber
{
	FSubscriberBatteryState(agxROS2::sensorMsgs::SubscriberBatteryState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberBatteryState> Native;
};

struct FSubscriberChannelFloat32 : public FROS2Subscriber
{
	FSubscriberChannelFloat32(agxROS2::sensorMsgs::SubscriberChannelFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberChannelFloat32> Native;
};

struct FSubscriberCompressedImage : public FROS2Subscriber
{
	FSubscriberCompressedImage(agxROS2::sensorMsgs::SubscriberCompressedImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberCompressedImage> Native;
};

struct FSubscriberFluidPressure : public FROS2Subscriber
{
	FSubscriberFluidPressure(agxROS2::sensorMsgs::SubscriberFluidPressure* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberFluidPressure> Native;
};

struct FSubscriberIlluminance : public FROS2Subscriber
{
	FSubscriberIlluminance(agxROS2::sensorMsgs::SubscriberIlluminance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberIlluminance> Native;
};

struct FSubscriberImage : public FROS2Subscriber
{
	FSubscriberImage(agxROS2::sensorMsgs::SubscriberImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberImage> Native;
};

struct FSubscriberImu : public FROS2Subscriber
{
	FSubscriberImu(agxROS2::sensorMsgs::SubscriberImu* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberImu> Native;
};

struct FSubscriberJointState : public FROS2Subscriber
{
	FSubscriberJointState(agxROS2::sensorMsgs::SubscriberJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberJointState> Native;
};

struct FSubscriberJoy : public FROS2Subscriber
{
	FSubscriberJoy(agxROS2::sensorMsgs::SubscriberJoy* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberJoy> Native;
};

struct FSubscriberJoyFeedback : public FROS2Subscriber
{
	FSubscriberJoyFeedback(agxROS2::sensorMsgs::SubscriberJoyFeedback* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberJoyFeedback> Native;
};

struct FSubscriberJoyFeedbackArray : public FROS2Subscriber
{
	FSubscriberJoyFeedbackArray(agxROS2::sensorMsgs::SubscriberJoyFeedbackArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberJoyFeedbackArray> Native;
};

struct FSubscriberLaserEcho : public FROS2Subscriber
{
	FSubscriberLaserEcho(agxROS2::sensorMsgs::SubscriberLaserEcho* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberLaserEcho> Native;
};

struct FSubscriberLaserScan : public FROS2Subscriber
{
	FSubscriberLaserScan(agxROS2::sensorMsgs::SubscriberLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberLaserScan> Native;
};

struct FSubscriberMagneticField : public FROS2Subscriber
{
	FSubscriberMagneticField(agxROS2::sensorMsgs::SubscriberMagneticField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberMagneticField> Native;
};

struct FSubscriberMultiDOFJointState : public FROS2Subscriber
{
	FSubscriberMultiDOFJointState(agxROS2::sensorMsgs::SubscriberMultiDOFJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberMultiDOFJointState> Native;
};

struct FSubscriberMultiEchoLaserScan : public FROS2Subscriber
{
	FSubscriberMultiEchoLaserScan(agxROS2::sensorMsgs::SubscriberMultiEchoLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberMultiEchoLaserScan> Native;
};

struct FSubscriberNavSatStatus : public FROS2Subscriber
{
	FSubscriberNavSatStatus(agxROS2::sensorMsgs::SubscriberNavSatStatus* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberNavSatStatus> Native;
};

struct FSubscriberNavSatFix : public FROS2Subscriber
{
	FSubscriberNavSatFix(agxROS2::sensorMsgs::SubscriberNavSatFix* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberNavSatFix> Native;
};

struct FSubscriberPointCloud : public FROS2Subscriber
{
	FSubscriberPointCloud(agxROS2::sensorMsgs::SubscriberPointCloud* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberPointCloud> Native;
};

struct FSubscriberPointField : public FROS2Subscriber
{
	FSubscriberPointField(agxROS2::sensorMsgs::SubscriberPointField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberPointField> Native;
};

struct FSubscriberPointCloud2 : public FROS2Subscriber
{
	FSubscriberPointCloud2(agxROS2::sensorMsgs::SubscriberPointCloud2* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberPointCloud2> Native;
};

struct FSubscriberRange : public FROS2Subscriber
{
	FSubscriberRange(agxROS2::sensorMsgs::SubscriberRange* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberRange> Native;
};

struct FSubscriberRegionOfInterest : public FROS2Subscriber
{
	FSubscriberRegionOfInterest(agxROS2::sensorMsgs::SubscriberRegionOfInterest* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberRegionOfInterest> Native;
};

struct FSubscriberCameraInfo : public FROS2Subscriber
{
	FSubscriberCameraInfo(agxROS2::sensorMsgs::SubscriberCameraInfo* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberCameraInfo> Native;
};

struct FSubscriberRelativeHumidity : public FROS2Subscriber
{
	FSubscriberRelativeHumidity(agxROS2::sensorMsgs::SubscriberRelativeHumidity* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberRelativeHumidity> Native;
};

struct FSubscriberTemperature : public FROS2Subscriber
{
	FSubscriberTemperature(agxROS2::sensorMsgs::SubscriberTemperature* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberTemperature> Native;
};

struct FSubscriberTimeReference : public FROS2Subscriber
{
	FSubscriberTimeReference(agxROS2::sensorMsgs::SubscriberTimeReference* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxROS2::sensorMsgs::SubscriberTimeReference> Native;
};
