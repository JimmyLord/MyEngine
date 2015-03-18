-- test script

SimpleCharacterController =
{

Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 10 },
	{ "Animation", "Float", 1 },
};

OnLoad = function()
end,

OnPlay = function()
	-- math.randomseed( os.time() );
	this.dirx = 0;
	this.dirz = 0;
end,

OnStop = function()
end,

OnTouch = function(action, id, x, y, pressure, size)
	-- LogInfo( "OnTouch " .. id .. "\n" );
	if( x < 200 ) then
		this.Animation = 1;
	else
		this.Animation = 2;
	end
end,

OnButtons = function(action, id)
	-- LogInfo( "OnButtons " .. id .. "\n" );

	if( action == 2 ) then -- button held
		if( id == 1 ) then -- left
			this.dirx = -1;
		end
		if( id == 2 ) then -- right
			this.dirx = 1;
		end
		if( id == 3 ) then -- up
			this.dirz = -1;
		end
		if( id == 4 ) then -- down
			this.dirz = 1;
		end
	end
end,

Tick = function(timepassed)
	
	local transform = this.gameobject:GetTransform();
	local pos = transform:GetPosition();
	local rot = transform:GetLocalRotation();

	local animplayer = this.gameobject:GetAnimationPlayer();

	if( animplayer ) then
		animplayer:SetCurrentAnimation( this.Animation );
	end

	pos.x = pos.x + this.dirx * timepassed * this.Speed;
	pos.z = pos.z + this.dirz * timepassed * this.Speed;

	if( this.dirx ~= 0 or this.dirz ~= 0 ) then
		rot.y = math.atan2( this.dirz, this.dirx ) / math.pi * 180 - 90;
		this.Animation = 2;
	else
		this.Animation = 1;
	end

	transform:SetPosition( pos );
	transform:SetRotation( rot );

	this.dirx = 0;
	this.dirz = 0;
end,

}