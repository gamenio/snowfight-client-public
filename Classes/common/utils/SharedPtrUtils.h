#ifndef __SHARED_PTR_UTILS_H__
#define __SHARED_PTR_UTILS_H__

#include <memory>

#include "common/Common.h"

NS_BEGIN

namespace shared_ptr_utils {

template< class T, class U >
std::shared_ptr<T> staticPointerCast(std::shared_ptr<U> const& r) noexcept
{
	auto p = static_cast<typename std::shared_ptr<T>::element_type*>(r.get());
	return std::shared_ptr<T>(r, p);
}


template< class T, class U >
std::shared_ptr<T> dynamicPointerCast(std::shared_ptr<U> const& r) noexcept
{
	if (auto p = dynamic_cast<typename std::shared_ptr<T>::element_type*>(r.get())) 
	{
		return std::shared_ptr<T>(r, p);
	}
	else 
	{
		return std::shared_ptr<T>();
	}
}


template< class T, class U >
std::shared_ptr<T> constPointerCast(std::shared_ptr<U> const& r) noexcept
{
	auto p = const_cast<typename std::shared_ptr<T>::element_type*>(r.get());
	return std::shared_ptr<T>(r, p);
}

} //namespace shared_ptr_utils

NS_END

#endif //__SHARED_PTR_UTILS_H__