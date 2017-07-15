#pragma once

#include "module.h"
#include "scene.h"
#include "thread_pool.h"

class ModuleWP : public Module, public SceneHotkeyListener
{
public:
	ModuleWP();
	virtual ~ModuleWP();
	virtual const char *GetName() { return "WP"; };
	virtual void OnLoad(Scene *s);
	virtual void OnShutdown();
	virtual void OnDrawMenu();
	virtual void OnDrawUI();
	virtual void OnDrawOptions();
	virtual void OnSceneLoad(const char *zone_name);
	virtual void OnSuspend();
	virtual void OnResume();
	virtual bool HasWork();
	virtual bool CanSave();
	virtual void Save();
	virtual void OnHotkey(int ident);
	virtual void OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *non_collide_hit, const glm::vec3 *select_hit, Entity *selected);
private:
};
