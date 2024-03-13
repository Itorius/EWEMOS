using Blazorise;
using Blazorise.Bootstrap;
using EWEMOSBlazor.Data;
using MQTTnet.AspNetCore;

var builder = WebApplication.CreateBuilder(args);

builder.WebHost.UseKestrel(o =>
{
	o.ListenAnyIP(1883, l => l.UseMqtt()); // MQTT pipeline
	o.ListenAnyIP(5116); // Default HTTP pipeline
});

// Add services to the container.
builder.Services.AddRazorPages();
builder.Services.AddServerSideBlazor();
builder.Services.AddBlazorise(options => { options.Immediate = true; });
builder.Services.AddBootstrapProviders();

builder.Services.AddHostedMqttServer(mqttServer => mqttServer.WithoutDefaultEndpoint())
	.AddMqttConnectionHandler()
	.AddConnections()
	.AddSingleton<MQTTController>();

var app = builder.Build();

// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment())
{
	app.UseExceptionHandler("/Error");
}

app.UseStaticFiles();

app.UseRouting();

app.MapGet("/mqtt", () => { });

app.UseMqttServer(server =>
{
	var mqttController = app.Services.GetService<MQTTController>();
	mqttController?.Init(server);
});

app.MapBlazorHub();
app.MapFallbackToPage("/_Host");

app.Run();