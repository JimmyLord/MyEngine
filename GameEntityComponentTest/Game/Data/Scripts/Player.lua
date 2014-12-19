
Player = {}

Player.OnPlay = function ()
	--LogInfo( "OnPlay\n" );
end

Player.OnStop = function ()
	--LogInfo( "OnStop\n" );
end

Player.OnTouch = function(action, id, x, y, pressure, size)
	--LogInfo( "OnTouch\n" );
	transform = this:GetTransform();
	pos = transform:GetPosition();

	pos.x = x;
	pos.y = y;

	transform:SetPosition( pos );
end

Player.OnButtons = function(action, id)
	LogInfo( "OnButtons\n" );
end

Player.Tick = function (timepassed)
	--LogInfo( "Tick Start\n" );

	transform = this:GetTransform();
	pos = transform:GetPosition();

	--LogInfo( "pos: " .. pos.x .. "\n" );
	speed = 100;
	pos.x = pos.x + timepassed*speed;
	
	transform:SetPosition( pos );
	
	--LogInfo( "Tick End\n" );
end
