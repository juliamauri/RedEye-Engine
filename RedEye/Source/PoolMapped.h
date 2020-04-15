#ifndef __POOL_H__
#define __POOL_H__

#include <vector>
#include <map>

#define MAX_POOL 1024

template<class TYPEVALUE, class TYPEKEY>
class PoolMapped
{
public:
	PoolMapped() {
		pool_ = new TYPEVALUE[MAX_POOL];
	}
	virtual ~PoolMapped() {
		delete[] pool_;
	}

	virtual TYPEKEY Push(TYPEVALUE val) { return TYPEKEY(); };

	virtual bool Push(TYPEVALUE val, TYPEKEY key) {
		if (poolmapped_.find(key) == poolmapped_.end()) {
			pool_[lastAvaibleIndex] = val;
			poolmapped_.insert(std::pair<TYPEKEY, unsigned int>(key, lastAvaibleIndex++));
			return true;
		}
		return false;
	}

	virtual TYPEVALUE Pop(TYPEKEY key) {
		unsigned int index = poolmapped_.at(key);
		TYPEVALUE ret = pool_[index];
		memcpy(&pool_[index], &pool_[index + 1], sizeof(TYPEVALUE) * (lastAvaibleIndex - 1 - index));
		typename std::map<TYPEKEY, unsigned int>::iterator i = poolmapped_.find(key);
		while (i != poolmapped_.end())
		{
			i->second--;
			i++;
		}
		poolmapped_.erase(poolmapped_.find(key));
		lastAvaibleIndex--;
		return ret;
	}

	const TYPEVALUE At(TYPEKEY key)const { return pool_[poolmapped_.at(key)]; }
	TYPEVALUE& At(TYPEKEY key) { return pool_[poolmapped_.at(key)]; }
	TYPEVALUE* AtPtr(TYPEKEY key)const { return &pool_[poolmapped_.at(key)]; }

	std::vector<TYPEKEY> GetAllKeys() {
		std::vector<TYPEKEY> ret;
		typename std::map<TYPEKEY, unsigned int>::iterator i = poolmapped_.begin();
		while (i != poolmapped_.end())
		{
			ret.push_back(i->first);
			i++;
		}
		return ret;
	}

	int GetCount()const { return poolmapped_.size(); }

protected:
	TYPEVALUE* pool_;
	std::map<TYPEKEY, unsigned int> poolmapped_;

	int lastAvaibleIndex = 0;
};

#endif // !__POOL_H__