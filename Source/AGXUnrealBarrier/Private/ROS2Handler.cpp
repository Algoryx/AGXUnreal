#include "ROS2Handler.h"

#include "agxROS2/Publisher.h"

#include <cstdint>

void FROS2Handler::SendPointCloud(const TArray<FVector4>& Points)
{
	// Send 45.2k points each time according to spec.
	static constexpr int NUM_POINTS_TO_SEND = 45200;
	static constexpr int POINT_SIZE = sizeof(FVector4);
	static constexpr int ELEM_SIZE = POINT_SIZE / 4;


	if (Points.Num() < NUM_POINTS_TO_SEND)
	{
		UE_LOG(LogTemp, Error, TEXT("Only got %d points in SendPointCloud!"), Points.Num());
		return;
	}

	static agxIO::ROS2::sensorMsgs::PublisherPointCloud2 publisher("point_cloud");
	static agxIO::ROS2::sensorMsgs::PointCloud2 msg;
	msg.data.clear(); // todo : look in anymessageparser for performance hints

	static uint32_t height = 0;

	msg.height = height++;
	msg.data.reserve(NUM_POINTS_TO_SEND * POINT_SIZE);
	for (int i = 0; i < NUM_POINTS_TO_SEND; i++)
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(&Points[i].X);
		// Don't do this in production code, here we assume that X,Y,Z,W is packed together in
		// sequence in memory.
		for (int j = 0; j < ELEM_SIZE * 2; j++) 
		{
			msg.data.push_back(p[j]);
		}
	}

	publisher.sendMessage(msg);
}
