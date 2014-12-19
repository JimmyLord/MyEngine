
Enemy = {}

Enemy.Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 1 },
	{ "ObjectChasing", "GameObject", "Player Object" },
}

Enemy.OnPlay = function()
	--LogInfo( "OnPlay\n" );
end

Enemy.OnStop = function()
	--LogInfo( "OnStop\n" );
end

Enemy.OnTouch = function(action, id, x, y, pressure, size)
	LogInfo( "OnTouch\n" );
end

Enemy.OnButtons = function(action, id)
	LogInfo( "OnButtons\n" );
end

Enemy.Tick = function(timepassed)
	--LogInfo( "Tick Start\n" );

	local transform = this:GetTransform();

	local pos = transform:GetPosition();
	local posChasing = Enemy.ObjectChasing:GetTransform():GetPosition();

	local diff = posChasing:Sub( pos );
	diff = diff:Scale( timepassed * Enemy.Speed );

	pos = pos:Add( diff );

	transform:SetPosition( pos );
	
	--obj = ComponentSystemManager:CreateGameObject( true );
	--ComponentSystemManager:DeleteGameObject( obj );
	--ComponentSystemManager:CopyGameObject( this );
	--obj = ComponentSystemManager:FindGameObjectByName( "Player Object" );

	--LogInfo( "Tick End\n" );
end
