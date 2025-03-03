diag_log format["Buldozer Tools: initialized with roadpath: '%1'",roadpath];

roadReloadDIK = roadReload select 0;
weatherTimeSetDIK = weatherTimeSet select 0;
roadDiagDIK = roadDiag select 0;
helpDIK = help select 0;
NVG_IRDIK =  NVG_IR select 0;
debugDIK = debug select 0;

_bulKeybinds = [] spawn {
	keyBind = (findDisplay -1) displayAddEventHandler['KeyDown','
		switch (_this select 1) do {
			case roadReloadDIK: {
				result = buldozer_LoadNewRoads roadpath;
				diag_log "Buldozer Tools: roads reloaded";
			};
			case weatherTimeSetDIK: {
				setDate dateArray;
				0 setRain 0;
				0 setOvercast 0;
				diag_log "Buldozer Tools: weather and time reset";
			};
			case roadDiagDIK: {
				if(buldozer_isEnabledRoadDiag)then{
					buldozer_enableRoadDiag false;
					diag_log "Buldozer Tools: road diag disabled";
				} else {
					buldozer_enableRoadDiag true;
					diag_log "Buldozer Tools: road diag enabled";
				};
			};
			case NVG_IRDIK: {

				switch(visionModeIndex)do{
					case 0: {
						false setCamUseTi tiMode;
						camUseNVG false;
					};
					case 1: {
						false setCamUseTi tiMode;
						camUseNVG true;
					};
					case 2: {
						true setCamUseTi tiMode;
						camUseNVG false;
					};
				};
				if(visionModeIndex>=2) then {
					visionModeIndex = 0;
				} else {
					visionModeIndex = visionModeIndex+1;
				};
			};
			case helpDIK: {_handle = [buldozerToolsHelpTitle,buldozerToolsHelpText] call LAP_fnc_buldozerHint;};
			case debugDIK: {_handle = execVM "BuldozerTools\debug.sqf";};
			default {diag_log "hello"};
		};
	'];
	if(showHelpOnStartup) then {
		waitUntil {!isNull findDisplay -1};
		_handle = [buldozerToolsHelpTitle,buldozerToolsHelpText] call LAP_fnc_buldozerHint;
	};
	if(saveNotificationIntervall!=0) then {
		while{true} do {
			sleep (saveNotificationIntervall * 60);
			_handle = [saveNotificationTitle,saveNotificationText] call LAP_fnc_buldozerHint;
		};
	};
};