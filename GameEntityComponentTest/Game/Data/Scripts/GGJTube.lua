-- test script

GGJTube =
{

Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 1 },
};

OnLoad = function()
end,

OnPlay = function()
end,

OnStop = function()
end,

OnTouch = function(action, id, x, y, pressure, size)
	LogInfo( "OnTouch " .. id .. "\n" );
end,

OnButtons = function(action, id)
end,

Tick = function(timepassed)
	
	local transform = this.gameobject:GetTransform();
	local pos = transform:GetPosition();

	pos.z = pos.z + timepassed * this.Speed;

	if( pos.z >= 20 ) then
		pos.z = pos.z - 160;
	end

	transform:SetPosition( pos );

end,

}