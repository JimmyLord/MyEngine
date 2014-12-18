
Enemy = {}

Enemy.Externs =
{
	{ "Speed", "Int" },
}

Enemy.OnPlay = function ()
	--LogInfo( "OnPlay\n" );
end

Enemy.OnStop = function ()
	--LogInfo( "OnStop\n" );
end

Enemy.Tick = function (timepassed)
	--LogInfo( "Tick Start\n" );

	transform = this:GetTransform();
	pos = transform:GetPosition();

	--LogInfo( "pos: " .. pos.y .. "\n" );
	--speed = 100;
	speed = Enemy.Speed;
	pos.y = pos.y + timepassed*speed;
	
	transform:SetPosition( pos );
	
	--LogInfo( "Tick End\n" );
end
