#ifndef __POOL_H__
#define __POOL_H__

#include <EASTL/vector.h>
#include <EASTL/map.h>

template<class TYPEVALUE, class TYPEKEY, unsigned int size, unsigned int increment>
class PoolMapped
{
public:
	PoolMapped() { pool_ = new TYPEVALUE[currentSize = size]; }
	virtual ~PoolMapped() { delete[] pool_; }

	virtual TYPEKEY Push(TYPEVALUE val) { return TYPEKEY(); };

	virtual bool Push(TYPEVALUE val, const TYPEKEY key)
	{
		bool ret = false;
		if (poolmapped_.find(key) == poolmapped_.end())
		{
			if (static_cast<unsigned int>(lastAvaibleIndex) == currentSize) IncrementArray();
			pool_[lastAvaibleIndex] = val;
			poolmapped_.insert({ key, lastAvaibleIndex++ });
			ret = true;
		}
		return ret;
	}

	virtual void Pop(const TYPEKEY key)
	{
		unsigned int index = poolmapped_.at(key);
		int lastIndex = lastAvaibleIndex - 1;
		memcpy(&pool_[index], &pool_[index + 1], sizeof(TYPEVALUE) * (lastIndex - index));
		poolmapped_.erase(poolmapped_.find(key));

		typename eastl::map<TYPEKEY, unsigned int>::iterator i = poolmapped_.begin();
		for (; i != poolmapped_.end(); ++i)
			if(i->second > index) i->second--;

		lastAvaibleIndex--;
	}

	TYPEVALUE At(const TYPEKEY key) const { return pool_[poolmapped_.at(key)]; }
	TYPEVALUE& At(const TYPEKEY key) { return pool_[poolmapped_.at(key)]; }
	TYPEVALUE* AtPtr(const TYPEKEY key) const { return &pool_[poolmapped_.at(key)]; }
	const TYPEVALUE* AtCPtr(const TYPEKEY key) const { return &pool_[poolmapped_.at(key)]; }

	virtual eastl::vector<TYPEKEY> GetAllKeys() const = 0;

	int GetCount() const { return poolmapped_.size(); }

private:

	void IncrementArray()
	{
		unsigned int newSize = currentSize + increment;
		poolTmp_ = new TYPEVALUE[newSize];
		eastl::copy(pool_, pool_ + currentSize, poolTmp_);
		currentSize = newSize;
		delete[] pool_;
		pool_ = poolTmp_;
	}

protected:

	TYPEVALUE* pool_ = nullptr;
	TYPEVALUE* poolTmp_ = nullptr;
	eastl::map<TYPEKEY, unsigned int> poolmapped_;

	int lastAvaibleIndex = 0;
	unsigned int currentSize = 0;
};

#endif // !__POOL_H__