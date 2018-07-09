#include "map_edit_app.h"

MapEditApp::MapEditApp()
	: EQ::Graphics::App("map_edit.log")
{
}

MapEditApp::~MapEditApp()
{
}

void MapEditApp::OnStart()
{
	mScene.reset(new Scene());
	mScene->RegisterModule(new ModuleNavigation());
	mScene->RegisterModule(new ModuleVolume());
	mScene->RegisterModule(new ModuleWP());
	mScene->Init(mWindow);
}

void MapEditApp::Update(double timeSinceLastFrame)
{
	mScene->Tick();
}

void MapEditApp::Render()
{
	mScene->Render();
	EQ::Graphics::App::Render();
}

void MapEditApp::OnShutdown()
{
	mScene->UnregisterAllModules();
}

void MapEditApp::OnResize(int width, int height)
{
	mScene->Resize(width, height);
	EQ::Graphics::App::OnResize(width, height);
}
