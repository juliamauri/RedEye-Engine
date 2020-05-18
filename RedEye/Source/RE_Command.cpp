#include "RE_Command.h"

#include "RE_GameObject.h"
#include "RE_CompTransform.h"

void RE_CMDTransformPosition::execute()
{
	go_->GetTransform()->SetPosition(p_);
}

void RE_CMDTransformPosition::undo()
{
	go_->GetTransform()->SetPosition(pBefore_);
}

void RE_CMDTransformRotation::execute()
{
	go_->GetTransform()->SetRotation(r_);
}

void RE_CMDTransformRotation::undo()
{
	go_->GetTransform()->SetRotation(rBefore_);
}

void RE_CMDTransformScale::execute()
{
	go_->GetTransform()->SetScale(s_);
}

void RE_CMDTransformScale::undo()
{
	go_->GetTransform()->SetScale(sBefore_);
}
