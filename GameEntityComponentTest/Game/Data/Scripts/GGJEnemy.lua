-- test script

GGJEnemy =
{

Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 10 },
};

OnLoad = function()
end,

OnPlay = function()
	math.randomseed( os.time() );
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
		r = math.random();
		pos.x = r * 6.28;
	end

	transform:SetPosition( pos );

end,

}