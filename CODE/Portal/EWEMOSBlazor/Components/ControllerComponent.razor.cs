using EWEMOSBlazor.Data;
using Microsoft.AspNetCore.Components;

namespace EWEMOSBlazor.Components;

public partial class ControllerComponent : ComponentBase
{
	[Inject] private MQTTController MQTTController { get; set; } = null!;

	[Inject] private NavigationManager Navigation { get; set; } = null!;

	[Parameter] public EMController? Controller { get; set; }

	protected override void OnInitialized()
	{
		MQTTController.OnControllerChanged += () => { InvokeAsync(StateHasChanged); };
	}
	
	private void ControllerClick()
	{
		if (Controller is null) return;

		Navigation.NavigateTo("controller/" + Controller.ID);
	}
}