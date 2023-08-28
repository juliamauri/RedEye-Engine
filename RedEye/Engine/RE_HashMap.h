#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <EASTL/vector.h>
#include <EASTL/map.h>

template<class TYPEVALUE, class TYPEKEY, const size_t Size, const size_t Increment>
class RE_HashMap
{
public:
	RE_HashMap() { pool_ = new TYPEVALUE[currentSize = Size]; }
	virtual ~RE_HashMap() { delete[] pool_; }

	virtual TYPEKEY Push(TYPEVALUE val) { return TYPEKEY(); };

	virtual bool Push(TYPEVALUE val, const TYPEKEY key)
	{
		bool ret = false;
		if (key_map.find(key) == key_map.end())
		{
			if (static_cast<unsigned int>(lastAvaibleIndex) == currentSize) IncrementArray();
			pool_[lastAvaibleIndex] = val;
			key_map.insert({ key, lastAvaibleIndex++ });
			ret = true;
		}
		return ret;
	}

	virtual void Pop(const TYPEKEY key)
	{
		unsigned int index = key_map.at(key);
		int lastIndex = lastAvaibleIndex - 1;

		size_t size = sizeof(TYPEVALUE);
		size *= lastIndex - static_cast<size_t>(index);

		memcpy(&pool_[index], &pool_[index + 1], size);
		key_map.erase(key_map.find(key));

		typename eastl::map<TYPEKEY, unsigned int>::iterator i = key_map.begin();
		for (; i != key_map.end(); ++i)
			if(i->second > index) i->second--;

		lastAvaibleIndex--;
	}

	TYPEVALUE At(const TYPEKEY key) const { return pool_[key_map.at(key)]; }
	TYPEVALUE& At(const TYPEKEY key) { return pool_[key_map.at(key)]; }
	TYPEVALUE* AtPtr(const TYPEKEY key) const { return &pool_[key_map.at(key)]; }
	const TYPEVALUE* AtCPtr(const TYPEKEY key) const { return &pool_[key_map.at(key)]; }

	virtual eastl::vector<TYPEKEY> GetAllKeys() const = 0;

	size_t GetCount() const { return key_map.size(); }

private:

	void IncrementArray()
	{
		unsigned int newSize = currentSize + Increment;
		poolTmp_ = new TYPEVALUE[newSize];
		eastl::copy(pool_, pool_ + currentSize, poolTmp_);
		currentSize = newSize;
		delete[] pool_;
		pool_ = poolTmp_;
	}

protected:

	TYPEVALUE* pool_ = nullptr;
	TYPEVALUE* poolTmp_ = nullptr;
	eastl::map<TYPEKEY, unsigned int> key_map;

	int lastAvaibleIndex = 0;
	unsigned int currentSize = 0;
};

#endif // !__HASH_MAP_H__