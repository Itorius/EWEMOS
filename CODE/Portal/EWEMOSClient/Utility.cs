using System.Runtime.InteropServices;

namespace MQTTClient;

public static class Utility
{
	private static Random random = new();
	
	public static string GetRandomID()
	{
		byte[] bytes = new byte[8];
		random.NextBytes(bytes);
		
		string ID = Convert.ToBase64String(bytes);
		ID = ID.TrimEnd('=');
		ID = ID.Replace('+', '-');
		ID = ID.Replace('/', '_');

		return ID;
	}
	
	public static byte[] ToBytes<T>(T value) where T : struct
	{
		int size = Marshal.SizeOf(value);
		byte[] arr = new byte[size];

		IntPtr ptr = IntPtr.Zero;
		try
		{
			ptr = Marshal.AllocHGlobal(size);
			Marshal.StructureToPtr(value, ptr, true);
			Marshal.Copy(ptr, arr, 0, size);
		}
		finally
		{
			Marshal.FreeHGlobal(ptr);
		}

		return arr;
	}

	public static T FromBytes<T>(byte[] data) where T : struct
	{
		int size = Marshal.SizeOf(typeof(T));
		if (data.Length < size)
			throw new Exception("Invalid parameter");

		IntPtr ptr = Marshal.AllocHGlobal(size);
		try
		{
			Marshal.Copy(data, 0, ptr, size);
			return (T)Marshal.PtrToStructure(ptr, typeof(T));
		}
		finally
		{
			Marshal.FreeHGlobal(ptr);
		}
	}
}