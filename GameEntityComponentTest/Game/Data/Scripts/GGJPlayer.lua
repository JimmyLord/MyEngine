-- test script

GGJPlayer =
{

--local heldw = false;
--local helda = false;
--local helds = false;
--local heldd = false;

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
		if( id == 87 or id == 119 ) then -- w
			heldw = true;
		end
		if( id == 65 or id == 97 ) then -- a
			helda = true;
		end
		if( id == 83 or id == 115 ) then -- s
			helds = true;
		end
		if( id == 68 or id == 100 ) then -- d
			heldd = true;
		end
	end
	--key up
	if( action == 1 ) then
		if( id == 87 or id == 119 ) then -- w
			heldw = false;
		end
		if( id == 65 or id == 97 ) then -- a
			helda = false;
		end
		if( id == 83 or id == 115 ) then -- s
			helds = false;
		end
		if( id == 68 or id == 100 ) then -- d
			heldd = false;
		end
	end
end,

Tick = function(timepassed)
	local force = Vector3( 0,0,0 );

	if( heldw == true ) then
		force:Set( 0,0,-1 );
	end
	if( helda == true ) then
		force:Set( -1,0,0 );
	end
	if( helds == true ) then
		force:Set( 0,0,1 );
	end
	if( heldd == true ) then
		force:Set( 1,0,0 );
	end

	if( force:LengthSquared() ~= 0 ) then
		local physicsobject = this.gameobject:GetCollisionObject();
		physicsobject:ApplyForce( force:Scale(10), Vector3(0,0,0) );
	end
end,

}