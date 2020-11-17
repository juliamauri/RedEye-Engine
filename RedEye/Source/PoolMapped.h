#ifndef __POOL_H__
#define __POOL_H__

#include <EASTL/vector.h>
#include <EASTL/map.h>

template<class TYPEVALUE, class TYPEKEY, unsigned int size, unsigned int increment>
class PoolMapped
{
public:
	PoolMapped() {
		currentSize = size;
		pool_ = new TYPEVALUE[currentSize];
	}
	virtual ~PoolMapped() {
		delete[] pool_;
	}

	virtual TYPEKEY Push(TYPEVALUE val) { return TYPEKEY(); };

	virtual bool Push(TYPEVALUE val, TYPEKEY key) {
		if (poolmapped_.find(key) == poolmapped_.end()) {
			if (static_cast<unsigned int>(lastAvaibleIndex) == currentSize) IncrementArray();
			pool_[lastAvaibleIndex] = val;
			poolmapped_.insert(eastl::pair<TYPEKEY, unsigned int>(key, lastAvaibleIndex++));
			return true;
		}
		return false;
	}

	virtual TYPEVALUE Pop(TYPEKEY key) {
		unsigned int index = poolmapped_.at(key);
		TYPEVALUE ret = pool_[index];

		int lastIndex = lastAvaibleIndex - 1;
		memcpy(&pool_[index], &pool_[index + 1], sizeof(TYPEVALUE) * (lastIndex - index));

		poolmapped_.erase(poolmapped_.find(key));

		typename eastl::map<TYPEKEY, unsigned int>::iterator i = poolmapped_.begin();
		while (i != poolmapped_.end())
		{
			if(i->second > index) 
				i->second--;

			i++;
		}
		lastAvaibleIndex--;
		return ret;
	}

	const TYPEVALUE At(TYPEKEY key) const { return pool_[poolmapped_.at(key)]; }
	TYPEVALUE& At(TYPEKEY key) { return pool_[poolmapped_.at(key)]; }
	TYPEVALUE* AtPtr(TYPEKEY key) const { return &pool_[poolmapped_.at(key)]; }
	const TYPEVALUE* AtCPtr(TYPEKEY key) const { return &pool_[poolmapped_.at(key)]; }

	virtual eastl::vector<TYPEKEY> GetAllKeys() const = 0;

	int GetCount() const { return poolmapped_.size(); }

private:
	void IncrementArray() {
		unsigned int newSize = currentSize + increment;
		poolTmp_ = new TYPEVALUE[newSize];
		eastl::copy(pool_, pool_ + currentSize, poolTmp_);
		currentSize = newSize;
		delete[] pool_;
		pool_ = poolTmp_;
	}

protected:
	TYPEVALUE* pool_;
	TYPEVALUE* poolTmp_;
	eastl::map<TYPEKEY, unsigned int> poolmapped_;

	int lastAvaibleIndex = 0;
	unsigned int currentSize;
};

#endif // !__POOL_H__