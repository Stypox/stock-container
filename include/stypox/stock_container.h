#ifndef _STYPOX_STOCK_CONTAINER_H_
#define _STYPOX_STOCK_CONTAINER_H_

#include <cstdlib>
#include <utility>
#include <vector>

namespace stypox {
	void f() {
		std::vector<int>::iterator a;
		a.base();
	}

	template<class T>
	class StockContainer;

	template<class T>
	class M_StockContainerIterator {
		friend class StockContainer<T>;
	public:
		using value_type = T;
	private:
		using data_type = std::pair<value_type, M_StockContainerIterator<value_type>*>;

		data_type* m_data;

		M_StockContainerIterator(data_type* data);

		void update(data_type* data);

	public:
		M_StockContainerIterator(const M_StockContainerIterator<value_type>& other) = delete;
		M_StockContainerIterator(M_StockContainerIterator<value_type>&& other);

		M_StockContainerIterator& operator=(const M_StockContainerIterator<value_type>& other) = delete;
		M_StockContainerIterator& operator=(M_StockContainerIterator<value_type>&& other) = delete;

		~M_StockContainerIterator();

		value_type& operator*() const;
		value_type* operator->() const;
	};

	template<class T>
	M_StockContainerIterator<T>::
	M_StockContainerIterator(data_type* data) :
		m_data{data} {
		m_data->second = this;
	}

	template<class T>
	void
	M_StockContainerIterator<T>::
	update(data_type* data) {
		m_data = data;
	}

	template<class T>
	M_StockContainerIterator<T>::
	M_StockContainerIterator(M_StockContainerIterator<value_type>&& other) :
		m_data{other.m_data} {
		other.m_data = nullptr;
		m_data->second = this;
	}

	template<class T>
	M_StockContainerIterator<T>::
	~M_StockContainerIterator() {
		if (m_data) {
			delete m_data->first;
			m_data->second = nullptr;
		}
	}

	template<class T>
	auto
	M_StockContainerIterator<T>::
	operator*() const
	-> value_type& {
		return m_data->first;
	}
	template<class T>
	auto
	M_StockContainerIterator<T>::
	operator->() const
	-> value_type* {
		return &m_data->first;
	}
}

#endif
