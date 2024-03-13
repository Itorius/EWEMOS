using EWEMOSBlazor.Data;

namespace EWEMOSClient;

public struct JsonController
{
	public string ID { get; set; }
	public string Name { get; set; }
	public List<JsonSensor> Sensors { get; set; }
}

public struct JsonSensor
{
	public uint ID { get; set; }
	public string Name { get; set; }
	public SensorType Type { get; set; }
	public uint Interval { get; set; }
}