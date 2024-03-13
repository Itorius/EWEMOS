using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using System.Text;
using MQTTnet;
using MQTTnet.Server;

namespace EWEMOSBlazor.Data;

public class MQTTController
{
	[StructLayout(LayoutKind.Explicit, Size = 44)]
	public struct MqttMessageConfig
	{
		[FieldOffset(0)] public uint ID;
		[FieldOffset(4)] public SensorType Type;
		[FieldOffset(6)] public ushort ConfigSet; // NOTE: this is a temporary solution until I switch to MQTT5
		[FieldOffset(8)] public uint Interval;
		[FieldOffset(12)] public unsafe fixed byte Name[32];
	}

	[StructLayout(LayoutKind.Explicit, Size = 68)]
	public struct MqttMessageName
	{
		[FieldOffset(0)] public unsafe fixed byte Name[64];
		[FieldOffset(64)] public uint ConfigSet; // NOTE: this is a temporary solution until I switch to MQTT5
	}

	[StructLayout(LayoutKind.Explicit, Size = 8)]
	public struct MqttMessageState
	{
		[FieldOffset(0)] public uint ID;
		[FieldOffset(4)] public ConnectionState State;
	}

	[StructLayout(LayoutKind.Explicit, Size = 8)]
	public struct MqttMessageData
	{
		[FieldOffset(0)] public uint ID;
		[FieldOffset(4)] public float Data;
	}

	public static readonly ConcurrentDictionary<string, EMController> Controllers = new();

	public Action? OnControllerChanged;
	public MqttServer Server = null!;
	private object clientLock = new();

	public void Init(MqttServer server)
	{
		Server = server;

		server.ValidatingConnectionAsync += ValidateConnection;
		server.ClientConnectedAsync += ClientConnected;
		server.ClientDisconnectedAsync += ClientDisconnected;
		server.InterceptingPublishAsync += ServerOnInterceptingPublishAsync;
	}

	#region Callbacks
	public Task ValidateConnection(ValidatingConnectionEventArgs args)
	{
		Console.WriteLine($"Client '{args.ClientId}' wants to connect. Accepting!");

		return Task.CompletedTask;
	}

	public Task ClientConnected(ClientConnectedEventArgs args)
	{
		Console.WriteLine($"Client '{args.ClientId}' connected.");

		lock (clientLock)
		{
			if (!Controllers.ContainsKey(args.ClientId))
			{
				Controllers.TryAdd(args.ClientId, new EMController(args.ClientId));
				OnControllerChanged?.Invoke();
			}
		}

		return Task.CompletedTask;
	}

	// NOTE: connection status for controller?
	private Task ClientDisconnected(ClientDisconnectedEventArgs args)
	{
		Console.WriteLine($"Client '{args.ClientId}' disconnected. {args.DisconnectType}");

		lock (clientLock)
		{
			if (Controllers.ContainsKey(args.ClientId))
			{
				Controllers.TryRemove(args.ClientId, out _);
				OnControllerChanged?.Invoke();
			}
		}

		return Task.CompletedTask;
	}

	private unsafe Task ServerOnInterceptingPublishAsync(InterceptingPublishEventArgs message)
	{
		Console.WriteLine($"Received message from \"{message.ClientId}\", topic \"{message.ApplicationMessage.Topic}\", payload length: {message.ApplicationMessage.PayloadSegment.Count}");

		string[] args = message.ApplicationMessage.Topic.Split('/');
		if (args.Length < 2) return Task.CompletedTask;

		if (!Controllers.TryGetValue(message.ClientId, out EMController? controller))
			return Task.CompletedTask;

		switch (args[1])
		{
			case "name":
			{
				var mqtt_message = Utility.FromBytes<MqttMessageName>(message.ApplicationMessage.PayloadSegment.Array);

				controller.Name = Encoding.Default.GetString(mqtt_message.Name, 64).TrimEnd('\0');
				Controllers[message.ClientId] = controller;

				OnControllerChanged?.Invoke();

				Console.WriteLine($"Controller name change {controller.Name}");

				break;
			}
			case "config":
			{
				var mqtt_message = Utility.FromBytes<MqttMessageConfig>(message.ApplicationMessage.PayloadSegment.Array);

				if (!controller.Sensors.TryGetValue(mqtt_message.ID, out var sensor))
				{
					sensor = new EMSensor(mqtt_message.ID, message.ClientId, mqtt_message.Type, mqtt_message.Interval, Encoding.Default.GetString(mqtt_message.Name, 32).TrimEnd('\0'));
					controller.Sensors.Add(mqtt_message.ID, sensor);
					controller.OnChanged?.Invoke();

					Console.WriteLine($"Received new sensor '{sensor.Name}' ({sensor.ID}): type {sensor.Type}, interval {sensor.Interval} ms");
				}

				break;
			}
			case "state":
			{
				var mqtt_message = Utility.FromBytes<MqttMessageState>(message.ApplicationMessage.PayloadSegment.Array);

				if (controller.Sensors.TryGetValue(mqtt_message.ID, out var sensor))
				{
					sensor.State = mqtt_message.State;
					sensor.OnChanged?.Invoke();

					Console.WriteLine($"Received state from sensor '{sensor.Name}' ({sensor.ID}): state {Enum.GetName(sensor.State)}");
				}

				break;
			}
			case "data":
			{
				var mqtt_message = Utility.FromBytes<MqttMessageData>(message.ApplicationMessage.PayloadSegment.Array);

				if (controller.Sensors.TryGetValue(mqtt_message.ID, out var sensor))
				{
					sensor.Data = mqtt_message.Data;
					sensor.OnChanged?.Invoke();

					Console.WriteLine($"Received data from sensor '{sensor.Name}' ({sensor.ID}): data {mqtt_message.Data}");
				}

				break;
			}
		}

		return Task.CompletedTask;
	}
	#endregion

	// bug: we dont know if the controller received the data successfully, potential desync
	public async Task SendControllerName(EMController controller, string name)
	{
		controller.Name = name; // this will be removed

		MqttMessageName mqttMessage = new()
		{
			ConfigSet = 1
		};
		unsafe
		{
			byte[] arr = Encoding.Default.GetBytes(name);
			Marshal.Copy(arr, 0, new IntPtr(mqttMessage.Name), arr.Length);
		}

		var message = new MqttApplicationMessageBuilder().WithTopic($"{controller.ID}/name").WithPayload(Utility.ToBytes(mqttMessage)).Build();
		await Server.InjectApplicationMessage(new InjectedMqttApplicationMessage(message) { SenderClientId = controller.ID }, CancellationToken.None);
	}

	public async Task SendSensorConfig(EMSensor sensor, string name, uint interval)
	{
		sensor.Name = name;
		sensor.Interval = interval;

		MqttMessageConfig mqttMessage = new()
		{
			ID = sensor.ID,
			Interval = interval,
			ConfigSet = 1
		};
		unsafe
		{
			byte[] arr = Encoding.Default.GetBytes(name);
			Marshal.Copy(arr, 0, new IntPtr(mqttMessage.Name), arr.Length);
		}

		var message = new MqttApplicationMessageBuilder().WithTopic($"{sensor.ControllerID}/config").WithPayload(Utility.ToBytes(mqttMessage)).Build();
		await Server.InjectApplicationMessage(new InjectedMqttApplicationMessage(message) { SenderClientId = sensor.ControllerID }, CancellationToken.None);
	}
}