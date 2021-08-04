#ifndef __RE_COMMAND__
#define __RE_COMMAND__

#include "RE_GameObject.h"
#include "MathGeoLib\include\MathGeoLib.h"

class RE_Command
{
public:
	virtual ~RE_Command() {};
	virtual void execute() = 0;
	virtual void Undo() = 0;
};

class RE_CMDTransformPosition : public RE_Command{
public:

	RE_CMDTransformPosition(UID go, math::vec pBefore, math::vec p) : go_(go), pBefore_(pBefore), p_(p) { }

	void execute() override;

	void Undo() override;

private:
	UID go_;
	math::vec pBefore_;
	math::vec p_;

};

class RE_CMDTransformRotation : public RE_Command {
public:

	RE_CMDTransformRotation(UID go, math::vec rBefore, math::vec r) : go_(go), rBefore_(rBefore), r_(r) { }

	void execute() override;

	void Undo() override;

private:
	UID go_;
	math::vec rBefore_;
	math::vec r_;
};

class RE_CMDTransformScale : public RE_Command {
public:
	RE_CMDTransformScale(UID go, math::vec sBefore, math::vec s) : go_(go), sBefore_(sBefore), s_(s) { }

	void execute() override;

	void Undo() override;

private:
	UID go_;
	math::vec sBefore_;
	math::vec s_;
};

#endif // !__RE_COMMAND__