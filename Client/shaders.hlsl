cbuffer ConstBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
};

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
	
	float4 pos = float4(input.posModel, 1.0);

	pos = mul(pos, world);
	pos = mul(pos, view);
	pos = mul(pos, proj);

	output.posProj = pos;
	output.color = input.color;

	return output;
}

float4 PSMain(PSInput input) : SV_Target0
{
	return input.color;
}



