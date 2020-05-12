#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "core/ehm/result.h"

// epoch math matrix transform
// epoch/src/core/math/matrix/transform.h

// A base transform matrix class that other transformation matrices should
// derive from 

#include "core/math/matrix/matrix.h"

template <typename TValue = float>
class transform :
	public matrix<TValue, 4, 4>
{
public:
	enum class type : uint8_t {
		TRANSFORM,
		PROJECTION,
		ROTATION,
		TRANSLATION,
		SCALE,
		CUSTOM
	};

public:
	transform() {
		identity(1.0f);
	}

	~transform() = default;

	virtual GetType() { return type::TRANSFORM; }
};


#endif // ! TRANSFORM_H_