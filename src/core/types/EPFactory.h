#ifndef EP_FACTORY_H_
#define EP_FACTORY_H_

#include "core/ehm/ehm.h"

// epoch factory
// epoch/src/core/types/EPProcess.h

// A factory pattern

#include <type_traits>

#include "core/types/EPObj.h"
#include "core/types/epref.h"

// TODO: Fix this, C++20!!
#ifndef NOCONCEPTS
	template <typename TEPObj> requires EPObjectDerived<TEPObj>
#else
	template <typename TEPObj>
#endif
class EPFactory : 
	public EPObj
	// TODO: singleton?
	
{
public:
	class EPObjectPackage {
	public:
		RESULT r;
		EPRef<TEPObj> pEPObj;

		EPObjectPackage(RESULT r, EPRef<TEPObj> pEPObj) :
			r(r),
			pEPObj(pEPObj)
		{}

		~EPObjectPackage() {
			pEPObj = nullptr;
		}
	};

protected:
	EPFactory() {
		// 
	}

	~EPFactory() {
		// 
	}

public:
	// TODO: Might make more sense to return "object package"
	static EPObjectPackage make() {
		RESULT r = R::OK;

		EPRef<TEPObj> pEPObj = TEPObj::InternalMake();
		CNM(pEPObj, "Failed to create factory object");

	Error:
		return EPObjectPackage(r, pEPObj);
	}

private:
	//virtual epref<TEPObj> InternalMake() = 0;
};

#endif // EP_FACTORY_H_