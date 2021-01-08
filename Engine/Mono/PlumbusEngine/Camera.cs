using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using GlmNet;

namespace PlumbusEngineMono
{
	public static class Camera
	{
		[DllImport("__Internal", EntryPoint = "Camera_GetViewMatrix", CallingConvention = CallingConvention.Cdecl)]
		private static extern mat4 Camera_GetViewMatrix();

        [DllImport("__Internal", EntryPoint = "Camera_SetViewMatrix", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Camera_SetViewMatrix(mat4 viewMat);
        public static mat4 ViewMatrix
        {
            get => Camera_GetViewMatrix();
            set => Camera_SetViewMatrix(value);
        }

        [DllImport("__Internal", EntryPoint = "Camera_GetProjectionMatrix", CallingConvention = CallingConvention.Cdecl)]
		private static extern mat4 Camera_GetProjectionMatrix();

        [DllImport("__Internal", EntryPoint = "Camera_SetProjectionMatrix", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Camera_SetProjectionMatrix(mat4 projMat);
        public static mat4 ProjectionMatrix
        {
            get => Camera_GetProjectionMatrix();
            set => Camera_SetProjectionMatrix(value);
        }

		[DllImport("__Internal", EntryPoint = "Camera_GetPosition", CallingConvention = CallingConvention.Cdecl)]
		private static extern vec3 Camera_GetPosition();

        [DllImport("__Internal", EntryPoint = "Camera_SetPosition", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Camera_SetPosition(vec3 pos);
		public static vec3 Position
        {
            get => Camera_GetPosition();
            set => Camera_SetPosition(value);
        }

		[DllImport("__Internal", EntryPoint = "Camera_GetRotation", CallingConvention = CallingConvention.Cdecl)]
		private static extern vec3 Camera_GetRotation();

        [DllImport("__Internal", EntryPoint = "Camera_SetRotation", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Camera_SetRotation(vec3 rot);
        public static vec3 Rotation
        {
            get => Camera_GetRotation();
            set => Camera_SetRotation(value);
        }

        [DllImport("__Internal", EntryPoint = "Camera_GetForward", CallingConvention = CallingConvention.Cdecl)]
        private static extern vec3 Camera_GetForward();
        public static vec3 Forward => Camera_GetForward();
    }
}
