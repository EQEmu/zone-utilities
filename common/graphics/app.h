#pragma once

#include <string>

struct GLFWwindow;

namespace EQ
{
	namespace Graphics
	{
		class App
		{
		public:
			App(const std::string &logfile);
			virtual ~App();
			
			int Run(const std::string &name);
			virtual void OnStart();
			virtual void Update(double timeSinceLastFrame);
			virtual void Render();
			virtual void OnShutdown();
			virtual void OnResize(int width, int height);
		protected:
			GLFWwindow *mWindow;
		};
	}
}
