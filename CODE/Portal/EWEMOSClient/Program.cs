using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using EWEMOSBlazor.Data;
using EWEMOSClient;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Internal;
using MQTTnet.Protocol;
using Utility = EWEMOSBlazor.Data.Utility;

internal class Program
{
	private static TestClient[] mqttClient;

	private class TestClient
	{
		public IMqttClient Client;

		public string Name;
		public Dictionary<uint, TestSensor> Sensors = new();
	}

	private class TestSensor(uint id, string name, SensorType type, uint interval)
	{
		public uint ID = id;
		public string Name = name;
		public SensorType Type = type;
		public uint Interval = interval;
	}

	public static string GetRandomID()
	{
		byte[] bytes = new byte[8];
		random.NextBytes(bytes);

		string clientID = Convert.ToBase64String(bytes);
		clientID = clientID.TrimEnd('=');
		clientID = clientID.Replace('+', '-');
		clientID = clientID.Replace('/', '_');
		return clientID;
	}

	public static readonly Random random = new();

	public static async Task Main(string[] args)
	{
		List<JsonController> controllers = [];
		controllers = JsonSerializer.Deserialize<List<JsonController>>(await File.ReadAllTextAsync("data.json"));
		// for (int i = 0; i < 5; i++)
		// {
		// 	JsonController controller = new JsonController
		// 	{
		// 		ID = GetRandomID(),
		// 		Name = "Controller" + (i + 1),
		// 		Sensors = new List<JsonSensor>()
		// 	};
		// 	for (int j = 0; j < random.Next(2, 6); j++)
		// 	{
		// 		var sensors = Enum.GetValues<SensorType>();
		// 		JsonSensor sensor = new JsonSensor
		// 		{
		// 			ID = GetRandomID(),
		// 			Name = "Sensor" + (j + 1),
		// 			Type = sensors[random.Next(sensors.Length)],
		// 			Interval = (uint)(random.Next(5, 11) * 100)
		// 		};
		// 		controller.Sensors.Add(sensor);
		// 	}
		//
		// 	controllers.Add(controller);
		// }

		// string json = JsonSerializer.Serialize(controllers);

		MqttFactory mqttFactory = new MqttFactory();

		mqttClient = new TestClient[controllers.Count];
		for (int i = 0; i < controllers.Count; i++)
		{
			var i1 = i;
			var controller = controllers[i];
			mqttClient[i] = new TestClient
			{
				Client = mqttFactory.CreateMqttClient(),
				Name = controller.Name
			};

			// var rpc = mqttFactory.CreateMqttRpcClient(mqttClient[i].Client);

			for (int j = 0; j < controller.Sensors.Count; j++)
			{
				var sensor = controller.Sensors[j];
				mqttClient[i].Sensors.Add(sensor.ID, new TestSensor(sensor.ID, sensor.Name, sensor.Type, sensor.Interval));
			}

			mqttClient[i].Client.DisconnectedAsync += args => OnDisconnected(mqttClient[i1], args);
			mqttClient[i].Client.ConnectedAsync += args => OnConnected(mqttClient[i1], args);
			mqttClient[i].Client.ApplicationMessageReceivedAsync += args => ApplicationMessageReceived(mqttClient[i1], args);

			try
			{
				MqttClientOptions mqttClientOptions = new MqttClientOptionsBuilder().WithClientId(controller.ID).WithTcpServer("localhost").Build();
				var response = await mqttClient[i].Client.ConnectAsync(mqttClientOptions, CancellationToken.None);
			}
			catch
			{
			}
		}

		Task.Run(async () =>
		{
			ConnectionState state = 0;
			while (true)
			{
				if (mqttClient[0].Client.IsConnected)
				{
					var (key, value) = mqttClient[0].Sensors.FirstOrDefault();
					if (key != default)
					{
						// MQTTController.MqttMessageState mqtt_message = new()
						// {
						// 	ID = key,
						// 	State = state
						// };
						// state = (ConnectionState)(((int)state + 1) % 3);
						//
						// var applicationMessage = new MqttApplicationMessageBuilder()
						// 	.WithTopic($"{mqttClient[0].Client.Options.ClientId}/state")
						// 	.WithPayload(Utility.ToBytes(mqtt_message))
						// 	.Build();
						//
						// await mqttClient[0].Client.PublishAsync(applicationMessage, CancellationToken.None);

						MQTTController.MqttMessageData mqttMessage = new()
						{
							ID = value.ID,
							Data = random.NextSingle() * 100f
						};

						var applicationMessage = new MqttApplicationMessageBuilder()
							.WithTopic($"{mqttClient[0].Client.Options.ClientId}/data")
							.WithPayload(Utility.ToBytes(mqttMessage))
							.Build();

						await mqttClient[0].Client.PublishAsync(applicationMessage, CancellationToken.None);

						await Task.Delay((int)value.Interval);
					}
					else
						await Task.Delay(5000);
				}
				else

					await Task.Delay(500);
			}
		}).GetAwaiter().GetResult();
	}

