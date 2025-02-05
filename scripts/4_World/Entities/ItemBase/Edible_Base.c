class Edible_Base : ItemBase
{
	const string DIRECT_COOKING_SLOT_NAME 	= "DirectCooking";

	const string SOUND_BAKING_START 		= "Baking_SoundSet";
	const string SOUND_BAKING_DONE 			= "Baking_Done_SoundSet";
	const string SOUND_BURNING_DONE 		= "Food_Burning_SoundSet";

	protected bool m_MakeCookingSounds;
	protected SoundOnVehicle m_SoundCooking; //! DEPRECATED
	protected EffectSound m_SoundEffectCooking;
	protected string m_SoundPlaying;
	ref FoodStage m_FoodStage;
	protected float m_DecayTimer;
	protected float m_DecayDelta = 0.0;
	protected FoodStageType m_LastDecayStage = FoodStageType.NONE;
	protected ParticleSource 	m_HotVaporParticle;
	
	private CookingMethodType m_CookedByMethod;
		
	void Edible_Base()
	{
		if (HasFoodStage())
		{
			m_FoodStage = new FoodStage(this);
			
			RegisterNetSyncVariableInt("m_FoodStage.m_FoodStageType",  FoodStageType.NONE, FoodStageType.COUNT);
			RegisterNetSyncVariableFloat("m_FoodStage.m_CookingTime",  0, 600, 0);

			m_SoundPlaying = "";
			m_CookedByMethod = CookingMethodType.NONE;
			RegisterNetSyncVariableInt("m_CookedByMethod", CookingMethodType.NONE, CookingMethodType.COUNT);
			RegisterNetSyncVariableBool("m_MakeCookingSounds");
		}
	}
	
	override void EEInit()
	{
		super.EEInit();
		
		UpdateVisualsEx(true); //forced init visuals, mostly for debugs and SP
	}
	
	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		RemoveAudio();
		
		if (m_HotVaporParticle)
			m_HotVaporParticle.Stop();
	}
	
	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);

		//! disable sounds (from cooking)		
		if (oldLoc.GetType() == InventoryLocationType.ATTACHMENT || oldLoc.GetType() == InventoryLocationType.CARGO)
		{
			switch (oldLoc.GetParent().GetType())
			{
				case "FryingPan":
				case "Pot":
				case "Cauldron":
				case "SharpWoodenStick":
					MakeSoundsOnClient(false);
				break;
			}
			
			//! check for DirectCooking slot name
 			if (oldLoc.GetSlot() > -1 && InventorySlots.GetSlotName(oldLoc.GetSlot()).Contains(DIRECT_COOKING_SLOT_NAME))
			{
				MakeSoundsOnClient(false);
			}
		}
		
		if (oldLoc.IsValid())
			ResetCookingTime();
		
		if (CanHaveTemperature())
			UpdateVaporParticle();
	}

	void UpdateVisualsEx(bool forced = false)
	{
		if (GetFoodStage())
			GetFoodStage().UpdateVisualsEx(forced);
	}
	
	bool Consume(float amount, PlayerBase consumer)
	{
		AddQuantity(-amount, false, false);
		OnConsume(amount, consumer);

		return true;
	}
	
	void OnConsume(float amount, PlayerBase consumer)
	{
		if (GetTemperature() > PlayerConstants.CONSUMPTION_DAMAGE_TEMP_THRESHOLD)
		{
			consumer.ProcessDirectDamage(DamageType.CUSTOM, this, "", "EnviroDmg", "0 0 0", PlayerConstants.CONSUMPTION_DAMAGE_PER_BITE);
		}
	}
	
	//! Filter agents from the item (override on higher implements)
	int FilterAgents(int agentsIn)
	{
		return agentsIn;
	}
	
	//food staging
	override bool CanBeCooked()
	{
		return false;
	}
	
	override bool CanBeCookedOnStick()
	{
		return false;
	}
	
	override float GetTemperatureFreezeTime()
	{
		if (GetFoodStage())
		{
			switch (m_FoodStage.GetFoodStageType())
			{
				case FoodStageType.DRIED:
					return super.GetTemperatureFreezeTime() * GameConstants.TEMPERATURE_FREEZE_TIME_COEF_DRIED;
				
				case FoodStageType.BURNED:
					return super.GetTemperatureFreezeTime() * GameConstants.TEMPERATURE_FREEZE_TIME_COEF_BURNED;
				
				default:
					return super.GetTemperatureFreezeTime();
			}
		}
		
		return super.GetTemperatureFreezeTime();
	}
	
	override float GetTemperatureThawTime()
	{
		if (GetFoodStage())
		{
			switch (m_FoodStage.GetFoodStageType())
			{
				case FoodStageType.DRIED:
					return super.GetTemperatureThawTime() * GameConstants.TEMPERATURE_THAW_TIME_COEF_DRIED;
				
				case FoodStageType.BURNED:
					return super.GetTemperatureThawTime() * GameConstants.TEMPERATURE_THAW_TIME_COEF_BURNED;
				
				default:
					return super.GetTemperatureThawTime();
			}
		}
		
		return super.GetTemperatureThawTime();
	}
	
	override bool CanItemOverheat()
	{
		return super.CanItemOverheat() && (!GetFoodStage() || (IsFoodBurned() || !GetFoodStage().CanTransitionToFoodStageType(FoodStageType.BURNED))); //for foodstaged items - either is burned, or it can't get burned
	}
	
	//================================================================
	// SYNCHRONIZATION
	//================================================================	
	void Synchronize()
	{
		SetSynchDirty();
	}
	
	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		
		//UpdateVisualsEx(); //performed on client only
		
		//update audio
		if (m_MakeCookingSounds)
		{
			RefreshAudio();
		}
		else
		{
			RemoveAudio();
		}
		
		if (CanHaveTemperature())
			UpdateVaporParticle();
	}

	//================================================================
	// AUDIO EFFECTS (WHEN ON DCS)
	//================================================================	
	void MakeSoundsOnClient(bool soundstate, CookingMethodType cookingMethod = CookingMethodType.NONE)
	{
		m_MakeCookingSounds = soundstate;
		m_CookedByMethod 	= cookingMethod;

		Synchronize();
	}

	protected void RefreshAudio()
	{
		string soundName = "";
		
		FoodStageType nextFoodState = GetNextFoodStageType(m_CookedByMethod);

		switch (GetFoodStageType())
		{
			case FoodStageType.RAW:
				soundName = SOUND_BAKING_START;
				if (nextFoodState == FoodStageType.BOILED)
					soundName = "";
				break;
			case FoodStageType.BAKED:
				soundName = SOUND_BAKING_DONE;
				break;
			case FoodStageType.BURNED:
				soundName = SOUND_BURNING_DONE;
				break;
			default:
				soundName = "";
				break;
		}

		SoundCookingStart(soundName);
	}

	protected void RemoveAudio()
	{
		m_MakeCookingSounds = false;
		SoundCookingStop();
	}

	//================================================================
	// SERIALIZATION
	//================================================================	
	override void OnStoreSave(ParamsWriteContext ctx)
	{   
		super.OnStoreSave(ctx);

		if (GetFoodStage())
		{
			GetFoodStage().OnStoreSave(ctx);
		}
		
		// food decay
		ctx.Write(m_DecayTimer);
		ctx.Write(m_LastDecayStage);
	}
	
	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		if (GetFoodStage())
		{
			if (!GetFoodStage().OnStoreLoad(ctx, version))
				return false;
		}
		
		if (version >= 115)
		{
			if (!ctx.Read(m_DecayTimer))
			{
				m_DecayTimer = 0.0;
				return false;
			}
			if (!ctx.Read(m_LastDecayStage))
			{
				m_LastDecayStage = FoodStageType.NONE;
				return false;
			}
		}
		
		UpdateVisualsEx(true); //forced init visuals
		Synchronize();
		
		return true;
	}
	
	override void AfterStoreLoad()
	{	
		super.AfterStoreLoad();
		
		Synchronize();
	}	

	//get food stage
	override FoodStage GetFoodStage()
	{
		return m_FoodStage;
	}
	
	//food types
	override bool IsMeat()
	{
		return false;
	}
	
	override bool IsCorpse()
	{
		return false;
	}
	
	override bool IsFruit()
	{
		return false;
	}
	
	override bool IsMushroom()
	{
		return false;
	}	
	
	//================================================================
	// NUTRITIONAL VALUES
	//================================================================	
	//food properties
	static float GetFoodTotalVolume(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			 return FoodStage.GetFullnessIndex(food_item.GetFoodStage());
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetFullnessIndex(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetFloat( class_path + " fullnessIndex" );

	}
	
	static float GetFoodEnergy(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			 return FoodStage.GetEnergy(food_item.GetFoodStage());
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetEnergy(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetFloat( class_path + " energy" );			
	}
	
	static float GetFoodWater(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			return FoodStage.GetWater(food_item.GetFoodStage());
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetWater(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetFloat( class_path + " water" );			
	}
	
	static float GetFoodNutritionalIndex(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			return FoodStage.GetNutritionalIndex(food_item.GetFoodStage());	
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetNutritionalIndex(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetFloat( class_path + " nutritionalIndex" );		
		
	}
	
	static float GetFoodToxicity(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			return FoodStage.GetToxicity(food_item.GetFoodStage());
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetToxicity(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetFloat( class_path + " toxicity" );			
	}
	
	static int GetFoodAgents(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			return FoodStage.GetAgents(food_item.GetFoodStage());
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetAgents(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetInt( class_path + " agents" );
	}
	
	static float GetFoodDigestibility(ItemBase item, string classname = "", int food_stage = 0)
	{
		Edible_Base food_item = Edible_Base.Cast(item);
		if (food_item && food_item.GetFoodStage())
		{
			return FoodStage.GetDigestibility(food_item.GetFoodStage());
		}
		else if (classname != "" && food_stage)
		{
			return FoodStage.GetDigestibility(null, food_stage, classname);
		}
		string class_path = string.Format("cfgVehicles %1 Nutrition", classname);
		return GetGame().ConfigGetInt( class_path + " digestibility" );
	}
	
	static NutritionalProfile GetNutritionalProfile(ItemBase item, string classname = "", int food_stage = 0)
	{
		return new NutritionalProfile(GetFoodEnergy(item, classname, food_stage),GetFoodWater(item, classname, food_stage),GetFoodNutritionalIndex(item, classname, food_stage),GetFoodTotalVolume(item, classname, food_stage), GetFoodToxicity(item, classname, food_stage),  GetFoodAgents(item, classname,food_stage), GetFoodDigestibility(item, classname,food_stage));
	}
	
	//================================================================
	// FOOD STAGING
	//================================================================
	FoodStageType GetFoodStageType()
	{
		return GetFoodStage().GetFoodStageType();
	}
	
	//food stage states
	bool IsFoodRaw()
	{
		if ( GetFoodStage() ) 
		{
			return GetFoodStage().IsFoodRaw();
		}
		
		return false;
	}

	bool IsFoodBaked()
	{
		if ( GetFoodStage() ) 
		{
			return GetFoodStage().IsFoodBaked();
		}
		
		return false;
	}
	
	bool IsFoodBoiled()
	{
		if ( GetFoodStage() ) 
		{
			return GetFoodStage().IsFoodBoiled();
		}
		
		return false;
	}
	
	bool IsFoodDried()
	{
		if ( GetFoodStage() ) 
		{
			return GetFoodStage().IsFoodDried();
		}
		
		return false;
	}
	
	bool IsFoodBurned()
	{
		if ( GetFoodStage() ) 
		{
			return GetFoodStage().IsFoodBurned();
		}
		
		return false;
	}
	
	bool IsFoodRotten()
	{
		if ( GetFoodStage() ) 
		{
			return GetFoodStage().IsFoodRotten();
		}
		
		return false;
	}				
	
	//food stage change
	void ChangeFoodStage( FoodStageType new_food_stage_type )
	{
		GetFoodStage().ChangeFoodStage( new_food_stage_type );
	}
	
	FoodStageType GetNextFoodStageType( CookingMethodType cooking_method )
	{
		return GetFoodStage().GetNextFoodStageType( cooking_method );
	}
	
	string GetFoodStageName( FoodStageType food_stage_type )
	{
		return GetFoodStage().GetFoodStageName( food_stage_type );
	}
	
	bool CanChangeToNewStage( CookingMethodType cooking_method )
	{
		return GetFoodStage().CanChangeToNewStage( cooking_method );
	}
	
	//Use this to receive food stage from another Edible_Base
	void TransferFoodStage( notnull Edible_Base source )
	{
		if ( !source.GetFoodStage())
			return;
		m_LastDecayStage = source.GetLastDecayStage();
		ChangeFoodStage(source.GetFoodStage().GetFoodStageType());
		m_DecayTimer = source.GetDecayTimer();
		m_DecayDelta = source.GetDecayDelta();
	}
	
	//! called on server 
	void OnFoodStageChange(FoodStageType stageOld, FoodStageType stageNew)
	{
		HandleFoodStageChangeAgents(stageOld,stageNew);
		UpdateVisualsEx(); //performed on server only
	}
	
	//! removes select agents on foodstage transitions
	void HandleFoodStageChangeAgents(FoodStageType stageOld, FoodStageType stageNew)
	{
		switch (stageNew)
		{
			case FoodStageType.BAKED:
			case FoodStageType.BOILED:
			case FoodStageType.DRIED:
				RemoveAllAgentsExcept(eAgents.BRAIN|eAgents.HEAVYMETAL);
			break;
			
			case FoodStageType.BURNED:
				RemoveAllAgentsExcept(eAgents.HEAVYMETAL);
			break;
		}
	}
	
	//================================================================
	// COOKING
	//================================================================
	//cooking time
	float GetCookingTime()
	{
		return GetFoodStage().GetCookingTime();
	}
	
	void SetCookingTime( float time )
	{
		GetFoodStage().SetCookingTime( time );
		
		//synchronize when calling on server
		Synchronize();
	}
	
	void ResetCookingTime()
	{
		if (GetFoodStage())
		{
			GetFoodStage().SetCookingTime(0);
			Synchronize();
		}
	}
	
	//replace edible with new item (opening cans)
	void ReplaceEdibleWithNew( string typeName )
	{
		PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
		if (player)
		{
			ReplaceEdibleWithNewLambda lambda = new ReplaceEdibleWithNewLambda(this, typeName, player);
			player.ServerReplaceItemInHandsWithNew(lambda);
		}
		else
			Error("ReplaceEdibleWithNew - cannot use edible without player");
	}

	override void SetActions()
	{
		super.SetActions();

		AddAction(ActionAttach);
		AddAction(ActionDetach);
	}

	protected void SoundCookingStart(string sound_name)
	{
		#ifndef SERVER
		if (m_SoundPlaying != sound_name)
		{
			SoundCookingStop();

			m_SoundEffectCooking = SEffectManager.PlaySound(sound_name, GetPosition(), 0, 0, true);
			m_SoundPlaying = sound_name;
		}
		#endif
	}

	protected void SoundCookingStop()
	{
		#ifndef SERVER
		if (m_SoundEffectCooking)
		{
			m_SoundEffectCooking.Stop();
			m_SoundEffectCooking = null;
			m_SoundPlaying = "";
		}
		#endif
	}
	
	override bool CanDecay()
	{
		return false;
	}
	
	override bool CanProcessDecay()
	{
		return !GetIsFrozen() && ( GetFoodStageType() != FoodStageType.ROTTEN );
	}
	
	override void ProcessDecay( float delta, bool hasRootAsPlayer )
	{
		m_DecayDelta = 0.0;
		
		delta *= DayZGame.Cast(GetGame()).GetFoodDecayModifier();
		m_DecayDelta += ( 1 + ( 1 - GetHealth01( "", "" ) ) );
		if ( hasRootAsPlayer )
			m_DecayDelta += GameConstants.DECAY_RATE_ON_PLAYER;
		
		/*Print( "-------------------------" );
		Print( this );
		Print( m_DecayTimer );
		Print( m_DecayDelta );
		Print( m_LastDecayStage );*/
		
		if ( IsFruit() || IsMushroom() )
		{
			// fruit, vegetables and mushrooms
			if ( m_LastDecayStage != GetFoodStageType() )
			{
				switch ( GetFoodStageType() )
				{
					case FoodStageType.RAW:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_RAW_FRVG + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_RAW_FRVG * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.RAW;
						break;
					
					case FoodStageType.BOILED:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_BOILED_FRVG + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_BOILED_FRVG * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.BOILED;
						break;
					
					case FoodStageType.BAKED:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_BAKED_FRVG + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_BAKED_FRVG * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.BAKED;
						break;
					
					case FoodStageType.DRIED:
					case FoodStageType.BURNED:
					case FoodStageType.ROTTEN:
					default:
						m_DecayTimer = -1;
						m_LastDecayStage = FoodStageType.NONE;
						return;
				}
				
				//m_DecayTimer = m_DecayTimer / 1000.0;
			}
			
			m_DecayTimer -= ( delta * m_DecayDelta );
						
			if ( m_DecayTimer <= 0 ) 
			{
				if ( m_LastDecayStage != FoodStageType.NONE )
				{
					// switch to decayed stage
					if ( ( m_LastDecayStage == FoodStageType.BOILED ) || ( m_LastDecayStage == FoodStageType.BAKED ) )
					{
						ChangeFoodStage( FoodStageType.ROTTEN );
					} 
					if ( m_LastDecayStage == FoodStageType.RAW )
					{
						int rng = Math.RandomIntInclusive( 0, 100 );
						if ( rng > GameConstants.DECAY_FOOD_FRVG_DRIED_CHANCE )
						{
							ChangeFoodStage( FoodStageType.ROTTEN );
						}
						else
						{
							if ( CanChangeToNewStage( FoodStageType.DRIED ) )
							{
								ChangeFoodStage( FoodStageType.DRIED );
							}
							else
							{
								ChangeFoodStage( FoodStageType.ROTTEN );
							}
						}
					}
				}
			}

		}
		else if ( IsMeat() )
		{
			// meat
			if ( m_LastDecayStage != GetFoodStageType() )
			{
				switch ( GetFoodStageType() )
				{
					case FoodStageType.RAW:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_RAW_MEAT + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_RAW_MEAT * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.RAW;
						break;
					
					case FoodStageType.BOILED:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_BOILED_MEAT + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_BOILED_MEAT * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.BOILED;
						break;
					
					case FoodStageType.BAKED:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_BAKED_MEAT + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_BAKED_MEAT * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.BAKED;
						break;
					
					case FoodStageType.DRIED:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_DRIED_MEAT + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_DRIED_MEAT * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.DRIED;
						break;

					case FoodStageType.BURNED:
					case FoodStageType.ROTTEN:
					default:
						m_DecayTimer = -1;
						m_LastDecayStage = FoodStageType.NONE;
						return;
				}
			}
			
			m_DecayTimer -= ( delta * m_DecayDelta );
			
			if ( m_DecayTimer <= 0 ) 
			{
				if ( m_LastDecayStage != FoodStageType.NONE )
				{
					// switch to decayed stage
					if ( ( m_LastDecayStage == FoodStageType.DRIED ) || ( m_LastDecayStage == FoodStageType.RAW ) || ( m_LastDecayStage == FoodStageType.BOILED ) || ( m_LastDecayStage == FoodStageType.BAKED ) )
					{
						ChangeFoodStage( FoodStageType.ROTTEN );
					}
				}
			}
		}
		else if ( IsCorpse() )
		{
			// corpse
			if ( m_LastDecayStage != GetFoodStageType() )
			{
				switch ( GetFoodStageType() )
				{
					case FoodStageType.RAW:
						m_DecayTimer = ( GameConstants.DECAY_FOOD_RAW_CORPSE + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_RAW_CORPSE * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
						m_LastDecayStage = FoodStageType.RAW;
						break;

					case FoodStageType.BURNED:
					case FoodStageType.ROTTEN:
					default:
						m_DecayTimer = -1;
						m_LastDecayStage = FoodStageType.NONE;
						return;
				}
			}
			
			m_DecayTimer -= ( delta * m_DecayDelta );
			
			if ( m_DecayTimer <= 0 ) 
			{
				if ( m_LastDecayStage != FoodStageType.NONE )
				{
					// switch to decayed stage
					if ( ( m_LastDecayStage == FoodStageType.DRIED ) || ( m_LastDecayStage == FoodStageType.RAW ) || ( m_LastDecayStage == FoodStageType.BOILED ) || ( m_LastDecayStage == FoodStageType.BAKED ) )
					{
						ChangeFoodStage( FoodStageType.ROTTEN );
					}
				}
			}
		}
		else
		{
			// opened cans
			m_DecayTimer -= ( delta * m_DecayDelta );

			if ( ( m_DecayTimer <= 0 ) && ( m_LastDecayStage == FoodStageType.NONE ) )
			{
				m_DecayTimer = ( GameConstants.DECAY_FOOD_CAN_OPEN + ( Math.RandomFloat01() * ( GameConstants.DECAY_FOOD_DRIED_MEAT * ( GameConstants.DECAY_TIMER_RANDOM_PERCENTAGE / 100.0 ) ) ) );
				m_LastDecayStage = FoodStageType.RAW;
				//m_DecayTimer = m_DecayTimer / 1000.0;
			}
			else
			{
				if ( m_DecayTimer <= 0 )
				{
					InsertAgent(eAgents.FOOD_POISON, 1);
					m_DecayTimer = -1;
				}
			}
		}
	}
	
	protected void UpdateVaporParticle()
	{
		if (GetGame().IsDedicatedServer())
			return;
		
		if (m_VarTemperature >= GameConstants.STATE_HOT_LVL_TWO && !m_HotVaporParticle)
		{
			InventoryLocation invLoc = new InventoryLocation();
			GetInventory().GetCurrentInventoryLocation(invLoc);
			if (invLoc && (invLoc.GetType() == InventoryLocationType.GROUND || invLoc.GetType() == InventoryLocationType.HANDS))
			{
				ParticleManager ptcMgr = ParticleManager.GetInstance();
				if (ptcMgr)
				{
					m_HotVaporParticle = ParticleManager.GetInstance().PlayOnObject(ParticleList.ITEM_HOT_VAPOR, this);
					m_HotVaporParticle.SetParticleParam(EmitorParam.SIZE, 0.3);
					m_HotVaporParticle.SetParticleParam(EmitorParam.BIRTH_RATE, 10);
					m_HotVaporParticle.SetParticleAutoDestroyFlags(ParticleAutoDestroyFlags.ON_STOP);
				}
			}
		}	
		else if (m_HotVaporParticle)
		{
			if (m_VarTemperature <= GameConstants.STATE_HOT_LVL_TWO)
			{
				m_HotVaporParticle.Stop();
				m_HotVaporParticle = null;
				return;
			}
			
			InventoryLocation inventoryLoc = new InventoryLocation();
			GetInventory().GetCurrentInventoryLocation(inventoryLoc);
			if (invLoc && (invLoc.GetType() != InventoryLocationType.GROUND && invLoc.GetType() != InventoryLocationType.HANDS))
			{
				m_HotVaporParticle.Stop();
				m_HotVaporParticle = null;
			}
		}
	}
	
	override void GetDebugActions(out TSelectableActionInfoArrayEx outputList)
	{
		super.GetDebugActions(outputList);
		
		if (GetFoodStage())
		{
			outputList.Insert(new TSelectableActionInfoWithColor(SAT_DEBUG_ACTION, EActions.FOOD_STAGE_PREV, "Food Stage Prev", FadeColors.WHITE));
			outputList.Insert(new TSelectableActionInfoWithColor(SAT_DEBUG_ACTION, EActions.FOOD_STAGE_NEXT, "Food Stage Next", FadeColors.WHITE));
		}
	}
	
	override bool OnAction(int action_id, Man player, ParamsReadContext ctx)
	{
		super.OnAction(action_id, player, ctx);
		
		if ( GetGame().IsServer() )
		{
			if ( action_id == EActions.FOOD_STAGE_PREV )
			{
				int food_stage_prev = GetFoodStageType() - 1;
				if (food_stage_prev <= 0)
				{
					food_stage_prev = FoodStageType.COUNT - 1;
				}
				ChangeFoodStage(food_stage_prev);
				return true;
			}
			else if ( action_id == EActions.FOOD_STAGE_NEXT )
			{
				int food_stage_next = GetFoodStageType() + 1;
				if (food_stage_next >= FoodStageType.COUNT )
				{
					food_stage_next = FoodStageType.RAW;
				}
				ChangeFoodStage(food_stage_next);
				return true;
			}
		}
		return false;
	}
	
	override string GetDebugText()
	{
		string debug_output;

		debug_output = super.GetDebugText();
		
		debug_output+="m_CookedByMethod:"+m_CookedByMethod+"\n";
		debug_output+="m_MakeCookingSounds:"+m_MakeCookingSounds+"\n";

		return debug_output;
	}

	//================================================================
	// GENERAL GETTERS
	//================================================================
	
	override float GetBaitEffectivity()
	{
		float ret = super.GetBaitEffectivity();
		
		if (IsFoodRotten())
		{
			ret *= 0.5;
		}
		
		return ret;
	}
	
	float GetDecayTimer()
	{
		return m_DecayTimer;
	}
	
	float GetDecayDelta()
	{
		return m_DecayDelta;
	}
	
	FoodStageType GetLastDecayStage()
	{
		return m_LastDecayStage;
	}
	
	//////////////////////////////////////////
	//DEPRECATED
	//////////////////////////////////////////
	void UpdateVisuals()
	{
		UpdateVisualsEx();
	}
}

class ReplaceEdibleWithNewLambda : TurnItemIntoItemLambda
{
	void ReplaceEdibleWithNewLambda(EntityAI old_item, string new_item_type, PlayerBase player) { }
};
