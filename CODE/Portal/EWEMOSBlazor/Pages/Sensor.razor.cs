using Blazorise.Charts;
using EWEMOSBlazor.Data;
using Microsoft.AspNetCore.Components;
using Microsoft.JSInterop;

namespace EWEMOSBlazor.Pages;

public partial class Sensor : ComponentBase
{
	[Inject] private MQTTController MQTTController { get; set; } = null!;

	[Inject] private IJSRuntime JS { get; set; } = null!;

	[Parameter] public required string ControllerID { get; set; }
	[Parameter] public required long SensorID { get; set; }

	private string SensorName { get; set; } = "N/A";
	private uint SensorInterval { get; set; }

	private EMSensor? EMSensor;

	// TODO: add back button to controller

	protected override void OnInitialized()
	{
		// options = new LineChartOptions
		// {
		// 	Scales = new ChartScales
		// 	{
		// 		X = new ChartAxis
		// 		{
		// 			Title = new ChartScaleTitle
		// 			{
		// 				Display = true,
		// 				Text = "Time"
		// 			},
		// 			Time = new ChartAxisTime
		// 			{
		// 				Unit = "second"
		// 			},
		// 			Type = "time"
		// 		},
		// 		Y = new ChartAxis
		// 		{
		// 			Title = new ChartScaleTitle
		// 			{
		// 				Display = true,
		// 				Text = "Value"
		// 			}
		// 		}
		// 	}
		// };
		//
		// horizontalLineChart = new LineChart<LiveDataPoint>();
		//
		// dataset = new LineChartDataset<LiveDataPoint>
		// {
		// 	Data = new List<LiveDataPoint>(),
		// 	Label = "Dataset 1 (linear interpolation)",
		// 	BackgroundColor = backgroundColors[0],
		// 	BorderColor = borderColors[0],
		// 	Fill = false,
		// 	Tension = 0,
		// 	BorderDash = new List<int> { 8, 4 }
		// };
		// horizontalLineChart.AddDataSet(dataset);

		if (MQTTController.Controllers.TryGetValue(ControllerID, out EMController? controller))
		{
			if (controller.Sensors.TryGetValue((uint)SensorID, out EMSensor))
			{
				SensorName = EMSensor.Name;
				SensorInterval = EMSensor.Interval;

				EMSensor.OnChanged += () =>
				{
					InvokeAsync(StateHasChanged);
					// dataset.Data.Add( new LiveDataPoint { X = DateTime.Now, Y = EMSensor.Data });

					// horizontalLineChart?.AddData(0, new LiveDataPoint { X = DateTime.Now, Y = EMSensor.Data });
					// horizontalLineChart?.Update();
					// if (horizontalLineChart.Data.Datasets[0].Data.Count > 10) horizontalLineChart.ShiftData(0);
				};
			}
		}

		// PlotSin();
	}

	private async void OnSubmit()
	{
		if (EMSensor is null) return;

		await JS.InvokeVoidAsync("closeDialog");
		await MQTTController.SendSensorConfig(EMSensor, SensorName, SensorInterval);
		// 	await InvokeAsync(StateHasChanged);
	}

	private async void OnEditClick()
	{
		if (EMSensor is null) return;

		SensorName = EMSensor.Name;
		SensorInterval = EMSensor.Interval;

		await JS.InvokeVoidAsync("openDialog");
	}

	// LineChart<LiveDataPoint> horizontalLineChart;

	// private LineChartDataset<LiveDataPoint>? dataset;

	// string[] Labels = { "Red", "Blue", "Yellow", "Green", "Purple", "Orange" };
	// List<string> backgroundColors = new List<string> { ChartColor.FromRgba(255, 99, 132, 0.2f), ChartColor.FromRgba(54, 162, 235, 0.2f), ChartColor.FromRgba(255, 206, 86, 0.2f), ChartColor.FromRgba(75, 192, 192, 0.2f), ChartColor.FromRgba(153, 102, 255, 0.2f), ChartColor.FromRgba(255, 159, 64, 0.2f) };
	// List<string> borderColors = new List<string> { ChartColor.FromRgba(255, 99, 132, 1f), ChartColor.FromRgba(54, 162, 235, 1f), ChartColor.FromRgba(255, 206, 86, 1f), ChartColor.FromRgba(75, 192, 192, 1f), ChartColor.FromRgba(153, 102, 255, 1f), ChartColor.FromRgba(255, 159, 64, 1f) };
	//
	// public struct LiveDataPoint
	// {
	// 	public object X { get; set; }
	//
	// 	public object Y { get; set; }
	// }
	//
	// private LineChartOptions options = new()
	// {
	// 	Scales = new ChartScales
	// 	{
	// 		X = new ChartAxis
	// 		{
	// 			Title = new ChartScaleTitle
	// 			{
	// 				Display = true,
	// 				Text = "Time"
	// 			},
	// 			Time = new ChartAxisTime
	// 			{
	// 				Unit = "second"
	// 			},
	// 			Type = "time"
	// 		},
	// 		Y = new ChartAxis
	// 		{
	// 			Title = new ChartScaleTitle
	// 			{
	// 				Display = true,
	// 				Text = "Value"
	// 			}
	// 		}
	// 	}
	// };
	//
	// // object horizontalLineChartOptions = new
	// // {
	// // 	Scales = new
	// // 	{
	// // 		Y = new
	// // 		{
	// // 			Title = new
	// // 			{
	// // 				Display = true,
	// // 				Text = "Value"
	// // 			}
	// // 		},
	// // 		X = new
	// // 		{
	// // 			Type = "time",
	// // 			Time = new
	// // 			{
	// // 				Unit = "second"
	// // 			}
	// // 		}
	// // 	},
	// // 	Interaction = new
	// // 	{
	// // 		intersect = false
	// // 	}
	// // };
	//
	// protected override async Task OnAfterRenderAsync(bool firstRender)
	// {
	// 	if (firstRender)
	// 	{
	// 		await horizontalLineChart.Clear();
	// 		await horizontalLineChart.SetOptions(options);
	// 		if(dataset!=null) await horizontalLineChart.AddDataSet(dataset);
	// 		// await Task.WhenAll(HandleRedraw(horizontalLineChart));
	// 	}
	// }
	//
	// async Task HandleRedraw<TDataSet, TItem, TOptions, TModel>(BaseChart<TDataSet, TItem, TOptions, TModel> chart) where TDataSet : ChartDataset<TItem> where TOptions : ChartOptions where TModel : ChartModel
	// {
	// 	await chart.Clear();
	//
	// 	// await chart.AddDatasetsAndUpdate(dataset);
	// 	// await chart.AddLabelsDatasetsAndUpdate(Labels, getDataSets.Select(x => x.Invoke()).ToArray());
	// }
}