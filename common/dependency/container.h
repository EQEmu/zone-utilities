#pragma once

#include <typeinfo>
#include <memory>
#include <functional>
#include <map>

namespace EQEmu
{
	class BaseContainerFactory
	{
	public:
		virtual ~BaseContainerFactory() { }
	};
	
	template<typename T>
	class ContainerFactory : public BaseContainerFactory
	{
	public:
		ContainerFactory(std::function<std::shared_ptr<T>()> f) {
			mFactoryFunction = f;
		}
		virtual ~ContainerFactory() = default;

		std::shared_ptr<T> Construct() {
			return mFactoryFunction();
		}
	private:
		std::function<std::shared_ptr<T>()> mFactoryFunction;
	};
	
	class Container
	{
	public:
		~Container() = default;
		
		static Container& Get() {
			static Container inst;
			return inst;
		}

		template<typename Interface, typename Instance>
		void RegisterSingleton()
		{
			const std::type_info &info = typeid(Interface);
			std::string name = info.name();

			mRegisteredFactories[name] = std::make_shared<ContainerFactory<Interface>>([=] {
				static std::shared_ptr<Instance> inst(new Instance());
				return inst;
			});
		}

		template<typename Interface, typename Instance>
		void RegisterTransient()
		{
			const std::type_info &info = typeid(Interface);
			std::string name = info.name();

			mRegisteredFactories[name] = std::make_shared<ContainerFactory<Interface>>([=] {
				return std::shared_ptr<Instance>(new Instance());
			});
		}

		template<typename Interface>
		std::shared_ptr<Interface> Resolve() {
			const std::type_info &info = typeid(Interface);
			std::string name = info.name();

			auto baseFactory = mRegisteredFactories[name];
			auto factory = std::static_pointer_cast<ContainerFactory<Interface>>(baseFactory);
			return factory->Construct();
		}

	private:
		Container() = default;
		Container(const Container&) = delete;
		Container& operator=(const Container&) = delete;

		std::map<std::string, std::shared_ptr<BaseContainerFactory>> mRegisteredFactories;
	};
}
