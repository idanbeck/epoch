#ifndef EP_DYNAMIC_STORAGE_H_
#define EP_DYNAMIC_STORAGE_H_

#include "core/ehm/ehm.h"

// epoch vector (storage) class
// epoch/src/core/types/EPVector.h

// EP Vector storage class that works the way WE FUCKING WANT IT TO

#include "core/types/EPObj.h"
#include "core/types/EPRef.h"

// TODO: Needs test suite (all the core shits need test suites)

template <typename TStorage>
class EPVector {

public:
	class iterator {
	public:
		iterator(TStorage* pValue) noexcept : m_pValue(pValue) {}
		TStorage& operator*() { return (*m_pValue); }
		TStorage* operator->() { return m_pValue; }

		bool operator==(const iterator& rhs) { return m_pValue == rhs.m_pValue; }
		bool operator!=(const iterator& rhs) { return m_pValue != rhs.m_pValue; }

		iterator& operator++() {
			++m_pValue;
			return *this;
		}

		iterator operator++(int) {
			++m_pValue;
			return *this;
		}

		iterator& operator--() {
			--m_pValue;
			return *this;
		}

		iterator operator--(int) {
			--m_pValue;
			return *this;
		}

	protected:
		TStorage *m_pValue = nullptr;
	};

	class const_iterator : public iterator {
	public:
		const_iterator(TStorage* pValue) noexcept : iterator(pValue) {}
		TStorage& operator*() { return (*(this->m_pValue)); }
		TStorage* operator->() { return this->m_pValue; }
        bool operator==(const const_iterator& rhs) { return this->m_pValue == rhs.m_pValue; }
        bool operator!=(const const_iterator& rhs) { return this->m_pValue != rhs.m_pValue; }
    };

	iterator begin() noexcept { 
		return iterator(&m_pBuffer[0]); 
	}

	const_iterator begin() const noexcept {
		return const_iterator(&m_pBuffer[0]);
	}

	iterator end() noexcept { 
		return iterator(&m_pBuffer[m_pBuffer_c]);
	}

	const_iterator end() const noexcept {
		return const_iterator(&m_pBuffer[m_pBuffer_c]);
	}

public:
	EPVector() :
		m_pBuffer_n(0),
		m_pBuffer(nullptr),
		m_pBuffer_c(0)
	{
		//
	}

	~EPVector() {
		if (m_pBuffer != nullptr) {
			free(m_pBuffer);
			m_pBuffer = nullptr;
		}

		m_pBuffer_c = 0;
		m_pBuffer_n = 0;
	}

