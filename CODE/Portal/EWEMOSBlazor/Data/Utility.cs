using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace EWEMOSBlazor.Data;

public static class Utility
{
	// public static string Format(float value, SensorType sensorType) => sensorType switch
	// {
	// 	SensorType.Temperature => $"{value:F2} C",
	// 	SensorType.Humidity => $"{value:F2} %",
	// 	SensorType.Pressure => $"{value:F2} kPa",
	// 	_ => value.ToString()
	// };

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
	
	[MethodImpl(MethodImplOptions.AggressiveInlining)]
	public static UInt128 ToUInt128(ReadOnlySpan<byte> value)
	{
		// if (value.Length < sizeof(float))
			// ThrowHelper.ThrowArgumentOutOfRangeException(ExceptionArgument.value);
		return Unsafe.ReadUnaligned<UInt128>(ref MemoryMarshal.GetReference(value));
	}
}