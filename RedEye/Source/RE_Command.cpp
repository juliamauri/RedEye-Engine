#include "RE_Command.h"

#include "ModuleScene.h"
#include "RE_GameObject.h"
#include "RE_CompTransform.h"

void RE_CMDTransformPosition::execute()
{
	ModuleScene::GetScenePool()->GetGOPtr(go_)->GetTransformPtr()->SetPosition(p_);
}

void RE_CMDTransformPosition::Undo()
{
	ModuleScene::GetScenePool()->GetGOPtr(go_)->GetTransformPtr()->SetPosition(pBefore_);
}

void RE_CMDTransformRotation::execute()
{
	ModuleScene::GetScenePool()->GetGOPtr(go_)->GetTransformPtr()->SetRotation(r_);
}

void RE_CMDTransformRotation::Undo()
{
	ModuleScene::GetScenePool()->GetGOPtr(go_)->GetTransformPtr()->SetRotation(rBefore_);
}

void RE_CMDTransformScale::execute()
{
	ModuleScene::GetScenePool()->GetGOPtr(go_)->GetTransformPtr()->SetScale(s_);
}

void RE_CMDTransformScale::Undo()
{
	ModuleScene::GetScenePool()->GetGOPtr(go_)->GetTransformPtr()->SetScale(sBefore_);
}
