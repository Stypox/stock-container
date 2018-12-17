#ifndef _STYPOX_STOCK_CONTAINER_H_
#define _STYPOX_STOCK_CONTAINER_H_

#include <cstdlib>
#include <cstring>
#include <new>

namespace stypox {
	template<class T>
	class StockContainer;
	template<class T>
	class M_StockContainerHandler;

	template<class T>
	class M_StockContainerData {
		friend class StockContainer<T>;
		friend class M_StockContainerHandler<T>;

		M_StockContainerHandler<T>* iter;
	public:
		T value;
	};

	template<class T>
	class M_StockContainerHandler {
		friend class StockContainer<T>;
	public:
		using value_type = T;
	private:
		using data_type = M_StockContainerData<value_type>;

		data_type* m_data;

		M_StockContainerHandler(data_type* data);

		void update(data_type* data);
		void makeInvalid();

	public:
		M_StockContainerHandler(const M_StockContainerHandler<value_type>& other) = delete;
		M_StockContainerHandler(M_StockContainerHandler<value_type>&& other);

		M_StockContainerHandler& operator=(const M_StockContainerHandler<value_type>& other) = delete;
		M_StockContainerHandler& operator=(M_StockContainerHandler<value_type>&& other);

		~M_StockContainerHandler();

		value_type& operator*() const;
		value_type* operator->() const;

		bool operator==(nullptr_t) const;
	};

	template<class T>
	class StockContainer {
	public:
		using value_type = T;
		using iterator = M_StockContainerHandler<value_type>;
	private:
		using data_type = M_StockContainerData<value_type>;

		data_type* m_first,
				 * m_space,
				 * m_onePastLast;

		void reserveWithoutChecking(size_t newSize);
		void growIfNeeded();

		void updateAllIterators();
	public:
		StockContainer();
		StockContainer(const StockContainer& other) = delete;
		StockContainer(StockContainer&& other);

		StockContainer& operator=(const StockContainer& other) = delete;
		StockContainer& operator=(StockContainer&& other);

		~StockContainer();

		iterator push(const value_type& value);
		iterator push(value_type&& value);

		template<class... Args>
		iterator emplace(Args&&... value);

		data_type* data() const;

		bool empty() const;
		size_t size() const;
		size_t capacity() const;
		size_t used() const;

		void reserve(size_t newSize);
		void shrink_to_fit();
		void squash();
	};

	template<class T>
	M_StockContainerHandler<T>::M_StockContainerHandler(data_type* data) :
		m_data{data} {
		m_data->iter = this;
	}

	template<class T>
	void M_StockContainerHandler<T>::update(data_type* data) {
		m_data = data;
	}
	template<class T>
	void M_StockContainerHandler<T>::makeInvalid() {
		m_data = nullptr;
	}

	template<class T>
	M_StockContainerHandler<T>::M_StockContainerHandler(M_StockContainerHandler<value_type>&& other) :
		m_data{other.m_data} {
		other.m_data = nullptr;
		m_data->iter = this;
	}
	
	template<class T>
	auto M_StockContainerHandler<T>::operator=(M_StockContainerHandler<value_type>&& other) -> M_StockContainerHandler& {
		this->~M_StockContainerHandler();
		m_data = other.m_data;
		other.m_data = nullptr;
		m_data->iter = this;
		return *this;
	}

	template<class T>
	M_StockContainerHandler<T>::~M_StockContainerHandler() {
		if (m_data) {
			m_data->value.~value_type();
			m_data->iter = nullptr;
		}
	}

	template<class T>
	auto M_StockContainerHandler<T>::operator*() const -> value_type& {
		return m_data->value;
	}
	template<class T>
	auto M_StockContainerHandler<T>::operator->() const -> value_type* {
		return &m_data->value;
	}

	template<class T>
	bool M_StockContainerHandler<T>::operator==(nullptr_t) const {
		return m_data == nullptr;
	}



