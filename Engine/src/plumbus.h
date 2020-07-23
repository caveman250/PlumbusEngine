#pragma once

#if VULKAN_RENDERER
#include "vulkan/vulkan.h"
#define GLFW_INCLUDE_VULKAN
#include "renderer/vk/vk_types_fwd.h"
#endif

#include <vector>
#include <array>
#include <assert.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <cstring>
#include <GLFW/glfw3.h>
#include <string>
#include <map>
#include <fstream>
#include "imgui_impl/Log.h"
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <set>
#include <algorithm>
#include <chrono>
#include <set>
#include <unordered_map>

#if PL_PLATFORM_LINUX
#include <gtk/gtk.h>
#endif

namespace plumbus
{
#if PL_PLATFORM_WINDOWS
#if NDEBUG
#define PL_ASSERT(expr, ...) \
	do { \
			if (!(expr))\
			{\
				char msg_buf[1024]; \
				PL_ASSERT_MESSAGE(msg_buf, __VA_ARGS__)\
				char buf[1024]; \
				snprintf(buf, 1024, "%s\n\nMessage: %s\n", #expr, (const char*)&msg_buf); \
				fprintf(stderr, "\033[0;37;41mAssertion failed: %s - at %s:%i\033[0m\n\n", (const char*)&buf, __FILE__, __LINE__);\
			}\
	} while (0)
#else
#define PL_ASSERT(expr, ...) \
	do { \
			if (!(expr))\
			{\
				char msg_buf[1024]; \
				PL_ASSERT_MESSAGE(msg_buf, __VA_ARGS__)\
				char buf[1024]; \
				snprintf(buf, 1024, "%s\n\nMessage: %s\n", #expr, (const char*)&msg_buf); \
				bool assertResult = _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "Plumbus Application", buf); \
				printf("\033[0;37;41mAssertion failed: %s - at %s:%i\033[0m\n\n", (const char*)&buf, __FILE__, __LINE__); \
				if (assertResult == 0)\
				{\
					break; \
				}\
				else\
				{\
					__debugbreak(); \
				}\
			}\
	} while (0)
#endif
#else
#define PL_ASSERT(expr, ...) \
	do { \
		if (!(expr))\ 
		{\
			char msg_buf[1024]; \
			PL_ASSERT_MESSAGE(msg_buf, __VA_ARGS__)\
			char buf[1024]; \
			snprintf(buf, 1024, "Expression: %s\n\nMessage: %s\n", #expr, (const char*)&msg_buf); \
			GtkWidget* dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, "Assertion Failed!");\
			gtk_dialog_add_buttons(GTK_DIALOG(dialog), "Abort", 0, "Break", 1, "Ignore", 2, NULL);\
			gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", (char*)&buf);\
			int result = gtk_dialog_run(GTK_DIALOG (dialog));\
			printf("\033[0;37;41mAssertion failed:\n%s - at %s:%i\033[0m\n\n", (const char*)&buf, __FILE__, __LINE__); \
			switch (result)\
			{\
				case 0:\
				exit(0);\
				break;\
				case 1:\
				raise(SIGINT);\
				default:\
				break;\
			}\
			gtk_widget_destroy(GTK_WIDGET(dialog));\
		}\
	} while (0)
#endif

#define PL_VERIFY(expr, ...) (!(expr) ? (::std::invoke([&](bool result) -> bool  { PL_ASSERT(expr, __VA_ARGS__); return result; }, !!(expr))), false : true)

#define PLUMBUS_CAT_III(_, expr) expr
#define PLUMBUS_CAT_II(a, b) PLUMBUS_CAT_III(~, a ## b)
#define PLUMBUS_CAT(a, b) PLUMBUS_CAT_II(a, b)

#define PLUMBUS_VARG_COUNT(...) PLUMBUS_EXPAND_ARGS(PLUMBUS_AUGMENT_ARGS(__VA_ARGS__), PLUMBUS_VARG_COUNT_HELPER())
#define PLUMBUS_VARG_COUNT_01N(...) PLUMBUS_EXPAND_ARGS(PLUMBUS_AUGMENT_ARGS(__VA_ARGS__), PLUMBUS_VARG_COUNT_01N_HELPER())

#define PLUMBUS_VARG_COUNT_HELPER() 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#if PL_PLATFORM_LINUX //wat, TODO
#define PLUMBUS_VARG_COUNT_01N_HELPER() N, N, N, N, N, N, N, N, N, N, N, N, N, N, 1, 0, 0 
#else
#define PLUMBUS_VARG_COUNT_01N_HELPER() N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, 1, 0
#endif

#define PLUMBUS_EXPAND(x) x

#define PLUMBUS_AUGMENT_ARGS(...) unused, __VA_ARGS__
#define PLUMBUS_EXPAND_ARGS(...) PLUMBUS_EXPAND(PLUMBUS_GETARGCOUNT(__VA_ARGS__))
#define PLUMBUS_GETARGCOUNT(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, count, ...) count

#define PL_ASSERT_MESSAGE_0(msg_buf, ...) snprintf(msg_buf, 1024, " ");
#define PL_ASSERT_MESSAGE_1(msg_buf, ...) snprintf(msg_buf, 1024, __VA_ARGS__);
#define PL_ASSERT_MESSAGE_N(msg_buf, msg, ...) snprintf(msg_buf, 1024, msg, __VA_ARGS__);
#define PL_ASSERT_MESSAGE(msg_buf, ...) PLUMBUS_CAT(PL_ASSERT_MESSAGE_, PLUMBUS_VARG_COUNT_01N(__VA_ARGS__))(msg_buf, __VA_ARGS__)
}

