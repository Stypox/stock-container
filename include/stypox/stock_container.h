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
	class M_StockContainerHandler {
		friend class StockContainer<T>;
	public:
		using value_type = T;
		using container_type = StockContainer<value_type>;
	private:
		value_type* m_value;
		container_type* m_container;

		M_StockContainerHandler(value_type* value, container_type* container) :
			m_value{value}, m_container{container} { m_container->updateCorrespondingHandler(m_value, this); }

		void updateValue(value_type* value) { m_value = value; }
		void updateContainer(container_type* container) { m_container = container; }
		void setInvalid() { m_value = nullptr; }

	public:
		M_StockContainerHandler(const M_StockContainerHandler<value_type>& other) = delete;
		M_StockContainerHandler(M_StockContainerHandler<value_type>&& other);

		M_StockContainerHandler& operator=(const M_StockContainerHandler<value_type>& other) = delete;
		M_StockContainerHandler& operator=(M_StockContainerHandler<value_type>&& other);

		~M_StockContainerHandler();
		void remove();

		value_type& operator*() const { return *m_value; }
		value_type* operator->() const { return m_value; }

		bool operator==(nullptr_t) const { return m_value == nullptr; }
		bool operator!=(nullptr_t) const { return m_value != nullptr; }
		operator bool() const { return m_value != nullptr; }
	};

	template<class T>
	class StockContainer {
		friend class M_StockContainerHandler<T>;
	public:
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = typename std::pointer_traits<pointer>::template rebind<const value_type>;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using reverse_const_iterator = std::reverse_iterator<const_iterator>;
		using handler = M_StockContainerHandler<value_type>;
	private:
		value_type* m_first,
				  * m_space,
				  * m_onePastLast;
		handler** m_firstHandler;

		void reserveWithoutChecking(size_type newSize);
		void growIfNeeded();

		void updateIteratorValues();
		void updateIteratorContainer();

		void updateCorrespondingHandler(value_type* value, handler* newHandler) { m_firstHandler[value - m_first] = newHandler; }
		void remove(value_type* value);
	public:
		StockContainer() : m_first{nullptr}, m_space{nullptr}, m_onePastLast{nullptr}, m_firstHandler{nullptr} {}
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

		value_type* data() const { return m_first; }

		bool empty() const { return m_space == m_first; }
		size_type size() const { return m_space - m_first; }
		size_type capacity() const { return m_onePastLast - m_first; }

		void reserve(size_type newSize);
		void shrink_to_fit();
		void squash();

		value_type& operator[](size_type n) { return m_first[n]; }
		value_type& at(size_type n) { if (n >= size()) throw std::out_of_range{"stypox::StockContainer::at()"}; return m_first[n]; }
		value_type& back() { return m_space[-1]; }
		value_type& front() { return *m_first; }

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
		m_value{other.m_value}, m_container{other.m_container} {
		other.m_value = nullptr;
		if (m_value)
			m_container->updateCorrespondingHandler(m_value, this);
	}
	
	template<class T>
	auto M_StockContainerHandler<T>::operator=(M_StockContainerHandler<value_type>&& other) -> M_StockContainerHandler& {
		this->~M_StockContainerHandler();
		m_value = other.m_value;
		m_container = other.m_container;
		other.m_value = nullptr;
		if (m_value)
			m_container->updateCorrespondingHandler(m_value, this);
		return *this;
	}

	template<class T>
	M_StockContainerHandler<T>::~M_StockContainerHandler() {
		if (m_value)
			m_container->remove(m_value);
	}

	template<class T>
	void M_StockContainerHandler<T>::remove() {
		m_container->remove(m_value);
		m_value = nullptr;
	}



	template<class T>
	void StockContainer<T>::reserveWithoutChecking(size_type newCapacity) {
		size_type oldSize = size();

		m_first = static_cast<value_type*>(std::realloc(m_first, newCapacity * sizeof(value_type)));
		m_firstHandler = static_cast<handler**>(std::realloc(m_firstHandler, newCapacity * sizeof(handler*)));

		m_space = m_first + oldSize;
		m_onePastLast = m_first + newCapacity;

		updateIteratorValues();
	}
	template<class T>
	void StockContainer<T>::growIfNeeded() {
		if (m_space == m_onePastLast) {
			// growth similar to golden ratio
			reserveWithoutChecking(capacity() * 3/2 + 1);
		}
	}

	template<class T>
	void StockContainer<T>::updateIteratorValues() {
		for (size_t i = 0, sz = size(); i != sz; ++i)
			m_firstHandler[i]->updateValue(m_first + i);
	}
	template<class T>
	void StockContainer<T>::updateIteratorContainer() {
		for (handler** first = m_firstHandler, ** last = m_firstHandler + (m_space - m_first); first != last; ++first)
			(*first)->updateContainer(this);
	}

	template<class T>
	void StockContainer<T>::remove(value_type* value) {
		value->~value_type();
		--m_space;

		if (value != m_space) {
			std::memcpy(value, m_space, sizeof(value_type));
			m_firstHandler[value - m_first] = m_firstHandler[m_space - m_first];
			m_firstHandler[value - m_first]->updateValue(value);
		}
	}

	template<class T>
	StockContainer<T>::StockContainer(StockContainer&& other) :
		m_first{other.m_first}, m_space{other.m_space},
		m_onePastLast{other.m_onePastLast}, m_firstHandler{other.m_firstHandler} {
		updateIteratorContainer();

		// other.m_onePastLast = other.m_firstHandler = nullptr
		// ^ this is not done since the other container will not be used anymore
		// and the destructor only needs m_first and m_space
		other.m_first = other.m_space = nullptr;
		other.m_firstHandler = nullptr;
	}

	template<class T>
	auto StockContainer<T>::operator=(StockContainer&& other) -> StockContainer& {
		this->~StockContainer();

		m_first = other.m_first;
		m_space = other.m_space;
		m_onePastLast = other.m_onePastLast;
		m_firstHandler = other.m_firstHandler;
		
		updateIteratorContainer();

		// other.m_onePastLast = nullptr
		// ^ this is not done since the other container will not be used anymore
		// and the destructor only needs m_first and m_space
		other.m_first = other.m_space = nullptr;
		other.m_firstHandler = nullptr;
		return *this;
	}

	template<class T>
	StockContainer<T>::~StockContainer() {
		for (size_t i = 0, sz = size(); i != sz; ++i) {
			m_first[i].~value_type();
			m_firstHandler[i]->setInvalid();
		}
		std::free(m_first);
		std::free(m_firstHandler);
	}

	template<class T>
	auto StockContainer<T>::push(const value_type& value) -> handler {
		growIfNeeded();
		new(static_cast<void*>(m_space)) value_type{value};
		return handler{m_space++, this};
	}
	template<class T>
	auto StockContainer<T>::push(value_type&& value) -> handler {
		growIfNeeded();
		new(static_cast<void*>(m_space)) value_type{value};
		return handler{m_space++, this};
	}
	template<class T>
	template<class... Args>
	auto StockContainer<T>::emplace(Args&&... value) -> handler {
		growIfNeeded();
		new(static_cast<void*>(m_space)) value_type{value...};
		return handler{m_space++, this};
	}

	template<class T>
	auto StockContainer<T>::append(size_type count, const value_type& value) -> std::vector<handler> {
		reserveWithoutChecking(size() + count);

		std::vector<handler> result;
		for (size_type c = 0; c < count; ++c, ++m_space) {
			new(static_cast<void*>(m_space)) value_type{value};
			result.push_back(handler{m_space, this});
		}

		return result;
	}
	template<class T>
	template<class InputIt>
	auto StockContainer<T>::append(InputIt first, InputIt last) -> std::vector<handler> {
		reserveWithoutChecking(size() + (last - first));

		std::vector<handler> result;
		for (; first != last; ++first, ++m_space) {
			new(static_cast<void*>(m_space)) value_type{*first};
			result.push_back(handler{m_space, this});
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
			m_first = static_cast<value_type*>(std::realloc(m_first, newCapacity * sizeof(value_type)));
			m_firstHandler = static_cast<handler**>(std::realloc(m_firstHandler, newCapacity * sizeof(handler*)));

			m_space = m_onePastLast = m_first + newCapacity;

			updateIteratorValues();
		}
	}

	template<class T>
	void StockContainer<T>::swap(StockContainer<value_type>& other) {
		value_type* tmp = m_first;
		m_first = other.m_first;
		other.m_first = tmp;

		tmp = m_space;
		m_space = other.m_space;
		other.m_space = tmp;

		tmp = m_onePastLast;
		m_onePastLast = other.m_onePastLast;
		other.m_onePastLast = tmp;

		std::swap(m_firstHandler, other.m_firstHandler);

		updateIteratorContainer();
		other.updateIteratorContainer();
	}
}

#endif
