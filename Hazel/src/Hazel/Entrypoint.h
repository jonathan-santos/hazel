#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern Hazel::Application* Hazel::CreateApplication();

int main(int argc, char** argv)
{
	Hazel::Log::Init();
	HZ_CORE_INFO("Core Logger initialized");
	HZ_INFO("Client Logger initialized");
	
	auto app = Hazel::CreateApplication();
	app->Run();
	delete app;
}

#endif
