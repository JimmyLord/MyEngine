-- test script

SimpleCharacterController =
{

Externs =
{
	-- name, type, initial value
	{ "Speed", "Float", 1 },
	{ "Animation", "Float", 1 },
};

OnLoad = function()
end,

OnPlay = function()
	-- math.randomseed( os.time() );
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

	if( action == 0 ) then -- button pressed
		if( id == 1 ) then -- left
			LogInfo( "OnButtons - Left pressed " .. id .. "\n" );
		end
		if( id == 2 ) then -- right
			LogInfo( "OnButtons - right pressed " .. id .. "\n" );
		end
		if( id == 3 ) then -- up
			LogInfo( "OnButtons - up pressed " .. id .. "\n" );
		end
		if( id == 4 ) then -- down
			LogInfo( "OnButtons - down pressed " .. id .. "\n" );
		end
	end
end,

Tick = function(timepassed)
	
	local transform = this.gameobject:GetTransform();
	-- local pos = transform:GetPosition();

	local animplayer = this.gameobject:GetAnimationPlayer();

	if( animplayer ) then
		animplayer:SetCurrentAnimation( this.Animation );
	end

	-- pos.z = pos.z + timepassed * this.Speed;
	-- 
	-- if( pos.z >= 20 ) then
	-- 	pos.z = pos.z - 160;
	-- 	r = math.random();
	-- 	pos.x = r * 6.28;
	-- end
	-- 
	-- transform:SetPosition( pos );

end,

}