	EPVector(size_t initialSize, const TStorage& initValue) :
		m_pBuffer_n(initialSize),
		m_pBuffer_c(m_pBuffer_n)
	{
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));
		for (int i = 0; i < m_pBuffer_c; i++)
			m_pBuffer[i] = initValue;
	}

	EPVector(size_t initialSize, bool fPopulate = false) :
		m_pBuffer_n(initialSize),
		m_pBuffer_c(fPopulate ? initialSize : 0)
	{
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));
		memset(m_pBuffer, 0, m_pBuffer_n * sizeof(TStorage));
	}

	EPVector(std::initializer_list<TStorage> list) :
		m_pBuffer_n(list.size()),
		m_pBuffer_c(0)
	{
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));
		for (auto &val : list)
			m_pBuffer[m_pBuffer_c++] = val;
	}

	EPVector& operator=(std::initializer_list<TStorage> list) {
		m_pBuffer_n = list.size();
		m_pBuffer_c = 0;
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));

		for (auto& val : list)
			m_pBuffer[m_pBuffer_c++] = val;

		return *this;
	}

	EPVector(const TStorage staticValueArray[], size_t staticValueArray_n) {
		m_pBuffer_n = staticValueArray_n;
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));

		memset(m_pBuffer, 0, m_pBuffer_n);
		memcpy(m_pBuffer, staticValueArray, m_pBuffer_n);

		m_pBuffer_c = m_pBuffer_n;
	}

	EPVector(const EPVector& rhs) {
		if (m_pBuffer != nullptr) {
			delete[] m_pBuffer;
			m_pBuffer = nullptr;
		}

		m_pBuffer_n = rhs.m_pBuffer_n;
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));
		m_pBuffer_c = rhs.m_pBuffer_c;
		
		memset(m_pBuffer, 0, m_pBuffer_n);
		memcpy(m_pBuffer, rhs.m_pBuffer, m_pBuffer_n);
	}

	EPVector& operator=(const EPVector& rhs) {
		if (m_pBuffer != nullptr) {
			delete[] m_pBuffer;
			m_pBuffer = nullptr;
		}

		m_pBuffer_n = rhs.m_pBuffer_n;
		m_pBuffer = (TStorage*)malloc(m_pBuffer_n * sizeof(TStorage));
		m_pBuffer_c = rhs.m_pBuffer_c;

		memset(m_pBuffer, 0, m_pBuffer_n);
		memcpy(m_pBuffer, rhs.m_pBuffer, m_pBuffer_n);

		return *this;
	}

	EPVector(EPVector&& rhs) {
		m_pBuffer_n = rhs.m_pBuffer_n;
		m_pBuffer = rhs.m_pBuffer;
		m_pBuffer_c = rhs.m_pBuffer_c;
		rhs.m_pBuffer = nullptr;
	}

	EPVector& operator=(EPVector&& rhs) {
		if (m_pBuffer != nullptr) {
			delete[] m_pBuffer;
			m_pBuffer = nullptr;
		}

		m_pBuffer_n = rhs.m_pBuffer_n;
		m_pBuffer = rhs.m_pBuffer;
		m_pBuffer_c = rhs.m_pBuffer_c;

		rhs.m_pBuffer = nullptr;

		return *this;
	}

	inline RESULT Allocate(size_t newSize) {
		RESULT r = R::OK;

		size_t pTempBuffer_n = newSize;
		TStorage* pTempBuffer = (TStorage*)malloc(pTempBuffer_n * sizeof(TStorage));
		CNR(pTempBuffer, R::MEMORY_ALLOCATION_FAILED);

		// set to zero and swap
		memset((void*)pTempBuffer, 0, sizeof(TStorage) * pTempBuffer_n);	// this will ensure null termination

		std::swap(pTempBuffer, m_pBuffer);
		std::swap(pTempBuffer_n, m_pBuffer_n);

	Error:
		if (pTempBuffer != nullptr) {
			free(pTempBuffer);		
			pTempBuffer = nullptr;
		}

		return r;
	}

	//RESULT PushBack(const TStorage& value) noexcept  {	
	RESULT PushBack(TStorage value) noexcept {
		RESULT r = R::OK;

		if (m_pBuffer == nullptr) {
			if (this->Allocate(1) != R::OK)
				return R::MEMORY_ALLOCATION_FAILED;
		}

		if (m_pBuffer_c == m_pBuffer_n) {
			if (DoubleSize() != R::OK)
				return R::MEMORY_ALLOCATION_FAILED;
		}

		m_pBuffer[m_pBuffer_c++] = value;

	Error:
		return r;
	}

	RESULT PushBackUnique(const TStorage &value) noexcept {
		if (exists(value))
			return R::NOT_UNIQUE;

		return PushBack(value);
	}

	inline RESULT PushFront(const TStorage& value) noexcept {
		if (m_pBuffer == nullptr) {
			if (this->Allocate(1) != R::OK)
				return R::MEMORY_ALLOCATION_FAILED;
		}

		if (ShiftRight() != R::OK)
			return R::MEMORY_ALLOCATION_FAILED;

		m_pBuffer[0] = value;

		return R::OK;
	}

	RESULT SetAll(const TStorage& value) noexcept {
		for (size_t i = 0; i < m_pBuffer_c; i++) {
			m_pBuffer[i] = value;
		}

		return R::OK;
	}

	bool CompareAll(const TStorage& value) noexcept {
		for (size_t i = 0; i < m_pBuffer_c; i++) {
			if (m_pBuffer[i] != value)
				return false;
		}

		return true;
	}

	// This is just providing a pointer to the data
	const TStorage* GetCBuffer() const {
		const TStorage* pData = (const TStorage*)m_pBuffer;
		return pData;
	}

	bool empty() const {
		return (m_pBuffer_c == 0);
	}

	const TStorage* data() const {
		return m_pBuffer;
	}

	TStorage* data() {
		return m_pBuffer;
	}

	TStorage* data(size_t counterSetValue) {
		if (counterSetValue > m_pBuffer_n)
			return nullptr;

		m_pBuffer_c = counterSetValue;
		return m_pBuffer;
	}

	const TStorage& operator[](size_t index) const { 
		return m_pBuffer[index]; 
	}

	TStorage& operator [](size_t index) { 
		return m_pBuffer[index];
	}

	size_t size() const {
		return m_pBuffer_c;
	}

	size_t byte_size() const {
		return (m_pBuffer_c * sizeof(TStorage));
	}

	bool exists(const TStorage &obj) {
		for (int i = 0; i < m_pBuffer_c; i++) {
			if (obj == m_pBuffer[i])
				return true;
		}

		return false;
	}
	
	inline void clear(bool fDeallocate = true) {
		if (fDeallocate) {
			if (m_pBuffer != nullptr) {
				free(m_pBuffer);
				m_pBuffer = nullptr;
			}
			m_pBuffer_c = 0;
			m_pBuffer_n = 0;
		}
		else {
			memset(m_pBuffer, 0, sizeof(TStorage) * m_pBuffer_n);
		}
	}

	inline void destroy(bool fDeallocate = true) {
		for (size_t i = 0; i < m_pBuffer_c; i++) {
//			if (typeid(TStorage) == typeid(EPRef)) {
//				m_pBuffer[i] = nullptr;
//			}
//			else {
				(m_pBuffer[i]).~TStorage();
				m_pBuffer[i] = nullptr;
			//}
		}

		clear(fDeallocate);
	}

