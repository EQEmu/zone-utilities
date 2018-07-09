#pragma once

#include <graphics/app.h>
#include "scene.h"
#include "module_navigation.h"
#include "module_wp.h"
#include "module_volume.h"

class MapEditApp : public EQ::Graphics::App
{
public:
	MapEditApp();
	virtual ~MapEditApp();

	virtual void OnStart();
	virtual void Update(double timeSinceLastFrame);
	virtual void Render();
	virtual void OnShutdown();
	virtual void OnResize(int width, int height);
private:
	std::unique_ptr<Scene> mScene;
};