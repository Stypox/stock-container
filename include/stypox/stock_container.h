#ifndef _STYPOX_STOCK_CONTAINER_H_
#define _STYPOX_STOCK_CONTAINER_H_

#include <cstdlib>
#include <cstring>
#include <vector>
#include <stdexcept>

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
	class M_StockContainerIterator {
		friend class StockContainer<T>;
	public:
		using value_type = T;
		using difference_type = ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::random_access_iterator_tag;
	private:
		using data_type = M_StockContainerData<value_type>;

		data_type* m_pointer;

		M_StockContainerIterator(data_type* p) : m_pointer{p} {};
	public:
		M_StockContainerIterator(const M_StockContainerIterator&) = default;
		M_StockContainerIterator(M_StockContainerIterator&&) = default;
		M_StockContainerIterator& operator=(const M_StockContainerIterator&) = default;
		M_StockContainerIterator& operator=(M_StockContainerIterator&&) = default;

		reference operator*() const { return m_pointer->value; }
		pointer operator->() const { return &m_pointer->value; }
		reference operator[](difference_type n) const { return m_pointer[n].value; }

		M_StockContainerIterator& operator++() { ++m_pointer; return *this; }
		M_StockContainerIterator operator++(int) { return m_pointer++; }
		M_StockContainerIterator& operator--() { --m_pointer; return *this; }
		M_StockContainerIterator operator--(int) { return m_pointer--; }

		M_StockContainerIterator& operator+=(difference_type n) { m_pointer += n; return *this; }
		M_StockContainerIterator& operator-=(difference_type n) { m_pointer -= n; return *this; }

		M_StockContainerIterator operator+(difference_type n) const { return m_pointer + n; }
		friend M_StockContainerIterator operator+(difference_type n, const M_StockContainerIterator& iter) { return iter.m_pointer + n; }
		M_StockContainerIterator operator-(difference_type n) const { return m_pointer - n; }
		difference_type operator-(const M_StockContainerIterator& other) const { return m_pointer - other.m_pointer; }

		bool operator==(const M_StockContainerIterator& other) const { return m_pointer == other.m_pointer; }
		bool operator!=(const M_StockContainerIterator& other) const { return m_pointer != other.m_pointer; }
		bool operator<(const M_StockContainerIterator& other) const { return m_pointer < other.m_pointer; }
		bool operator>(const M_StockContainerIterator& other) const { return m_pointer > other.m_pointer; }
		bool operator<=(const M_StockContainerIterator& other) const { return m_pointer <= other.m_pointer; }
		bool operator>=(const M_StockContainerIterator& other) const { return m_pointer >= other.m_pointer; }
	};

	template<class T>
	class M_StockContainerHandler {
		friend class StockContainer<T>;
	public:
		using value_type = T;
	private:
		using data_type = M_StockContainerData<value_type>;

		data_type* m_data;

		M_StockContainerHandler(data_type* data) : m_data{data} { m_data->iter = this; }

		void update(data_type* data) { m_data = data; }

	public:
		M_StockContainerHandler(const M_StockContainerHandler<value_type>& other) = delete;
		M_StockContainerHandler(M_StockContainerHandler<value_type>&& other);

		M_StockContainerHandler& operator=(const M_StockContainerHandler<value_type>& other) = delete;
		M_StockContainerHandler& operator=(M_StockContainerHandler<value_type>&& other);

		~M_StockContainerHandler();
		void remove();

		value_type& operator*() const { return m_data->value; }
		value_type* operator->() const { return &m_data->value; }

		bool operator==(nullptr_t) const { return m_data == nullptr; }
		bool operator!=(nullptr_t) const { return m_data != nullptr; }
		operator bool() const { return m_data != nullptr; }
	};

	template<class T>
	class StockContainer {
	public:
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = typename std::pointer_traits<pointer>::template rebind<const value_type>;
		using iterator = M_StockContainerIterator<value_type>;
		using const_iterator = const M_StockContainerIterator<value_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using reverse_const_iterator = std::reverse_iterator<const_iterator>;
		using handler = M_StockContainerHandler<value_type>;
	private:
		using data_type = M_StockContainerData<value_type>;

		data_type* m_first,
				 * m_space,
				 * m_onePastLast;

		void reserveWithoutChecking(size_type newSize);
		void growIfNeeded();

		void updateAllIterators();
	public:
		StockContainer() : m_first{nullptr}, m_space{nullptr}, m_onePastLast{nullptr} {}
		StockContainer(const StockContainer& other) = delete;
		StockContainer(StockContainer&& other);

		StockContainer& operator=(const StockContainer& other) = delete;
		StockContainer& operator=(StockContainer&& other);

		~StockContainer();

		handler push(const value_type& value);
		handler push(value_type&& value);
		template<class... Args>
		handler emplace(Args&&... value);

		std::vector<handler> append(size_type count, const value_type& value);
		template<class InputIt>
		std::vector<handler> append(InputIt first, InputIt last);
		std::vector<handler> append(std::initializer_list<value_type> ilist) { return append(ilist.begin(), ilist.end()); }

		data_type* data() const { return m_first; }

		bool empty() const { return m_space == m_first; }
		size_type size() const { return m_space - m_first; }
		size_type capacity() const { return m_onePastLast - m_first; }
		size_type used() const;

		void reserve(size_type newSize);
		void shrink_to_fit();
		void squash();

		value_type& operator[](size_type n) { return m_first[n].value; }
		value_type& at(size_type n) { if (n >= size()) throw std::out_of_range{"stypox::StockContainer::at()"}; return m_first[n].value; }
		value_type& back() { return m_space[-1].value; }
		value_type& front() { return m_first->value; }

		void swap(StockContainer<value_type>& other);

		bool operator==(const StockContainer<value_type>& other) const { return std::equal(begin(), end(), other.begin(), other.end()); }
		bool operator!=(const StockContainer<value_type>& other) const { return !(*this == other); }
		bool operator<(const StockContainer<value_type>& other) const { return std::lexicographical_compare(begin(), end(), other.begin(), other.end()); }
		bool operator<=(const StockContainer<value_type>& other) const { return !(other < *this); }
		bool operator>(const StockContainer<value_type>& other) const { return other < *this; }
		bool operator>=(const StockContainer<value_type>& other) const { return !(*this < other); }

		iterator begin() { return m_first; }
		const_iterator begin() const { return m_first; }
		const_iterator cbegin() const { return m_first; }
		iterator end() { return m_space; }
		const_iterator end() const { return m_space; }
		const_iterator cend() const { return m_space; }

		reverse_iterator rbegin() { return reverse_iterator{m_space}; }
		reverse_const_iterator rbegin() const { return reverse_const_iterator{m_space}; }
		reverse_const_iterator crbegin() const { return reverse_const_iterator{m_space}; }
		reverse_iterator rend() { return reverse_iterator{m_first}; }
		reverse_const_iterator rend() const { return reverse_const_iterator{m_first}; }
		reverse_const_iterator crend() const { return reverse_const_iterator{m_first}; }
	};

	template<class T>
	M_StockContainerHandler<T>::M_StockContainerHandler(M_StockContainerHandler<value_type>&& other) :
		m_data{other.m_data} {
		other.m_data = nullptr;
		if (m_data)
			m_data->iter = this;
	}
	
	template<class T>
	auto M_StockContainerHandler<T>::operator=(M_StockContainerHandler<value_type>&& other) -> M_StockContainerHandler& {
		this->~M_StockContainerHandler();
		m_data = other.m_data;
		other.m_data = nullptr;
		if (m_data)
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
	void M_StockContainerHandler<T>::remove() {
		if (m_data) {
			m_data->value.~value_type();
			m_data->iter = nullptr;
			m_data = nullptr;
		}
	}



	template<class T>
	void StockContainer<T>::reserveWithoutChecking(size_type newCapacity) {
		size_type oldSize = size();

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
	StockContainer<T>::StockContainer(StockContainer&& other) :
		m_first{other.m_first}, m_space{other.m_space},
		m_onePastLast{other.m_onePastLast} {
		// other.m_onePastLast = nullptr
		// ^ this is not done since the other container will not be used anymore
		// and the destructor only needs m_first and m_space
		other.m_first = other.m_space = nullptr;
	}

	template<class T>
	auto StockContainer<T>::operator=(StockContainer&& other) -> StockContainer& {
		this->~StockContainer();

		m_first = other.m_first;
		m_space = other.m_space;
		m_onePastLast = other.m_onePastLast;

		// other.m_onePastLast = nullptr
		// ^ this is not done since the other container will not be used anymore
		// and the destructor only needs m_first and m_space
		other.m_first = other.m_space = nullptr;
		return *this;
	}

	template<class T>
	StockContainer<T>::~StockContainer() {
		for (data_type* i = m_first; i < m_space; ++i) {
			if (i->iter)
				i->iter->remove();
		}
		std::free(m_first);
	}

	template<class T>
	auto StockContainer<T>::push(const value_type& value) -> handler {
		growIfNeeded();
		new(static_cast<void*>(&m_space->value)) value_type{value};
		return handler{m_space++};
	}
	template<class T>
	auto StockContainer<T>::push(value_type&& value) -> handler {
		growIfNeeded();
		new(static_cast<void*>(&m_space->value)) value_type{value};
		return handler{m_space++};
	}
	template<class T>
	template<class... Args>
	auto StockContainer<T>::emplace(Args&&... value) -> handler {
		growIfNeeded();
		new(static_cast<void*>(&m_space->value)) value_type{value...};
		return handler{m_space++};
	}

	template<class T>
	auto StockContainer<T>::append(size_type count, const value_type& value) -> std::vector<handler> {
		reserveWithoutChecking(size() + count);

		std::vector<handler> result;
		for (size_type c = 0; c < count; ++c, ++m_space) {			
			new(static_cast<void*>(&m_space->value)) value_type{value};
			result.emplace_back(handler{m_space});
		}

		return result;
	}
	template<class T>
	template<class InputIt>
	auto StockContainer<T>::append(InputIt first, InputIt last) -> std::vector<handler> {
		reserveWithoutChecking(size() + (last - first));

		std::vector<handler> result;
		for (; first != last; ++first, ++m_space) {			
			new(static_cast<void*>(&m_space->value)) value_type{*first};
			result.push_back(handler{m_space});
		}

		return result;
	}

	template<class T>
	void StockContainer<T>::reserve(size_type newCapacity) {
		if (newCapacity > capacity())
			reserveWithoutChecking(newCapacity);
	}
	template<class T>
	void StockContainer<T>::shrink_to_fit() {
		if (size_type newCapacity = size(); newCapacity) {
			m_first = static_cast<data_type*>(std::realloc(m_first, newCapacity * sizeof(data_type)));

			m_space = m_onePastLast = m_first + newCapacity;

			updateAllIterators();
		}
	}
	template<class T>
	void StockContainer<T>::squash() {
		data_type* fromBeginning = m_first, * fromEnd = m_space - 1;

		while (1) {
			if (fromBeginning == m_space)
				return;
			if (!fromBeginning->iter) {
				while (1) {
					if (fromBeginning == fromEnd) {
						m_space = fromBeginning;
						return;
					}
					if (fromEnd->iter) {
						std::memcpy(fromBeginning, fromEnd, sizeof(data_type));
						fromEnd->iter = nullptr;
						fromBeginning->iter->update(fromBeginning);

						--fromEnd;
						if (fromBeginning == fromEnd) {
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
	void StockContainer<T>::swap(StockContainer<value_type>& other) {
		data_type* tmp = m_first;
		m_first = other.m_first;
		other.m_first = tmp;

		tmp = m_space;
		m_space = other.m_space;
		other.m_space = tmp;

		tmp = m_onePastLast;
		m_onePastLast = other.m_onePastLast;
		other.m_onePastLast = tmp;
	}

	template<class T>
	auto StockContainer<T>::used() const -> size_type {
		size_type counter = 0;
		for (data_type* i = m_first; i < m_space; ++i) {
			if (i->iter)
				++counter;
		}
		return counter;
	}
}

#endif
