#include "LocationInfo.h"

#include <iomanip>

#include "DataUnit.h"


NS_BEGIN

LocationInfo::LocationInfo() :
	guid(0),
	position(cocos2d::Point::ZERO),
	time(0)
{
}

LocationInfo::~LocationInfo()
{
}

LocationInfo::LocationInfo(LocationInfo const& right) :
	Parcel(right)
{
	this->copyFrom(right);
}

LocationInfo& LocationInfo::operator=(LocationInfo const& right)
{
	if (this != &right)
	{
		Parcel::operator=(right);
		this->copyFrom(right);
	}

	return *this;
}

void LocationInfo::Clear()
{
	guid.clear();
	position = Point::ZERO;
	time = 0;

	Parcel::Clear();
}

size_t LocationInfo::sizeInBytes() const
{
	size_t totalSize = 0;
	totalSize += Parcel::uint32Size(guid.getRawValue());
	totalSize += kFloatSize * 2; // position
	totalSize += Parcel::int32Size(time);

	return totalSize;
}


bool LocationInfo::readFromStream(DataInputStream* input)
{
	uint32 _guid;
	CHECK_READ(Parcel::readUInt32(input, &_guid));
	guid.setRawValue(_guid);

	CHECK_READ(Parcel::readFloat(input, &position.x));
	CHECK_READ(Parcel::readFloat(input, &position.y));

	CHECK_READ(Parcel::readInt32(input, &time));

	return true;
}


void LocationInfo::writeToStream(DataOutputStream* output) const
{
	Parcel::writeUInt32(guid.getRawValue(), output);

	Parcel::writeFloat(position.x, output);
	Parcel::writeFloat(position.y, output);

	Parcel::writeInt32(time, output);

}

std::string LocationInfo::description() const
{
	std::ostringstream ss;

	ss << "LOCATOR LOCATION"
		<< std::setfill('0')
		<< " guid: 0x" << std::setw(8) << std::hex << guid.getRawValue()
		<< std::dec
		<< " pos: [" << position.x << ", " << position.y << "]"
		<< " time: " << time;

	return ss.str();
}


void LocationInfo::copyFrom(LocationInfo const& right)
{
	guid = right.guid;
	position = right.position;
	time = right.time;
}

NS_END

