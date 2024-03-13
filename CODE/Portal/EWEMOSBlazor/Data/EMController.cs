namespace EWEMOSBlazor.Data;

public enum SensorType : ushort
{
	Temperature = 0,
	Pressure = 1,
	Humidity = 2
}

public enum ConnectionState : uint
{
	Unconfigured = 0,
	Connected = 1,
	NotConnected = 2
}

public class EMController(string id)
{
	public readonly string ID = id;
	public string Name = "N/A";
	public readonly Dictionary<uint, EMSensor> Sensors = new();

	public Action? OnChanged;
}

public class EMSensor(uint id, string controllerID, SensorType type, uint interval, string name)
{
	public readonly uint ID = id;
	public readonly SensorType Type = type;
	public uint Interval = interval;
	public string Name = name;
	public ConnectionState State = ConnectionState.Connected;

	public Action? OnChanged;

	public float Data;

	public readonly string ControllerID = controllerID;
}