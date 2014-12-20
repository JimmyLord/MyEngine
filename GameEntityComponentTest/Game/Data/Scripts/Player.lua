-- test Player script, moves to where you click.

Player =
{

OnLoad = function()
    -- TODO: adds ref each time script is loaded, and nothing removes them.
	--g_pFileManager:RequestFile( "Data/OBJs/Teapot.obj" );
end,

OnPlay = function ()
	--LogInfo( "OnPlay\n" );
end,

OnStop = function ()
	--LogInfo( "OnStop\n" );
end,

OnTouch = function(action, id, x, y, pressure, size)
	--LogInfo( "OnTouch\n" );
	transform = this:GetTransform();
	pos = transform:GetPosition();

	pos.x = x;
	pos.y = y;

	transform:SetPosition( pos );
end,

OnButtons = function(action, id)
	--LogInfo( "OnButtons\n" );
end,

Tick = function (timepassed)
	--LogInfo( "Tick Start\n" );

	--transform = this:GetTransform();
	--pos = transform:GetPosition();

	--speed = 100;
	--pos.x = pos.x + timepassed*speed;
	
	--transform:SetPosition( pos );
	
	--LogInfo( "Tick End\n" );
end,

}