	template<class T>
	void StockContainer<T>::reserveWithoutChecking(size_t newCapacity) {
		size_t oldSize = size();

		m_first = static_cast<data_type*>(std::realloc(m_first, newCapacity * sizeof(data_type)));
		std::memset(m_first + oldSize, 0, (newCapacity - oldSize) * sizeof(data_type));

		m_space = m_first + oldSize;
		m_onePastLast = m_first + newCapacity;

		updateAllIterators();
	}
	template<class T>
	void StockContainer<T>::growIfNeeded() {
		if (m_space == m_onePastLast) {
			// growth similar to golden ratio
			reserveWithoutChecking(capacity() * 3/2 + 1);
		}
	}

	template<class T>
	void StockContainer<T>::updateAllIterators() {
		for (data_type* i = m_first; i < m_space; ++i) {
			if (i->iter)
				i->iter->update(i);
		}
	}

	template<class T>
	StockContainer<T>::StockContainer() :
		m_first{nullptr}, m_space{nullptr},
		m_onePastLast{nullptr} {}

	template<class T>
	StockContainer<T>::StockContainer(StockContainer&& other) :
		m_first{other.m_first}, m_space{other.m_space},
		m_onePastLast{other.m_onePastLast} {
		// other.m_space = other.m_onePastLast = nullptr
		// ^ this is not done since the other container will not be used anymore
		other.m_first = nullptr;
	}

	template<class T>
	auto StockContainer<T>::operator=(StockContainer&& other) -> StockContainer& {
		std::free(m_first);

		m_first = other.m_first;
		m_space = other.m_space;
		m_onePastLast = other.m_onePastLast;

		// other.m_space = other.m_onePastLast = nullptr
		// ^ this is not done since the other container will not be used anymore
		other.m_first = nullptr;
		return *this;
	}

	template<class T>
	StockContainer<T>::~StockContainer() {
		std::free(m_first);
	}

	template<class T>
	auto StockContainer<T>::push(const value_type& value) -> iterator {
		growIfNeeded();

		new(static_cast<void*>(&m_space->value)) value_type(value);

		return iterator{m_space++};
	}

	template<class T>
	void StockContainer<T>::reserve(size_t newCapacity) {
		if (newCapacity > capacity())
			reserveWithoutChecking(newCapacity);
	}
	template<class T>
	void StockContainer<T>::shrink_to_fit() {
		if (size_t newCapacity = size(); newCapacity) {
			m_first = static_cast<data_type*>(std::realloc(m_first, newCapacity * sizeof(data_type)));

			m_space = m_onePastLast = m_first + newCapacity;

			updateAllIterators();
		}
	}
	template<class T>
	void StockContainer<T>::squash() {
		data_type* fromBeginning = m_first, * fromEnd = m_space - 1;

		while (1) {
			if (fromBeginning == m_onePastLast) {
				m_space = m_onePastLast;
				return;
			}
			if (!fromBeginning->iter) {
				while (1) {
					if (fromEnd == fromBeginning) {
						m_space = fromBeginning;
						return;
					}
					if (fromEnd->iter) {
						std::memcpy(fromBeginning, fromEnd, sizeof(data_type));
						fromEnd->iter = nullptr;
						fromBeginning->iter->update(fromBeginning);

						--fromEnd;
						if (fromEnd == fromBeginning) {
							m_space = fromBeginning + 1;
							return;
						}
						break;
					}
					--fromEnd;
				}
			}
			++fromBeginning;
		}
	}

	template<class T>
	auto StockContainer<T>::data() const -> data_type* {
		return m_first;
	}	

	template<class T>
	bool StockContainer<T>::empty() const {
		return m_space == m_first;
	}
	template<class T>
	size_t StockContainer<T>::size() const {
		return m_space - m_first;
	}
	template<class T>
	size_t StockContainer<T>::capacity() const {
		return m_onePastLast - m_first;
	}
	template<class T>
	size_t StockContainer<T>::used() const {
		size_t counter = 0;
		for (data_type* i = m_first; i < m_space; ++i) {
			if (i->iter)
				++counter;
		}
		return counter;
	}
}

#endif
