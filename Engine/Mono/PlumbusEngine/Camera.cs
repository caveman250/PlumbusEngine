using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using GlmNet;

namespace PlumbusEngineMono
{
	static class Camera
	{
		[DllImport("__Internal", EntryPoint = "Camera_GetViewMatrix", CallingConvention = CallingConvention.Cdecl)]
		private static extern mat4 Camera_GetViewMatrix();
		public static mat4 GetViewMatrix()
		{
			return Camera_GetViewMatrix();
		}

		[DllImport("__Internal", EntryPoint = "Camera_GetProjectionMatrix", CallingConvention = CallingConvention.Cdecl)]
		private static extern mat4 Camera_GetProjectionMatrix();
		public static mat4 GetProjectionMatrix()
		{
			return Camera_GetProjectionMatrix();
		}

		[DllImport("__Internal", EntryPoint = "Camera_GetPosition", CallingConvention = CallingConvention.Cdecl)]
		private static extern vec3 Camera_GetPosition();
		public static vec3 GetPosition()
		{
			return Camera_GetPosition();
		}

		[DllImport("__Internal", EntryPoint = "Camera_GetRotation", CallingConvention = CallingConvention.Cdecl)]
		private static extern vec3 Camera_GetRotation();
		public static vec3 GetRotation()
		{
			return Camera_GetRotation();
		}

		[DllImport("__Internal", EntryPoint = "Camera_SetViewMatrix", CallingConvention = CallingConvention.Cdecl)]
		private static extern void Camera_SetViewMatrix(mat4 viewMat);
		public static void SetViewMatrix(mat4 viewMat)
		{
			Camera_SetViewMatrix(viewMat);
		}

		[DllImport("__Internal", EntryPoint = "Camera_SetProjectionMatrix", CallingConvention = CallingConvention.Cdecl)]
		private static extern void Camera_SetProjectionMatrix(mat4 projMat);
		public static void SetProjectionMatrix(mat4 viewMat)
		{
			Camera_SetProjectionMatrix(viewMat);
		}

		[DllImport("__Internal", EntryPoint = "Camera_SetPosition", CallingConvention = CallingConvention.Cdecl)]
		private static extern void Camera_SetPosition(vec3 pos);
		public static void SetPosition(vec3 pos)
		{
			Camera_SetPosition(pos);
		}

		[DllImport("__Internal", EntryPoint = "Camera_SetRotation", CallingConvention = CallingConvention.Cdecl)]
		private static extern void Camera_SetRotation(vec3 rot);
		public static void SetRotation(vec3 rot)
		{
			Camera_SetRotation(rot);
		}
	}
}
