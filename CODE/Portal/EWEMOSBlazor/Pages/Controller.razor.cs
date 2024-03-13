using EWEMOSBlazor.Data;
using Microsoft.AspNetCore.Components;
using Microsoft.JSInterop;

namespace EWEMOSBlazor.Pages;

public partial class Controller : ComponentBase
{
	[Inject] private MQTTController MQTTController { get; set; } = null!;

	[Inject] private IJSRuntime JS { get; set; } = null!;

	[Parameter] public required string ControllerID { get; set; }

	private string ControllerName { get; set; } = "N/A";

	private EMController? EMController;

	protected override void OnInitialized()
	{
		if (MQTTController.Controllers.TryGetValue(ControllerID, out EMController))
		{
			ControllerName = EMController.Name;
			
			EMController.OnChanged = () => { InvokeAsync(StateHasChanged); };
		}
	}

	private async void OnSubmit()
	{
		if (EMController is null) return;
		
		await MQTTController.SendControllerName(EMController, ControllerName);
		await JS.InvokeVoidAsync("closeDialog");
	}
	
	private async void OnEditClick()
	{
		await JS.InvokeVoidAsync("openDialog");
	}
}