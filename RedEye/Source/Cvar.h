#ifndef __CVAR__
#define __CVAR__

class RE_GameObject;

class Cvar // Global Value Container
{
public:
	Cvar();
	Cvar(Cvar& copy);
	Cvar(bool bool_v);
	Cvar(int int_v);
	Cvar(unsigned int uint_v);
	Cvar(long long int int64_v);
	Cvar(unsigned long long int uint64_v);
	Cvar(double double_v);
	Cvar(float float_v);
	Cvar(const char* char_p_v);
	Cvar(RE_GameObject* go_v);

protected:
	enum VAR_TYPE : unsigned short int
	{
		UNDEFINED,
		BOOL,
		INT,
		UINT,
		INT64,
		UINT64,
		DOUBLE,
		FLOAT,
		CHAR_P,
		GAMEOBJECT
	}  type;

	union VAR_data
	{
		bool bool_v;
		int int_v;
		unsigned int uint_v;
		long long int int64_v;
		unsigned long long int uint64_v;
		double double_v;
		float float_v;
		const char* char_p_v;
		RE_GameObject* go_v;
	} value;

public:
	virtual bool SetValue(bool bool_v, bool force_type = false);
	virtual bool SetValue(int int_v, bool force_type = false);
	virtual bool SetValue(unsigned int uint_v, bool force_type = false);
	virtual bool SetValue(long long int int64_v, bool force_type = false);
	virtual bool SetValue(unsigned long long int uint64_v, bool force_type = false);
	virtual bool SetValue(double double_v, bool force_type = false);
	virtual bool SetValue(float float_v, bool force_type = false);
	virtual bool SetValue(const char* char_p_v, bool force_type = false);
	virtual bool SetValue(RE_GameObject* go_v, bool force_type = false);

	VAR_TYPE				GetType() const;
	bool					AsBool() const;
	int						AsInt() const;
	unsigned int			AsUInt() const;
	long long int			AsInt64() const;
	unsigned long long int	AsUInt64() const;
	double					AsDouble() const;
	float					AsFloat() const;
	const char*				AsCharP() const;
	RE_GameObject*			AsGO() const;
};

/*class DoubleCvar : public Cvar
{
public:
	DoubleCvar(bool bool_v);
	DoubleCvar(int int_v);
	DoubleCvar(unsigned int uint_v);
	DoubleCvar(long long int int64_v);
	DoubleCvar(unsigned long long int uint64_v);
	DoubleCvar(double double_v);
	DoubleCvar(float float_v);
	DoubleCvar(const char* char_p_v);

	bool ValueHasChanged() const;

	bool SetValue(bool bool_v, bool force_type = false) override;
	bool SetValue(int int_v, bool force_type = false) override;
	bool SetValue(unsigned int uint_v, bool force_type = false) override;
	bool SetValue(long long int int64_v, bool force_type = false) override;
	bool SetValue(unsigned long long int uint64_v, bool force_type = false) override;
	bool SetValue(double double_v, bool force_type = false) override;
	bool SetValue(float float_v, bool force_type = false) override;
	bool SetValue(const char* char_p_v, bool force_type = false) override;

private:
	VAR_data original_value;
};
*/

#endif // !__CVAR__