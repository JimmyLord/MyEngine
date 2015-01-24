-- test script

GGJPlayer =
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
	--key down
	if( action == 0 ) then
		LogInfo( "key down " .. id .. "\n" );
	end
	--key up
	if( action == 1 ) then
		LogInfo( "key up " .. id .. "\n" );
	end
end,

Tick = function(timepassed)
end,

}