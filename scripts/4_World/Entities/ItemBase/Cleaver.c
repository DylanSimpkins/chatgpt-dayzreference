class Cleaver extends ToolBase
{
	void Cleaver()
	{
	}
	
	override bool IsMeleeFinisher()
	{
		return true;
	}
	
	override array<int> GetValidFinishers()
	{
		return {EMeleeHitType.FINISHER_NECKSTAB};
	}
	
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionUnrestrainTarget);
		AddAction(ActionSkinning);
		AddAction(ActionMineBush);
		AddAction(ActionMineTreeBark);
		AddAction(ActionDigWorms);
		AddAction(ActionShaveTarget);
		AddAction(ActionShave);
		AddAction(ActionCraftBolts);
	}
}