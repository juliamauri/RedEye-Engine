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
		std::map<TYPEKEY, unsigned int>::iterator w = poolmapped_.find(key);
		unsigned int index = w->second;
		TYPEVALUE ret = pool_[index];
		memcpy(&pool_[index], &pool_[index + 1], sizeof(TYPEVALUE*) * (lastAvaibleIndex - index));
		while (w != poolmapped_.end())
		{
			w->second--;
			w++;
		}
		lastAvaibleIndex--;
		return ret;
	}

	const TYPEVALUE At(TYPEKEY key)const { return pool_[poolmapped_.at(key)]; }

	int GetLastIndex()const {
		return lastAvaibleIndex - 1;
	}

protected:
	TYPEVALUE* pool_;
	std::map<TYPEKEY, unsigned int> poolmapped_;

	int lastAvaibleIndex = 0;
};

#endif // !__POOL_H__