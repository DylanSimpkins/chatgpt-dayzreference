class PileOfWoodenPlanks extends ItemBase
{	
	void PileOfWoodenPlanks()
	{
		if ( GetGame().IsServer() )
		{
			SetAllowDamage(false);
		}
	}
	
	// Shows/Hides selections. Call this in init or after every quantity change.
	void UpdateSelections()
	{
		RemoveProxyPhysics( "stage_1" );
		RemoveProxyPhysics( "stage_2" );
		RemoveProxyPhysics( "stage_3" );
		
		// Show/Hide selections according to the current quantity
		if ( this )
		{
			float quantity = GetQuantity();
			float quantity_max = GetQuantityMax();
			
			if ( quantity > GetQuantityMax() *0.66 )
			{
				// Show 3/3 amount of planks
				ShowSelection ( "stage_3" );
				HideSelection ( "stage_2" );
				HideSelection ( "stage_1" );
				
				AddProxyPhysics( "stage_3" );
			}
			
			if ( quantity > quantity_max *0.33  &&  quantity <= quantity_max *0.66 )
			{
				// Show 2/3 amount of planks
				HideSelection ( "stage_3" );
				ShowSelection ( "stage_2" );
				HideSelection ( "stage_1" );
				
				AddProxyPhysics( "stage_2" );
			}
			
			if ( quantity > 0  &&  quantity <= quantity_max *0.33 )
			{
				// Show 1/3 amount of planks
				HideSelection ( "stage_3" );
				HideSelection ( "stage_2" );
				ShowSelection ( "stage_1" );
				
				AddProxyPhysics( "stage_1" );
			}
			
			if ( quantity == 0 )
			{
				// Show 0 planks. Object should be deleted now.
				HideSelection ( "stage_3" );
				HideSelection ( "stage_2" );
				HideSelection ( "stage_1" );
			}
		}
	}
	
	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);
		UpdateSelections();
	}
	
	// Removes a number of long planks. Note that one (4m) long plank has the length of 3x normal planks!
	// Returns the number of removed long planks
	int RemovePlanks( int needed_planks )
	{
		// Make sure to not give more long planks than we have available
		int available_planks = needed_planks;
		if ( available_planks > GetQuantity() )
		{
			available_planks = GetQuantity();
		}
		
		// Remove long planks from this object
		AddQuantity( -available_planks, true ); // Autodelete enabled
		
		// Return the number of removed long planks
		return available_planks;
	}
	
	override bool SetQuantity(float value, bool destroy_config = true, bool destroy_forced = false, bool allow_client = false, bool clamp_to_stack_max = true)
	{
		bool result = super.SetQuantity(value, destroy_config, destroy_forced, allow_client, clamp_to_stack_max);
		UpdateSelections();
		
		return result;
	}
	
	override bool CanPutIntoHands( EntityAI parent )
	{
		return false;
	}
	
	override bool CanPutInCargo (EntityAI parent)
	{
		return false;
	}
}