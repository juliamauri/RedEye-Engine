#pragma once
#include "MathGeoLib\include\MathGeoLib.h"

class RE_GameObject;

class RE_Command
{
public:
	virtual ~RE_Command() {};
	virtual void execute() = 0;
	virtual void undo() = 0;
};

class RE_CMDTransformPosition : public RE_Command{
public:

	RE_CMDTransformPosition(RE_GameObject* go, math::vec pBefore, math::vec p) : go_(go), pBefore_(pBefore), p_(p) { }

	void execute() override;

	void undo() override;

private:
	RE_GameObject* go_;
	math::vec pBefore_;
	math::vec p_;

};

class RE_CMDTransformRotation : public RE_Command {
public:

	RE_CMDTransformRotation(RE_GameObject* go, math::vec rBefore, math::vec r) : go_(go), rBefore_(rBefore), r_(r) { }

	void execute() override;

	void undo() override;

private:
	RE_GameObject* go_;
	math::vec rBefore_;
	math::vec r_;
};

class RE_CMDTransformScale : public RE_Command {
public:
	RE_CMDTransformScale(RE_GameObject* go, math::vec sBefore, math::vec s) : go_(go), sBefore_(sBefore), s_(s) { }

	void execute() override;

	void undo() override;

private:
	RE_GameObject* go_;
	math::vec sBefore_;
	math::vec s_;
};