#ifndef EP_DYNAMIC_STORAGE_H_
#define EP_DYNAMIC_STORAGE_H_

#include "core/ehm/ehm.h"

// epoch dynamic storage class
// epoch/src/core/types/EPDynamicStorage.h

// Dynamic storage (similar to vector) class that works the way WE FUCKING WANT IT TO

#include "core/types/EPObj.h"

// TODO: Needs test suite (all the core shits need test suites)

template <typename TStorage>
class EPDynamicStorage {
public:
	EPDynamicStorage() :
		m_pBuffer_n(0),
		m_pBuffer(nullptr),
		m_pBuffer_c(0)
	{
		//
	}

	EPDynamicStorage(size_t initialSize, const TStorage& initValue) :
		m_pBuffer_n(initialSize),
		m_pBuffer_c(m_pBuffer_n)
	{
		m_pBuffer = new TStorage[m_pBuffer_n];
	}

	EPDynamicStorage(const TStorage staticValueArray[], size_t staticValueArray_n) {
		m_pBuffer_n = staticValueArray_n;
		m_pBuffer = new TStorage[m_pBuffer_n];

		memset(m_pBuffer, 0, m_pBuffer_n);
		memcpy(m_pBuffer, staticValueArray, m_pBuffer_n);

		m_pBuffer_c = m_pBuffer_n;
	}

	RESULT Allocate(size_t newSize) {
		RESULT r = R::OK;

		size_t pTempBuffer_n = newSize;
		TStorage* pTempBuffer = new TStorage[pTempBuffer_n];
		CNR(pTempBuffer, R::MEMORY_ALLOCATION_FAILED);

		// set to zero and swap
		memset(pTempBuffer, 0, pTempBuffer_n);	// this will ensure null termination 

		std::swap(pTempBuffer, m_pBuffer);
		std::swap(pTempBuffer_n, m_pBuffer_n);

	Error:
		if (pTempBuffer != nullptr) {
			delete[] pTempBuffer;
			pTempBuffer = nullptr;
		}

		return r;
	}

	RESULT PushBack(const TStorage& value) {
		RESULT r = R::OK;

		if (m_pBuffer == nullptr) {
			CR(this->Allocate(1));
		}

		if (m_pBuffer_c == m_pBuffer_n) {
			CR(this->DoubleSize());
		}

		m_pBuffer[m_pBuffer_c++] = value;

	Error:
		return r;
	}

	RESULT PushFront(const TStorage& value) {
		RESULT r = R::OK;

		if (m_pBuffer_c == m_pBuffer_n) {
			CR(this->DoubleSize());
		}

		CR(ShiftRight());

		m_pBuffer[0] = value;

	Error:
		return r;
	}

	// This is just providing a pointer to the data
	const TStorage* GetCBuffer() {
		TStorage* pData = (TStorage*)m_pBuffer;
		return pData;
	}

private:
	RESULT ShiftRight() {
		RESULT r = R::OK;

		if (m_pBuffer_c == m_pBuffer_n) {
			CR(this->DoubleSize());
		}
		
		TStorage lastValue;
		for (size_t i = 0; i <= m_pBuffer_c; i++) {
			TStorage tempVal = m_pBuffer[i];

			if (i == 0) 
				m_pBuffer[i] = 0;
			else 
				m_pBuffer[i] = lastValue;

			lastValue = tempVal;
		}

		m_pBuffer_c++;

	Error:
		return r;
	}

	RESULT DoubleSize() {
		RESULT r = R::OK;

		size_t pTempBuffer_n = m_pBuffer_n << 1;
		TStorage* pTempBuffer = new TStorage[pTempBuffer_n];
		CNR(pTempBuffer, R::MEMORY_ALLOCATION_FAILED);

		// Copy and swap
		memset(pTempBuffer, 0, pTempBuffer_n);	// this will ensure null termination 
		memcpy(pTempBuffer, m_pBuffer, m_pBuffer_n);

		std::swap(pTempBuffer, m_pBuffer);
		std::swap(pTempBuffer_n, m_pBuffer_n);

	Error:
		if (pTempBuffer != nullptr) {
			delete[] pTempBuffer;
			pTempBuffer = nullptr;
		}

		return r;
	}

private:
	TStorage* m_pBuffer = nullptr;
	size_t m_pBuffer_n = 0;
	size_t m_pBuffer_c = 0;
};

#endif // ! EP_DYNAMIC_STORAGE_H_