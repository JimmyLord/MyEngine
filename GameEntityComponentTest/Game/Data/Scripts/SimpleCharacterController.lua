-- test script

SimpleCharacterController =
{

Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 10 },
	{ "SpeedRotation", "Float", 10 },
	{ "Animation", "Float", 1 },
};

OnLoad = function()
end,

OnPlay = function()
	-- math.randomseed( os.time() );

	-- initialize some local variables
	this.dirx = 0; -- todo - figure out how to declare a Vector2
	this.dirz = 0;
	this.targetangle = 0;
end,

OnStop = function()
end,

OnTouch = function(action, id, x, y, pressure, size)
	-- LogInfo( "OnTouch " .. id .. "\n" );
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
	
	-- todo - do these 2 lookups once in OnPlay?
	local animplayer = this.gameobject:GetAnimationPlayer();
	local transform = this.gameobject:GetTransform();

	local pos = transform:GetPosition();
	local rot = transform:GetLocalRotation();

	-- play the correct animation and figure out facing direction based on input.
	if( this.dirx ~= 0 or this.dirz ~= 0 ) then
		this.targetangle = math.atan2( this.dirz, this.dirx ) / math.pi * 180 - 90;
		this.Animation = 2;
	else
		this.Animation = 1;
	end

	-- move the player
	-- todo, dirx/dirz should be a vec2 and needs to be normalized
	pos.x = pos.x + this.dirx * timepassed * this.Speed;
	pos.z = pos.z + this.dirz * timepassed * this.Speed;

	-- rotate towards the target angle
	local anglediff = this.targetangle - rot.y;
	if( anglediff > 180 ) then anglediff = anglediff - 360;	end
	if( anglediff < -180 ) then anglediff = anglediff + 360; end
	--LogInfo( "rot.y " .. rot.y .. " " .. "this.targetangle " .. this.targetangle .. " " .. "anglediff " .. anglediff .. "\n" );
	rot.y = rot.y + anglediff * timepassed * this.SpeedRotation;
	rot.y = rot.y % 360;

	-- set the new position and rotation values
	transform:SetPosition( pos );
	transform:SetRotation( rot );

	-- set the current animation
	if( animplayer ) then
		animplayer:SetCurrentAnimation( this.Animation );
	end

	-- zero out the input vector
	this.dirx = 0;
	this.dirz = 0;
end,

}