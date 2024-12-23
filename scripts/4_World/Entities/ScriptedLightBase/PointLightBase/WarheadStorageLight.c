class WarheadStorageLight extends PointLightBase
{
	void WarheadStorageLight()
	{
		SetVisibleDuringDaylight(true);
		SetRadiusTo(7.5);
		SetBrightnessTo(6.5);
		FadeIn(1);
		SetFadeOutTime(2);
		SetFlareVisible(false);
		SetCastShadow(true);
		SetAmbientColor(1, 0.7, 0.3);
		SetDiffuseColor(1, 0.7, 0.3);
	}
}
