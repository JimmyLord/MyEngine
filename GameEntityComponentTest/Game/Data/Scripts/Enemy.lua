
Enemy = {}

Enemy.Externs =
{
	-- name, type, default value
	{ "Speed", "Float", 1 },
	{ "ObjectChasing", "GameObject", "Player Object" },
}

Enemy.OnPlay = function ()
	--LogInfo( "OnPlay\n" );
end

Enemy.OnStop = function ()
	--LogInfo( "OnStop\n" );
end

Enemy.Tick = function (timepassed)
	--LogInfo( "Tick Start\n" );

	local transform = this:GetTransform();
	local pos = transform:GetPosition();

	local posChasing = Enemy.ObjectChasing:GetTransform():GetPosition();

	local speed = Enemy.Speed;

	local diffx = posChasing.x - pos.x;
	local diffy = posChasing.y - pos.y;

	pos.x = pos.x + diffx * timepassed*speed;
	pos.y = pos.y + diffy * timepassed*speed;
	
	transform:SetPosition( pos );
	
	--LogInfo( "Tick End\n" );
end
