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
		DEL_A(pool_);
	}

	virtual TYPEKEY Push(TYPEVALUE val) { return TYPEKEY(); };

	virtual void Push(TYPEVALUE val, TYPEKEY key) {
		pool_[lastAvaibleIndex] = val;
		poolmapped_.insert(std::pair<TYPEKEY, unsigned int>(key, lastAvaibleIndex++));
	}

	virtual TYPEVALUE Pop(TYPEKEY key) {
		unsigned int index = poolmapped_.at(key);
		TYPEVALUE ret = pool_[index];
		for (unsigned int i = index; i < lastAvaibleIndex; i++)
			pool_[i] = pool_[i + 1];
		std::map<TYPEKEY, unsigned int>::iterator i = poolmapped_.find(key);
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

	int GetLastIndex() const
	{
		return lastAvaibleIndex - 1;
	}

	int GetCount() const
	{
		return lastAvaibleIndex;
	}

protected:
	TYPEVALUE* pool_;
	std::map<TYPEKEY, unsigned int> poolmapped_;

	int lastAvaibleIndex = 0;
};

#endif // !__POOL_H__