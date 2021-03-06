#include "TypesTestSuite.h"

#define TEST_INT_ARRAY_LENGTH 100000

#include <vector>

#include "test/EPTest.h"
#include "test/EPTestCase.h"

RESULT TypesTestSuite::TestEPVector(EPTestBase *pEPTestBase) {
	RESULT r = R::OK;

	EPVector<int> IntEPVector;
	std::vector<int> IntSTLVector;

	int IntArray[TEST_INT_ARRAY_LENGTH];
	

	// EPVector test case labels
	const char *kConstruct = "construct";
	const char *kPushBack = "pushback";
	const char *kCheck = "check";
	const char *kPushFront = "pushfront";
	const char* kForEach = "foreach";
	const char* kForIterator = "foriterator";

	const char* kEPVector = "EPVector";
	const char* kSTLVector = "std::vector";

	EPTest<RESULT(EPTestBase*)>* pEPTest = dynamic_cast<EPTest<RESULT(EPTestBase*)>*>(pEPTestBase);
	CNM(pEPTest, "EPTest is nullptr");

	// Initialize int array
	for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
		IntArray[i] = i;
	}

	// Case 1 - Check push back
	// TODO: get rid of result return

	pEPTest->RegisterAndRunTC(kConstruct, kEPVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				IntEPVector = EPVector<int>();
				return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kPushBack, kEPVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
					IntEPVector.PushBack(i);
				}
				return R::OK;
			}
	));

	CBM(IntEPVector.size() == TEST_INT_ARRAY_LENGTH,
		"Size %zu differs from expected size %d", 
			IntEPVector.size(), TEST_INT_ARRAY_LENGTH);

	pEPTest->RegisterAndRunTC(kCheck, kEPVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
					if (IntEPVector[i] != i) {
						DEBUG_LINEOUT("intArray[%d]:%d differed from value %d expected", i, IntEPVector[i], i);
						return R::FAIL;
					}
				}
				return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kForEach, kEPVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				int count = 0;
				for (auto &val : IntEPVector) {
					if (val != count) {
						DEBUG_LINEOUT("intArray[%d]:%d differed from value %d expected", count, val, count);
						return R::FAIL;
					}
					count++;
				}
				return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kForIterator, kEPVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				int count = 0;
				for (auto it = IntEPVector.begin(); it != IntEPVector.end(); it++) {
					int val = *it;
					if (val != count) {
						DEBUG_LINEOUT("intArray[%d]:%d differed from value %d expected", count, val, count);
						return R::FAIL;
					}
					count++;
				}
				return R::OK;
			}
	));

	// Case 2 - Check front insertion
	IntEPVector = EPVector<int>();

	pEPTest->RegisterAndRunTC(kPushFront, kEPVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				for (int i = TEST_INT_ARRAY_LENGTH - 1; i >= 0; i--) {
					IntEPVector.PushFront(i);
				}
				return R::OK;
			}
	));

	CBM(IntEPVector.size() == TEST_INT_ARRAY_LENGTH,
		"Size %zu differs from expected size %d", 
			IntEPVector.size(), TEST_INT_ARRAY_LENGTH);

	for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
		CBM(IntEPVector[i] == i, 
			"intArray[%d]:%d differed from value %d expected", i, IntEPVector[i], i);
	}

	// Case 3 - Test performance against std::vector

	pEPTest->RegisterAndRunTC(kConstruct, kSTLVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				IntSTLVector = std::vector<int>();
			return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kPushBack, kSTLVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
					IntSTLVector.push_back(i);
				}
				return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kCheck, kSTLVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
					if (IntSTLVector[i] != i) {
						DEBUG_LINEOUT("intArray[%d]:%d differed from value %d expected", i, IntEPVector[i], i);
						return R::FAIL;
					}
				}			
				return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kForEach, kSTLVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				
				int count = 0;
				for (auto& val : IntSTLVector) {
					if (val != count) {
						DEBUG_LINEOUT("stdIntArray[%d]:%d differed from value %d expected", count, val, count);
						return R::FAIL;
					}
					count++;
				}

				return R::OK;
			}
	));

	pEPTest->RegisterAndRunTC(kForIterator, kSTLVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				int count = 0;
				for (auto it = IntSTLVector.begin(); it != IntSTLVector.end(); it++) {
					int val = *it;
					if (val != count) {
						DEBUG_LINEOUT("intArray[%d]:%d differed from value %d expected", count, val, count);
						return R::FAIL;
					}
					count++;
				}
				return R::OK;
			}
	));

	IntSTLVector = std::vector<int>();

	pEPTest->RegisterAndRunTC(kPushFront, kSTLVector, EPTestCase::expected::COMPARE,
		EPTimedFunction<RESULT(void)>(
			[&]() -> RESULT {
				for (int i = TEST_INT_ARRAY_LENGTH - 1; i >= 0; i--) {
					IntSTLVector.insert(IntSTLVector.begin(), i);
				}
				return R::OK;
			}
	));

	for (int i = 0; i < TEST_INT_ARRAY_LENGTH; i++) {
		CBM(IntSTLVector[i] == i, "intArray[%d]:%d differed from value %d expected", i, IntEPVector[i], i);
	}
	
Error:
	return r;
}