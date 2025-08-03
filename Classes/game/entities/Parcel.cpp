#include "Parcel.h"

NS_BEGIN

Parcel::Parcel() :
	m_cachedSize(0)
{
}

Parcel::Parcel(Parcel const& right)
{
	this->copyFrom(right);
}

Parcel::Parcel(Parcel&& right) :
	m_cachedSize(0)
{
	this->moveFrom(right);
}

Parcel& Parcel::operator=(Parcel const& right)
{
	if (this != &right)
		this->copyFrom(right);

	return *this;
}

void Parcel::Clear()
{
	m_cachedSize = 0;
}

bool Parcel::MergePartialFromCodedStream(::google::protobuf::io::CodedInputStream* input)
{

	if (!this->readFromStream(input))
		return false;

	bool ret = input->ExpectAtEnd();
	NS_ASSERT_LOG(ret, "Parcel did not read all bytes as expected.");

	return ret;
}

void Parcel::SerializeWithCachedSizes(::google::protobuf::io::CodedOutputStream* output) const
{
	this->writeToStream(output);

	NS_ASSERT_LOG(m_cachedSize == output->ByteCount() && !output->HadError(), "Byte size calculation and serialization of Parcel were inconsistent. ");
}

size_t Parcel::ByteSizeLong() const
{
	size_t totalSize = 0;
	totalSize += this->sizeInBytes();

	m_cachedSize = ::google::protobuf::internal::ToCachedSize(totalSize);
	return totalSize;
}

void Parcel::copyFrom(Parcel const& right)
{
	m_cachedSize = right.m_cachedSize;
}

void Parcel::moveFrom(Parcel& right)
{
	std::swap(m_cachedSize, right.m_cachedSize);
}




NS_END