	private static unsafe Task ApplicationMessageReceived(TestClient mqttClient, MqttApplicationMessageReceivedEventArgs arg)
	{
		string[] args = arg.ApplicationMessage.Topic.Split('/');
		if (args.Length < 2) return Task.CompletedTask;

		switch (args[1])
		{
			case "name":
			{
				var mqtt_message = Utility.FromBytes<MQTTController.MqttMessageName>(arg.ApplicationMessage.PayloadSegment.Array);
				mqttClient.Name = Encoding.Default.GetString(mqtt_message.Name, 64).TrimEnd('\0');

				// NOTE: sync could be solved by resending the message from the controller to server

				Console.WriteLine($"Received controller name {mqttClient.Name}");
				break;
			}
			case "config":
			{
				var mqtt_message = Utility.FromBytes<MQTTController.MqttMessageConfig>(arg.ApplicationMessage.PayloadSegment.Array);

				if (mqttClient.Sensors.TryGetValue(mqtt_message.ID, out TestSensor sensor))
				{
					sensor.Interval = mqtt_message.Interval;
					sensor.Name = Encoding.Default.GetString(mqtt_message.Name, 32).TrimEnd('\0');

					Console.WriteLine($"Received sensor config {sensor.Name}/{sensor.Interval} ms");
				}

				break;
			}
		}

		return Task.CompletedTask;
	}

	private static async Task OnConnected(TestClient mqttClient, MqttClientConnectedEventArgs args)
	{
		Console.WriteLine($"[{mqttClient.Client.Options.ClientId}] Connected.");

		MQTTController.MqttMessageName message = new();
		unsafe
		{
			byte[] arr = Encoding.Default.GetBytes(mqttClient.Name);
			Marshal.Copy(arr, 0, new IntPtr(message.Name), arr.Length);
		}

		var applicationMessage = new MqttApplicationMessageBuilder()
			.WithTopic($"{mqttClient.Client.Options.ClientId}/name")
			.WithPayload(Utility.ToBytes(message))
			.Build();

		await mqttClient.Client.PublishAsync(applicationMessage, CancellationToken.None);

		foreach (var sensor in mqttClient.Sensors)
		{
			MQTTController.MqttMessageConfig messageConfig = new()
			{
				ID = sensor.Value.ID,
				Type = sensor.Value.Type,
				Interval = sensor.Value.Interval
			};
			unsafe
			{
				byte[] arr = Encoding.Default.GetBytes(sensor.Value.Name);
				Marshal.Copy(arr, 0, new IntPtr(messageConfig.Name), arr.Length);
			}

			applicationMessage = new MqttApplicationMessageBuilder()
				.WithTopic($"{mqttClient.Client.Options.ClientId}/config")
				.WithPayload(Utility.ToBytes(messageConfig))
				.Build();

			await mqttClient.Client.PublishAsync(applicationMessage, CancellationToken.None);

			await Task.Delay(250);
		}

		await mqttClient.Client.SubscribeAsync($"{mqttClient.Client.Options.ClientId}/config", MqttQualityOfServiceLevel.AtMostOnce, CancellationToken.None);
		await mqttClient.Client.SubscribeAsync($"{mqttClient.Client.Options.ClientId}/name", MqttQualityOfServiceLevel.AtMostOnce, CancellationToken.None);
	}

	private static async Task OnDisconnected(TestClient mqttClient, MqttClientDisconnectedEventArgs args)
	{
		Console.WriteLine($"[{mqttClient.Client.Options.ClientId}] Disconnected, retrying in 5 seconds.");
		await Task.Delay(TimeSpan.FromSeconds(5));
		await mqttClient.Client.ReconnectAsync(CancellationToken.None);
	}
}