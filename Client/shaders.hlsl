struct VSInput
{
	float3 posModel : POSITION;
	float4 color : COLOR;
};

struct PSInput
{
	float4 posProj : SV_POSITION;
	float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput output;
	output.posProj = float4(input.posModel, 1.0);
	output.color = input.color;
	return output;
}

float4 PSMain(PSInput input) : SV_Target0
{
	return input.color;
}