private:
	inline RESULT ShiftRight() noexcept  {
		if (m_pBuffer_c == m_pBuffer_n) {
			if (DoubleSize() != R::OK)
				return R::MEMORY_ALLOCATION_FAILED;
		}

		memcpy(&m_pBuffer[1], m_pBuffer, m_pBuffer_c * sizeof(TStorage));
		memset(&m_pBuffer[0], 0, sizeof(TStorage));

		m_pBuffer_c++;

		return R::OK;
	}

	inline RESULT DoubleSize() noexcept {
		size_t pTempBuffer_n = m_pBuffer_n << 1;
		TStorage* pTempBuffer = (TStorage*)malloc(pTempBuffer_n * sizeof(TStorage));  

		if(pTempBuffer == nullptr)
			return R::MEMORY_ALLOCATION_FAILED;

		// Copy and swap
		memset((void*)pTempBuffer, 0, sizeof(TStorage) * pTempBuffer_n);	// this will ensure null termination
		memcpy((void*)pTempBuffer, (void*)m_pBuffer, sizeof(TStorage) * m_pBuffer_n);

		std::swap(pTempBuffer, m_pBuffer);
		std::swap(pTempBuffer_n, m_pBuffer_n);

		if (pTempBuffer != nullptr) {
			free(pTempBuffer);  
			pTempBuffer = nullptr;
		}

		return R::OK;
	}

private:
	TStorage* m_pBuffer = nullptr;
	size_t m_pBuffer_n = 0;
	size_t m_pBuffer_c = 0;
};

#endif // ! EP_DYNAMIC_STORAGE_H_