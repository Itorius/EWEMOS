using System.Runtime.InteropServices;

namespace EWEMOSBlazor.Data;

[StructLayout(LayoutKind.Explicit, Size = 44)]
public struct MqttMessageConfig
{
	[FieldOffset(0)] public uint ID;
	[FieldOffset(4)] public SensorType Type;
	[FieldOffset(6)] public ushort ConfigSet; // this is a temporary solution until I switch to MQTT5
	[FieldOffset(8)] public uint Interval;
	[FieldOffset(12)] public unsafe fixed byte Name[32];
}

[StructLayout(LayoutKind.Explicit, Size = 68)]
public struct MqttMessageName
{
	[FieldOffset(0)] public unsafe fixed byte Name[64];
	[FieldOffset(64)] public uint ConfigSet; // this is a temporary solution until I switch to MQTT5
}

[StructLayout(LayoutKind.Explicit, Size = 8)]
public struct MqttMessageState
{
	[FieldOffset(0)] public uint ID;
	[FieldOffset(4)] public ConnectionState State;
}

[StructLayout(LayoutKind.Explicit, Size = 60)]
public struct MqttMessageData
{
	[FieldOffset(0)] public uint ID;
	[FieldOffset(4)] public DataType Type;
	[FieldOffset(6)] public ushort Length;
	[FieldOffset(8)] public unsafe fixed byte Data[52];
}