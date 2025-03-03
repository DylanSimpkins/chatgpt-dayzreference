// Buldozer Tools by Lappihuan

// [texts]

roadReloadKey = roadReload select 1;
weatherTimeSetKey = weatherTimeSet select 1;
roadDiagKey = roadDiag select 1;
helpKey = help select 1;
NVG_IRKey = NVG_IR select 1;
debugKey = debug select 1;

visionModeIndex = 1;

saveNotificationTitle = "Save Notification";

saveNotificationText = "<t size='0.75'>Please save your project in TB before you continue to work<t/>";

buldozerToolsHelpText = format["<t size='0.75'>Press %4 to show this help<t/><br/><br/>
						<t size='0.75'>Controlls Help:<t/><br/>
						<t size='0.75'>Press %1 to reload roads<t/><br/>
						<t size='0.75'>Press %2 to reset weather and time<t/><br/>
						<t size='0.75'>Press %3 to toggle road diagnostic mode<t/><br/>
						<t size='0.75'>Press %6 to toggle IR and NVG mode<t/><br/>
						<t size='0.75'>Press %5 to execute debug.sqf<t/><br/><br/>
<t size='0.75' color='#ff0000'>Only move/rotate your camera while road diagnostic is enabled, other actions can crash buldozer!<t/>",
roadReloadKey,
weatherTimeSetKey,
roadDiagKey,
helpKey,
debugKey,
NVG_IRKey];

buldozerToolsHelpTitle = "Buldozer Tools Help v0.2";