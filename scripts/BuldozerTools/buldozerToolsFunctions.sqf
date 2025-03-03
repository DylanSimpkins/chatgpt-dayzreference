// Buldozer Tools by Lappihuan

LAP_fnc_buldozerHint = {

	with uiNamespace do
	{
   		disableSerialization;
		_buldozerHintTitle = _this select 0;
		_buldozerHintText = _this select 1;

		//(findDisplay -1) createDisplay "RscDisplayDebug";
		_buldozerDisplay = (findDisplay -1);

		buldozerHint = _buldozerDisplay createDisplay "RscDisplayHintCEx";

		_ctrlBcgCommonTop =		buldozerHint displayctrl 103;
		_ctrlBcgCommon =		buldozerHint displayctrl 101;
		_ctrlButtonContinue =	buldozerHint displayctrl 104;
		_ctrlText =				buldozerHint displayctrl 102;


		//--- Calculate spacing
		_ctrlButtonContinuePos = ctrlposition _ctrlButtonContinue;
		_ctrlBcgCommonPos = ctrlposition _ctrlBcgCommon;
		_bottomSpaceY = (_ctrlButtonContinuePos select 1) - ((_ctrlBcgCommonPos select 1) + (_ctrlBcgCommonPos select 3));

		//--- Calculate text padding
		_ctrlTextPos = ctrlposition _ctrlText;
		_marginX = (_ctrlTextPos select 0) - (_ctrlBcgCommonPos select 0);
		_marginY = (_ctrlTextPos select 1) - (_ctrlBcgCommonPos select 1);


		if (typename _buldozerHintText == typename "") then {_buldozerHintText = parsetext _buldozerHintText;};

		_ctrlBcgCommonTop ctrlsettext _buldozerHintTitle;


		_ctrlText ctrlsetstructuredtext _buldozerHintText;
		_ctrlTextPosH = ctrltextheight _ctrlText;

		//--- Move text area
		_ctrlBcgCommon ctrlsetposition [
		(_ctrlBcgCommonPos select 0),
		(_ctrlBcgCommonPos select 1),
		(_ctrlBcgCommonPos select 2),
		_ctrlTextPosH + (_marginY * 2)
		];
		_ctrlBcgCommon ctrlcommit 0;

		_ctrlText ctrlsetposition [
		(_ctrlBcgCommonPos select 0) + _marginX,
		(_ctrlBcgCommonPos select 1) + _marginY,
		(_ctrlBcgCommonPos select 2) - _marginX * 2,
		_ctrlTextPosH
		];

		_ctrlText ctrlcommit 0;

		//--- Move bottom bar
		_bottomPosY = (_ctrlBcgCommonPos select 1) + _ctrlTextPosH + (_marginY * 2) + _bottomSpaceY;
		{
		_xPos = ctrlposition _x;
		_xPos set [1,_bottomPosY];
		_x ctrlsetposition _xPos;
		_x ctrlcommit 0;
		} foreach [
		_ctrlButtonContinue
		];

		_ctrlButtonContinue ctrlcommit 0;
		_ctrlButtonContinue ctrlseteventhandler ["buttonclick","closeDialog 1;"];
	};
};
