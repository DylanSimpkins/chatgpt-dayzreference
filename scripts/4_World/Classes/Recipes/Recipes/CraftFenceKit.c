class CraftFenceKit extends RecipeBase
{	
	override void Init()
	{
		m_Name = "#STR_craftfencebase0";
		m_IsInstaRecipe = false;								// should this recipe be performed instantly without animation
		m_AnimationLength = 0.5;								// animation length in relative time units

		//conditions
		m_MinDamageIngredient[0]			= -1;				//-1 = disable check
		m_MaxDamageIngredient[0]			= 3;				//-1 = disable check
		m_MinQuantityIngredient[0]			= 1;				//-1 = disable check
		m_MaxQuantityIngredient[0]			= -1;				//-1 = disable check
		
		m_MinDamageIngredient[1]			= -1;
		m_MaxDamageIngredient[1]			= 3;
		m_MinQuantityIngredient[1]			= 2;
		m_MaxQuantityIngredient[1]			= -1;

		//----------------------------------------------------------------------------------------------------------------------
		//INGREDIENTS
		//ingredient 1
		InsertIngredientEx(0,"Rope","ShelterKit_craft");		// you can insert multiple ingredients this way
		
		m_IngredientAddHealth[0]			= 0;				// 0 = do nothing
		m_IngredientSetHealth[0]			= -1;				// -1 = do nothing
		m_IngredientAddQuantity[0]			= -1;				// 0 = do nothing
		m_IngredientDestroy[0]				= true;				// true = destroy, false = do nothing

		//ingredient 2
		InsertIngredient(1,"WoodenStick");
		InsertIngredient(1,"Ammo_SharpStick");
		
		m_IngredientAddHealth[1]			= 0;
		m_IngredientSetHealth[1]			= -1;
		m_IngredientAddQuantity[1]			= -2;
		m_IngredientDestroy[1]				= false;

		//----------------------------------------------------------------------------------------------------------------------
		//RESULTS
		//result1
		AddResult("FenceKit");									// add results here

		m_ResultSetFullQuantity[0]			= false;			// true = set full quantity, false = do nothing
		m_ResultSetQuantity[0]				= -1;				// -1 = do nothing
		m_ResultSetHealth[0]				= -1;				// -1 = do nothing
		m_ResultInheritsHealth[0]			= -2;				// (value) == -1 means do nothing; a (value) >= 0 means this result will inherit health from ingredient number (value);(value) == -2 means this result will inherit health from all ingredients averaged(result_health = combined_health_of_ingredients / number_of_ingredients)
		m_ResultInheritsColor[0]			= -1;				// (value) == -1 means do nothing; a (value) >= 0 means this result classname will be a composite of the name provided in AddResult method and config value "color" of ingredient (value)
		m_ResultToInventory[0]				= -2;				// (value) == -2 spawn result on the ground;(value) == -1 place anywhere in the players inventory, (value) >= 0 means switch position with ingredient number(value)
		m_ResultReplacesIngredient[0]		= -1;				// (value) == -1 means do nothing; a value >= 0 means this result will transfer item propertiesvariables, attachments etc.. from an ingredient value
	}

	//final check for recipe's validity
	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[0].GetInventory().IsAttachment() )
			return false;
		if ( ingredients[1].GetInventory().IsAttachment() )
			return false;
		return true;
	}
};
