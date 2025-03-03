class ActionCraftImprovisedTorsoCoverCB : ActionContinuousBaseCB
{
	private const float TIME_TO_CRAFT_CLOTHES = 5.0;
	
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(TIME_TO_CRAFT_CLOTHES);
	}
};

class ActionCraftImprovisedTorsoCover: ActionContinuousBase
{
	void ActionCraftImprovisedTorsoCover()
	{
		m_CallbackClass = ActionCraftImprovisedTorsoCoverCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_CRAFTING;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH;
		m_SpecialtyWeight = UASoftSkillsWeight.ROUGH_HIGH;
		m_Text = "#STR_CraftImprovisedTorsoCover";
	}
	
	override void CreateConditionComponents()  
	{		
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNone;
	}
	
	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if( item.GetQuantity() ==6 )
		{
			return true;
		}
		return false;
	}
	
	override bool HasTarget()
	{
		return false;
	}
	
	override void OnFinishProgressServer( ActionData action_data )
	{
		EntityAI item_ingredient = action_data.m_MainItem;
		EntityAI cover = action_data.m_Player.SpawnEntityOnGroundRaycastDispersed("TorsoCover_Improvised");
		action_data.m_MainItem.AddQuantity(-6);
		
		MiscGameplayFunctions.TransferItemProperties(item_ingredient, cover);
	}
	
	override string GetSoundCategory(ActionData action_data)
	{
		return "ImprovisedCloth_craft";
	}
};
