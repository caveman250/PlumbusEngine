#include "plumbus.h"
#include "Application.h"
#include "TesterScene.h"

int app_argc;
char** app_argv;


#if PL_PLATFORM_ANDROID
// platform/android/android.cpp
#include "android_native_app_glue.h"
android_app* Android_application = nullptr;

bool windowCreated = false;
float androidWindowHeight;
float androidWindowWidth;

void on_app_cmd(android_app *app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
        {
            androidWindowHeight = ANativeWindow_getWidth(app->window);
            androidWindowWidth = ANativeWindow_getHeight(app->window);
            windowCreated = true;
            break;
        }
    }
}

void android_main(struct android_app *app)
{
    Android_application = app;
    Android_application->onAppCmd = on_app_cmd;

    while(!windowCreated)
    {
        int events;
        struct android_poll_source* source;

        while (ALooper_pollAll(0, NULL, &events, (void**)&source) >= 0)
        {
            // Process this event.
            if (source != NULL)
            {
                source->process(Android_application, source);
            }
        }
    }

    using namespace plumbus::tester;
    Application::CreateInstance();
    Application::Get().CreateScene<TesterScene>();
    Application::Get().Run();
}
#else
int main(int argc, char** argv)
{
	app_argc = argc;
	app_argv = argv;
	using namespace plumbus::tester;

	Application::CreateInstance();
	Application::Get().CreateScene<TesterScene>();
	Application::Get().Run();

	return 0;
}
#endif
